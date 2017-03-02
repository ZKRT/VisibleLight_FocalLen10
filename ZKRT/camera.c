#include "camera.h"
#include "zkrt.h"
#include "can.h"
#include "pwm.h"

volatile uint32_t paizhao_count = 0XFFFFFFFF;
volatile uint32_t return_count = 0XFFFFFFFF;

uint16_t last_paizhao = 0;
uint16_t last_shexiang = 0;
uint8_t last_jiaoju_re = 0;
uint8_t last_jiaoju_in = 0;
uint8_t last_three_paizhao = 0;

uint16_t pwm_hangxiang = 0;		//通道1，航向
uint16_t pwm_fuyang = 0;			//通道2，俯仰
uint16_t pwm_henggun = 0;			//通道3，横滚
uint16_t pwm_paizhao = 0;			//通道5，拍照
uint16_t pwm_shexiang = 0;		//通道6，摄像
uint8_t pwm_jiaoju_re = 0;		//通道7，焦距变小
uint8_t pwm_jiaoju_in = 0;		//通道7，焦距变大		
uint16_t pwm_mode = 0;				//通道8，模式切换
uint8_t pwm_three_paizhao = 0;//字节28，三连拍
uint8_t pwm_init_flag = 1; 		//字节29，初始化标识：1、未初始化；0、初始化完成，可以正常操作
uint8_t can_value;
//add by yanly
uint8_t dongzuo_mode_value = DONGZUO_MODE_REC;  //初始时模式为录像
uint8_t record_value  = START_RECORD;
uint8_t record_state = RECORD_NONE;  //当前录像状态


//在子模块中，逐个字节的判断接收的数据，对zkrt指令解包，然后是设置的就设置完了返回响应，是读取的就读取完了返回值
zkrt_packet_t sub_camera_zkrt_packet_can1_rx;
uint8_t sub_camera_zkrt_recv_decode_and_zkrt_encode_ack(void)
{
	while (CAN1_rx_check() == 1)//check函数，通过buffer_get和buffer_store对比是否相等，来检验buffer里是否有数据。有数据返回1，无数据返回0
	{
		can_value = CAN1_rx_byte();//取得位于buffer_get位置上的数据
		if (zkrt_decode_char(&sub_camera_zkrt_packet_can1_rx,can_value)==1)//每个字符都判断一下，当接收到完整的数据时返回1
		{
			pwm_hangxiang = (sub_camera_zkrt_packet_can1_rx.data[1] << 8) | sub_camera_zkrt_packet_can1_rx.data[0];   //通道1，航向，字1-0
			pwm_fuyang = (sub_camera_zkrt_packet_can1_rx.data[3] << 8) | sub_camera_zkrt_packet_can1_rx.data[2];  	  //通道2，俯仰，字3-2
			pwm_henggun = (sub_camera_zkrt_packet_can1_rx.data[5] << 8) | sub_camera_zkrt_packet_can1_rx.data[4];  	  //通道3，横滚，字5-4
			
			//这四个对应3个通道的命令，根据命令调用相应的值
			pwm_paizhao = (sub_camera_zkrt_packet_can1_rx.data[9] << 8) | sub_camera_zkrt_packet_can1_rx.data[8];     //通道5，拍照，字8-9
			pwm_shexiang = (sub_camera_zkrt_packet_can1_rx.data[11] << 8) | sub_camera_zkrt_packet_can1_rx.data[10];  //通道6，录像，字10-11
			pwm_jiaoju_re = sub_camera_zkrt_packet_can1_rx.data[12];  																							  //通道7，焦距缩小，字12
			pwm_jiaoju_in = sub_camera_zkrt_packet_can1_rx.data[13];   																								//通道7，焦距增大，字13
			
			//这个对应工作模式，默认2000
			pwm_mode = (sub_camera_zkrt_packet_can1_rx.data[15] << 8) | sub_camera_zkrt_packet_can1_rx.data[14];   		//通道8，工作模式，字14-15
			pwm_three_paizhao = sub_camera_zkrt_packet_can1_rx.data[28];																							//三连拍
			pwm_init_flag = sub_camera_zkrt_packet_can1_rx.data[29];																									//初始化标识
			
			return 1;//解析成功了
		}
	}
	
	return 0;//没有最新的解析成功的操作
}


void action_hangxiang(void)
{
	if (pwm_mode == 2)					 //慢速模式
	{
		if (pwm_hangxiang > 1500)  //实际测试发现，给1000的时候向右，给2000向左，所以我们要取它的反向值
		{
			PWM_HANGXAING(1700);
		}
		else if (pwm_hangxiang < 1500)
		{
			PWM_HANGXAING(1300);
		}
		else
		{
			PWM_HANGXAING(1500);
		}
	}
	else
	{
		PWM_HANGXAING(3000-pwm_hangxiang);
	}
}

void action_fuyang(void)
{
	if (pwm_mode == 2)								//慢速模式
	{
		if (pwm_fuyang > 1500)
		{
			PWM_FUYANG(1700);
		}
		else if (pwm_fuyang < 1500)
		{
			PWM_FUYANG(1300);
		}
		else
		{
			PWM_FUYANG(1500);
		}
	}
	else
	{
		PWM_FUYANG(pwm_fuyang);
	}
}

void action_henggun(void)						//这里没有定义横滚！！！
{
	
}

////切换模式，从1500到1000
//void action_paizhao(void)
//{
//	if (GET_DONGZUO == 1500)
//	{
//		PWM_DONGZUO(1000);
//	}
//	else
//	{
//		PWM_DONGZUO(1500);
//	}
//}

////拍照是：空闲情况下1500，给2000拍一次
//void action_shexiang(void)
//{
//	if (GET_DONGZUO == 1500)
//	{
//		PWM_DONGZUO(2000);
//	}
//	else
//	{
//		PWM_DONGZUO(1500);
//	}

//}

//切换拍照/录像模式，中到低
void action_dongzuo_mode(uint8_t mode)
{
	if(dongzuo_mode_value == mode)
		return;

	if(record_state == START_RECORD)//当处于录像状态时，不能执行切换模式
	{
		return;
	}
	
	dongzuo_mode_value = mode;
	
	PWM_DONGZUO_MODE(1500);
	delay_ms(200);
	PWM_DONGZUO_MODE(1000);
}
//拍照 中到高
void action_paizhao(void)
{
	if(record_state == START_RECORD)//当处于录像状态时，不能执行拍照
	{
		return;
	}
	
	if (GET_DONGZUO == 1500)
	{
		PWM_DONGZUO(2000);
	}
	else
	{
		PWM_DONGZUO(1500);
	}
}
//开始录像: 中到高 
void action_start_record(void)
{
	if (GET_DONGZUO == 1500)
	{
		PWM_DONGZUO(2000);
	}
	else
	{
		PWM_DONGZUO(1500);
	}			
}	
//停止录像: 中到低
void action_stop_record(void)
{
	if (GET_DONGZUO == 1500)
	{
		PWM_DONGZUO(1000);
	}
	else
	{
		PWM_DONGZUO(1500);
	}				
}	
//执行开始录像或者停止录像
void action_shexiang(void)
{
	if(record_value == START_RECORD)
	{
		action_start_record();
		record_value = STOP_RECORD;
		record_state = START_RECORD;
	}	
	else
	{
		action_stop_record();
		record_value = START_RECORD;
		record_state = STOP_RECORD;
	}
}

//更改焦距，缩焦1000，扩焦2000，保持1500
void action_jiaoju(void)
{
	if ((last_jiaoju_re != pwm_jiaoju_re)&&(last_jiaoju_in == pwm_jiaoju_in))
	{
		PWM_JIAOJU(1000);    
		
	}
	else if ((last_jiaoju_re == pwm_jiaoju_re)&&(last_jiaoju_in != pwm_jiaoju_in))
	{
		PWM_JIAOJU(2000);      
	}
	else
	{
		PWM_JIAOJU(1500);						
	}
	
//	int jiaoju_value = 1500;
//	jiaoju_value = GET_JIAOJU;
//	
//	if ((last_jiaoju_re != pwm_jiaoju_re)&&(last_jiaoju_in == pwm_jiaoju_in))
//	{
//		if((jiaoju_value-200) <=1000)
//		{
//			jiaoju_value = 1000;
//		}
//		else
//		{	
//			jiaoju_value = jiaoju_value-200;	
//		}
//		PWM_JIAOJU(jiaoju_value);
//	}
//	else if ((last_jiaoju_re == pwm_jiaoju_re)&&(last_jiaoju_in != pwm_jiaoju_in))
//	{
//		if((jiaoju_value+200) >=2000)
//		{
//			jiaoju_value = 2000;
//		}
//		else
//		{	
//			jiaoju_value = jiaoju_value+200;	
//		}
//		PWM_JIAOJU(jiaoju_value); 
//	}
//	else
//	{
//		PWM_JIAOJU(1500);						
//	}
}

//有两种模式：低速锁头模式、高速跟随模式，要跟随模式。说明书推荐1500
void action_mode(void)
{
	if (pwm_mode == 0)				//跟随模式
	{
		PWM_MODE(2000);
	}
	else if (pwm_mode == 1)		//锁头模式
	{
		PWM_MODE(1000);
	}
	else if (pwm_mode == 2)		//跟随模式
	{
		PWM_MODE(1500);
	}
}


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

uint16_t pwm_hangxiang = 0;		//ͨ��1������
uint16_t pwm_fuyang = 0;			//ͨ��2������
uint16_t pwm_henggun = 0;			//ͨ��3�����
uint16_t pwm_paizhao = 0;			//ͨ��5������
uint16_t pwm_shexiang = 0;		//ͨ��6������
uint8_t pwm_jiaoju_re = 0;		//ͨ��7�������С
uint8_t pwm_jiaoju_in = 0;		//ͨ��7��������		
uint16_t pwm_mode = 0;				//ͨ��8��ģʽ�л�
uint8_t pwm_three_paizhao = 0;//�ֽ�28��������
uint8_t pwm_init_flag = 1; 		//�ֽ�29����ʼ����ʶ��1��δ��ʼ����0����ʼ����ɣ�������������
uint8_t can_value;
//add by yanly
uint8_t dongzuo_mode_value = DONGZUO_MODE_REC;  //��ʼʱģʽΪ¼��
uint8_t record_value  = START_RECORD;
uint8_t record_state = RECORD_NONE;  //��ǰ¼��״̬


//����ģ���У�����ֽڵ��жϽ��յ����ݣ���zkrtָ������Ȼ�������õľ��������˷�����Ӧ���Ƕ�ȡ�ľͶ�ȡ���˷���ֵ
zkrt_packet_t sub_camera_zkrt_packet_can1_rx;
uint8_t sub_camera_zkrt_recv_decode_and_zkrt_encode_ack(void)
{
	while (CAN1_rx_check() == 1)//check������ͨ��buffer_get��buffer_store�Ա��Ƿ���ȣ�������buffer���Ƿ������ݡ������ݷ���1�������ݷ���0
	{
		can_value = CAN1_rx_byte();//ȡ��λ��buffer_getλ���ϵ�����
		if (zkrt_decode_char(&sub_camera_zkrt_packet_can1_rx,can_value)==1)//ÿ���ַ����ж�һ�£������յ�����������ʱ����1
		{
			pwm_hangxiang = (sub_camera_zkrt_packet_can1_rx.data[1] << 8) | sub_camera_zkrt_packet_can1_rx.data[0];   //ͨ��1��������1-0
			pwm_fuyang = (sub_camera_zkrt_packet_can1_rx.data[3] << 8) | sub_camera_zkrt_packet_can1_rx.data[2];  	  //ͨ��2����������3-2
			pwm_henggun = (sub_camera_zkrt_packet_can1_rx.data[5] << 8) | sub_camera_zkrt_packet_can1_rx.data[4];  	  //ͨ��3���������5-4
			
			//���ĸ���Ӧ3��ͨ��������������������Ӧ��ֵ
			pwm_paizhao = (sub_camera_zkrt_packet_can1_rx.data[9] << 8) | sub_camera_zkrt_packet_can1_rx.data[8];     //ͨ��5�����գ���8-9
			pwm_shexiang = (sub_camera_zkrt_packet_can1_rx.data[11] << 8) | sub_camera_zkrt_packet_can1_rx.data[10];  //ͨ��6��¼����10-11
			pwm_jiaoju_re = sub_camera_zkrt_packet_can1_rx.data[12];  																							  //ͨ��7��������С����12
			pwm_jiaoju_in = sub_camera_zkrt_packet_can1_rx.data[13];   																								//ͨ��7������������13
			
			//�����Ӧ����ģʽ��Ĭ��2000
			pwm_mode = (sub_camera_zkrt_packet_can1_rx.data[15] << 8) | sub_camera_zkrt_packet_can1_rx.data[14];   		//ͨ��8������ģʽ����14-15
			pwm_three_paizhao = sub_camera_zkrt_packet_can1_rx.data[28];																							//������
			pwm_init_flag = sub_camera_zkrt_packet_can1_rx.data[29];																									//��ʼ����ʶ
			
			return 1;//�����ɹ���
		}
	}
	
	return 0;//û�����µĽ����ɹ��Ĳ���
}


void action_hangxiang(void)
{
	if (pwm_mode == 2)					 //����ģʽ
	{
		if (pwm_hangxiang > 1500)  //ʵ�ʲ��Է��֣���1000��ʱ�����ң���2000������������Ҫȡ���ķ���ֵ
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
	if (pwm_mode == 2)								//����ģʽ
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

void action_henggun(void)						//����û�ж�����������
{
	
}

////�л�ģʽ����1500��1000
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

////�����ǣ����������1500����2000��һ��
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

//�л�����/¼��ģʽ���е���
void action_dongzuo_mode(uint8_t mode)
{
	if(dongzuo_mode_value == mode)
		return;

	if(record_state == START_RECORD)//������¼��״̬ʱ������ִ���л�ģʽ
	{
		return;
	}
	
	dongzuo_mode_value = mode;
	
	PWM_DONGZUO_MODE(1500);
	delay_ms(200);
	PWM_DONGZUO_MODE(1000);
}
//���� �е���
void action_paizhao(void)
{
	if(record_state == START_RECORD)//������¼��״̬ʱ������ִ������
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
//��ʼ¼��: �е��� 
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
//ֹͣ¼��: �е���
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
//ִ�п�ʼ¼�����ֹͣ¼��
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

//���Ľ��࣬����1000������2000������1500
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

//������ģʽ��������ͷģʽ�����ٸ���ģʽ��Ҫ����ģʽ��˵�����Ƽ�1500
void action_mode(void)
{
	if (pwm_mode == 0)				//����ģʽ
	{
		PWM_MODE(2000);
	}
	else if (pwm_mode == 1)		//��ͷģʽ
	{
		PWM_MODE(1000);
	}
	else if (pwm_mode == 2)		//����ģʽ
	{
		PWM_MODE(1500);
	}
}


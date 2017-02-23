#include "sys.h"
#include "led.h"
#include "zkrt.h"
#include "adc.h"
#include "can.h"
#include "pwm.h"
#include "camera.h"

void bsp_init(void)
{
	SystemInit ();		/*系统初始化*/
	RCC_Configuration();
	SysTick_Init();
	LED_Init();
	ADC1_Init();
	CAN_Mode_Init(CAN_Mode_Normal);//CAN初始化环回模式,波特率1Mbps    	
	TIM_Init();
}

uint8_t status_camera[8] = {0XAA, 0XBB, 0XCC, 0XDD, 0XEE, 0X04, 0X00, 0X00};
uint8_t three_paizhao_flag = 0;//用于记录三连拍的次数

int main()
{
  bsp_init();
	while (1)
	{		
		if ((sub_camera_zkrt_recv_decode_and_zkrt_encode_ack()) == 1)
		{
			if (pwm_init_flag == 0)//如果初始化过了，该怎么处理怎么处理
			{
				action_hangxiang();
				action_fuyang();
				
				//判断拍照值
				if ((last_paizhao != pwm_paizhao) && (paizhao_count - TimingDelay > _TIM_PAIZHAO)) //如果这次的数据不等于上次的数据，并且超过了1S没有操作，那么是执行拍照
				{
					action_dongzuo_mode(DONGZUO_MODE_PIC);
					paizhao_count = TimingDelay;
					return_count = TimingDelay;
					action_paizhao();
				}
				
				//判断录像值
				if ((last_shexiang != pwm_shexiang)&&(paizhao_count - TimingDelay > _TIM_PAIZHAO)) //如果这次的数据不等于上次的数据，并且超过了1S没有操作，那么是执行录像
				{
					action_dongzuo_mode(DONGZUO_MODE_REC);
					paizhao_count = TimingDelay;
					return_count = TimingDelay;
					action_shexiang();
				}
				
				//判断焦距变化
				action_jiaoju();
				
				//模式切换
				action_mode();
			}
			
			//不管有没有初始化过，都要在处理完事后记录上一次操作，不过一般情况下不会有变化
			last_paizhao = pwm_paizhao;  		  
			last_shexiang = pwm_shexiang;   	 
			last_jiaoju_re = pwm_jiaoju_re;   
			last_jiaoju_in = pwm_jiaoju_in;   
		}
		
		//该动作由于是1个指令控制连续3次动作，不能放到上面包裹起来，要独立出来
		if ((last_three_paizhao != pwm_three_paizhao) && (paizhao_count - TimingDelay > _TIM_PAIZHAO))
		{
			paizhao_count = TimingDelay;
			action_shexiang();
			
			three_paizhao_flag++;
			if (three_paizhao_flag == 3)
			{
				last_three_paizhao = pwm_three_paizhao;
				three_paizhao_flag = 0;
			}
		}
		
		if (_10ms_count - TimingDelay >= 10)								
		{
			_10ms_count = TimingDelay;
			ADC_StartOfConversion(ADC1);											
			
			if ((_10ms_flag%10) == 0)												
			{				
				if (MAVLINK_TX_INIT_VAL - TimingDelay > 2000)	
				{
					bat_read();
					bat_check();
				}
			}
			
			if ((_10ms_flag%100) == 0)												
			{
				if (MAVLINK_TX_INIT_VAL - TimingDelay > 3000)	
				{
					status_camera[7]++;
					if (status_camera[7] == 0XFF)
					{
						status_camera[7] = 0;
					}
					Can_Send_Msg(status_camera, 8);							
				}
			}
			_10ms_flag++;
		}
		
		if (led_rx_count - TimingDelay > 50)
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_7);
		}
		
		if (led_tx_count - TimingDelay > 50)
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_6);
		}
		
		if (return_count - TimingDelay > _TIM_RETURN)
		{
			PWM_DONGZUO(1500);
			PWM_DONGZUO_MODE(1500);
			return_count = TimingDelay;
		}
	}
}




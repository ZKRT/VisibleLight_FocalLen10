#include "sys.h"
#include "core_cm0.h"
/*********************************************ÏµÍ³Ê±ÖÓ**********************/

void RCC_Configuration(void)
{
	uint32_t StartUpCounter = 0, HSIStatus = 0;
	
	RCC_DeInit();
	RCC_HSICmd(ENABLE);
	
	while((RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET) && (StartUpCounter != HSI_STARTUP_TIMEOUT))
	{
		StartUpCounter++;
	}
	
	if (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) != RESET)
	{
		HSIStatus = (uint32_t)0x01;
	}
	else
	{
		HSIStatus = (uint32_t)0x00;
	}	
	
	if (HSIStatus == (uint32_t)0x01)
  {
    /* Enable Prefetch Buffer */
		FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;
		
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		RCC_PCLKConfig(RCC_HCLK_Div1);
		
		RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_12);
		
		RCC_PLLCmd(ENABLE);

		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		{
		}
		
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

		while (RCC_GetSYSCLKSource() != (uint8_t)RCC_CFGR_SWS_PLL)
		{
		}
  }
  else
  {
		//say something else!
  }  
}

/************************************delay*********************************/

void SysTick_Init(void)
{ 
  while (SysTick_Config(SystemCoreClock / 1000) !=  0);	
}

void delay_us(uint16_t nus)	
{
	uint32_t start_num;	
	uint32_t temp;	
	uint32_t nus_pai;
	
	nus_pai = 48 * nus;
	start_num = SysTick->VAL;					
	do
	{
		temp=SysTick->VAL;								
		if (temp < start_num)					
		{
			temp = start_num - temp;				
		}
		else															
		{
			temp = start_num+48000-temp;	
		}
	}while (temp < nus_pai);      				
}

volatile uint32_t TimingDelay = 0XFFFFFFFF;		
volatile uint32_t led_rx_count = 0XFFFFFFFF;
volatile uint32_t led_tx_count = 0XFFFFFFFF;
volatile uint32_t _10ms_count = 0XFFFFFFFF;
volatile uint32_t _10ms_flag = 0;

void delay_ms(uint16_t nms)						
{
	uint32_t start_num;
	
	start_num = TimingDelay;								
	while ((start_num - TimingDelay) < nms);
}

void SysTick_Handler(void)
{
	TimingDelay--;
}




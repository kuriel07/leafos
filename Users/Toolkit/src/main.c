/**************************************************************
** 	火牛开发板
**	功能：RTC实验
**  版本：V1.0  
**	论坛：www.openmcu.com
**	淘宝：http://shop36995246.taobao.com/   
**  技术支持群：121939788 
***************************************************************/  

/* Includes ------------------------------------------------------------------*/
#include "..\..\defs.h"
#include "..\..\config.h"
//#include "usart.h"
//#include "ili932x.h"
//#include "rtc.h"
//#include "icc.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define RTCClockOutput_Enable  // RTC Clock/64 is output on tamper pin(PC.13)

/* Private macro -------------------------------------------------------------*/
extern uint8_t gImage_11[];

static __IO uint32_t TimingDelay;
void GPIO_Configuration(void);
void NVIC_Configuration(void);
void Time_Display(uint32_t TimeVar);
uint8_t KEY_Scan(void);
void Delay(__IO uint32_t nTime);

void PWM_SetDutyCycle(uint16_t cycle) {
	uint16_t compare_value = (((uint32_t)cycle * 50000) / 100) ;
	TIM_SetCompare1(TIM1, compare_value);
	TIM_SetCompare2(TIM1, 50000);
}
/* Private functions ---------------------------------------------------------*/

/*****************************************************************************
** 函数名称: main
** 功能描述: 主函数入口
** 作  　者: Dream
** 日　  期: 2010年12月17日
*****************************************************************************/
u8 g_baAtr[38];
u8 g_bAtrLen;
int main(void)
{
	TIM_ICInitTypeDef TIM_ICInitStructure;
	TIM_TimeBaseInitTypeDef TIM_BaseInitStructure; 
  	NVIC_InitTypeDef NVIC_InitStructure;
//	icc_context scard_ctx;
	SystemInit();	   		//配置系统时钟72M(包括clock, PLL and Flash configuration)
	while(SysTick_Config(SystemFrequency / 1000));	//Systick 配置延时n*ms
	//while(SysTick_Config(20));	//Systick 配置延时n*ms
	GPIO_Configuration();
	//TIM_DeInit( TIM1);                              //复位TIM2定时苼E
	//RCC_APB1PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);   //使能定时苼E   
	//USART_Configuration();	//异步通信初始化
	NVIC_Configuration();	//系统中断配置
	//NVIC_EnableIRQ(TIM1_CC_IRQn);
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
	

  	/* Enable the TIM1_CC_IRQ Interrupt */
  	//NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
  	//NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  	//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	//NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	///NVIC_Init(&NVIC_InitStructure);
		//initialize input capture
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICFilter = 0;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
		//TIM_PWMIConfig(TIM1, &TIM_ICInitStructure);
		
		TIM_BaseInitStructure.TIM_Period = 50000; 		//设置自动重载寄存器值为畜值
	TIM_BaseInitStructure.TIM_Prescaler = 71;  		//自定义预分频
	TIM_BaseInitStructure.TIM_ClockDivision = 0; 
	TIM_BaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;   //向上计数模式
	TIM_BaseInitStructure.TIM_RepetitionCounter = 0; 
	//TIM_TimeBaseInit(TIM1, &TIM_BaseInitStructure); 
	
	//清中断，以免一启用中断后立即产生中断 
	//TIM_ClearFlag(TIM1, TIM_FLAG_Update); 
	
	//TIM_ITConfig(TIM1, TIM_IT_CC1, ENABLE);		//interrupt cc1 enabled
	//TIM_ITConfig(TIM1, TIM_IT_CC2, ENABLE);		//interrupt cc2 enabled
	//TIM_Cmd(TIM1, ENABLE); 
	//LCD_Init();					// LCD初始化

	//LCD_Clear(BLACK);
	/*
	icc_init(&scard_ctx);
		while(1) {
			while(icc_active(&scard_ctx) != 0) ;		//wait for card inserted
			//Delay(200);
			icc_reset_card(&scard_ctx, 0);
			g_bAtrLen = icc_get_atr(&scard_ctx, g_baAtr, 1000);
			//Delay(2000);
			//Delay(200);
			icc_transmit(&scard_ctx, (BYTE *)"\xDD\xA4\x04\x00\x08\xA0\x00\x00\x00\x03\x00\x00\x00", 13, g_baAtr);
			icc_transmit(&scard_ctx, (BYTE *)"\x00\xA4\x04\x00\x08\xA0\x00\x00\x00\x03\x00\x00\x00", 13, g_baAtr);
			while(1);
		}
		*/
		while(1);
}
/*****************************************************************************
** 函数名称: GPIO_Configuration
** 功能描述: 配置IO口
** 作  　者: Dream
** 日　  期: 2010年12月17日
*****************************************************************************/
void GPIO_Configuration(void)
{
	
		
   	/* 配置 USART1 Tx (PA9) */
  	//GPIO_InitStructure.GPIO_Pin = GPIO_PIN_9;
  	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	 		//复用开漏输出模式
  	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//输出最大频率为50MHz
  	//GPIO_Init(GPIOA, &GPIO_InitStructure);
    
  	/* 配置 USART1 Rx (PA10)  */
  	//GPIO_InitStructure.GPIO_Pin = GPIO_PIN_10;
  	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入模式
  	//GPIO_Init(GPIOA, &GPIO_InitStructure);
}
/*****************************************************************************
** 函数名称: KEY_Scan
** 功能描述: 按键处理函数
				0，没有任何按键按下	  1，KEY1按下 
				 2，KEY2按下 
** 作  　者: Dream
** 日　  期: 2010年12月17日
*****************************************************************************/
uint8_t KEY_Scan(void)
{	 
	static uint8_t KEY_UP=1;	//按键按松开标志	   
	if(KEY_UP&&(KEY1==0||KEY2==0))
	{
		Delay(10);				//去抖动 
		KEY_UP=0;
		if(KEY1==0)return 1;
		else if(KEY2==0)return 2;
	}else if(KEY1==1&&KEY2==1)KEY_UP=1; 	    
	return 0;					//无按键按下
}
/*****************************************************************************
** 函数名称: Delay
** 功能描述: 用Systick延时
** 作  　者: Dream
** 日　  期: 2010年12月17日
*****************************************************************************/
void Delay(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;

  while(TimingDelay != 0);
}
/*****************************************************************************
** 函数名称: TimingDelay_Decrement
** 功能描述: Systick中断进入函数
** 作  　者: Dream
** 日　  期: 2010年12月17日
*****************************************************************************/
void TimingDelay_Decrement(void)
{
	static u8 btick =0;
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
  btick = !btick;
  if(btick) {
	//GPIO_ResetBits(GPIOA, GPIO_PIN_4);
  } else {
	//GPIO_SetBits(GPIOA, GPIO_PIN_4);	
  }
}
/*****************************************************************************
** 函数名称: NVIC_Configuration
** 功能描述: 系统中断管理
** 作  　者: Dream
** 日　  期: 2010年12月17日
*****************************************************************************/
void NVIC_Configuration(void)
{
  	NVIC_InitTypeDef NVIC_InitStructure;

	#ifdef  VECT_TAB_RAM  
	  /* Set the Vector Table base location at 0x20000000 */ 
	  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
	#else  /* VECT_TAB_FLASH  */
	  /* Set the Vector Table base location at 0x08000000 */ 
	  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
	#endif

	/* Configure one bit for preemption priority */
  	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  	/* Enable the RTC Interrupt */
  	//NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
  	//NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  	//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	//NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	//NVIC_Init(&NVIC_InitStructure);
  	//NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  	//NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  	//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	//NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	//NVIC_Init(&NVIC_InitStructure);
}
#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/*******************************************************************************
** End of File
********************************************************************************/

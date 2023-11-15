/**************************************************************
** 	��ţ������
**	���ܣ�RTCʵ��
**  �汾��V1.0  
**	��̳��www.openmcu.com
**	�Ա���http://shop36995246.taobao.com/   
**  ����֧��Ⱥ��121939788 
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
** ��������: main
** ��������: ���������
** ��  ����: Dream
** �ա�  ��: 2010��12��17��
*****************************************************************************/
u8 g_baAtr[38];
u8 g_bAtrLen;
int main(void)
{
	TIM_ICInitTypeDef TIM_ICInitStructure;
	TIM_TimeBaseInitTypeDef TIM_BaseInitStructure; 
  	NVIC_InitTypeDef NVIC_InitStructure;
//	icc_context scard_ctx;
	SystemInit();	   		//����ϵͳʱ��72M(����clock, PLL and Flash configuration)
	while(SysTick_Config(SystemFrequency / 1000));	//Systick ������ʱn*ms
	//while(SysTick_Config(20));	//Systick ������ʱn*ms
	GPIO_Configuration();
	//TIM_DeInit( TIM1);                              //��λTIM2��ʱƁE
	//RCC_APB1PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);   //ʹ�ܶ�ʱƁE   
	//USART_Configuration();	//�첽ͨ�ų�ʼ��
	NVIC_Configuration();	//ϵͳ�ж�����
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
		
		TIM_BaseInitStructure.TIM_Period = 50000; 		//�����Զ����ؼĴ���ֵΪ����ֵ
	TIM_BaseInitStructure.TIM_Prescaler = 71;  		//�Զ���Ԥ��Ƶ
	TIM_BaseInitStructure.TIM_ClockDivision = 0; 
	TIM_BaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;   //���ϼ���ģʽ
	TIM_BaseInitStructure.TIM_RepetitionCounter = 0; 
	//TIM_TimeBaseInit(TIM1, &TIM_BaseInitStructure); 
	
	//���жϣ�����һ�����жϺ����������ж� 
	//TIM_ClearFlag(TIM1, TIM_FLAG_Update); 
	
	//TIM_ITConfig(TIM1, TIM_IT_CC1, ENABLE);		//interrupt cc1 enabled
	//TIM_ITConfig(TIM1, TIM_IT_CC2, ENABLE);		//interrupt cc2 enabled
	//TIM_Cmd(TIM1, ENABLE); 
	//LCD_Init();					// LCD��ʼ��

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
** ��������: GPIO_Configuration
** ��������: ����IO��
** ��  ����: Dream
** �ա�  ��: 2010��12��17��
*****************************************************************************/
void GPIO_Configuration(void)
{
	
		
   	/* ���� USART1 Tx (PA9) */
  	//GPIO_InitStructure.GPIO_Pin = GPIO_PIN_9;
  	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	 		//���ÿ�©���ģʽ
  	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//������Ƶ��Ϊ50MHz
  	//GPIO_Init(GPIOA, &GPIO_InitStructure);
    
  	/* ���� USART1 Rx (PA10)  */
  	//GPIO_InitStructure.GPIO_Pin = GPIO_PIN_10;
  	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//��������ģʽ
  	//GPIO_Init(GPIOA, &GPIO_InitStructure);
}
/*****************************************************************************
** ��������: KEY_Scan
** ��������: ����������
				0��û���κΰ�������	  1��KEY1���� 
				 2��KEY2���� 
** ��  ����: Dream
** �ա�  ��: 2010��12��17��
*****************************************************************************/
uint8_t KEY_Scan(void)
{	 
	static uint8_t KEY_UP=1;	//�������ɿ���־	   
	if(KEY_UP&&(KEY1==0||KEY2==0))
	{
		Delay(10);				//ȥ���� 
		KEY_UP=0;
		if(KEY1==0)return 1;
		else if(KEY2==0)return 2;
	}else if(KEY1==1&&KEY2==1)KEY_UP=1; 	    
	return 0;					//�ް�������
}
/*****************************************************************************
** ��������: Delay
** ��������: ��Systick��ʱ
** ��  ����: Dream
** �ա�  ��: 2010��12��17��
*****************************************************************************/
void Delay(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;

  while(TimingDelay != 0);
}
/*****************************************************************************
** ��������: TimingDelay_Decrement
** ��������: Systick�жϽ��뺯��
** ��  ����: Dream
** �ա�  ��: 2010��12��17��
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
** ��������: NVIC_Configuration
** ��������: ϵͳ�жϹ���
** ��  ����: Dream
** �ա�  ��: 2010��12��17��
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

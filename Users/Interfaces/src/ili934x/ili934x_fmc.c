/* Includes ------------------------------------------------------------------*/
#include "defs.h"
#include "config.h"
#if (SHARD_LCD_TYPE & 0xFFF0) == 0x9340
#include "..\..\inc\ili932x\ili932x.h"
#include "..\..\inc\ili932x\fonts.h"
#include "..\..\inc\ili932x\hz16.h"

//FMC configuration for LCD (bank 1)
#define LCD_BUFFER		(*((volatile short *)0x60800000))
#define LCD_REG			(*((volatile short *)0x60000000))

#ifdef DMA2D_ENABLED

#endif

#define ILI9341_TFTWIDTH  240
#define ILI9341_TFTHEIGHT 320
	
#define ILI9341_NOP     0x00
#define ILI9341_SWRESET 0x01
#define ILI9341_RDDID   0x04
#define ILI9341_RDDST   0x09
	
#define ILI9341_SLPIN   0x10
#define ILI9341_SLPOUT  0x11
#define ILI9341_PTLON   0x12
#define ILI9341_NORON   0x13
	
#define ILI9341_RDMODE  0x0A
#define ILI9341_RDMADCTL  0x0B
#define ILI9341_RDPIXFMT  0x0C
#define ILI9341_RDIMGFMT  0x0D
#define ILI9341_RDSELFDIAG  0x0F
	
#define ILI9341_INVOFF  0x20
#define ILI9341_INVON   0x21
#define ILI9341_GAMMASET 0x26
#define ILI9341_DISPOFF 0x28
#define ILI9341_DISPON  0x29
	
#define ILI9341_CASET   0x2A
#define ILI9341_PASET   0x2B
#define ILI9341_RAMWR   0x2C
#define ILI9341_RAMRD   0x2E
	
#define ILI9341_PTLAR   0x30
#define ILI9341_MADCTL  0x36
#define ILI9341_PIXFMT  0x3A
	
#define ILI9341_FRMCTR1 0xB1
#define ILI9341_FRMCTR2 0xB2
#define ILI9341_FRMCTR3 0xB3
#define ILI9341_INVCTR  0xB4
#define ILI9341_DFUNCTR 0xB6
	
#define ILI9341_PWCTR1  0xC0
#define ILI9341_PWCTR2  0xC1
#define ILI9341_PWCTR3  0xC2
#define ILI9341_PWCTR4  0xC3
#define ILI9341_PWCTR5  0xC4
#define ILI9341_VMCTR1  0xC5
#define ILI9341_VMCTR2  0xC7
	
#define ILI9341_RDID1   0xDA
#define ILI9341_RDID2   0xDB
#define ILI9341_RDID3   0xDC
#define ILI9341_RDID4   0xDD
	
#define ILI9341_GMCTRP1 0xE0
#define ILI9341_GMCTRN1 0xE1

uint16_t POINT_COLOR = BLUE, BACK_COLOR = WHITE;  

SRAM_HandleTypeDef hsram1;
DMA_HandleTypeDef dma1;

static uint32_t FMC_Initialized = 0;
static void HAL_DMA_TransferCompleted(DMA_HandleTypeDef *hdma) {
	uint32_t err = HAL_DMA_GetError(hdma);
	if(err == 0) {
		err = 0;
	}
}

static void HAL_FMC_MspInit(void){
  /* USER CODE BEGIN FMC_MspInit 0 */

  /* USER CODE END FMC_MspInit 0 */
  GPIO_InitTypeDef GPIO_InitStruct;
  if (FMC_Initialized) {
    return;
  }
  FMC_Initialized = 1;
  /* Peripheral clock enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_FMC_CLK_ENABLE();
  
  //initialize DMA configuration
  __HAL_RCC_DMA2_CLK_ENABLE();
  dma1.Instance = DMA2_Stream0;
  dma1.Init.Channel = DMA_CHANNEL_0;
  dma1.Init.Direction = DMA_MEMORY_TO_MEMORY;//DMA_MEMORY_TO_PERIPH;
  dma1.Init.Mode = DMA_NORMAL;
  dma1.Init.Priority = DMA_PRIORITY_MEDIUM;
  dma1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  dma1.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
  //source
  dma1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  dma1.Init.PeriphInc = DMA_PINC_ENABLE;
  dma1.Init.PeriphBurst = DMA_PBURST_INC4;
  //destination
  dma1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  dma1.Init.MemBurst = DMA_MBURST_SINGLE;
  dma1.Init.MemInc = DMA_MINC_ENABLE;
  
  HAL_DMA_Init(&dma1);
  HAL_DMA_IRQHandler(&dma1);
  HAL_DMA_RegisterCallback(&dma1, HAL_DMA_XFER_CPLT_CB_ID, HAL_DMA_TransferCompleted);
  //__HAL_DMA_ENABLE(&dma1);
  /** FMC GPIO Configuration  
  PE6   ------> FMC_A22
  PE7   ------> FMC_D4
  PE8   ------> FMC_D5
  PE9   ------> FMC_D6
  PE10   ------> FMC_D7
  PE11   ------> FMC_D8
  PE12   ------> FMC_D9
  PE13   ------> FMC_D10
  PE14   ------> FMC_D11
  PE15   ------> FMC_D12
  PD8   ------> FMC_D13
  PD9   ------> FMC_D14
  PD10   ------> FMC_D15
  PD14   ------> FMC_D0
  PD15   ------> FMC_D1
  PD0   ------> FMC_D2
  PD1   ------> FMC_D3
  PD4   ------> FMC_NOE
  PD5   ------> FMC_NWE
  PD7   ------> FMC_NE1
  */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9 
                          |GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13 
                          |GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
#if 1
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
#else
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
#endif
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  //A22
  //GPIO_InitStruct.Pin = GPIO_PIN_6;
  //GPIO_InitStruct.Pull = GPIO_PULLUP;
  //GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  //HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_14 
                          |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4 
                          |GPIO_PIN_5|GPIO_PIN_7;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
#if 1
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
#else
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
#endif
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

void HAL_SRAM_MspInit(SRAM_HandleTypeDef* hsram){
  /* USER CODE BEGIN SRAM_MspInit 0 */

  /* USER CODE END SRAM_MspInit 0 */
  HAL_FMC_MspInit();
  /* USER CODE BEGIN SRAM_MspInit 1 */

  /* USER CODE END SRAM_MspInit 1 */
}

static uint32_t FMC_DeInitialized = 0;

static void HAL_FMC_MspDeInit(void){
  /* USER CODE BEGIN FMC_MspDeInit 0 */

  /* USER CODE END FMC_MspDeInit 0 */
  if (FMC_DeInitialized) {
    return;
  }
  FMC_DeInitialized = 1;
  /* Peripheral clock enable */
  __HAL_RCC_FMC_CLK_DISABLE();
  
  /** FMC GPIO Configuration  
  PE6   ------> FMC_A22
  PE7   ------> FMC_D4
  PE8   ------> FMC_D5
  PE9   ------> FMC_D6
  PE10   ------> FMC_D7
  PE11   ------> FMC_D8
  PE12   ------> FMC_D9
  PE13   ------> FMC_D10
  PE14   ------> FMC_D11
  PE15   ------> FMC_D12
  PD8   ------> FMC_D13
  PD9   ------> FMC_D14
  PD10   ------> FMC_D15
  PD14   ------> FMC_D0
  PD15   ------> FMC_D1
  PD0   ------> FMC_D2
  PD1   ------> FMC_D3
  PD4   ------> FMC_NOE
  PD5   ------> FMC_NWE
  PD7   ------> FMC_NE1
  */
  HAL_GPIO_DeInit(GPIOE, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9 
                          |GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13 
                          |GPIO_PIN_14|GPIO_PIN_15);

  HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_14 
                          |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4 
                          |GPIO_PIN_5|GPIO_PIN_7);
}

void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef* hdma2d)
{
  if(hdma2d->Instance==DMA2D)
  {
    __HAL_RCC_DMA2D_CLK_ENABLE();
  }

}

void HAL_DMA2D_MspDeInit(DMA2D_HandleTypeDef* hdma2d)
{
  if(hdma2d->Instance==DMA2D)
  {
    __HAL_RCC_DMA2D_CLK_DISABLE();
  }

}

void LCD_WriteDataBurst(uint16_t addr, void * data, uint8 size) {
	uint8 i;
	for(i=0;i<size;i++) {
		Write_Dat(((uint8 *)data)[i]);
	}
}

void LCD_WriteRegBurst(uint16_t addr, void * data, uint8 size) {
	Write_Cmd(addr);
	LCD_WriteDataBurst(0, data, size);
}
/*****************************************************************************
** º¯ÊýÃû³Æ: LCD_Write_Reg
** ¹¦ÄÜÃèÊE Ð´Ö¸ÁûØ°Êý¾Ý
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_Dat)
{
	Write_Cmd(LCD_Reg);
	Write_Dat(LCD_Dat);
}


#define W_CS  	PDout(7)
#define W_RS	PEout(6)
#define W_WR  	PDout(5)
#define W_RD	PDout(4)
#define CS0		GPIO_ResetBits(GPIOD, GPIO_PIN_7)
#define CS1		GPIO_SetBits(GPIOD, GPIO_PIN_7)
#define RS0		GPIO_ResetBits(GPIOE, GPIO_PIN_6)
#define RS1		GPIO_SetBits(GPIOE, GPIO_PIN_6)
#define WR0		GPIO_ResetBits(GPIOD, GPIO_PIN_5)
#define WR1		GPIO_SetBits(GPIOD, GPIO_PIN_5)
#define RD0		GPIO_ResetBits(GPIOD, GPIO_PIN_4)
#define RD1		GPIO_SetBits(GPIOD, GPIO_PIN_4)
void Write_Cmd(uint16_t cmd)
{
#if 1
	uint8 i;
	LCD_REG = cmd;
	for(i=0;i<27;i++);
#else
	int i;
	uint16 edat, ddat;
	RS0;
	CS0;
	RD1;
	edat = (cmd >> 4) & 0x1FF;
	edat = (edat << 7);
	GPIOE->ODR = (GPIOE->ODR&0x007f)|(edat&0xff80);
	ddat = (cmd & 0x03) << 14;
	ddat |= (cmd >> 2) & 0x03;
	ddat |= (cmd & 0xe000) >> 5;
	GPIOD->ODR = (GPIOD->ODR&0x38Fc)|(ddat&0xc703); 
	WR0; //= 0;
	WR1; //= 1;	
	CS1; //= 1;
#endif
}

void Write_Dat(uint16_t dat)
{
#if 1
	uint8 i;
	LCD_BUFFER = dat;
	for(i=0;i<27;i++);
#else
	int i;
	uint16 edat, ddat;
	RS1;
	CS0;
	RD1;
	edat = (dat >> 4) & 0x1FF;
	edat = (edat << 7);
	GPIOE->ODR = (GPIOE->ODR&0x007f)|(edat&0xff80);
	ddat = (dat & 0x03) << 14;
	ddat |= (dat >> 2) & 0x03;
	ddat |= (dat & 0xe000) >> 5;
	GPIOD->ODR = (GPIOD->ODR&0x38Fc)|(ddat&0xc703); 
	WR0; //= 0;
	WR1; //= 1;	
	CS1; //= 1;
#endif
}

uint16_t LCD_ReadReg(uint16_t addr)
{
	uint16_t temp;
	LCD_REG = addr;
	temp = LCD_BUFFER;
	return temp;   
}

uint16_t LCD_ReadDat()
{
	uint16_t temp;		
	temp = LCD_BUFFER;
	return temp;   
}

TIM_HandleTypeDef g_TIM3_BaseInitStructure;
void LCD_Configuration()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//init backlight	
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	//init GPIOC_12 for non-pwm backlight
	GPIO_InitStructure.Pin = GPIO_PIN_12;
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (output function push pull)	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
	//init GPIOB_0 for PWM backlight
	GPIO_InitStructure.Pin = GPIO_PIN_0;
  	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;			//peripheral (output function push pull)
	//GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Alternate = GPIO_AF2_TIM3;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	LCD_SetBacklight(0, 50);		//default duty cycle = 50%
}

void LCD_SetBacklight(uint8_t mode, uint8 intensity) {
	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_SlaveConfigTypeDef sSlaveConfig;
	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_OC_InitTypeDef sConfigOC;
	//RCC_APB1PeriphClockCmd(
	__HAL_RCC_TIM3_CLK_ENABLE();
	uint32 freq = HAL_RCC_GetPCLK1Freq();
	uint32 period = freq / PWM_MAX_PERIOD;
	
	if(intensity > 100) intensity = 100;
	freq = HAL_RCC_GetPCLK1Freq();
	period = freq / PWM_MAX_PERIOD;
	//__HAL_RCC_TIM3_CLK_ENABLE();
	
	g_TIM3_BaseInitStructure.Instance = TIM3;
	g_TIM3_BaseInitStructure.Init.Prescaler = 0;
	g_TIM3_BaseInitStructure.Init.CounterMode = TIM_COUNTERMODE_UP;
	g_TIM3_BaseInitStructure.Init.Period = 10000;
	g_TIM3_BaseInitStructure.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	g_TIM3_BaseInitStructure.Init.RepetitionCounter = 0;
#ifdef STM32F7
	g_TIM3_BaseInitStructure.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
#endif
	
	if (HAL_TIM_PWM_Init(&g_TIM3_BaseInitStructure) != HAL_OK)
	{
		//_Error_Handler(__FILE__, __LINE__);
	}

	//sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	//if (HAL_TIM_ConfigClockSource(&g_TIM3_BaseInitStructure, &sClockSourceConfig) != HAL_OK)
	//{
		//_Error_Handler(__FILE__, __LINE__);
	//}

	//sSlaveConfig.SlaveMode = TIM_SLAVEMODE_TRIGGER;
	//sSlaveConfig.InputTrigger = TIM_TS_ITR0;
	//if (HAL_TIM_SlaveConfigSynchronization(&g_TIM3_BaseInitStructure, &sSlaveConfig) != HAL_OK)
	//{
		//_Error_Handler(__FILE__, __LINE__);
	//}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&g_TIM3_BaseInitStructure, &sMasterConfig) != HAL_OK)
	{
		//_Error_Handler(__FILE__, __LINE__);
	}

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = intensity * 100;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_SET;
	if (HAL_TIM_PWM_ConfigChannel(&g_TIM3_BaseInitStructure, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
	{
		//_Error_Handler(__FILE__, __LINE__);
	}

	//HAL_TIM_MspPostInit(&g_TIM3_BaseInitStructure);
	
	if(mode) {
		//HAL_TIM_PWM_Start(&g_TIM3_BaseInitStructure, 3);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);		//activate backlight non-pwm
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);		//activate backlight non-pwm
		HAL_TIM_Base_Start(&g_TIM3_BaseInitStructure);
		HAL_TIM_PWM_Start(&g_TIM3_BaseInitStructure, TIM_CHANNEL_3);
	} else {
		//HAL_TIM_PWM_Stop(&g_TIM3_BaseInitStructure, 3);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);		//shut-down backlight non-pwm
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);		//activate backlight non-pwm
		HAL_TIM_PWM_Stop(&g_TIM3_BaseInitStructure, TIM_CHANNEL_3);
	}
}

void LCD_DMA_Init(void * p) {
	DMA2D_HandleTypeDef * handle = (DMA2D_HandleTypeDef *)p;
	FMC_NORSRAM_TimingTypeDef Timing;
	//initialize FSMC
	HAL_FMC_MspInit();
	//enable DMA2D
    __HAL_RCC_DMA2D_CLK_ENABLE();
	/** Perform the SRAM1 memory initialization sequence
	*/
	hsram1.Instance = FMC_NORSRAM_DEVICE;
	hsram1.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
	/* hsram1.Init */
	hsram1.Init.NSBank = FMC_NORSRAM_BANK1;							//0x60000000, RS = 0x60400000
	hsram1.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16;
	hsram1.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
	hsram1.Init.MemoryType = FMC_MEMORY_TYPE_NOR;
	hsram1.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_ENABLE;
	hsram1.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
	hsram1.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
	hsram1.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
	hsram1.Init.WaitSignal = FMC_WAIT_SIGNAL_ENABLE;
	hsram1.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
	hsram1.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
	hsram1.Init.WriteBurst = FMC_WRITE_BURST_ENABLE;
	hsram1.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ASYNC;
	hsram1.Init.WriteFifo = FMC_WRITE_FIFO_ENABLE;
	hsram1.Init.PageSize = FMC_PAGE_SIZE_NONE;

	Timing.AddressSetupTime = 0;
	Timing.AddressHoldTime = 0;
	Timing.DataSetupTime = 3;
	Timing.BusTurnAroundDuration = 0;
	Timing.CLKDivision = 1;
	Timing.DataLatency = 0;
	Timing.AccessMode = FMC_ACCESS_MODE_A;
	/* ExtTiming */

	if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
	{
		//Error_Handler();
		while(1);
	}
	
	//DMA2D configuration
	handle->Instance = DMA2D;
	handle->Init.Mode = DMA2D_M2M;
	handle->Init.ColorMode = DMA2D_OUTPUT_RGB565;				//LCD output
	handle->Init.OutputOffset = 0;
	handle->XferCpltCallback  = NULL;		//no callback
	handle->XferErrorCallback = NULL;		//no callback
	//foreground layer configuration
	handle->LayerCfg[1].AlphaMode = DMA2D_COMBINE_ALPHA;		//top layer combine alpha with background
	handle->LayerCfg[1].InputAlpha = 0;
	handle->LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
	handle->LayerCfg[1].InputOffset = 0;
	
	/* Background layer Configuration */
	handle->LayerCfg[0].AlphaMode = DMA2D_NO_MODIF_ALPHA;		//background layer, no alpha transparency
	handle->LayerCfg[0].InputAlpha = 0x7F; /* 127 : semi-transparent */
	handle->LayerCfg[0].InputColorMode = DMA2D_INPUT_ARGB8888;	//with transparency
	handle->LayerCfg[0].InputOffset = 0x0; /* No offset in input */
	if (HAL_DMA2D_Init(handle) != HAL_OK)
	{
		//Error_Handler();
		while(1);
	}

	/* Apply DMA2D Foreground configuration */
	HAL_DMA2D_ConfigLayer(handle, 1);

	/* Apply DMA2D Background configuration */
	HAL_DMA2D_ConfigLayer(handle, 0);
	/*
	hal_status = HAL_DMA2D_BlendingStart_IT(handle,
                                          (uint32_t)&RGB565_240x130_1,
                                          (uint32_t)&RGB565_240x130_2,
                                          (uint32_t)&aBlendedImage,
                                          LAYER_SIZE_X,
                                          LAYER_SIZE_Y);
	*/
}

void LCD_Init(void)
{
	static uint16_t DeviceCode;
	uint8 ret;
	LCD_Configuration(); 
	
	LCD_WriteRegBurst(ILI9341_SWRESET, 0, 0);
	LCD_Delay(120); 			// LCD_Delay 120 ms
	
	LCD_WriteRegBurst(0xCF, (uint8[]){0x00,0xC3, 0x30}, 3);
       
	LCD_WriteRegBurst(0xED, (uint8[]){0x64, 0x03, 0X12, 0X81}, 4);
      
	LCD_WriteRegBurst(0xE8, (uint8[]){0x85, 0x10, 0x79}, 3);

	LCD_WriteRegBurst(0xCB, (uint8[]){0x39, 0x2C, 0x00, 0x34, 0x02}, 5);
       
	LCD_WriteRegBurst(0xF7, (uint8[]){0x20}, 1);
      
	LCD_WriteRegBurst(0xEA, (uint8[]){0x00, 0x00}, 2);
	
	//Power control 
	LCD_WriteRegBurst(0xC0, (uint8[]){0x22}, 1);
      
 	//Power control 
	LCD_WriteRegBurst(0xC1, (uint8[]){0x11}, 1);
      
 	//VCM control 
	LCD_WriteRegBurst(0xC5, (uint8[]){0x3d, 0x20}, 2);
      
 	//VCM control2 
	LCD_WriteRegBurst(0xC7, (uint8[]){0xAA}, 1);
    
 	// Memory Access Control 
	LCD_WriteRegBurst(0x36, (uint8[]){0x08}, 1);
      
 	//pixel format
	LCD_WriteRegBurst(0x3A, (uint8[]){0x55}, 1);
    
	LCD_WriteRegBurst(0xB1, (uint8[]){0x00, 0x1B}, 2); 
      
 	// Display Function Control  
	LCD_WriteRegBurst(0xB6, (uint8[]){0x08, 0x82, 0x27}, 3);
    
	LCD_WriteRegBurst(0xF6, (uint8[]){0x01, 0x30}, 2); 
      
 	// 3Gamma Function Disable 
	LCD_WriteRegBurst(0xF2, (uint8[]){0x00}, 1);
      
 	//Gamma curve selected 
	LCD_WriteRegBurst(0x26, (uint8[]){0x01}, 1);
      
 	//Set Gamma 
	LCD_WriteRegBurst(0xE0, (uint8[]){0x0F, 0x3F, 0x2F, 0x0C, 0x10, 0x0A, 0x53, 0XD5, 0x40, 0x0A, 0x13, 0x03, 0x08, 0x03, 0x00}, 15);
      
 	//Set Gamma 
	LCD_WriteRegBurst(0XE1, (uint8[]){0x00, 0x00, 0x10, 0x03, 0x0F, 0x05, 0x2C, 0xA2, 0x3F, 0x05, 0x0E, 0x0C, 0x37, 0x3C, 0x0F}, 15); 
      
	//Write_Cmd(0x11);    //Exit Sleep 
	LCD_WriteRegBurst(0x11, 0, 0); 
     
	LCD_Delay(120);
 	//Write_Cmd(0x29);    //Display on 
	LCD_WriteRegBurst(0x29, 0, 0); 
	LCD_Delay(50);
}

void LCD_DisplayOn() {
	LCD_WriteRegBurst(ILI9341_SLPOUT, 0, 0); 
	LCD_WriteRegBurst(ILI9341_DISPON, 0, 0); 
}

void LCD_DisplayOff() {
	LCD_WriteRegBurst(ILI9341_DISPOFF, 0, 0); 
	LCD_Delay(10); 
	LCD_WriteRegBurst(ILI9341_SLPIN, 0, 0); 
	LCD_Delay(10); 
}

void LCD_DrawPoint(uint16_t x,uint16_t y)
{	
	LCD_SetDisplayWindow(x, y, 1, 1);
	Write_Dat(POINT_COLOR); 	
}

void LCD_DrawPointColor(uint16_t x,uint16_t y, uint16_t color)
{
	LCD_SetDisplayWindow(x, y, 1, 1);
	Write_Dat(color); 	
}

void LCD_WriteRAM_Prepare()
{
	LCD_WriteRegBurst(ILI9341_RAMWR, 0, 0); 
}

void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
		
} 

void LCD_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width)
{
	uint8 buffer[4];
	
	buffer[0] = Ypos>>8;
	buffer[1] = Ypos & 0xFF;
	buffer[2] = (Ypos+Width) >> 8;
	buffer[3] = (Ypos+Width) & 0xFF;
	LCD_WriteRegBurst(ILI9341_PASET, buffer, 4); 
	buffer[0] = Xpos >> 8;
	buffer[1] = Xpos & 0xFF;
	buffer[2] = (Xpos+Height) >> 8;
	buffer[3] = (Xpos+Height) & 0xFF;
	LCD_WriteRegBurst(ILI9341_CASET, buffer, 4); 

	LCD_WriteRegBurst(ILI9341_RAMWR, 0, 0); 
}

#define MAX_CHAR_POSX 304
#define MAX_CHAR_POSY 232 
void LCD_ShowString(uint8_t x,uint16_t y,uint8_t *p)
{         
    while(*p!='\0')
    {       
        if(x>MAX_CHAR_POSX){x=0;y+=16;}
        if(y>MAX_CHAR_POSY){y=x=0;LCD_Clear(WHITE);}
        LCD_ShowChar(x,y,*p,16,0);
        x+=8;
        p++;
    }  
}

void LCD_ShowChar(uint8_t x,uint16_t y,uint8_t chars,uint8_t size,uint8_t mode)
{
	uint8_t temp;
    uint8_t pos,t;      
    if(x>MAX_CHAR_POSX||y>MAX_CHAR_POSY) return;	    
											
	LCD_SetDisplayWindow(x,y,(size/2-1),size-1);  

	LCD_WriteRAM_Prepare();           
	if(!mode) 						
	{
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=ASCII_1206[chars-0x20][pos];
			else temp=ASCII_1608[chars-0x20][pos];		 
			for(t=0;t<size/2;t++)
		    {                 	 			
		        if((temp<<t)&0x80)						
				{
					Write_Dat(RED);
				}
				else 
				{
					Write_Dat(WHITE);	        
		        }
		    }
		}	
	}
	else
	{
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=ASCII_1206[chars-0x20][pos];	
			else temp=ASCII_1608[chars-0x20][pos];		 	
			for(t=0;t<size/2;t++)
		    {                 
		        if((temp<<t)&0x80)LCD_DrawPoint(x+t,y+pos);
		    }
		}
	}	    	
}

void LCD_Clear(uint16_t Color)
{
	uint32_t index=0;      
	LCD_SetCursor(0x00,0x0000); //ÉèÖÃ¹â±E»ÖÃ 
	LCD_WriteRAM_Prepare();     //¿ªÊ¼Ð´ÈERAM	 	  
	for(index=0;index<DISPLAY_SIZE;index++)
	{
		Write_Dat(Color);    
	}
}

void WriteString(uint16_t x0, uint16_t y0,uint8_t *pcStr, uint16_t color)
{
	uint16_t usIndex;
    uint16_t usWidth = 0;
	uint16 sta_x = x0;
    FNT_GB16 *ptGb16 = 0;
    
    ptGb16 = (FNT_GB16 *)GBHZ_16;  
	while(1)
    {
        if(*pcStr == 0) 
		{
            break;                                     
        }      
        x0 = x0 + (usWidth);                           
        if(*pcStr > 0x80)                              
        {
		    if((x0 + 16) > LCD_W)                      
            {
			    x0 = 0;
                y0 = y0 + 16;                          
                if(y0 > LCD_H)                         
                {
				    y0 = 0;
                }
            }
            usIndex = findHzIndex(pcStr);
            usWidth = WriteOneHzChar((uint8_t *)&(ptGb16[usIndex].Msk[0]), x0, y0, color);
                                                       
            pcStr += 2;
        }
		else 
		{                                               
            if (*pcStr == '\r')                         
            { 
			    x0 = sta_x;
                pcStr++;
                usWidth = 0;
                continue;
            } 
			else if (*pcStr == '\n')                    
            {
			    x0 = sta_x;
			    y0 = y0 + 16;                           
                if(y0 > LCD_H)                          
                {
				    y0 = 0;
                }
                pcStr++;
                usWidth = 0;
                continue;
            } 
			else 
			{
                if((x0 + 8) > LCD_W)                     
                {
				    x0 = 0;
                    y0 = y0 + 16;                        
                    if(y0 > LCD_H)                       
                    { 
					    y0 = 0;
                    }
                }
				usWidth = WriteOneASCII((uint8_t *)(g_font_8x16+ (*pcStr * 0x10)), x0, y0, color);
                pcStr += 1;
            }
		}
	}												  	  
}

uint16_t WriteOneHzChar(uint8_t *pucMsk,
                               uint16_t x0,
                               uint16_t y0,
                               uint16_t color)
{
    uint16_t i,j;
    uint16_t mod[16];                                      
    uint16_t *pusMsk;                                      
    uint16_t y;
    
    pusMsk = (uint16_t *)pucMsk;
    for(i=0; i<16; i++)                                    
    {
        mod[i] = *pusMsk++;                                
        mod[i] = ((mod[i] & 0xff00) >> 8) | ((mod[i] & 0x00ff) << 8);
                                                           
    }
    y = y0;
    for(i=0; i<16; i++)                                    
    { 
	    #ifdef __DISPLAY_BUFFER                            
        for(j=0; j<16; j++)                                
        {
		    if((mod[i] << j)& 0x8000)                      
            {
			    DispBuf[320*(y0+i) + x0+j] = color;
            }
        }
        #else                                              
        
		LCD_SetCursor(x0, y);                              
		LCD_WriteRAM_Prepare();        					     
        for(j=0; j<16; j++)                               
        {
		    if((mod[i] << j) & 0x8000)                     
            { 
			    Write_Dat(color);
            } 
        }
        y++;
        #endif
    }
    return (16);                                          
}

uint16_t WriteOneASCII(uint8_t *pucMsk,
                              uint16_t x0,
                              uint16_t y0,
                              uint16_t color)
{
    uint16_t i,j;
    uint16_t y;
    uint8_t ucChar;
	uint8 bit = 0;
    
    y = y0;
    for(i=0; i<16; i++) {                                 
        ucChar = *pucMsk++;
        #ifdef __DISPLAY_BUFFER                           
        for(j=0; j<8; j++) {                              
            if((ucChar << j)& 0x80) {                     
                DispBuf[320*(y0+i) + x0+j] = color;
            }
        }
        #else                                             
        bit = 0;
        //LCD_SetCursor(x0, y);                           
		LCD_WriteRAM_Prepare();        					  
        for(j=0; j<8; j++) {                              
            if((ucChar << j) & 0x80) { 
				LCD_DrawPointColor(x0 + j, y, color);	
				if(((ucChar << (j +1)) & 0x80) == 0 && j < 7) {
					if(color != 0) LCD_DrawPointColor(x0 + j + 1, y+1, 0);		//shadow
					bit = 0;
				}
			} else {
				bit = 0;
            }
        }
        y++;
        #endif
    }
    return (8);                                     
}

uint32_t Num_power(uint8_t m,uint8_t n)
{
	uint32 result=1;	 
	while(n--)result*=m;    
	return result;
}

void LCD_ShowNum(uint8_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/Num_power(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+(size/2)*t,y,' ',size,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,0); 
	}
}

void LCD_WriteBMP(uint8_t Xpos, uint16_t Ypos, uint8_t Height, uint16_t Width, uint8_t *bitmap)
{
  	uint32_t index;
  	uint32_t size = Height * Width;
  	uint16_t *bitmap_ptr = (uint16_t *)bitmap;

  	LCD_SetDisplayWindow(Xpos, Ypos, Width-1, Height-1);

  	LCD_WriteRAM_Prepare();

  	for(index = 0; index < size; index++)
  	{
    	Write_Dat(*bitmap_ptr++);
  	}
	LCD_SetDisplayWindow(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
}

void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 

	delta_x=x2-x1; 				 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; 		
	else if(delta_x==0)incx=0;	
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;	
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )	
	{  
		LCD_SetCursor(uRow,uCol);
		Write_Cmd(R34);
		Write_Dat(color); 	
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}

void Draw_Circle(uint8_t x0,uint16_t y0,uint8_t r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a);             //3           
		LCD_DrawPoint(x0+b,y0-a);             //0           
		LCD_DrawPoint(x0-a,y0+b);             //1       
		LCD_DrawPoint(x0-b,y0-a);             //7           
		LCD_DrawPoint(x0-a,y0-b);             //2             
		LCD_DrawPoint(x0+b,y0+a);             //4               
		LCD_DrawPoint(x0+a,y0-b);             //5
		LCD_DrawPoint(x0+a,y0+b);             //6 
		LCD_DrawPoint(x0-b,y0+a);             
		a++;    
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 
		LCD_DrawPoint(x0+a,y0+b);
	}
} 

void LCD_Fill(uint8_t xsta,uint16_t ysta,uint8_t xend,uint16_t yend,uint16_t color)
{                    
    uint32_t n, i, j;									
	LCD_WriteReg(R80, xsta); 
	LCD_WriteReg(R81, xend); 
	LCD_WriteReg(R82, ysta); 
	LCD_WriteReg(R83, yend); 
	LCD_SetCursor(xsta,ysta);
	LCD_WriteRAM_Prepare();   	   	   
	n=(uint32)(yend-ysta+1)*(xend-xsta+1);  
	for(j =0;j<(yend-ysta+1);j++) {
		for(i =0;i<(xend-xsta+1);i++) {
			Write_Dat(color);
		}
		__nop();
	} 
}

#if SHARD_RTOS_ENABLED == 0
void LCD_Delay (uint32_t nCount)
{
	__IO uint16_t i;	 	
	for (i=0;i<nCount*100;i++);
}
#endif


void LCD_Switch_Orientation(uint8_t mode) {
	uint8 buffer[1]; 
	mode = mode & 0x03;
	if(mode == 0) {
		buffer[0] = 0xE8;
	} else if(mode == 1)  {
		buffer[0] = 0x48;
	}  else if(mode == 2)  {
		buffer[0] = 0x38;
	}  else if(mode == 3)  {
		buffer[0] = 0x88;
	} 
	LCD_WriteRegBurst(ILI9341_MADCTL, buffer, 1); 
}


//display apis definition
void ili934x_wake(gui_handle_p ctx) {
	LCD_DisplayOn();
}

void ili934x_sleep(gui_handle_p ctx) {
	LCD_DisplayOff();
}

void ili934x_switch_orientation(gui_handle_p display, uint8 mode) {
	LCD_Switch_Orientation(mode);
}

static void ili934x_set_backlight(struct gui_handle * handle, uint8 mode) {
	if(mode) handle->status |= UI_STATUS_ACTIVE;		//display in active state (2017.03.04)
	else handle->status &= ~UI_STATUS_ACTIVE;			//display in sleep state
	LCD_SetBacklight(mode, handle->brightness);
}

static uint32 ili934x_area(struct gui_handle * handle, uint16 x, uint16 y, uint16 w, uint16 h) {
	if(h == 0) return 0;
	if(w == 0) return 0;
	LCD_SetDisplayWindow(x, y, w - 1, h - 1);
	LCD_SetCursor(x, y);
	return (w) * (h);
}

static void ili934x_begin(struct gui_handle * handle) {
	LCD_WriteRAM_Prepare();
}

static void ili934x_write(struct gui_handle * handle, uint32 color) {
	Write_Dat(color);
}

static void ili934x_present(struct gui_handle * handle) {
	//no action for non-framebuffer
}

static uint32 g_fb_col_start = 0;
static uint32 g_fb_row_start = 0;
static uint32 g_fb_col_end = 0;
static uint32 g_fb_row_end = 0;
static uint32 g_fb_col_cur = 0;
static uint32 g_fb_row_cur = 0;
static uint32 ili934x_area_fb(struct gui_handle * handle, uint16 x, uint16 y, uint16 w, uint16 h) {
	g_fb_col_start = x;
	g_fb_row_start = y;
	g_fb_col_end = x + (w-1);
	g_fb_row_end = y + (h-1);
	g_fb_col_cur = g_fb_col_start;
	g_fb_row_cur = g_fb_row_start;
	return (w) * (h);
}

static void ili934x_begin_fb(struct gui_handle * handle) {
	//LCD_WriteRAM_Prepare();
	g_fb_col_cur = g_fb_col_start;
	g_fb_row_cur = g_fb_row_start;
}

static void ili934x_write_fb(struct gui_handle * handle, uint32 color) {
	//Write_Dat(color);
	uint32 index;
	if(handle->fb_ptr == NULL) return;
	index = (g_fb_row_cur * handle->width) + g_fb_col_cur;
	if((index * 2) < handle->fb_size) {
		((uint16 *)handle->fb_ptr)[index] = color;
	}
	g_fb_col_cur++;
	if(g_fb_col_cur > g_fb_col_end) { 
		g_fb_col_cur = g_fb_col_start;
		g_fb_row_cur++;
		if(g_fb_row_cur > g_fb_row_end) {
			g_fb_row_cur = g_fb_row_start;
		}
	}
}

void ili934x_present_fb(struct gui_handle * handle) {
	//start copying pixel data from framebuffer to device
	uint32 i;
	uint16 * fb;
	uint32 size = ili934x_area(handle, 0, 0, handle->width, handle->height);
	fb = handle->fb_ptr;
	ili934x_begin(handle);
	HAL_DMA_Abort(&dma1);
	HAL_DMA_Start(&dma1, (uint32_t)handle->fb_ptr, 0x60800000, handle->fb_size / 4);
}


#define SDRAM_BASE 0xC0000000
#define SDRAM_SIZE 8 * 1024 * 1024

//uint8 _dfsbuffer[480*272*3] __attribute__((at(0xC0000000)));

uint8 ili934x_init(gui_handle_p handle) {
	uint8 esid[SHARD_ESID_SIZE];
	uint16 dev_id;
	uint8 ret = -1;
	if_flash_get_esid(esid, SHARD_ESID_SIZE);
	dev_id = ((esid[2] << 8) | esid[3]) & 0xFFF;
	switch(dev_id) {
		case STM32F765_SERIES:				//STM32F765  (CARDINAL)
			LCD_DMA_Init((DMA2D_HandleTypeDef *)handle);
			LCD_DisplayOff();
			LCD_Init();
			handle->width = DISPLAY_WIDTH;
			handle->height = DISPLAY_HEIGHT;
			handle->fps = 14;					//set display FPS
			handle->init = ili934x_init;
			handle->wake = ili934x_wake;
			handle->sleep = ili934x_sleep;
			handle->switch_orientation = ili934x_switch_orientation;
			handle->set_backlight = ili934x_set_backlight;
			//use framebuffer
			handle->fb_pxfmt = UI_RGB565;
			handle->fb_size = DISPLAY_WIDTH * DISPLAY_HEIGHT * 2;
			handle->fb_ptr = os_alloc(DISPLAY_WIDTH * DISPLAY_HEIGHT * 2);
			handle->set_area = ili934x_area_fb;
			handle->begin_burst = ili934x_begin_fb;
			handle->put_pixel = ili934x_write_fb;
			handle->present = ili934x_present_fb;
			handle->ac_support = UI_ANIM_ALPHA_BLENDING | UI_ANIM_SLIDE_LEFT | UI_ANIM_SLIDE_RIGHT;
			ret = 0;
			break;
		//case 0x419:				//STM32F42x
		case STM32F746_SERIES:				//STM32F746		//discovery board
		//default: 
			//LCD direct access no framebuffer
			//LCD_DMA_Init((DMA2D_HandleTypeDef *)handle);
			//LCD_DisplayOff();
			//LCD_Init();

		
		
			ret = -1;
			break;
			
		case STM32F407_SERIES:				//STM32F407
			LCD_DMA_Init((DMA2D_HandleTypeDef *)handle);
			LCD_DisplayOff();
			LCD_Init();
			handle->width = DISPLAY_WIDTH;
			handle->height = DISPLAY_HEIGHT;
			handle->fps = 14;						//set display FPS
			handle->init = ili934x_init;
			handle->wake = ili934x_wake;
			handle->sleep = ili934x_sleep;
			handle->switch_orientation = ili934x_switch_orientation;
			handle->set_backlight = ili934x_set_backlight;
			//use direct access
			handle->set_area = ili934x_area;
			handle->begin_burst = ili934x_begin;
			handle->put_pixel = ili934x_write;
			handle->present = ili934x_present;
			handle->ac_support = UI_ANIM_NONE;		//doesn't support animation
			ret = 0;
		break;
	}
	return ret;
}
/*********************************************************************************************************
** End of File
*********************************************************************************************************/

#endif
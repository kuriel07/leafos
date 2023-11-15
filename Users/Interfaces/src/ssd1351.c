/* Includes ------------------------------------------------------------------*/
#include "defs.h"
#include "config.h"
#include <stdint.h>
#include "..\inc\if_apis.h"

//FMC configuration for LCD (bank 1)
//#define LCD_BUFFER		(*((volatile short *)0x60800000))
//#define LCD_REG			(*((volatile short *)0x60000000))

static DMA_HandleTypeDef dma1;
static SPI_HandleTypeDef SSD1351_SPICtx;

/*
	(##) DMA Configuration if you need to use DMA process
	(+++) Declare a DMA_HandleTypeDef handle structure for the transmit or receive stream
	(+++) Enable the DMAx clock
	(+++) Configure the DMA handle parameters
	(+++) Configure the DMA Tx or Rx stream
	(+++) Associate the initialized hdma_tx handle to the hspi DMA Tx or Rx handle
	(+++) Configure the priority and enable the NVIC for the transfer complete interrupt on the DMA Tx or Rx stream
*/

static void HAL_DMA_TransferCompleted(DMA_HandleTypeDef *hdma) {
	uint32_t err = HAL_DMA_GetError(hdma);
	if(err == 0) {
		err = 0;
	}
}

static void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef* hdma2d)
{
  if(hdma2d->Instance==DMA2D)
  {
    __HAL_RCC_DMA2D_CLK_ENABLE();
  }

}

static void HAL_DMA2D_MspDeInit(DMA2D_HandleTypeDef* hdma2d)
{
  if(hdma2d->Instance==DMA2D)
  {
    __HAL_RCC_DMA2D_CLK_DISABLE();
  }

}

#define SSD1351_RST0	GPIO_ResetBits(GPIOA, GPIO_PIN_0)
#define SSD1351_RST1	GPIO_SetBits(GPIOA, GPIO_PIN_0)
#define SSD1351_CS0		GPIO_ResetBits(GPIOA, GPIO_PIN_4)
#define SSD1351_CS1		GPIO_SetBits(GPIOA, GPIO_PIN_4)
#define SSD1351_RS0		GPIO_ResetBits(GPIOA, GPIO_PIN_2)
#define SSD1351_RS1		GPIO_SetBits(GPIOA, GPIO_PIN_2)

spi_context soft_spi_ctx;

void SSD1351_ComWrite(uint8 cmd)
{
	//RD = 1;
	SSD1351_CS0;
	SSD1351_RS0;// = 0;
	//CS = 0;
	//RW = 0;
	//P1 = cmd;
	HAL_SPI_Transmit(&SSD1351_SPICtx, &cmd, 1, 100);
	//if_spi_write(&soft_spi_ctx, cmd);
	//RW = 1;
	//CS = 1;
	//SSD1351_RS1;
	SSD1351_CS1;
}

void SSD1351_DataWrite(uint8 dat)
{
	//RD =1;
	SSD1351_RS1;// = 1;
	SSD1351_CS0;
	//RW=0;
	//P1 = dat;
	HAL_SPI_Transmit(&SSD1351_SPICtx, &dat, 1, 100);
	//if_spi_write(&soft_spi_ctx, dat);
	//RW=1;
	SSD1351_CS1;
	//SSD1351_RS0;
}

void SSD1351_SendDATA(uint16 dat)
{
	SSD1351_DataWrite((uint8)(dat >> 8));
	SSD1351_DataWrite(dat & 0xFF);
}

void SSD1351_LCD_WriteDataBurst(uint16_t addr, void * data, uint8 size) {
	uint8 i;
	for(i=0;i<size;i++) {
		SSD1351_DataWrite(((uint8 *)data)[i]);
	}
}

void SSD1351_LCD_WriteRegBurst(uint16_t addr, void * data, uint8 size) {
	SSD1351_ComWrite(addr);
	SSD1351_LCD_WriteDataBurst(0, data, size);
}

void SSD1351_LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_Dat)
{
	SSD1351_ComWrite(LCD_Reg);
	SSD1351_SendDATA(LCD_Dat);
}

void SSD1351_LCD_SetBacklight(uint8_t mode, uint8 intensity) {
	if(mode) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);		//activate backlight non-pwm
	} else {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);		//de-activate backlight non-pwm
	}
}

void SSD1351_LCD_Select() {
	SSD1351_CS0;
}

void SSD1351_LCD_DeSelect() {
	SSD1351_CS1;
}

void SSD1351_LCD_Configuration()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//init backlight	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	//configure SPI bus
#if 1
	GPIO_InitStructure.Pin = GPIO_PIN_7 | GPIO_PIN_5 | GPIO_PIN_4;
  	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;			//peripheral (output function push pull)	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Alternate = GPIO_AF5_SPI1;
  	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
#else
	GPIO_InitStructure.Pin = GPIO_PIN_7 | GPIO_PIN_5 | GPIO_PIN_4;
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (output function push pull)	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	//GPIO_InitStructure.Alternate = GPIO_AF5_SPI1;
  	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
	soft_spi_ctx.cs = GPIO_PIN_4;
	soft_spi_ctx.mosi = GPIO_PIN_7;
	soft_spi_ctx.sck = GPIO_PIN_5;
	soft_spi_ctx.port = GPIOA;
	soft_spi_ctx.t_state = SPI_STATE_MSB_FIRST;
	soft_spi_ctx.deselect = SSD1351_LCD_DeSelect;
	soft_spi_ctx.select = SSD1351_LCD_Select;
#endif
	//configure control pin (RS, RST)
	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_2;
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (output function push pull)	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//configure display backlight pin
	GPIO_InitStructure.Pin = GPIO_PIN_0;
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	__HAL_RCC_SPI1_CLK_ENABLE();
	SSD1351_SPICtx.Instance = SPI1;
	//SSD1351_SPICtx.hdmatx = &dma1;
	SSD1351_SPICtx.Init.CLKPhase = SPI_PHASE_1EDGE;
	SSD1351_SPICtx.Init.CLKPolarity = SPI_POLARITY_LOW;
	SSD1351_SPICtx.Init.Direction = SPI_DIRECTION_2LINES;
	SSD1351_SPICtx.Init.FirstBit = SPI_FIRSTBIT_MSB;
	SSD1351_SPICtx.Init.DataSize = SPI_DATASIZE_8BIT;
	SSD1351_SPICtx.Init.Mode = SPI_MODE_MASTER;
	SSD1351_SPICtx.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	SSD1351_SPICtx.Init.NSS = SPI_NSS_HARD_OUTPUT;
	//SSD1351_SPICtx.Init.
	//SPI_InitStructure.Init.
	
	HAL_SPI_Init(&SSD1351_SPICtx);
	
    __HAL_LINKDMA(&SSD1351_SPICtx, hdmatx, dma1);
	//SSD1351_LCD_SetBacklight(0, 50);		//default duty cycle = 50%
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);		//activate backlight non-pwm
}

static void SSD1351_LCD_DMA_Completed(DMA_HandleTypeDef *hdma) {
	uint32_t err = HAL_DMA_GetError(hdma);
	if(err == 0) {
		err = 0;
	}
}

void DMA2_Stream3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream3_IRQn 0 */

  /* USER CODE END DMA2_Stream3_IRQn 0 */
  HAL_DMA_IRQHandler(&dma1);
  /* USER CODE BEGIN DMA2_Stream3_IRQn 1 */

  /* USER CODE END DMA2_Stream3_IRQn 1 */
}

void SSD1351_LCD_DMA_Init(void * p) {
	DMA2D_HandleTypeDef * handle = (DMA2D_HandleTypeDef *)p;
	//FMC_NORSRAM_TimingTypeDef Timing;
	//enable DMA2D
    __HAL_RCC_DMA2D_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
	
	dma1.Instance = DMA2_Stream3;
	dma1.Init.Channel = DMA_CHANNEL_3;
	dma1.Init.Direction = DMA_MEMORY_TO_PERIPH;//DMA_MEMORY_TO_PERIPH;
	dma1.Init.Mode = DMA_NORMAL;
	dma1.Init.Priority = DMA_PRIORITY_MEDIUM;
	dma1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	dma1.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
	  //source
	dma1.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	dma1.Init.PeriphInc = DMA_PINC_DISABLE;
	dma1.Init.PeriphBurst = DMA_PBURST_SINGLE;
	  //destination
	dma1.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	dma1.Init.MemInc = DMA_MINC_ENABLE;
	dma1.Init.MemBurst = DMA_MBURST_SINGLE;
	
	//dma1.StreamBaseAddress = SPI1->DR;
	//dma1.StreamIndex = 0;
	  
	HAL_DMA_Init(&dma1);
	HAL_DMA_IRQHandler(&dma1);
	HAL_DMA_RegisterCallback(&dma1, HAL_DMA_XFER_CPLT_CB_ID, SSD1351_LCD_DMA_Completed);
	HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
	
#if 0
	//DMA2D configuration (not available on F413)
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
#endif
	/*
	hal_status = HAL_DMA2D_BlendingStart_IT(handle,
                                          (uint32_t)&RGB565_240x130_1,
                                          (uint32_t)&RGB565_240x130_2,
                                          (uint32_t)&aBlendedImage,
                                          LAYER_SIZE_X,
                                          LAYER_SIZE_Y);
	*/
}

void SSD1351_LCD_Init(void)
{
	static uint16_t DeviceCode;
	uint8 ret;
	SSD1351_LCD_Configuration(); 
	//de-select device
	SSD1351_CS1;
	os_wait(10);
	//hardware reset
	SSD1351_RST0;
	os_wait(100);
	SSD1351_RST1;
	os_wait(50);

	SSD1351_ComWrite(0xfd); // Set Command Lock
	SSD1351_DataWrite(0xb1);
	SSD1351_ComWrite(0xae); // Display off
	SSD1351_ComWrite(0x15); //set column
	SSD1351_DataWrite(0x00);
	SSD1351_DataWrite(0x7f);
	SSD1351_ComWrite(0x75); //set row
	SSD1351_DataWrite(0x00);
	SSD1351_DataWrite(0x7f);
	SSD1351_ComWrite(0xa0); // Set Re-map / Color Depth 
	SSD1351_DataWrite(0x74);				//   Color sequence is swapped: C .. B .. A
	SSD1351_ComWrite(0xa2); // Set display offset
	SSD1351_DataWrite(0x00);
	SSD1351_ComWrite(0xa6); // Normal display
	SSD1351_ComWrite(0xab); // Set Function selection
	SSD1351_DataWrite(0x01);
	//SSD1351_ComWrite(0xaf); // Set Sleep mode
	SSD1351_ComWrite(0xb1); // Set pre & dis_charge
	SSD1351_DataWrite(0x32);
	SSD1351_ComWrite(0xb3); // clock & frequency
	SSD1351_DataWrite(0xf1);
	SSD1351_ComWrite(0xb4); // Set Segment LOW Voltage
	SSD1351_DataWrite(0xa0);
	SSD1351_DataWrite(0xb5);
	SSD1351_DataWrite(0x55);
	SSD1351_ComWrite(0xb5); // Set GPIO
	SSD1351_DataWrite(0x0A);
	SSD1351_ComWrite(0xb6); // Set Second Pre-charge Period
	SSD1351_DataWrite(0x01);
	/*
	SSD1351_ComWrite(0xb8); //Set Gray Table
	SSD1351_DataWrite(0); //0
	SSD1351_DataWrite(2); //1
	SSD1351_DataWrite(3); //2
	SSD1351_DataWrite(4); //3
	SSD1351_DataWrite(5); //4
	SSD1351_DataWrite(6); //5
	SSD1351_DataWrite(7); //6
	SSD1351_DataWrite(8); //7
	SSD1351_DataWrite(9); //8
	SSD1351_DataWrite(10); //9
	SSD1351_DataWrite(11); //10
	SSD1351_DataWrite(12); //11
	SSD1351_DataWrite(13); //12
	SSD1351_DataWrite(14); //13
	SSD1351_DataWrite(15); //14
	SSD1351_DataWrite(16); //15
	SSD1351_DataWrite(17); //16
	SSD1351_DataWrite(18); //17
	SSD1351_DataWrite(19); //18
	SSD1351_DataWrite(21); //19
	SSD1351_DataWrite(23); //20
	SSD1351_DataWrite(25); //21
	SSD1351_DataWrite(27); //22
	SSD1351_DataWrite(29); //23
	SSD1351_DataWrite(31); //24
	SSD1351_DataWrite(33); //25
	SSD1351_DataWrite(35); //26
	SSD1351_DataWrite(37); //27
	SSD1351_DataWrite(39); //28
	SSD1351_DataWrite(42); //29
	SSD1351_DataWrite(45); //30
	SSD1351_DataWrite(48); //31
	SSD1351_DataWrite(51); //32
	SSD1351_DataWrite(54); //33
	SSD1351_DataWrite(57); //34
	SSD1351_DataWrite(60); //35
	SSD1351_DataWrite(63); //36
	SSD1351_DataWrite(66); //37
	SSD1351_DataWrite(69); //38
	SSD1351_DataWrite(72); //39
	SSD1351_DataWrite(76); //40
	SSD1351_DataWrite(80); //41
	SSD1351_DataWrite(84); //42
	SSD1351_DataWrite(88); //43
	SSD1351_DataWrite(92); //44
	SSD1351_DataWrite(96); //45
	SSD1351_DataWrite(100); //46
	SSD1351_DataWrite(104); //47
	SSD1351_DataWrite(108); //48
	SSD1351_DataWrite(112); //49
	SSD1351_DataWrite(116); //50
	SSD1351_DataWrite(120); //51
	SSD1351_DataWrite(125); //52
	SSD1351_DataWrite(130); //53
	SSD1351_DataWrite(135); //54
	SSD1351_DataWrite(140); //55
	SSD1351_DataWrite(145); //56
	SSD1351_DataWrite(150); //57
	SSD1351_DataWrite(155); //58
	SSD1351_DataWrite(160); //59
	SSD1351_DataWrite(165); //60
	SSD1351_DataWrite(170); //61
	SSD1351_DataWrite(175); //62
	SSD1351_DataWrite(180); //63
	*/
	//SSD1351_ComWrite(0xbb); // Set pre-charge voltage of color A B C
	//SSD1351_DataWrite(0x17);
	SSD1351_ComWrite(0xbe); // Set VcomH
	SSD1351_DataWrite(0x05);
	//SSD1351_ComWrite(0xc1); // Set contrast current for A B C
	//SSD1351_DataWrite(0x88);
	//SSD1351_DataWrite(0x70);
	//SSD1351_DataWrite(0x88);
	//SSD1351_ComWrite(0xc7); // Set master contrast
	//SSD1351_DataWrite(0x0f);
	//SSD1351_ComWrite(0xca); // Duty
	//SSD1351_DataWrite(0x7f);
	SSD1351_ComWrite(0xaf); // Display on
	
}

void SSD1351_LCD_DisplayOn() {
	SSD1351_ComWrite(0xaf); // Display on
}

void SSD1351_LCD_DisplayOff() {
	SSD1351_ComWrite(0xae); // Display off
}

void SSD1351_LCD_WriteRAM_Prepare()
{
	//set RS to data
	SSD1351_ComWrite(0x5c);  //Enable MCU to write Data into RAM
	SSD1351_RS1;
}

void SSD1351_LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
	
} 

void SSD1351_LCD_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width)
{
	SSD1351_ComWrite(0x15); //set column
	SSD1351_DataWrite(Width >> 8);
	SSD1351_DataWrite(Width & 0xFF);
	SSD1351_ComWrite(0x75); //set row
	SSD1351_DataWrite(Height >> 8);
	SSD1351_DataWrite(Height & 0xFF);
	SSD1351_ComWrite(0x5c);  //Enable MCU to write Data into RAM
}

void SSD1351_LCD_Clear(uint16_t Color)
{
	uint32_t index=0;      
	SSD1351_LCD_SetCursor(0x00,0x0000); 
	SSD1351_LCD_WriteRAM_Prepare();    	 	  
	for(index=0;index<DISPLAY_SIZE;index++)
	{
		SSD1351_SendDATA(Color);    
	}
}

void SSD1351_LCD_Fill(uint8_t xsta,uint16_t ysta,uint8_t xend,uint16_t yend,uint16_t color)
{                    
    uint32_t n, i, j;		 
	SSD1351_LCD_SetDisplayWindow(xsta, ysta, xend, yend);
	SSD1351_LCD_SetCursor(xsta,ysta);
	SSD1351_LCD_WriteRAM_Prepare(); 
  	color = end_swap16(color);
	n=(uint32)(yend-ysta+1)*(xend-xsta+1);  
	for(j =0;j<(yend-ysta+1);j++) {
		for(i =0;i<(xend-xsta+1);i++) {
			SSD1351_SendDATA(color);
		}
		__nop();
	} 
}

void SSD1351_LCD_Switch_Orientation(uint8_t mode) {
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
	//LCD_WriteRegBurst(ILI9341_MADCTL, buffer, 1); 
}


//display apis definition
void ssd1351_wake(gui_handle_p ctx) {
	SSD1351_LCD_DisplayOn();
}

void ssd1351_sleep(gui_handle_p ctx) {
	SSD1351_LCD_DisplayOff();
}

void ssd1351_switch_orientation(gui_handle_p display, uint8 mode) {
	SSD1351_LCD_Switch_Orientation(mode);
}

static void ssd1351_set_backlight(struct gui_handle * handle, uint8 mode) {
	if(mode) handle->status |= UI_STATUS_ACTIVE;		//display in active state (2017.03.04)
	else handle->status &= ~UI_STATUS_ACTIVE;			//display in sleep state
	//SSD1351_LCD_SetBacklight(mode, handle->brightness);
}

static uint32 ssd1351_area(struct gui_handle * handle, uint16 x, uint16 y, uint16 w, uint16 h) {
	if(h == 0) return 0;
	if(w == 0) return 0;
	SSD1351_LCD_SetDisplayWindow(x, y, w - 1, h - 1);
	SSD1351_LCD_SetCursor(x, y);
	return (w) * (h);
}

static void ssd1351_begin(struct gui_handle * handle) {
	SSD1351_LCD_WriteRAM_Prepare();
}

static void ssd1351_write(struct gui_handle * handle, uint32 color) {
	//Write_Dat(color);
	uint16 w = color;
	SSD1351_SendDATA(end_swap16(w));
}

static void ssd1351_present(struct gui_handle * handle) {
	//no action for non-framebuffer
}

static uint32 g_fb_col_start = 0;
static uint32 g_fb_row_start = 0;
static uint32 g_fb_col_end = 0;
static uint32 g_fb_row_end = 0;
static uint32 g_fb_col_cur = 0;
static uint32 g_fb_row_cur = 0;
static uint32 ssd1351_area_fb(struct gui_handle * handle, uint16 x, uint16 y, uint16 w, uint16 h) {
	g_fb_col_start = x;
	g_fb_row_start = y;
	g_fb_col_end = x + (w-1);
	g_fb_row_end = y + (h-1);
	g_fb_col_cur = g_fb_col_start;
	g_fb_row_cur = g_fb_row_start;
	return (w) * (h);
}

static void ssd1351_begin_fb(struct gui_handle * handle) {
	g_fb_col_cur = g_fb_col_start;
	g_fb_row_cur = g_fb_row_start;
}

static void ssd1351_write_fb(struct gui_handle * handle, uint32 color) {
	uint32 index;
	if(handle->fb_ptr == NULL) return;
	index = (g_fb_row_cur * handle->width) + g_fb_col_cur;
	if((index * 2) < handle->fb_size) {
		((uint16 *)handle->fb_ptr)[index] = end_swap16(color);
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

void ssd1351_present_fb(struct gui_handle * handle) {
	//start copying pixel data from framebuffer to device
	uint32 i,j;
	//uint16 * fb;
	uint32 size;
	SSD1351_RS0;
	HAL_DMA_Abort(&dma1);
	size = ssd1351_area(handle, 0, 0, handle->width, handle->height);
	//fb = handle->fb_ptr;
	ssd1351_begin(handle);
	HAL_SPI_Transmit_DMA(&SSD1351_SPICtx, handle->fb_ptr, size * 2);
	//HAL_SPI_Transmit(&SSD1351_SPICtx, handle->fb_ptr, size * 2, 500);
}

uint8 ssd1351_init(gui_handle_p handle) {
	uint8 esid[SHARD_ESID_SIZE];
	uint16 dev_id;
	uint8 ret = -1;
	if_flash_get_esid(esid, SHARD_ESID_SIZE);
	dev_id = ((esid[2] << 8) | esid[3]) & 0xFFF;
	switch(dev_id) {
		case STM32F413_SERIES:				//STM32F413 (ARGOS)
			SSD1351_LCD_DMA_Init((DMA2D_HandleTypeDef *)handle);
			SSD1351_LCD_DisplayOff();
			SSD1351_LCD_Init();		
			//SSD1351_LCD_Fill(0, 0, 127,127, 0xFFFF);
			handle->width = 128;			//128 pixel width
			handle->height = 128;			//128 pixel height
			handle->fps = 8;				//set display FPS
			handle->init = ssd1351_init;
			handle->wake = ssd1351_wake;
			handle->sleep = ssd1351_sleep;
			handle->switch_orientation = ssd1351_switch_orientation;
			handle->set_backlight = ssd1351_set_backlight;
			handle->fb_pxfmt = UI_RGB565;
			handle->fb_size = 128 * 128 * 2;
			handle->fb_ptr = os_alloc(handle->fb_size);
			handle->set_area = ssd1351_area_fb;
			handle->begin_burst = ssd1351_begin_fb;
			handle->put_pixel = ssd1351_write_fb;
			handle->present = ssd1351_present_fb;
			handle->ac_support = UI_ANIM_SLIDE_LEFT | UI_ANIM_SLIDE_RIGHT;
			ret = 0;
			break;
	}
	return ret;
}

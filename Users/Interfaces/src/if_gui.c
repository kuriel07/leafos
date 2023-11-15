#ifdef WIN32
//#include "stdafx.h"
#endif
//#include <winscard.h>
//#include <memory.h>
#include <stdio.h>
//#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include "..\..\defs.h"
#include "..\..\config.h"
#include "..\inc\if_apis.h"
#include "..\inc\ili932x\ili932x.h"
#include "..\inc\ili932x\fonts.h"	 //◊÷ø‚Œƒº˛øÅE

#if 0

static uint32_t FMC_Initialized = 0;

static void HAL_FMC_MspInit(void){
  /* USER CODE BEGIN FMC_MspInit 0 */

  /* USER CODE END FMC_MspInit 0 */
  GPIO_InitTypeDef GPIO_InitStruct;
  if (FMC_Initialized) {
    return;
  }
  FMC_Initialized = 1;
  /* Peripheral clock enable */
  __HAL_RCC_FMC_CLK_ENABLE();
  
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
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_14 
                          |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4 
                          |GPIO_PIN_5|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN FMC_MspInit 1 */

  /* USER CODE END FMC_MspInit 1 */
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

  /* USER CODE BEGIN FMC_MspDeInit 1 */

  /* USER CODE END FMC_MspDeInit 1 */
}

void HAL_SRAM_MspDeInit(SRAM_HandleTypeDef* hsram){
  /* USER CODE BEGIN SRAM_MspDeInit 0 */

  /* USER CODE END SRAM_MspDeInit 0 */
  HAL_FMC_MspDeInit();
  /* USER CODE BEGIN SRAM_MspDeInit 1 */

  /* USER CODE END SRAM_MspDeInit 1 */
}

void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef* hdma2d)
{

  if(hdma2d->Instance==DMA2D)
  {
  /* USER CODE BEGIN DMA2D_MspInit 0 */

  /* USER CODE END DMA2D_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_DMA2D_CLK_ENABLE();
  /* USER CODE BEGIN DMA2D_MspInit 1 */

  /* USER CODE END DMA2D_MspInit 1 */
  }

}

void HAL_DMA2D_MspDeInit(DMA2D_HandleTypeDef* hdma2d)
{

  if(hdma2d->Instance==DMA2D)
  {
  /* USER CODE BEGIN DMA2D_MspDeInit 0 */

  /* USER CODE END DMA2D_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_DMA2D_CLK_DISABLE();
  /* USER CODE BEGIN DMA2D_MspDeInit 1 */

  /* USER CODE END DMA2D_MspDeInit 1 */
  }

}
#endif


static uint16_t if_gui_putc_16(struct gui_handle * handle, uint8_t *pucMsk,
                              uint16_t x0,
                              uint16_t y0,
                              uint32 color)
{
    uint16_t i,j;
    uint16_t y;
    uint8_t ucChar;
	uint8 bit = 0;
    y = y0;
    for(i=0; i<16; i++) {     
        ucChar = *pucMsk++;        
        bit = 0;
        for(j=0; j<8; j++) {                 
            if((ucChar << j) & 0x80) {   
				handle->fill_area(handle, color, handle->set_area(handle, x0 + j, y, 1, 1));
				if(((ucChar << (j +1)) & 0x80) == 0 && j < 7) {
					if((color & 0xFFFFFF) != 0) handle->fill_area(handle, 0, handle->set_area(handle, x0 + j + 1, y+1, 1, 1));
					bit = 0;
				}
			} else {
				bit = 0;
            }
        }
        y++;
    }
    return (8);   
}


static uint16_t if_gui_putc_8(struct gui_handle * handle, uint8_t *pucMsk,
                              uint16_t x0,
                              uint16_t y0,
                              uint32 color)
{
    uint16_t i,j;
    uint16_t y;
    uint8_t ucChar;
	uint8 bit = 0;
    y = y0;
    for(i=0; i<8; i++) {     
        ucChar = *pucMsk++;        
        bit = 0;
        for(j=0; j<8; j++) {                 
            if((ucChar << j) & 0x80) {   
				handle->fill_area(handle, color, handle->set_area(handle, x0 + j, y, 1, 1));
				if(((ucChar << (j +1)) & 0x80) == 0 && j < 7) {
					if((color & 0xFFFFFF) != 0) handle->fill_area(handle, 0, handle->set_area(handle, x0 + j + 1, y+1, 1, 1));
					bit = 0;
				}
			} else {
				bit = 0;
            }
        }
        y++;
    }
    return 6;   
}

static void if_gui_print(struct gui_handle * handle, uint8 size, uint16 x, uint16 y, uint8 * str, uint32 color) {
	uint16_t usIndex;
    uint16_t usWidth = 0;
	uint16 sta_x = x;
    uint16 x0 = x;
	uint16 y0 = y;
	uint8 * pcStr = str;
	while(1)
    {
        if(*pcStr == 0) 
		{
            break;                                     /* ◊÷∑˚¥ÆΩ· ÅE           */
        }      
        x0 = x0 + (usWidth);                           /* µ˜Ω⁄◊÷∑˚¥Æœ‘ æÀ…ΩÙ∂»         */
        if(*pcStr > 0x80)                              /* ≈–∂œŒ™∫∫◊÷                   */
        {
		    if((x0 + size) > handle->width)                      /* ºÅEÈ £”‡ø’º‰ «∑Ò◊„πª         */
            {
			    x0 = 0;
                y0 = y0 + size;                          /* ∏ƒ±‰œ‘ æ◊¯±ÅE                */
                if(y0 > handle->height) y0 = 0;
            }
            //usIndex = findHzIndex(pcStr);
            //usWidth = WriteOneHzChar((uint8_t *)&(ptGb16[usIndex].Msk[0]), x0, y0, color);
                                                       /* œ‘ æ◊÷∑ÅE                    */
            pcStr += 2;
        }
		else 
		{                                               /* ≈–∂œŒ™∑«∫∫◊÷                 */
            if (*pcStr == '\r')                         /* ªª––                         */
            { 
			    x0 = sta_x;
                pcStr++;
                usWidth = 0;
                continue;
            } 
			else if (*pcStr == '\n')                    /* ∂‘∆ÅEΩ∆µÅE                  */
            {
			    x0 = sta_x;
			    y0 = y0 + size;                           /* ∏ƒ±‰œ‘ æ◊¯±ÅE                */
                if(y0 > handle->height) y0 = 0;
                pcStr++;
                usWidth = 0;
                continue;
            } 
			else 
			{
                if((x0 + 8) > handle->width)                     /* ºÅEÈ £”‡ø’º‰ «∑Ò◊„πª         */
                {
				    x0 = 0;
                    y0 = y0 + size;                        /* ∏ƒ±‰œ‘ æ◊¯±ÅE                */
                    if(y0 > handle->height) y0 = 0;
                }
				switch(size) {
					case 16:
						//large font (8x16)
						usWidth = if_gui_putc_16(handle, (uint8_t *)(g_font_large + (*pcStr * 0x10)), x0, y0, color);
						break;
					case 8:
						//small font (6x8)
						usWidth = if_gui_putc_8(handle, (uint8_t *)(g_font_small2 + (*pcStr * 0x8)), x0, y0, color);
						break;
					default: break;
				} 
                pcStr += 1;
            }
		}
	}	
}

static void if_gui_line(struct gui_handle * handle, uint16 x1, uint16 y1, uint16 x2, uint16 y2, uint32 color) {
	LCD_DrawLine(x1, y1, x2, y2, IF_COLOR(color));
}

static void if_gui_fill(struct gui_handle * handle, uint32 color, uint32 size) {
	//LCD_Fill(xsta, ysta, xend, yend, IF_COLOR(color)); 
	LCD_WriteRAM_Prepare();  //ø™ º–¥»ÅERAM	 	   	   
	//n=(u32)(yend-ysta+1)*(xend-xsta+1);    
	while(size--){ Write_Dat(IF_COLOR(color)); }//œ‘ æÀ˘Ã˚œ‰µƒ—’…´. 
}

static void if_gui_bitmap(struct gui_handle * handle, uint32 forecolor, uint32 backcolor, uint8 * map, uint32 size) {
	//LCD_WriteBMP(xsta, ysta, xend - xsta, yend - ysta, uint8_t *bitmap);
	uint32_t index;
  	//uint32_t size = h * w;
	uint16_t fcolor = IF_COLOR(forecolor);
	uint16_t bcolor = IF_COLOR(backcolor);
  	//uint16_t *bitmap_ptr = (uint16_t *)bitmap;
  	LCD_WriteRAM_Prepare();

  	for(index = 0; index < size; index++)
  	{
    	//Write_Dat(*bitmap_ptr++);
		if(map[index>>3] & (1 << (index & 0x7))) Write_Dat(fcolor);
		else Write_Dat(bcolor);
  	}
}


static uint32 if_gui_area(struct gui_handle * handle, uint16 x, uint16 y, uint16 w, uint16 h) {
	//LCD_WriteReg(R80, x); //ÀÆ∆Ω∑ΩœÚGRAM∆ ºµÿ÷∑
	//LCD_WriteReg(R81, x+w); //ÀÆ∆Ω∑ΩœÚGRAMΩ· ¯µÿ÷∑
	//LCD_WriteReg(R82, y); //¥π÷±∑ΩœÚGRAM∆ ºµÿ÷∑
	//LCD_WriteReg(R83, y+h); //¥π÷±∑ΩœÚGRAMΩ· ¯µÿ÷∑	
	
  	//LCD_WriteReg(R80, y);	   	   	//ÀÆ∆Ω∑ΩœÚGRAM∆ ºµÿ÷∑
  	//LCD_WriteReg(R81, y+h); 	//ÀÆ∆Ω∑ΩœÚGRAMΩ· ¯µÿ÷∑ 
  	//LCD_WriteReg(R82, x);		  	//¥π÷±∑ΩœÚGRAM∆ ºµÿ÷∑
  	//LCD_WriteReg(R83, x+w);  	//¥π÷±∑ΩœÚGRAMΩ· ¯µÿ÷∑
	if(h == 0) return 0;
	if(w == 0) return 0;
	LCD_SetDisplayWindow(x, y, w - 1, h - 1);
  	//LCD_SetCursor(Xpos, Ypos);			//…Ë÷√π‚±ÅEª÷√
	LCD_SetCursor(x, y);//…Ë÷√π‚±ÅEª÷√ 
	return (w) * (h);
}

static void if_gui_begin(struct gui_handle * handle) {
	LCD_WriteRAM_Prepare();
}

static void if_gui_write(struct gui_handle * handle, uint32 color) {
	//Write_Dat((uint16)color);
	LCD_CS = 0;
	LCD_RS = 1;
	GPIOC->ODR = (GPIOC->ODR & 0xff00)|(color & 0x00ff);
	GPIOB->ODR = (GPIOB->ODR & 0x00ff)|(color & 0xff00);
	LCD_WR = 0;
	LCD_WR = 1;	
	LCD_CS = 1;
}

static void if_gui_rectangle(struct gui_handle * handle, uint16 x, uint16 y, uint16 w, uint16 h, uint32 color, uint8 radius) {
	uint16 x1 = x;
	uint16 x2 = x+w;
	uint16 y1 = y;
	uint16 y2 = y+h;
	//if(radius != 0) radius = 1;
	if(radius > 4) radius = 4;
	switch(radius) {
		case 0:
		case 1:
			if_gui_fill(handle, color, if_gui_area(handle, x1+radius, y1, (x2 - x1) - (radius << 1), 1));
			if_gui_fill(handle, color, if_gui_area(handle, x1, y1+radius, 1, (y2 - y1) - (radius << 1)));
			if_gui_fill(handle, color, if_gui_area(handle, x2, y1+radius, 1, (y2 - y1) - (radius << 1)));
			if_gui_fill(handle, color, if_gui_area(handle, x1+radius, y2-1, (x2 - x1) - (radius << 1), 1));
			break;
		case 2:
			if_gui_fill(handle, color, if_gui_area(handle, x1+radius, y1, (x2 - x1) - (radius << 1), 1));
			if_gui_fill(handle, color, if_gui_area(handle, x1, y1+radius, 1, (y2 - y1) - (radius << 1)));
			if_gui_fill(handle, color, if_gui_area(handle, x2, y1+radius, 1, (y2 - y1) - (radius << 1)));
			if_gui_fill(handle, color, if_gui_area(handle, x1+radius, y2, (x2 - x1) - (radius << 1), 1));
			//top left
			if_gui_fill(handle, color, if_gui_area(handle, x1 + (radius - 1), y1 + 1, 1, 1));		//horizontal
			if_gui_fill(handle, color, if_gui_area(handle, x1 + 1, y1 + (radius - 1), 1, 1));		//vertical
			//top right
			if_gui_fill(handle, color, if_gui_area(handle, x2 - (radius - 1), y1 + 1, 1, 1));		//horizontal
			if_gui_fill(handle, color, if_gui_area(handle, x2 - 1, y1 + (radius - 1), 1, 1));		//vertical
			//bottom left
			if_gui_fill(handle, color, if_gui_area(handle, x1 + (radius - 1), y2 - 1, 1, 1));
			if_gui_fill(handle, color, if_gui_area(handle, x1 + 1, y2 - (radius - 1), 1, 1));
			//top right
			if_gui_fill(handle, color, if_gui_area(handle, x2 - (radius - 1), y2 - 1, 1, 1));
			if_gui_fill(handle, color, if_gui_area(handle, x2  - 1, y2 - (radius -1), 1, 1));
			break;
	}
}

static void if_gui_set_backlight(struct gui_handle * handle, uint8 mode) {
	if(mode) handle->status |= UI_STATUS_ACTIVE;		//display in active state (2017.03.04)
	else handle->status &= ~UI_STATUS_ACTIVE;			//display in sleep state
	LCD_SetBacklight(mode);
}

//display apis definition
void if_display_wake(gui_handle_p ctx) {
	LCD_DisplayOn();
}

void if_display_sleep(gui_handle_p ctx) {
	LCD_DisplayOff();
}

void if_gui_init(gui_handle_p handle, uint8 orientation) {
	LCD_DisplayOff();
	LCD_Init();
	LCD_DMA_Init(handle);
	//LCD_Clear(BLACK);
	handle->width = DISPLAY_WIDTH;
	handle->height = DISPLAY_HEIGHT;
	handle->body = NULL;
	handle->header = NULL;
	handle->set_backlight = if_gui_set_backlight;
	handle->set_area = if_gui_area;
	handle->begin_burst = if_gui_begin;
	handle->put_pixel = if_gui_write;
	handle->print_string = if_gui_print;
	handle->draw_rectangle = if_gui_rectangle;
	handle->draw_line = if_gui_line;
	handle->fill_area = if_gui_fill;
	handle->fill_bitmap = if_gui_bitmap;
	handle->status = 0;
	//init default user interface
	ui_display_init(handle);
	ui_switch_orientation(handle, orientation);			//change orientation based configuration (2017.10.22)
	LCD_DisplayOn();
	if_gui_fill(handle, UI_COLOR_BLACK, if_gui_area(handle, 0, 0, ((gui_handle_p)handle)->width , ((gui_handle_p)handle)->height)) ;
	//activate backlight
	if_delay(300);
	LCD_SetBacklight(1);
}


void * if_gui_present(gui_handle_p display) {
#if SHARD_RTOS_ENABLED
	ui_present(display);
	return NULL;
#else
	uint16 counter = 0;
	void * obj = ui_process_events(display);
	if_delay(20);
	ui_present(display);
	return obj;
#endif
}

void if_gui_switch_orientation(gui_handle_p display, uint8 mode) {
	LCD_Switch_Orientation(mode);
}


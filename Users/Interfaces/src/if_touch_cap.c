/**************************************************************
** 	ª≈£ø™∑¢∞ÅE
**	π¶ƒ‹ΩÈ…‹£∫ ¥•√˛∆¡«˝∂Ø¥˙¬ÅE
**  ∞Ê±æ£∫V1.0  
**	¬€Ã≥£∫www.openmcu.com
**	Ã‘±¶£∫http://shop36995246.taobao.com/   
**  ºº ı÷ß≥÷»∫£∫121939788 
***************************************************************/ 
/* Includes ------------------------------------------------------------------*/

#include "defs.h"
#include "config.h"
#include "..\inc\if_apis.h"
#include "..\..\gui\inc\ui_core.h"
#include "..\..\core\inc\os_core.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
//#include "stm32f4xx_hal.h"
//#include "stm32f4xx_hal_pwr.h"
//#include "stm32f4xx_hal_gpio.h"
//#include "stm32f4xx_hal_i2c.h"

#if SHARD_TS_TYPE == 0x6236

static ui_config * g_cur_tconfig = NULL;
I2C_HandleTypeDef dev_ft6236;

extern void Delay(__IO uint32_t nTime);

#define FT6X36_ADDR					0x70			//device I2C slave address
#define TM_I2C_TIMEOUT				20000		//default I2C timeout
//FT6236 register entries
#define FT6X36_REG_MODE		0x00
#define FT6X36_REG_GEST			0x01
#define FT6X36_REG_STAT			0x02
#define FT6X36_REG_TOUCH1		0x03
#define FT6X36_REG_TOUCH2		0x09
#define FT6X36_REG_TOUCH3		0x0F
#define FT6X36_REG_TOUCH4		0x15
#define FT6X36_REG_THOLD		0x80


uint8_t TS_I2C_Read(I2C_HandleTypeDef* I2Cx, uint8_t address, uint8_t reg);
void TS_I2C_ReadMulti(I2C_HandleTypeDef* I2Cx, uint8_t address, uint8_t reg, uint8_t* data, uint16_t count) ;
void TS_I2C_Write(I2C_HandleTypeDef* I2Cx, uint8_t address, uint8_t reg, uint8_t data) ;
void TS_I2C_WriteMulti(I2C_HandleTypeDef* I2Cx, uint8_t address, uint8_t reg, uint8_t* data, uint16_t count);
uint8_t TS_I2C_IsDeviceConnected(I2C_HandleTypeDef* I2Cx, uint8_t address) ;

void Pen_Int_Set(uint8_t en)
{
	if(en)EXTI->IMR|=1<<13;   //ø™∆Ùline13…œµƒ÷–∂œ	  	
	else EXTI->IMR&=~(1<<13); //πÿ±’line13…œµƒ÷–∂œ	   
}	 

void ft_reset() {
	
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_DisableIRQ(EXTI15_10_IRQn);
	Touch_Configuration();
}

static uint8 if_touch_get_xy(uint16 * x, uint16 * y) {
	uint8 buffer[9];
	memset(buffer, 0, sizeof(buffer));
	TS_I2C_ReadMulti(&dev_ft6236, FT6X36_ADDR, FT6X36_REG_MODE, buffer, 9);
	y[0] = ((buffer[3] & 0x0F) << 8) | buffer[4];
	x[0] = ((buffer[5] & 0x0F) << 8) | buffer[6];
	if((buffer[3] & 0xC0) == 0x40) return 1;
	else return 0;
}
				   
uint8_t if_touch_read_once(void)
{
	uint8 ret = 0;
	uint8_t t=0;	    
	Pen_Int_Set(0);		//disable interrupt tp
	g_cur_tconfig->t_state = UI_KEY_UP;
	if(if_touch_get_xy(&g_cur_tconfig->x, &g_cur_tconfig->y)) {
		while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET && t<=250)
		{
			t++;
		}; 
		if(t>=250) ret = 0;
		else ret = 1;	
	}
	Pen_Int_Set(1);		//enable interrupt tp
	return ret;
}
//////////////////////////////////////////////////

static void if_touch_draw_point(gui_handle_p display, uint16 x, uint16 y)
{
	display->fill_area(display, UI_COLOR_WHITE, display->set_area(display, x - 4, y - 4, 8, 8));
}	  

void if_touch_adjust(void * handle)
{								 
	uint16_t pos_temp[4][2];//◊¯±ÅE∫¥Ê÷µ
	uint8_t  cnt=0;	
	uint16_t d1,d2;
	uint32_t tem1,tem2;
	ui_config * conf = ((gui_handle_p)handle)->touch_config;
	float fac; 	   
	gui_handle_p display = (gui_handle_p)handle;
	cnt=0;				
	display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
	display->present(display);
	if_touch_draw_point(display, 20, 20);
	conf->t_state = UI_KEY_UP;//œ˚≥˝¥•∑¢–≈∫≈ 
	conf->xfac = 0;//xfac”√¿¥±ÅE« «∑Ò–£◊ºπ˝,À˘“‘–£◊º÷Æ«∞±ÿ–ÅEÂµÅE“‘√‚¥˙ÍÅE 
	while(1)
	{
		if(conf->t_state==UI_KEY_DOWN)//∞¥ºÅE¥œ¬¡À
		{
			if(if_touch_read_once())//µ√µΩµ•¥Œ∞¥ºÅEµ
			{  								   
				pos_temp[cnt][0]=conf->x;
				pos_temp[cnt][1]=conf->y;
				cnt++;
			}			 
			switch(cnt)
			{			   
				case 1:
					display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
					if_touch_draw_point(display, (display->width - 20), 20);
					display->present(display);
					break;
				case 2:
					display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
					if_touch_draw_point(display, 20, (display->height - 20));
					display->present(display);
					break;
				case 3:
					display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
					if_touch_draw_point(display, (display->width - 20), (display->height - 20));
					display->present(display);
					break;
				case 4:	 //»´≤øÀƒ∏ˆµ„“—æ≠µ√µΩ
	    		    	//∂‘±ﬂœ‡µ»
					tem1=abs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2
					tem2=abs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//µ√µΩ1,2µƒæ‡¿ÅE
					
					tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
					tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//µ√µΩ3,4µƒæ‡¿ÅE
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05||d1==0||d2==0)//≤ª∫œ∏ÅE
					{
						cnt=0;
						display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
						if_touch_draw_point(display, 20, 20);
						display->present(display);
						continue;
					}
					tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//µ√µΩ1,3µƒæ‡¿ÅE
					
					tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//µ√µΩ2,4µƒæ‡¿ÅE
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//≤ª∫œ∏ÅE
					{
						cnt=0;
						display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
						if_touch_draw_point(display, 20, 20);
						display->present(display);
						continue;
					}//’˝»∑¡À
								   
					//∂‘Ω«œﬂœ‡µ»
					tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//µ√µΩ1,4µƒæ‡¿ÅE
	
					tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//µ√µΩ2,3µƒæ‡¿ÅE
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//≤ª∫œ∏ÅE
					{
						cnt=0;
						display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
						if_touch_draw_point(display, 20, 20);
						display->present(display);
						continue;
					}
					conf->yfac=(float)(display->height - 40)/(pos_temp[0][1]-pos_temp[3][1]);//µ√µΩxfac		 
					conf->yoff=(display->height - conf->yfac*(pos_temp[0][1]+pos_temp[3][1]))/2;//µ√µΩxoff
						  
					conf->xfac=(float)(display->width - 40)/(pos_temp[3][0]-pos_temp[2][0]);//µ√µΩyfac
					conf->xoff=(display->width - conf->xfac*(pos_temp[3][0]+pos_temp[2][0]))/2;//µ√µΩyoff  
					return;//–£’˝ÕÅE…				 
			}
		}
	} 
}

static uint8 if_touch_read(gui_handle * display, uint16 * x, uint16 * y) {
	uint8 ret = 0;
	uint csr;
	ui_config * conf = display->touch_config;
	if(conf->t_state != UI_KEY_DOWN) return -1;
	conf->t_state = UI_KEY_UP;
#if 0
	NVIC_DisableIRQ(EXTI15_10_IRQn);
	csr = os_enter_critical();
	if_touch_wake(display);
	//start read here
	if(if_touch_get_xy(&conf->x,&conf->y))
	{
		if_touch_get(display, x, y);
	} else 
		ret = -1;
	
	if_touch_sleep(display);
    os_exit_critical(csr);
	NVIC_EnableIRQ(EXTI15_10_IRQn);
#endif
	if_touch_get(display, x, y);
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);
	return ret;
}

static uint8 if_touch_state(gui_handle * display) {
	ui_config * conf = display->touch_config;
	return conf->t_state;
}

//touch apis definition
void if_touch_wake(void * display) {
	
}

void if_touch_sleep(void * display) {
	
}
/*****************************************************************************
** ∫Ø ˝√˚≥∆: Touch_Init
** π¶ƒ‹√Ë ÅE ¥•√˛∆¡≥ı ºªØ
				¥¯–£◊ººÅEÅE
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
void if_task_touch() {
	//very high priority task
	uint8 reg;
	uint8 buffer[6];
	gui_handle_p display = (gui_handle_p)os_get_context();
	while(1) {
		os_suspend();
		os_wait(20);		//MUST ADD delay before reading any data from FT6x36
		if(g_cur_tconfig != NULL) {
			if(if_touch_get_xy(&g_cur_tconfig->x, &g_cur_tconfig->y)) {
				g_cur_tconfig->t_state=UI_KEY_DOWN;//∞¥ºÅE¥œ¬
			} else {
				TS_I2C_ReadMulti(&dev_ft6236, FT6X36_ADDR, FT6X36_REG_THOLD, buffer, 6);
				if(HAL_I2C_IsDeviceReady(&dev_ft6236, FT6X36_ADDR, 10, 100) == HAL_OK) {
					//reg = TS_I2C_Read(&dev_ft6236, FT6X36_ADDR, 0x00);
					TS_I2C_Write(&dev_ft6236, FT6X36_ADDR, FT6X36_REG_MODE, 0);
				} else {
					//re-init i2c
					//Touch_Configuration();
					//TS_I2C_Write(&dev_ft6236, FT6X36_ADDR, FT6X36_REG_MODE, 0);
					ft_reset();
				}
			}				
		}
	}
}

int8 if_touch_init(void * display)
{
	uint8 reg;
	ui_config * conf = ((gui_handle_p)display)->touch_config;
	g_cur_tconfig = conf;
	GPIO_InitTypeDef GPIO_InitStructure;
	Touch_Configuration();
	
	//for(addr =0;addr < 0xFF;addr++) {
	if(HAL_I2C_IsDeviceReady(&dev_ft6236, FT6X36_ADDR, 10, 2000) == HAL_OK) {
		reg = TS_I2C_Read(&dev_ft6236, FT6X36_ADDR, 0x00);
		reg = TS_I2C_Read(&dev_ft6236, FT6X36_ADDR, 0xBC);
		reg = TS_I2C_Read(&dev_ft6236, FT6X36_ADDR, 0x00);
		TS_I2C_ReadMulti(&dev_ft6236, FT6X36_ADDR, FT6X36_REG_THOLD, ((gui_handle_p)display)->touch_treshold, 6);
	} else 
		return -1;
	
	((gui_handle_p)display)->touch_state = if_touch_state;
	((gui_handle_p)display)->touch_read = if_touch_read;
	os_create_task(display, if_task_touch, "tc", 11, 1024);
	if(conf->magic != 0xEC) {
		memset((uint8 *)conf, 0, sizeof(ui_config));
		//if_gui_switch_orientation(display, );
		if_touch_adjust(display);
		conf->magic = 0xEC;
		if_config_write(conf, sizeof(ui_config));
	}
	return 0;
}

uint8 if_touch_get(void * display, uint16 * x, uint16 * y) {
	uint16 xx, yy, t;
	ui_config * conf = ((gui_handle_p)display)->touch_config;
	xx = conf->xfac * conf->x + conf->xoff;
	yy = (conf->yfac * conf->y + conf->yoff);
	switch(((gui_handle_p)display)->orientation) {
		case UI_DISP_ORIENTATION_0:
			xx = xx;
			yy = ((gui_handle_p)display)->height - yy;
			break;
		case UI_DISP_ORIENTATION_1:
			t = xx;
			xx = ((gui_handle_p)display)->width - yy;
			yy = ((gui_handle_p)display)->height - t;
			break;
		case UI_DISP_ORIENTATION_2:		//default orientation
			xx = ((gui_handle_p)display)->width - xx;
			yy = yy;
			break;
		case UI_DISP_ORIENTATION_3:
			t = xx;
			xx = yy;
			yy = t;
			break;
	}
	*x = xx;
	*y = yy;
	return 0;
}
/*****************************************************************************
** ∫Ø ˝√˚≥∆: Touch_Configuration
** π¶ƒ‹√Ë ÅE ¥•√˛∆¡IO≈‰÷√
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
void Touch_Configuration()
{
	uint8 reg;
	uint8 addr;
	GPIO_InitTypeDef GPIO_InitStructure;
	memset((void *)&dev_ft6236, 0, sizeof(I2C_HandleTypeDef));
  	//NVIC_InitTypeDef NVIC_InitStructure;
	
	__HAL_RCC_GPIOB_CLK_ENABLE();		//i2c1
	__HAL_RCC_GPIOC_CLK_ENABLE();		//tp interrupt
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	//œ¬√Ê «SPIœ‡πÿGPIO≥ı ºªØ
	GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;  //Õ®”√Õ∆ÕÅE‰≥ÅE
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStructure.Alternate = GPIO_AF4_I2C1;
	HAL_GPIO_Init(GPIOB,&GPIO_InitStructure);

	//Configure PC13 pin: TP_INT pin 
	GPIO_InitStructure.Pin = GPIO_PIN_13; 
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP; 	//…œ¿≠ ‰»ÅE
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOC,&GPIO_InitStructure);
	//wakeup
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
	if_delay(20);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
	if_delay(50);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
	
	//__HAL_RCC_I2C1_CONFIG(RCC_I2C1CLKSOURCE_PCLK1);
	__HAL_RCC_I2C1_CLK_ENABLE();
	dev_ft6236.Instance = I2C1;
#ifdef STM32F407xx
	dev_ft6236.Init.ClockSpeed = 400000;
	dev_ft6236.Init.DutyCycle = I2C_DUTYCYCLE_2;
#endif
#ifdef STM32F7
	//dev_ft6236.Init.Timing = 0x20303E5D;		//100Khz (Standard Mode)
	dev_ft6236.Init.Timing = 0x2010091A;		//400Khz (Fast Mode)
	//dev_ft6236.Init.Timing = 0x20100843;		//200Khz (Fast Mode)
#endif
#ifdef STM32F4
	dev_ft6236.Init.ClockSpeed = 400000;
#endif
	dev_ft6236.Init.OwnAddress1 = 0;
	dev_ft6236.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	dev_ft6236.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	dev_ft6236.Init.OwnAddress2 = 0;
	dev_ft6236.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	dev_ft6236.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	dev_ft6236.Mode= HAL_I2C_MODE_MASTER;

	//__HAL_I2C_DISABLE(&dev_ft6236);
	HAL_I2C_Init(&dev_ft6236);
	//__HAL_I2C_ENABLE(&dev_ft6236);
    /**Configure Analogue filter 
    */
	if (HAL_I2CEx_ConfigAnalogFilter(&dev_ft6236, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
	{
		//_Error_Handler(__FILE__, __LINE__);
	}

    /**Configure Digital filter 
    */
	if (HAL_I2CEx_ConfigDigitalFilter(&dev_ft6236, 0) != HAL_OK)
	{
		//_Error_Handler(__FILE__, __LINE__);
	}
	if_delay(10);
	GPIO_InitStructure.Pin = GPIO_PIN_13;							//touch interrupt
  	GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;			//peripheral (alternate function push pull)
	GPIO_InitStructure.Pull = GPIO_PULLUP;
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;	
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);	
	NVIC_EnableIRQ(EXTI15_10_IRQn);
}

uint8_t TS_I2C_Read(I2C_HandleTypeDef* I2Cx, uint8_t address, uint8_t reg) {
	HAL_I2C_Mem_Read(I2Cx, address, reg, 1, &reg, 1, 200);
	return reg;
}

void TS_I2C_ReadMulti(I2C_HandleTypeDef* I2Cx, uint8_t address, uint8_t reg, uint8_t* data, uint16_t count) {
	HAL_I2C_Mem_Read(I2Cx, address, reg, 1, data, count, 200);
}

void TS_I2C_Write(I2C_HandleTypeDef* I2Cx, uint8_t address, uint8_t reg, uint8_t data) {
	uint8_t bufout[2] ;
	bufout[0] = reg;
	bufout[1] = data;
	HAL_I2C_Master_Transmit(I2Cx, address, bufout, 2, 200);
}

void TS_I2C_WriteMulti(I2C_HandleTypeDef* I2Cx, uint8_t address, uint8_t reg, uint8_t* data, uint16_t count) {
	
	uint8 buffer[0x81];
	buffer[0] = reg;
	memcpy(buffer + 1, data, count);
	
	HAL_I2C_Master_Transmit(I2Cx, address, buffer, count + 1, 400);
}

uint8_t TS_I2C_IsDeviceConnected(I2C_HandleTypeDef* I2Cx, uint8_t address) {
	//uint8_t connected = 1;
	//return connected;
	return HAL_I2C_IsDeviceReady(I2Cx, address, 3, 2000);
}

/*****************************************************************************
** ∫Ø ˝√˚≥∆: EXTI15_10_IRQHandler
** π¶ƒ‹√Ë ÅE ÷–∂œ¥¶¿˙÷Ø ˝
				÷–∂œ,ºÅE‚µΩPENΩ≈µƒ“ª∏ˆœ¬Ωµ—ÿ.
					÷√ŒªPen_Point.Key_StaŒ™∞¥œ¬◊¥Ã¨
						÷–∂œœﬂ4œﬂ…œµƒ÷–∂œºÅEÅE
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
void EXTI15_10_IRQHandler()
{
	uint16_t i;
	void * task;
  	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_13) != RESET)
	{
    	__HAL_GPIO_EXTI_CLEAR_FLAG(GPIO_PIN_13);
		task = os_find_task_by_name("tc");
		if(task != NULL) os_resume(task);		
		if_pwr_set_interrupt_source(IF_INT_WAKE | IF_INT_TOUCH);
	}
}
/*********************************************************************************************************
** End of File
*********************************************************************************************************/
#endif
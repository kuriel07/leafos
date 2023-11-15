/**************************************************************
** 	»ğÅ£¿ª·¢°E
**	LCDÇı¶¯´úÂE
**  °æ±¾£ºV1.0  
**	ÂÛÌ³£ºwww.openmcu.com
**	ÌÔ±¦£ºhttp://shop36995246.taobao.com/   
**  ¼¼ÊõÖ§³ÖÈº£º121939788 
***************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "..\..\defs.h"
#include "..\..\config.h"
#if (SHARD_LCD_TYPE & 0xFFF0) == 0x9340
#include "..\..\inc\ili932x\ili932x.h"
#include "..\..\inc\ili932x\fonts.h"	 //×Ö¿âÎÄ¼ş¿E
#include "..\..\inc\ili932x\hz16.h"


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

uint16_t POINT_COLOR = BLUE, BACK_COLOR = WHITE;  /* ·Ö±ğÉèÖÃµãµÄÑÕÉ«ºÍµ×É«	*/

/*****************************************************************************
** º¯ÊıÃû³Æ: LCD_Write_Reg
** ¹¦ÄÜÃèÊE Ğ´Ö¸ÁûØ°Êı¾İ
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_Dat)
{
	Write_Cmd(LCD_Reg);
	Write_Dat(LCD_Dat);
}
/*****************************************************************************
** º¯ÊıÃû³Æ: Write_Cmd
** ¹¦ÄÜÃèÊE Ğ´Ö¸ÁE
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
void Write_Cmd(uint16_t LCD_Reg)
{
	int i;
	LCD_RS = 0;
	LCD_CS = 0;
	LCD_RD = 1;
	GPIOC->ODR = (GPIOC->ODR&0xff00)|(LCD_Reg&0x00ff);
	GPIOB->ODR = (GPIOB->ODR&0x00ff)|(LCD_Reg&0xff00); 
	LCD_WR = 0;
	//for(i=0;i<4;i++);
	LCD_WR = 1;	
	LCD_CS = 1;
	//for(i=0;i<2000;i++);
}
/*****************************************************************************
** º¯ÊıÃû³Æ: Write_Dat
** ¹¦ÄÜÃèÊE Ğ´Êı¾İ
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
void Write_Dat(uint16_t LCD_Dat)
{
	int i;
	LCD_RS = 1;
	LCD_CS = 0;
	LCD_RD = 1;
	GPIOC->ODR = (GPIOC->ODR&0xff00)|(LCD_Dat&0x00ff);
	GPIOB->ODR = (GPIOB->ODR&0x00ff)|(LCD_Dat&0xff00);
	
	LCD_WR = 0;
	//for(i=0;i<4;i++);
	LCD_WR = 1;	
	LCD_CS = 1;
	//for(i=0;i<2000;i++);
}
/*****************************************************************************
** º¯ÊıÃû³Æ: LCD_ReadReg
** ¹¦ÄÜÃèÊE ¶ÁÖ¸ÁE
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	uint16_t temp;
	Write_Cmd(LCD_Reg);  //Ğ´ÈEª¶ÁµÄ¼Ä´æÆ÷ºÅ  

	//GPIOB->CRH = (GPIOB->CRH & 0x00000000) | 0x44444444; //½«¶Ë¿Ú¸ß8Î»ÅäÖÃ³ÉÊäÈ
	GPIO_InitStructure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;					//pin 2
  	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;			//peripheral (alternate function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;	
	GPIO_InitStructure.Pull = GPIO_NOPULL;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	//GPIOC->CRL = (GPIOC->CRL & 0x00000000) | 0x44444444; //½«¶Ë¿ÚµÍ8Î»ÅäÖÃ³ÉÊäÈE 
	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;					//pin 2
  	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;			//peripheral (alternate function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;	
	GPIO_InitStructure.Pull = GPIO_NOPULL;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);	
	
	LCD_CS = 0;
	LCD_RS = 1;
	LCD_RD = 0;	
	LCD_RD = 1;
	temp = ((GPIOB->IDR&0xff00)|(GPIOC->IDR&0x00ff)); //¶ÁÈ¡Êı¾İ(¶Á¼Ä´æÆ÷Ê±,²¢²»ĞèÒª¶Á2´Î) 
	LCD_CS = 1; 
	//GPIOB->CRH = (GPIOB->CRH & 0x00000000) | 0x33333333; //ÊÍ·Å¶Ë¿Ú¸ß8Î»ÎªÊä³E
	//GPIOC->CRL = (GPIOC->CRL & 0x00000000) | 0x33333333; //ÊÍ·Å¶Ë¿ÚµÍ8Î»ÎªÊä³E

	GPIO_InitStructure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;					//pin 2
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (output function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	//GPIOC->CRL = (GPIOC->CRL & 0x00000000) | 0x44444444; //½«¶Ë¿ÚµÍ8Î»ÅäÖÃ³ÉÊäÈE 
	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;					//pin 2
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (output function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);	
	return temp;   
}
/*****************************************************************************
** º¯ÊıÃû³Æ: LCD_ReadDat
** ¹¦ÄÜÃèÊE ¶ÁÊı¾İ
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
uint16_t LCD_ReadDat()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	uint16_t temp;

	//GPIOE->CRH = (GPIOE->CRH & 0x00000000) | 0x44444444;  //½«¶Ë¿Ú¸ß8Î»ÅäÖÃ³ÉÊäÈE
	//GPIOE->CRL = (GPIOE->CRL & 0x00000000) | 0x44444444;  //½«¶Ë¿ÚµÍ8Î»ÅäÖÃ³ÉÊäÈE
	
	//GPIOB->CRH = (GPIOB->CRH & 0x00000000) | 0x44444444; //½«¶Ë¿Ú¸ß8Î»ÅäÖÃ³ÉÊäÈ
	GPIO_InitStructure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;					//pin 2
  	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;			//peripheral (alternate function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;	
	GPIO_InitStructure.Pull = GPIO_NOPULL;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	//GPIOC->CRL = (GPIOC->CRL & 0x00000000) | 0x44444444; //½«¶Ë¿ÚµÍ8Î»ÅäÖÃ³ÉÊäÈE 
	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;					//pin 2
  	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;			//peripheral (alternate function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;	
	GPIO_InitStructure.Pull = GPIO_NOPULL;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);		
	
	LCD_CS = 0;
	LCD_RS = 1;
	LCD_RD = 0;
	LCD_RD = 1;
	temp = ((GPIOB->IDR&0xff00)|(GPIOC->IDR&0x00ff));	//¶ÁÈ¡Êı¾İ(¶Á¼Ä´æÆ÷Ê±,²¢²»ĞèÒª¶Á2´Î)
	LCD_CS = 1;
	//GPIOE->CRH = (GPIOE->CRH & 0x00000000) | 0x33333333;  //ÊÍ·Å¶Ë¿Ú¸ß8Î»ÎªÊä³E
	//GPIOE->CRL = (GPIOE->CRL & 0x00000000) | 0x33333333;  //ÊÍ·Å¶Ë¿ÚµÍ8Î»ÎªÊä³E
	
	GPIO_InitStructure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;					//pin 2
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (output function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	//GPIOC->CRL = (GPIOC->CRL & 0x00000000) | 0x44444444; //½«¶Ë¿ÚµÍ8Î»ÅäÖÃ³ÉÊäÈE 
	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;					//pin 2
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (output function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);		

	return temp;   
}
/*****************************************************************************
** º¯ÊıÃû³Æ: LCD_Configuration
** ¹¦ÄÜÃèÊE LCD_IO¿ÚÅäÖÃ
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
void LCD_Configuration()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC,ENABLE);
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	/* ÅäÖÃÊı¾İIO Á¬½Óµ½GPIOB *********************/	
	GPIO_InitStructure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 
								| GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (output function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* ÅäÖÃ¿ØÖÆIO Á¬½Óµ½PD12.PD13.PD14.PD15 *********************/	
	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 
								| GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 
								| GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (output function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
	LCD_SetBacklight(0);
}

void LCD_SetBacklight(uint8_t mode) {
	if(mode) HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
	else HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);
}
/*****************************************************************************
** º¯ÊıÃû³Æ: LCD_Init
** ¹¦ÄÜÃèÊE LCD³õÊ¼»¯
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
void LCD_Init(void)
{
	static uint16_t DeviceCode;
	LCD_Configuration();
	
	Write_Cmd(ILI9341_SWRESET); 
	LCD_Delay(120); 			// LCD_Delay 120 ms
	
 	Write_Cmd(0xCF);   
	Write_Dat(0x00); 
	Write_Dat(0xc3); 
	Write_Dat(0X30); 
      
 	Write_Cmd(0xED);   
	Write_Dat(0x64); 
	Write_Dat(0x03); 
	Write_Dat(0X12); 
	Write_Dat(0X81); 
      
 	Write_Cmd(0xE8);   
	Write_Dat(0x85); 
	Write_Dat(0x10); 
	Write_Dat(0x79); 
      
 	Write_Cmd(0xCB);   
	Write_Dat(0x39); 
	Write_Dat(0x2C); 
	Write_Dat(0x00); 
	Write_Dat(0x34); 
	Write_Dat(0x02); 
      
 	Write_Cmd(0xF7);   
	Write_Dat(0x20); 
      
 	Write_Cmd(0xEA);   
	Write_Dat(0x00); 
	Write_Dat(0x00);
	
	Write_Cmd(0xC0);    //Power control 
	Write_Dat(0x22);   //VRH[5:0] 
      
 	Write_Cmd(0xC1);    //Power control 
	Write_Dat(0x11);   //SAP[2:0];BT[3:0] 
      
 	Write_Cmd(0xC5);    //VCM control 
	Write_Dat(0x3d); 
    //LCD_DataWrite_ILI9341(0x30); 
	Write_Dat(0x20); 
      
 	Write_Cmd(0xC7);    //VCM control2 
    //LCD_DataWrite_ILI9341(0xBD); 
	Write_Dat(0xAA); //0xB0 
    
 	Write_Cmd(0x36);    // Memory Access Control 
	Write_Dat(0x08); 
      
 	Write_Cmd(0x3A);   	//pixel format
	Write_Dat(0x55); 
    
 	Write_Cmd(0xB1);   
	Write_Dat(0x00);   
	Write_Dat(0x13); 
      
 	Write_Cmd(0xB6);    // Display Function Control 
	//Write_Dat(0x0A); 
	//Write_Dat(0xA2); 
	Write_Dat(0x08); 
	Write_Dat(0x82);
	Write_Dat(0x27);  
    
 	Write_Cmd(0xF6);     
	Write_Dat(0x01); 
	Write_Dat(0x30); 
      
 	Write_Cmd(0xF2);    // 3Gamma Function Disable 
	Write_Dat(0x00); 
      
 	Write_Cmd(0x26);    //Gamma curve selected 
	Write_Dat(0x01); 
      
 	Write_Cmd(0xE0);    //Set Gamma 
	Write_Dat(0x0F); 
	Write_Dat(0x3F); 
	Write_Dat(0x2F); 
	Write_Dat(0x0C); 
	Write_Dat(0x10); 
	Write_Dat(0x0A); 
	Write_Dat(0x53); 
	Write_Dat(0XD5); 
	Write_Dat(0x40); 
	Write_Dat(0x0A); 
	Write_Dat(0x13); 
	Write_Dat(0x03); 
	Write_Dat(0x08); 
	Write_Dat(0x03); 
	Write_Dat(0x00); 
      
 	Write_Cmd(0XE1);    //Set Gamma 
	Write_Dat(0x00); 
	Write_Dat(0x00); 
	Write_Dat(0x10); 
	Write_Dat(0x03); 
	Write_Dat(0x0F); 
	Write_Dat(0x05); 
	Write_Dat(0x2C); 
	Write_Dat(0xA2); 
	Write_Dat(0x3F); 
	Write_Dat(0x05); 
	Write_Dat(0x0E); 
	Write_Dat(0x0C); 
	Write_Dat(0x37); 
	Write_Dat(0x3C); 
	Write_Dat(0x0F); 
      
	//Write_Cmd(ILI9341_PIXFMT);    
	//Write_Dat(0x55); 
 	
	Write_Cmd(0x11);    //Exit Sleep 
	LCD_Delay(120);
 	Write_Cmd(0x29);    //Display on 
	LCD_Delay(50);
	
	LCD_Clear(0x0000);
}

void LCD_DisplayOn() {
	Write_Cmd(ILI9341_SLPOUT);
	Write_Cmd(ILI9341_DISPON);    //Display on
}

void LCD_DisplayOff() {
	Write_Cmd(ILI9341_DISPOFF);
	LCD_Delay(10); 
	Write_Cmd(ILI9341_SLPIN);
	LCD_Delay(10); 
}
/*****************************************************************************
** º¯ÊıÃû³Æ: LCD_DrawPoint
** ¹¦ÄÜÃèÊE Ğ´Ò»¸öµE
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
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
/*****************************************************************************
** º¯ÊıÃû³Æ: LCD_WriteRAM_Prepare
** ¹¦ÄÜÃèÊE Ğ©×¼±¸
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
void LCD_WriteRAM_Prepare()
{
	//Write_Cmd(R34);	
	Write_Cmd(ILI9341_RAMWR); // write to RAM
}
/*****************************************************************************
** º¯ÊıÃû³Æ: LCD_SetCursor
** ¹¦ÄÜÃèÊE ÉèÖÃ¹â±E¯Êı
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
	//LCD_WriteReg(R32, Ypos);
	//Write_Cmd(ILI9341_CASET); // Column addr set
	//Write_Dat(Ypos);
	//LCD_WriteReg(R33, Xpos);
	//Write_Cmd(ILI9341_PASET); // Row addr set
	//Write_Dat(Xpos);	
} 
/*****************************************************************************
** º¯ÊıÃû³Æ: LCD_SetDisplayWindow
** ¹¦ÄÜÃèÊE ÉèÖÃ´°¿Úº¯Êı (½«WidthºÍHeightµ÷»»Ò»ÏÂ£¬Ç°ÌáÊÇ)
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
void LCD_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width)
{
  	//LCD_WriteReg(R80, Ypos);	   	   	//Ë®Æ½·½ÏòGRAMÆğÊ¼µØÖ·
  	//LCD_WriteReg(R81, Ypos+Width); 	//Ë®Æ½·½ÏòGRAM½áÊøµØÖ· 
  	//LCD_WriteReg(R82, Xpos);		  	//´¹Ö±·½ÏòGRAMÆğÊ¼µØÖ·
  	//LCD_WriteReg(R83, Xpos+Height);  	//´¹Ö±·½ÏòGRAM½áÊøµØÖ·

  	//LCD_SetCursor(Xpos, Ypos);			//ÉèÖÃ¹â±E»ÖÃ
	
	Write_Cmd(ILI9341_PASET); // Row addr set
	Write_Dat(Ypos>>8);
	Write_Dat(Ypos & 0xFF);
	Write_Dat((Ypos+Width) >> 8);
	Write_Dat((Ypos+Width) & 0xFF);

	Write_Cmd(ILI9341_CASET); // Column addr set
	Write_Dat(Xpos >> 8);
	Write_Dat(Xpos & 0xFF);
	Write_Dat((Xpos+Height) >> 8);
	Write_Dat((Xpos+Height) & 0xFF);

	Write_Cmd(ILI9341_RAMWR); // write to RAM
}
/*****************************************************************************
** º¯ÊıÃû³Æ: LCD_ShowString
** ¹¦ÄÜÃèÊE ÏÔÊ¾×Ö·û´®º¯Êı
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
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
/*****************************************************************************
** º¯ÊıÃû³Æ: LCD_ShowChar
** ¹¦ÄÜÃèÊE ÏÔÊ¾Ò»¸ö×Ö·ûº¯Êı
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
void LCD_ShowChar(uint8_t x,uint16_t y,uint8_t chars,uint8_t size,uint8_t mode)
{
	uint8_t temp;
    uint8_t pos,t;      
    if(x>MAX_CHAR_POSX||y>MAX_CHAR_POSY) return;	    
											
	LCD_SetDisplayWindow(x,y,(size/2-1),size-1);  //ÉèÖÃ´°¿Ú

	LCD_WriteRAM_Prepare();        //¿ªÊ¼Ğ´ÈERAM	   
	if(!mode) 						//·Çµş¼Ó·½Ê½
	{
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=ASCII_1206[chars-0x20][pos];//µ÷ÓÃ1206×ÖÌE
			else temp=ASCII_1608[chars-0x20][pos];		 //µ÷ÓÃ1608×ÖÌE
			for(t=0;t<size/2;t++)
		    {     
		        //if(temp&0x01)            	 			//ÏÈ´«µÍÎ»£¬È¡Ä£ÓĞ¹ØÏµ
		        if((temp<<t)&0x80)						//ÏÈ´«¸ßÎ»
				{
					Write_Dat(RED);
				}
				else 
				{
					Write_Dat(WHITE);	        
		        }
				//temp>>=1; 	   							//Èç¹ûÏÈ´«µÍÎ»£¬È¥µôÆÁ±ÎÏß
		    }
		}	
	}
	else//µş¼Ó·½Ê½
	{
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=ASCII_1206[chars-0x20][pos];	//µ÷ÓÃ1206×ÖÌE
			else temp=ASCII_1608[chars-0x20][pos];		 	//µ÷ÓÃ1608×ÖÌE
			for(t=0;t<size/2;t++)
		    {                 
		        if((temp<<t)&0x80)LCD_DrawPoint(x+t,y+pos);	//»­Ò»¸öµE    
		        //temp>>=1; 								//Èç¹ûÏÈ´«µÍÎ»£¬È¥µôÆÁ±ÎÏß
		    }
		}
	}	    
	/* »Ö¸´´°Ìå´óĞ¡	*/
	//LCD_WriteReg(R80, 0x0000); //Ë®Æ½·½ÏòGRAMÆğÊ¼µØÖ·
	//LCD_WriteReg(R81, DISPLAY_HEIGHT - 1); //Ë®Æ½·½ÏòGRAM½áÊøµØÖ·
	//LCD_WriteReg(R82, 0x0000); //´¹Ö±·½ÏòGRAMÆğÊ¼µØÖ·
	//LCD_WriteReg(R83, DISPLAY_WIDTH - 1); //´¹Ö±·½ÏòGRAM½áÊøµØÖ·	
}
/*****************************************************************************
** º¯ÊıÃû³Æ: LCD_Clear
** ¹¦ÄÜÃèÊE ÇåÆÁÄ»º¯Êı
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
void LCD_Clear(uint16_t Color)
{
	uint32_t index=0;      
	LCD_SetCursor(0x00,0x0000); //ÉèÖÃ¹â±E»ÖÃ 
	LCD_WriteRAM_Prepare();     //¿ªÊ¼Ğ´ÈERAM	 	  
	for(index=0;index<DISPLAY_SIZE;index++)
	{
		Write_Dat(Color);    
	}
}
/*****************************************************************************
** º¯ÊıÃû³Æ: WriteString
** ¹¦ÄÜÃèÊE ÔÚÖ¸¶¨Î»ÖÃ¿ªÊ¼ÏÔÊ¾Ò»¸ö×Ö·û´®ºÍÒ»´®ºº×Ö
				Ö§³Ö×Ô¶¯»»ĞĞ
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/   			   
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
            break;                                     /* ×Ö·û´®½áÊE           */
        }      
        x0 = x0 + (usWidth);                           /* µ÷½Ú×Ö·û´®ÏÔÊ¾ËÉ½ô¶È         */
        if(*pcStr > 0x80)                              /* ÅĞ¶ÏÎªºº×Ö                   */
        {
		    if((x0 + 16) > LCD_W)                      /* ¼EéÊ£Óà¿Õ¼äÊÇ·ñ×ã¹»         */
            {
			    x0 = 0;
                y0 = y0 + 16;                          /* ¸Ä±äÏÔÊ¾×ø±E                */
                if(y0 > LCD_H)                         /* ×İ×ø±E¬³E                  */
                {
				    y0 = 0;
                }
            }
            usIndex = findHzIndex(pcStr);
            usWidth = WriteOneHzChar((uint8_t *)&(ptGb16[usIndex].Msk[0]), x0, y0, color);
                                                       /* ÏÔÊ¾×Ö·E                    */
            pcStr += 2;
        }
		else 
		{                                               /* ÅĞ¶ÏÎª·Çºº×Ö                 */
            if (*pcStr == '\r')                         /* »»ĞĞ                         */
            { 
			    x0 = sta_x;
                pcStr++;
                usWidth = 0;
                continue;
            } 
			else if (*pcStr == '\n')                    /* ¶ÔÆE½ÆğµE                  */
            {
			    x0 = sta_x;
			    y0 = y0 + 16;                           /* ¸Ä±äÏÔÊ¾×ø±E                */
                if(y0 > LCD_H)                          /* ×İ×ø±E¬³E                  */
                {
				    y0 = 0;
                }
                pcStr++;
                usWidth = 0;
                continue;
            } 
			else 
			{
                if((x0 + 8) > LCD_W)                     /* ¼EéÊ£Óà¿Õ¼äÊÇ·ñ×ã¹»         */
                {
				    x0 = 0;
                    y0 = y0 + 16;                        /* ¸Ä±äÏÔÊ¾×ø±E                */
                    if(y0 > LCD_H)                       /* ×İ×ø±E¬³E                  */
                    { 
					    y0 = 0;
                    }
                }
                //usWidth = WriteOneASCII((uint8_t *)&ASCII_1608[(*pcStr - 0x20)][0], x0, y0, color);
				usWidth = WriteOneASCII((uint8_t *)(g_font_large + (*pcStr * 0x10)), x0, y0, color);
                pcStr += 1;
            }
		}
	}												  	  
}
/*****************************************************************************
** º¯ÊıÃû³Æ: WriteOneHzChar
** ¹¦ÄÜÃèÊE ÏÔÊ¾Ò»¸öÖ¸¶¨´óĞ¡µÄºº×Ö
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
uint16_t WriteOneHzChar(uint8_t *pucMsk,
                               uint16_t x0,
                               uint16_t y0,
                               uint16_t color)
{
    uint16_t i,j;
    uint16_t mod[16];                                      /* µ±Ç°×ÖÄ£                     */
    uint16_t *pusMsk;                                      /* µ±Ç°×Ö¿âµØÖ·                 */
    uint16_t y;
    
    pusMsk = (uint16_t *)pucMsk;
    for(i=0; i<16; i++)                                    /* ±£´æµ±Ç°ºº×ÖµãÕóÊ½×ÖÄ£       */
    {
        mod[i] = *pusMsk++;                                /* È¡µÃµ±Ç°×ÖÄ££¬°EÖ¶ÔÆEÃÎÊ   */
        mod[i] = ((mod[i] & 0xff00) >> 8) | ((mod[i] & 0x00ff) << 8);/* ×ÖÄ£½»»»¸ßµÍ×Ö½Ú£¨ÎªÁËÏÔÊ¾   */
                                                           /* ĞèÒª£©                       */
    }
    y = y0;
    for(i=0; i<16; i++)                                    /* 16ĞĞ                         */
    { 
	    #ifdef __DISPLAY_BUFFER                            /* Ê¹ÓÃÏÔ´æÏÔÊ¾                 */
        for(j=0; j<16; j++)                                /* 16ÁĞ                         */
        {
		    if((mod[i] << j)& 0x8000)                      /* ÏÔÊ¾×ÖÄ£                     */
            {
			    DispBuf[320*(y0+i) + x0+j] = color;
            }
        }
        #else                                              /* Ö±½ÓÏÔÊ¾                     */
        
		LCD_SetCursor(x0, y);                              /* ÉèÖÃĞ´Êı¾İµØÖ·Ö¸ÕE          */
		LCD_WriteRAM_Prepare();        					   /*¿ªÊ¼Ğ´ÈERAM	*/   
        for(j=0; j<16; j++)                                /* 16ÁĞ                         */
        {
		    if((mod[i] << j) & 0x8000)                     /* ÏÔÊ¾×ÖÄ£                     */
            { 
			    Write_Dat(color);
            } 
			else 
			{
                //Write_Dat(BACK_COLOR);                     /* ÓÃ¶Á·½Ê½Ìø¹ıĞ´¿Õ°×µãµÄÏñËØ   */
            	//LCD_ReadDat();
			}
        }
        y++;
        #endif
    }
    return (16);                                          /* ·µ»Ø16Î»ÁĞ¿E                */
}
/*****************************************************************************
** º¯ÊıÃû³Æ: WriteOneASCII
** ¹¦ÄÜÃèÊE ÏÔÊ¾Ò»¸öÖ¸¶¨´óĞ¡µÄ×Ö·E
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
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
    for(i=0; i<16; i++) {                                 /* 16ĞĞ                         */
        ucChar = *pucMsk++;
        #ifdef __DISPLAY_BUFFER                           /* Ê¹ÓÃÏÔ´æÏÔÊ¾                 */
        for(j=0; j<8; j++) {                              /* 8ÁĞ                          */
            if((ucChar << j)& 0x80) {                     /* ÏÔÊ¾×ÖÄ£                     */
                DispBuf[320*(y0+i) + x0+j] = color;
            }
        }
        #else                                             /* Ö±½ÓÏÔÊ¾                     */
        bit = 0;
        //LCD_SetCursor(x0, y);                             /* ÉèÖÃĞ´Êı¾İµØÖ·Ö¸ÕE          */
		LCD_WriteRAM_Prepare();        					  /* ¿ªÊ¼Ğ´ÈERAM	    		  */
        for(j=0; j<8; j++) {                              /* 8ÁĞ                          */
            if((ucChar << j) & 0x80) {                    /* ÏÔÊ¾×ÖÄ£                     */
                //Write_Dat(color);
				//LCD_SetCursor(x0 + j, y);//ÉèÖÃ¹â±E»ÖÃ 
				//Write_Cmd(R34);//¿ªÊ¼Ğ´ÈERAM
				//Write_Dat(color);
				LCD_DrawPointColor(x0 + j, y, color);	
				//if(bit == 0) {
					//LCD_DrawPointColor(x0 + j, y, 0);
				//	bit = 1;
				//} else {		
				//LCD_DrawPointColor(x0 + j, y, color);	
				if(((ucChar << (j +1)) & 0x80) == 0 && j < 7) {
					if(color != 0) LCD_DrawPointColor(x0 + j + 1, y+1, 0);		//shadow
					bit = 0;
				}
				//}
			} else {
                //Write_Dat(BACK_COLOR);
				  //LCD_ReadDat();
				bit = 0;
            }
        }
        y++;
        #endif
    }
    return (8);                                           /* ·µ»Ø16Î»ÁĞ¿E                */
}
/*****************************************************************************
** º¯ÊıÃû³Æ: Num_power
** ¹¦ÄÜÃèÊE MµÄN´Î·½
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
uint32_t Num_power(uint8_t m,uint8_t n)
{
	uint32 result=1;	 
	while(n--)result*=m;    
	return result;
}
/*****************************************************************************
** º¯ÊıÃû³Æ: LCD_ShowNum
** ¹¦ÄÜÃèÊE ÔÚÖ¸¶¨Î»ÖÃÏÔÊ¾Ò»´®Êı×Ö
				num:ÊıÖµ(0~4294967295);
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/	 
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
/*****************************************************************************
** º¯ÊıÃû³Æ: LCD_WriteBMP
** ¹¦ÄÜÃèÊE ÔÚÖ¸¶¨µÄÎ»ÖÃÏÔÊ¾Ò»·ùÍ¼Æ¬
				XposºÍYposÎªÍ¼Æ¬ÏÔÊ¾µØÖ·£¬HeightºÍWidth ÎªÍ¼Æ¬µÄ¿úÒÈºÍ¸ß¶È
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
void LCD_WriteBMP(uint8_t Xpos, uint16_t Ypos, uint8_t Height, uint16_t Width, uint8_t *bitmap)
{
  	uint32_t index;
  	uint32_t size = Height * Width;
  	uint16_t *bitmap_ptr = (uint16_t *)bitmap;

  	LCD_SetDisplayWindow(Xpos, Ypos, Width-1, Height-1);

  	//LCD_WriteReg(0x03, 0x1038);	//Èç¹ûĞèÒªºáÏòÏÔÊ¾Í¼Æ¬£¬½«´ËÈ¥µôÆÁ±Î £¬Í¬Ê±½«WidthºÍHightµ÷»»Ò»ÏÂ¾Í¿ÉÒÔ

  	LCD_WriteRAM_Prepare();

  	for(index = 0; index < size; index++)
  	{
    	Write_Dat(*bitmap_ptr++);
  	}
	//»Ö¸´´°Ìå´óĞ¡	 
	//LCD_WriteReg(R80, 0x0000); //Ë®Æ½·½ÏòGRAMÆğÊ¼µØÖ·
	//LCD_WriteReg(R81, DISPLAY_HEIGHT - 1); //Ë®Æ½·½ÏòGRAM½áÊøµØÖ·
	//LCD_WriteReg(R82, 0x0000); //´¹Ö±·½ÏòGRAMÆğÊ¼µØÖ·
	//LCD_WriteReg(R83, DISPLAY_WIDTH - 1); //´¹Ö±·½ÏòGRAM½áÊøµØÖ·	
	LCD_SetDisplayWindow(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
}
/*****************************************************************************
** º¯ÊıÃû³Æ: LCD_DrawLine
** ¹¦ÄÜÃèÊE ÔÚÖ¸¶¨µÄÁ½²àÎ»ÖÃ»­Ò»ÌõÏß
				x1,y1:Æğµã×ø±EÈ  x2,y2:ÖÕµã×ø±E
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 

	delta_x=x2-x1; 				//¼ÆËã×ø±EöÁ¿ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; 		//ÉèÖÃµ¥²½·½ÏE
	else if(delta_x==0)incx=0;	//´¹Ö±Ïß 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;	//Ë®Æ½Ïß 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //Ñ¡È¡»ù±¾ÔöÁ¿×ø±EE
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )	//»­ÏßÊä³E
	{  
		//LCD_DrawPoint(uRow,uCol);//»­µE
		
		LCD_SetCursor(uRow,uCol);//ÉèÖÃ¹â±E»ÖÃ 
		Write_Cmd(R34);//¿ªÊ¼Ğ´ÈERAM
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
/*****************************************************************************
** º¯ÊıÃû³Æ: LCD_DrawLine
** ¹¦ÄÜÃèÊE ÔÚÖ¸¶¨Î»ÖÃ»­Ò»¸öÖ¸¶¨´óĞ¡µÄÔ²
				(x,y):ÖĞĞÄµE	 r    :°E¶
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
void Draw_Circle(uint8_t x0,uint16_t y0,uint8_t r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //ÅĞ¶ÏÏÂ¸öµãÎ»ÖÃµÄ±E¾
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
		//Ê¹ÓÃBresenhamËã·¨»­Ô²     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 
		LCD_DrawPoint(x0+a,y0+b);
	}
} 
/*****************************************************************************
** º¯ÊıÃû³Æ: LCD_Fill
** ¹¦ÄÜÃèÊE ÔÚÖ¸¶¨ÇøÓòÄÚÌûÏäÖ¸¶¨ÑÕÉ«
				ÇøÓò´óĞ¡:  (xend-xsta)*(yend-ysta) 
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
void LCD_Fill(uint8_t xsta,uint16_t ysta,uint8_t xend,uint16_t yend,uint16_t color)
{                    
    uint32_t n;
	//ÉèÖÃ´°¿Ú										
	LCD_WriteReg(R80, xsta); //Ë®Æ½·½ÏòGRAMÆğÊ¼µØÖ·
	LCD_WriteReg(R81, xend); //Ë®Æ½·½ÏòGRAM½áÊøµØÖ·
	LCD_WriteReg(R82, ysta); //´¹Ö±·½ÏòGRAMÆğÊ¼µØÖ·
	LCD_WriteReg(R83, yend); //´¹Ö±·½ÏòGRAM½áÊøµØÖ·	
	LCD_SetCursor(xsta,ysta);//ÉèÖÃ¹â±E»ÖÃ  
	LCD_WriteRAM_Prepare();  //¿ªÊ¼Ğ´ÈERAM	 	   	   
	n=(uint32)(yend-ysta+1)*(xend-xsta+1);    
	while(n--){Write_Dat(color);}//ÏÔÊ¾ËùÌûÏäµÄÑÕÉ«. 
	//»Ö¸´ÉèÖÃ
	//LCD_WriteReg(R80, 0x0000); //Ë®Æ½·½ÏòGRAMÆğÊ¼µØÖ·
	//LCD_WriteReg(R81, DISPLAY_HEIGHT - 1); //Ë®Æ½·½ÏòGRAM½áÊøµØÖ·
	//LCD_WriteReg(R82, 0x0000); //´¹Ö±·½ÏòGRAMÆğÊ¼µØÖ·
	//LCD_WriteReg(R83, DISPLAY_WIDTH - 1); //´¹Ö±·½ÏòGRAM½áÊøµØÖ·	    
	
	//LCD_SetDisplayWindow(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
}
/*****************************************************************************
** º¯ÊıÃû³Æ: LCD_Delay
** ¹¦ÄÜÃèÊE ÓÃÓÚLCDÇı¶¯ÑÓÊ±
** ×E ¡¡Õß: Dream
** ÈÕ¡¡  ÆÚ: 2010ÄE2ÔÂ06ÈÕ
*****************************************************************************/
#if SHARD_RTOS_ENABLED == 0
void LCD_Delay (uint32_t nCount)
{
	__IO uint16_t i;	 	
	for (i=0;i<nCount*100;i++);
}
#endif


void LCD_Switch_Orientation(uint8_t mode) {
	Write_Cmd(ILI9341_MADCTL);    // Memory Access Control 
	mode = mode & 0x03;
	if(mode == 0) {
		Write_Dat(0xE8);
	} else if(mode == 1)  {
		Write_Dat(0x48);
	}  else if(mode == 2)  {
		Write_Dat(0x38);
	}  else if(mode == 3)  {
		Write_Dat(0x88);
	} 
}
/*********************************************************************************************************
** End of File
*********************************************************************************************************/

#endif
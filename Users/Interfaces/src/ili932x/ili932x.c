/**************************************************************
** 	ª≈£ø™∑¢∞ÅE
**	LCD«˝∂Ø¥˙¬ÅE
**  ∞Ê±æ£∫V1.0  
**	¬€Ã≥£∫www.openmcu.com
**	Ã‘±¶£∫http://shop36995246.taobao.com/   
**  ºº ı÷ß≥÷»∫£∫121939788 
***************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "..\..\defs.h"
#include "..\..\config.h"
#if (SHARD_LCD_TYPE & 0xFFF0) == 0x9320
#include "..\..\inc\ili932x\ili932x.h"
#include "..\..\inc\ili932x\fonts.h"	 //◊÷ø‚Œƒº˛øÅE
#include "..\..\inc\ili932x\hz16.h"


uint16_t POINT_COLOR = BLUE, BACK_COLOR = WHITE;  /* ∑÷±…Ë÷√µ„µƒ—’…´∫Õµ◊…´	*/

/*****************************************************************************
** ∫Ø ˝√˚≥∆: LCD_Write_Reg
** π¶ƒ‹√Ë ÅE –¥÷∏¡˚ÿ∞ ˝æ›
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_Dat)
{
	Write_Cmd(LCD_Reg);
	Write_Dat(LCD_Dat);
}
/*****************************************************************************
** ∫Ø ˝√˚≥∆: Write_Cmd
** π¶ƒ‹√Ë ÅE –¥÷∏¡ÅE
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
void Write_Cmd(uint16_t LCD_Reg)
{
	LCD_CS = 0;
	LCD_RS = 0;
	GPIOC->ODR = (GPIOC->ODR&0xff00)|(LCD_Reg&0x00ff);
	GPIOB->ODR = (GPIOB->ODR&0x00ff)|(LCD_Reg&0xff00); 
	LCD_WR = 0;
	LCD_WR = 1;	
	LCD_CS = 1;
}
/*****************************************************************************
** ∫Ø ˝√˚≥∆: Write_Dat
** π¶ƒ‹√Ë ÅE –¥ ˝æ›
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
void Write_Dat(uint16_t LCD_Dat)
{
	LCD_CS = 0;
	LCD_RS = 1;
	GPIOC->ODR = (GPIOC->ODR&0xff00)|(LCD_Dat&0x00ff);
	GPIOB->ODR = (GPIOB->ODR&0x00ff)|(LCD_Dat&0xff00);
	LCD_WR = 0;
	LCD_WR = 1;	
	LCD_CS = 1;
}
/*****************************************************************************
** ∫Ø ˝√˚≥∆: LCD_ReadReg
** π¶ƒ‹√Ë ÅE ∂¡÷∏¡ÅE
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	uint16_t temp;
	Write_Cmd(LCD_Reg);  //–¥»ÅE™∂¡µƒºƒ¥Ê∆˜∫≈  

	//GPIOB->CRH = (GPIOB->CRH & 0x00000000) | 0x44444444; //Ω´∂Àø⁄∏ﬂ8Œª≈‰÷√≥… ‰»
	GPIO_InitStructure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;					//pin 2
  	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;			//peripheral (alternate function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;	
	GPIO_InitStructure.Pull = GPIO_NOPULL;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	//GPIOC->CRL = (GPIOC->CRL & 0x00000000) | 0x44444444; //Ω´∂Àø⁄µÕ8Œª≈‰÷√≥… ‰»ÅE 
	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;					//pin 2
  	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;			//peripheral (alternate function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;	
	GPIO_InitStructure.Pull = GPIO_NOPULL;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);	
	
	LCD_CS = 0;
	LCD_RS = 1;
	LCD_RD = 0;
	temp = ((GPIOB->IDR&0xff00)|(GPIOC->IDR&0x00ff)); //∂¡»° ˝æ›(∂¡ºƒ¥Ê∆˜ ±,≤¢≤ª–Ë“™∂¡2¥Œ) 	
	LCD_RD = 1;
	LCD_CS = 1; 
	//GPIOB->CRH = (GPIOB->CRH & 0x00000000) | 0x33333333; // Õ∑≈∂Àø⁄∏ﬂ8ŒªŒ™ ‰≥ÅE
	//GPIOC->CRL = (GPIOC->CRL & 0x00000000) | 0x33333333; // Õ∑≈∂Àø⁄µÕ8ŒªŒ™ ‰≥ÅE

	GPIO_InitStructure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;					//pin 2
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (output function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	//GPIOC->CRL = (GPIOC->CRL & 0x00000000) | 0x44444444; //Ω´∂Àø⁄µÕ8Œª≈‰÷√≥… ‰»ÅE 
	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;					//pin 2
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (output function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);	
	return temp;   
}
/*****************************************************************************
** ∫Ø ˝√˚≥∆: LCD_ReadDat
** π¶ƒ‹√Ë ÅE ∂¡ ˝æ›
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
uint16_t LCD_ReadDat()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	uint16_t temp;

	//GPIOE->CRH = (GPIOE->CRH & 0x00000000) | 0x44444444;  //Ω´∂Àø⁄∏ﬂ8Œª≈‰÷√≥… ‰»ÅE
	//GPIOE->CRL = (GPIOE->CRL & 0x00000000) | 0x44444444;  //Ω´∂Àø⁄µÕ8Œª≈‰÷√≥… ‰»ÅE
	
	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | 
										GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;	
  	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;			//peripheral (alternate function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;	
	GPIO_InitStructure.Pull = GPIO_NOPULL;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);	
	
	LCD_CS = 0;
	LCD_RS = 1;
	LCD_RD = 0;
	temp = ((GPIOB->IDR&0xff00)|(GPIOC->IDR&0x00ff));	//∂¡»° ˝æ›(∂¡ºƒ¥Ê∆˜ ±,≤¢≤ª–Ë“™∂¡2¥Œ)
	LCD_RD = 1;
	LCD_CS = 1;
	//GPIOE->CRH = (GPIOE->CRH & 0x00000000) | 0x33333333;  // Õ∑≈∂Àø⁄∏ﬂ8ŒªŒ™ ‰≥ÅE
	//GPIOE->CRL = (GPIOE->CRL & 0x00000000) | 0x33333333;  // Õ∑≈∂Àø⁄µÕ8ŒªŒ™ ‰≥ÅE
	
	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | 
										GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;	
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (output function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);	

	return temp;   
}
/*****************************************************************************
** ∫Ø ˝√˚≥∆: LCD_Configuration
** π¶ƒ‹√Ë ÅE LCD_IOø⁄≈‰÷√
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
void LCD_Configuration()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC,ENABLE);
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	/* ≈‰÷√ ˝æ›IO ¡¨Ω”µΩGPIOB *********************/	
	GPIO_InitStructure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 
								| GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (output function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* ≈‰÷√øÿ÷∆IO ¡¨Ω”µΩPD12.PD13.PD14.PD15 *********************/	
	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 
								| GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 
								| GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (output function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void LCD_SetBacklight(uint8_t mode) {
	if(mode) HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
}
/*****************************************************************************
** ∫Ø ˝√˚≥∆: LCD_Init
** π¶ƒ‹√Ë ÅE LCD≥ı ºªØ
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
void LCD_Init(void)
{
	static uint16_t DeviceCode;
	LCD_Configuration();
	LCD_WriteReg(0x0000,0x0001);
	LCD_Delay(5); // LCD_Delay 50 ms 
	DeviceCode = LCD_ReadReg(0x0000);   
	//printf(" ID=0x%x\n",DeviceCode); 
	if(DeviceCode==0x9325||DeviceCode==0x9328)//ILI9325
	{
 		LCD_WriteReg(0x00e5,0x78F0);      
//        LCD_WriteReg(0x0000,0x0001);
        LCD_WriteReg(0x0001,0x0100);     
        LCD_WriteReg(0x0002,0x0700);                   
        LCD_WriteReg(0x0003,0x1038); 		//entry mode
				
        LCD_WriteReg(0x0004,0x0000);                                   
        LCD_WriteReg(0x0008,0x0202);	           
        LCD_WriteReg(0x0009,0x0000);         
        LCD_WriteReg(0x000a,0x0000);         
        LCD_WriteReg(0x000c,0x0001);         
        LCD_WriteReg(0x000d,0x0000);          
        LCD_WriteReg(0x000f,0x0000);
//Power On sequence //
        LCD_WriteReg(0x0010,0x0000);   
        LCD_WriteReg(0x0011,0x0007);
        LCD_WriteReg(0x0012,0x0000);                                                                 
        LCD_WriteReg(0x0013,0x0000);
        LCD_WriteReg(0x0007,0x0001);          
        LCD_Delay(5); 
        LCD_WriteReg(0x0010,0x1690);   
        LCD_WriteReg(0x0011,0x0227);
        LCD_Delay(5); 
        LCD_WriteReg(0x0012,0x009d);                   
        LCD_Delay(5); 
        LCD_WriteReg(0x0013,0x1900);   
        LCD_WriteReg(0x0029,0x0025);
        LCD_WriteReg(0x002b,0x000d);
        LCD_Delay(5); 
        LCD_WriteReg(0x0020,0x0000);                                                            
        LCD_WriteReg(0x0021,0x0000);           
		LCD_Delay(5); 
		//Ÿ§¬˙Ï£’˝
        LCD_WriteReg(0x0030,0x0007); 
        LCD_WriteReg(0x0031,0x0303);   
        LCD_WriteReg(0x0032,0x0003);
        LCD_WriteReg(0x0035,0x0206);
        LCD_WriteReg(0x0036,0x0008); 
        LCD_WriteReg(0x0037,0x0406);
        LCD_WriteReg(0x0038,0x0304);        
        LCD_WriteReg(0x0039,0x0007);     
        LCD_WriteReg(0x003c,0x0602);
        LCD_WriteReg(0x003d,0x0008);
        LCD_Delay(5); 
        LCD_WriteReg(0x0050,0);
        LCD_WriteReg(0x0051,239);                   
        LCD_WriteReg(0x0052,0x0000);                   
        LCD_WriteReg(0x0053,319); 
        
        LCD_WriteReg(0x0060,0x2700);        
        LCD_WriteReg(0x0061,0x0001); 
        LCD_WriteReg(0x006a,0x0000);
        LCD_WriteReg(0x0080,0x0000);
        LCD_WriteReg(0x0081,0x0000);
        LCD_WriteReg(0x0082,0x0000);
        LCD_WriteReg(0x0083,0x0000);
        LCD_WriteReg(0x0084,0x0000);
        LCD_WriteReg(0x0085,0x0000);
      
        LCD_WriteReg(0x0090,0x0010);     
        LCD_WriteReg(0x0092,0x0600);  
		   
        LCD_WriteReg(0x0007,0x0133); 
	}
	else if(DeviceCode==0x9320||DeviceCode==0x9300)
	{
		LCD_WriteReg(0x00,0x0000);
		LCD_WriteReg(0x01,0x0100);	//Driver Output Contral.
		LCD_WriteReg(0x02,0x0700);	//LCD Driver Waveform Contral.
		LCD_WriteReg(0x03,0x1030);//Entry Mode Set.
		//LCD_WriteReg(0x03,0x1018);	//Entry Mode Set.
	
		LCD_WriteReg(0x04,0x0000);	//Scalling Contral.
		LCD_WriteReg(0x08,0x0202);	//Display Contral 2.(0x0207)
		LCD_WriteReg(0x09,0x0000);	//Display Contral 3.(0x0000)
		LCD_WriteReg(0x0a,0x0000);	//Frame Cycle Contal.(0x0000)
		LCD_WriteReg(0x0c,(1<<0));	//Extern Display Interface Contral 1.(0x0000)
		LCD_WriteReg(0x0d,0x0000);	//Frame Maker Position.
		LCD_WriteReg(0x0f,0x0000);	//Extern Display Interface Contral 2.	    
		LCD_Delay(10); 
		LCD_WriteReg(0x07,0x0101);	//Display Contral.
		LCD_Delay(10); 								  
		LCD_WriteReg(0x10,(1<<12)|(0<<8)|(1<<7)|(1<<6)|(0<<4));	//Power Control 1.(0x16b0)
		LCD_WriteReg(0x11,0x0007);								//Power Control 2.(0x0001)
		LCD_WriteReg(0x12,(1<<8)|(1<<4)|(0<<0));				//Power Control 3.(0x0138)
		LCD_WriteReg(0x13,0x0b00);								//Power Control 4.
		LCD_WriteReg(0x29,0x0000);								//Power Control 7.
	
		LCD_WriteReg(0x2b,(1<<14)|(1<<4));	    
		LCD_WriteReg(0x50,0);	//Set X Star
		//ÀÆ∆ΩGRAM÷’÷πŒª÷√Set X End.
		LCD_WriteReg(0x51,239);	//Set Y Star
		LCD_WriteReg(0x52,0);	//Set Y End.t.
		LCD_WriteReg(0x53,319);	//
	
		LCD_WriteReg(0x60,0x1D00);	//Driver Output Control.	//240 lines
		LCD_WriteReg(0x61,0x0001);	//Driver Output Control.
		LCD_WriteReg(0x6a,0x0000);	//Vertical Srcoll Control.
	
		LCD_WriteReg(0x80,0x0000);	//Display Position? Partial Display 1.
		LCD_WriteReg(0x81,0x0000);	//RAM Address Start? Partial Display 1.
		LCD_WriteReg(0x82,0x0000);	//RAM Address End-Partial Display 1.
		LCD_WriteReg(0x83,0x0000);	//Displsy Position? Partial Display 2.
		LCD_WriteReg(0x84,0x0000);	//RAM Address Start? Partial Display 2.
		LCD_WriteReg(0x85,0x0000);	//RAM Address End? Partial Display 2.
	
		LCD_WriteReg(0x90,(0<<7)|(16<<0));	//Frame Cycle Contral.(0x0013)
		LCD_WriteReg(0x92,0x0000);	//Panel Interface Contral 2.(0x0000)
		LCD_WriteReg(0x93,0x0001);	//Panel Interface Contral 3.
		LCD_WriteReg(0x95,0x0110);	//Frame Cycle Contral.(0x0110)
		LCD_WriteReg(0x97,(0<<8));	//
		LCD_WriteReg(0x98,0x0000);	//Frame Cycle Contral.	   
		LCD_WriteReg(0x07,0x0173);	//(0x0173)
		LCD_Delay(10);
	}
	//LCD_Clear(BACK_COLOR);	
}

void LCD_DisplayOff() {
	
	LCD_WriteReg(0x0007,0x0031); 
	LCD_Delay(10); 
	LCD_WriteReg(0x0007,0x0030); 
	LCD_Delay(10); 
	LCD_WriteReg(0x0010,0x0000);
	LCD_WriteReg(0x0012,0x0000); 
}
/*****************************************************************************
** ∫Ø ˝√˚≥∆: LCD_DrawPoint
** π¶ƒ‹√Ë ÅE –¥“ª∏ˆµÅE
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
void LCD_DrawPoint(uint16_t x,uint16_t y)
{
	LCD_SetCursor(x,y);//…Ë÷√π‚±ÅEª÷√ 
	Write_Cmd(R34);//ø™ º–¥»ÅERAM
	Write_Dat(POINT_COLOR); 	
}

void LCD_DrawPointColor(uint16_t x,uint16_t y, uint16_t color)
{
	LCD_SetCursor(x,y);//…Ë÷√π‚±ÅEª÷√ 
	Write_Cmd(R34);//ø™ º–¥»ÅERAM
	Write_Dat(POINT_COLOR); 	
}
/*****************************************************************************
** ∫Ø ˝√˚≥∆: LCD_WriteRAM_Prepare
** π¶ƒ‹√Ë ÅE –©◊º±∏
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
void LCD_WriteRAM_Prepare()
{
	Write_Cmd(R34);	
}
/*****************************************************************************
** ∫Ø ˝√˚≥∆: LCD_SetCursor
** π¶ƒ‹√Ë ÅE …Ë÷√π‚±ÅEØ ˝
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
	LCD_WriteReg(R32, Ypos);
	LCD_WriteReg(R33, Xpos);	
} 
/*****************************************************************************
** ∫Ø ˝√˚≥∆: LCD_SetDisplayWindow
** π¶ƒ‹√Ë ÅE …Ë÷√¥∞ø⁄∫Ø ˝ (Ω´Width∫ÕHeightµ˜ªª“ªœ¬£¨«∞Ã· «)
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
void LCD_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width)
{
  	LCD_WriteReg(R80, Ypos);	   	   	//ÀÆ∆Ω∑ΩœÚGRAM∆ ºµÿ÷∑
  	LCD_WriteReg(R81, Ypos+Width); 	//ÀÆ∆Ω∑ΩœÚGRAMΩ· ¯µÿ÷∑ 
  	LCD_WriteReg(R82, Xpos);		  	//¥π÷±∑ΩœÚGRAM∆ ºµÿ÷∑
  	LCD_WriteReg(R83, Xpos+Height);  	//¥π÷±∑ΩœÚGRAMΩ· ¯µÿ÷∑

  	LCD_SetCursor(Xpos, Ypos);			//…Ë÷√π‚±ÅEª÷√
}
/*****************************************************************************
** ∫Ø ˝√˚≥∆: LCD_ShowString
** π¶ƒ‹√Ë ÅE œ‘ æ◊÷∑˚¥Æ∫Ø ˝
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
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
** ∫Ø ˝√˚≥∆: LCD_ShowChar
** π¶ƒ‹√Ë ÅE œ‘ æ“ª∏ˆ◊÷∑˚∫Ø ˝
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
void LCD_ShowChar(uint8_t x,uint16_t y,uint8_t chars,uint8_t size,uint8_t mode)
{
	uint8_t temp;
    uint8_t pos,t;      
    if(x>MAX_CHAR_POSX||y>MAX_CHAR_POSY) return;	    
											
	LCD_SetDisplayWindow(x,y,(size/2-1),size-1);  //…Ë÷√¥∞ø⁄

	LCD_WriteRAM_Prepare();        //ø™ º–¥»ÅERAM	   
	if(!mode) 						//∑«µ˛º”∑Ω Ω
	{
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=ASCII_1206[chars-0x20][pos];//µ˜”√1206◊÷ÃÅE
			else temp=ASCII_1608[chars-0x20][pos];		 //µ˜”√1608◊÷ÃÅE
			for(t=0;t<size/2;t++)
		    {     
		        //if(temp&0x01)            	 			//œ»¥´µÕŒª£¨»°ƒ£”–πÿœµ
		        if((temp<<t)&0x80)						//œ»¥´∏ﬂŒª
				{
					Write_Dat(RED);
				}
				else 
				{
					Write_Dat(WHITE);	        
		        }
				//temp>>=1; 	   							//»Áπ˚œ»¥´µÕŒª£¨»•µÙ∆¡±Œœﬂ
		    }
		}	
	}
	else//µ˛º”∑Ω Ω
	{
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=ASCII_1206[chars-0x20][pos];	//µ˜”√1206◊÷ÃÅE
			else temp=ASCII_1608[chars-0x20][pos];		 	//µ˜”√1608◊÷ÃÅE
			for(t=0;t<size/2;t++)
		    {                 
		        if((temp<<t)&0x80)LCD_DrawPoint(x+t,y+pos);	//ª≠“ª∏ˆµÅE    
		        //temp>>=1; 								//»Áπ˚œ»¥´µÕŒª£¨»•µÙ∆¡±Œœﬂ
		    }
		}
	}	    
	/* ª÷∏¥¥∞ÃÂ¥Û–°	*/
	LCD_WriteReg(R80, 0x0000); //ÀÆ∆Ω∑ΩœÚGRAM∆ ºµÿ÷∑
	LCD_WriteReg(R81, DISPLAY_HEIGHT - 1); //ÀÆ∆Ω∑ΩœÚGRAMΩ· ¯µÿ÷∑
	LCD_WriteReg(R82, 0x0000); //¥π÷±∑ΩœÚGRAM∆ ºµÿ÷∑
	LCD_WriteReg(R83, DISPLAY_WIDTH - 1); //¥π÷±∑ΩœÚGRAMΩ· ¯µÿ÷∑	
}
/*****************************************************************************
** ∫Ø ˝√˚≥∆: LCD_Clear
** π¶ƒ‹√Ë ÅE «Â∆¡ƒª∫Ø ˝
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
void LCD_Clear(uint16_t Color)
{
	uint32_t index=0;      
	LCD_SetCursor(0x00,0x0000); //…Ë÷√π‚±ÅEª÷√ 
	LCD_WriteRAM_Prepare();     //ø™ º–¥»ÅERAM	 	  
	for(index=0;index<DISPLAY_SIZE;index++)
	{
		Write_Dat(Color);    
	}
}
/*****************************************************************************
** ∫Ø ˝√˚≥∆: WriteString
** π¶ƒ‹√Ë ÅE ‘⁄÷∏∂®Œª÷√ø™ ºœ‘ æ“ª∏ˆ◊÷∑˚¥Æ∫Õ“ª¥Æ∫∫◊÷
				÷ß≥÷◊‘∂Øªª––
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
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
            break;                                     /* ◊÷∑˚¥ÆΩ· ÅE           */
        }      
        x0 = x0 + (usWidth);                           /* µ˜Ω⁄◊÷∑˚¥Æœ‘ æÀ…ΩÙ∂»         */
        if(*pcStr > 0x80)                              /* ≈–∂œŒ™∫∫◊÷                   */
        {
		    if((x0 + 16) > LCD_W)                      /* ºÅEÈ £”‡ø’º‰ «∑Ò◊„πª         */
            {
			    x0 = 0;
                y0 = y0 + 16;                          /* ∏ƒ±‰œ‘ æ◊¯±ÅE                */
                if(y0 > LCD_H)                         /* ◊›◊¯±ÅE¨≥ÅE                  */
                {
				    y0 = 0;
                }
            }
            usIndex = findHzIndex(pcStr);
            usWidth = WriteOneHzChar((uint8_t *)&(ptGb16[usIndex].Msk[0]), x0, y0, color);
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
			    y0 = y0 + 16;                           /* ∏ƒ±‰œ‘ æ◊¯±ÅE                */
                if(y0 > LCD_H)                          /* ◊›◊¯±ÅE¨≥ÅE                  */
                {
				    y0 = 0;
                }
                pcStr++;
                usWidth = 0;
                continue;
            } 
			else 
			{
                if((x0 + 8) > LCD_W)                     /* ºÅEÈ £”‡ø’º‰ «∑Ò◊„πª         */
                {
				    x0 = 0;
                    y0 = y0 + 16;                        /* ∏ƒ±‰œ‘ æ◊¯±ÅE                */
                    if(y0 > LCD_H)                       /* ◊›◊¯±ÅE¨≥ÅE                  */
                    { 
					    y0 = 0;
                    }
                }
				usWidth = WriteOneASCII((uint8_t *)(g_font_large + (*pcStr * 0x10)), x0, y0, color);
                pcStr += 1;
            }
		}
	}												  	  
}
				
				
/*****************************************************************************
** ∫Ø ˝√˚≥∆: WriteOneHzChar
** π¶ƒ‹√Ë ÅE œ‘ æ“ª∏ˆ÷∏∂®¥Û–°µƒ∫∫◊÷
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
uint16_t WriteOneHzChar(uint8_t *pucMsk,
                               uint16_t x0,
                               uint16_t y0,
                               uint16_t color)
{
    uint16_t i,j;
    uint16_t mod[16];                                      /* µ±«∞◊÷ƒ£                     */
    uint16_t *pusMsk;                                      /* µ±«∞◊÷ø‚µÿ÷∑                 */
    uint16_t y;
    
    pusMsk = (uint16_t *)pucMsk;
    for(i=0; i<16; i++)                                    /* ±£¥Êµ±«∞∫∫◊÷µ„’Û Ω◊÷ƒ£       */
    {
        mod[i] = *pusMsk++;                                /* »°µ√µ±«∞◊÷ƒ££¨∞ÅE÷∂‘∆ÅE√Œ    */
        mod[i] = ((mod[i] & 0xff00) >> 8) | ((mod[i] & 0x00ff) << 8);/* ◊÷ƒ£Ωªªª∏ﬂµÕ◊÷Ω⁄£®Œ™¡Àœ‘ æ   */
                                                           /* –Ë“™£©                       */
    }
    y = y0;
    for(i=0; i<16; i++)                                    /* 16––                         */
    { 
	    #ifdef __DISPLAY_BUFFER                            /*  π”√œ‘¥Êœ‘ æ                 */
        for(j=0; j<16; j++)                                /* 16¡–                         */
        {
		    if((mod[i] << j)& 0x8000)                      /* œ‘ æ◊÷ƒ£                     */
            {
			    DispBuf[320*(y0+i) + x0+j] = color;
            }
        }
        #else                                              /* ÷±Ω”œ‘ æ                     */
        
		LCD_SetCursor(x0, y);                              /* …Ë÷√–¥ ˝æ›µÿ÷∑÷∏’ÅE          */
		LCD_WriteRAM_Prepare();        					   /*ø™ º–¥»ÅERAM	*/   
        for(j=0; j<16; j++)                                /* 16¡–                         */
        {
		    if((mod[i] << j) & 0x8000)                     /* œ‘ æ◊÷ƒ£                     */
            { 
			    Write_Dat(color);
            } 
			else 
			{
                //Write_Dat(BACK_COLOR);                     /* ”√∂¡∑Ω ΩÃ¯π˝–¥ø’∞◊µ„µƒœÒÀÿ   */
            	//LCD_ReadDat();
			}
        }
        y++;
        #endif
    }
    return (16);                                          /* ∑µªÿ16Œª¡–øÅE                */
}
/*****************************************************************************
** ∫Ø ˝√˚≥∆: WriteOneASCII
** π¶ƒ‹√Ë ÅE œ‘ æ“ª∏ˆ÷∏∂®¥Û–°µƒ◊÷∑ÅE
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
uint16_t WriteOneASCII(uint8_t *pucMsk,
                              uint16_t x0,
                              uint16_t y0,
                              uint16_t color)
{
    uint16_t i,j;
    uint16_t y;
    uint8_t ucChar;
    
    y = y0;
    for(i=0; i<16; i++) {                                 /* 16––                         */
        ucChar = *pucMsk++;
        #ifdef __DISPLAY_BUFFER                           /*  π”√œ‘¥Êœ‘ æ                 */
        for(j=0; j<8; j++) {                              /* 8¡–                          */
            if((ucChar << j)& 0x80) {                     /* œ‘ æ◊÷ƒ£                     */
                DispBuf[320*(y0+i) + x0+j] = color;
            }
        }
        #else                                             /* ÷±Ω”œ‘ æ                     */
        
        //LCD_SetCursor(x0, y);                             /* …Ë÷√–¥ ˝æ›µÿ÷∑÷∏’ÅE          */
		LCD_WriteRAM_Prepare();        					  /* ø™ º–¥»ÅERAM	    		  */
        for(j=0; j<8; j++) {                              /* 8¡–                          */
            if((ucChar << j) & 0x80) {                    /* œ‘ æ◊÷ƒ£                     */
                //Write_Dat(color);
				LCD_SetCursor(x0 + j, y);//…Ë÷√π‚±ÅEª÷√ 
				Write_Cmd(R34);//ø™ º–¥»ÅERAM
				Write_Dat(color); 	
			} else {
                //Write_Dat(BACK_COLOR);
				  //LCD_ReadDat();
            }
        }
        y++;
        #endif
    }
    return (8);                                           /* ∑µªÿ16Œª¡–øÅE                */
}
/*****************************************************************************
** ∫Ø ˝√˚≥∆: Num_power
** π¶ƒ‹√Ë ÅE MµƒN¥Œ∑Ω
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
uint32_t Num_power(uint8_t m,uint8_t n)
{
	uint32 result=1;	 
	while(n--)result*=m;    
	return result;
}
/*****************************************************************************
** ∫Ø ˝√˚≥∆: LCD_ShowNum
** π¶ƒ‹√Ë ÅE ‘⁄÷∏∂®Œª÷√œ‘ æ“ª¥Æ ˝◊÷
				num: ˝÷µ(0~4294967295);
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
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
** ∫Ø ˝√˚≥∆: LCD_WriteBMP
** π¶ƒ‹√Ë ÅE ‘⁄÷∏∂®µƒŒª÷√œ‘ æ“ª∑˘Õº∆¨
				Xpos∫ÕYposŒ™Õº∆¨œ‘ æµÿ÷∑£¨Height∫ÕWidth Œ™Õº∆¨µƒø˙“»∫Õ∏ﬂ∂»
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
void LCD_WriteBMP(uint8_t Xpos, uint16_t Ypos, uint8_t Height, uint16_t Width, uint8_t *bitmap)
{
  	uint32_t index;
  	uint32_t size = Height * Width;
  	uint16_t *bitmap_ptr = (uint16_t *)bitmap;

  	LCD_SetDisplayWindow(Xpos, Ypos, Width-1, Height-1);

  	//LCD_WriteReg(0x03, 0x1038);	//»Áπ˚–Ë“™∫·œÚœ‘ æÕº∆¨£¨Ω´¥À»•µÙ∆¡±Œ £¨Õ¨ ±Ω´Width∫ÕHightµ˜ªª“ªœ¬æÕø…“‘

  	LCD_WriteRAM_Prepare();

  	for(index = 0; index < size; index++)
  	{
    	Write_Dat(*bitmap_ptr++);
  	}
	//ª÷∏¥¥∞ÃÂ¥Û–°	 
	LCD_WriteReg(R80, 0x0000); //ÀÆ∆Ω∑ΩœÚGRAM∆ ºµÿ÷∑
	LCD_WriteReg(R81, DISPLAY_HEIGHT - 1); //ÀÆ∆Ω∑ΩœÚGRAMΩ· ¯µÿ÷∑
	LCD_WriteReg(R82, 0x0000); //¥π÷±∑ΩœÚGRAM∆ ºµÿ÷∑
	LCD_WriteReg(R83, DISPLAY_WIDTH - 1); //¥π÷±∑ΩœÚGRAMΩ· ¯µÿ÷∑	
}
/*****************************************************************************
** ∫Ø ˝√˚≥∆: LCD_DrawLine
** π¶ƒ‹√Ë ÅE ‘⁄÷∏∂®µƒ¡Ω≤‡Œª÷√ª≠“ªÃıœﬂ
				x1,y1:∆µ„◊¯±ÅE»  x2,y2:÷’µ„◊¯±ÅE
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 

	delta_x=x2-x1; 				//º∆À„◊¯±ÅEˆ¡ø 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; 		//…Ë÷√µ•≤Ω∑ΩœÅE
	else if(delta_x==0)incx=0;	//¥π÷±œﬂ 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;	//ÀÆ∆Ωœﬂ 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //—°»°ª˘±æ‘ˆ¡ø◊¯±ÅEÅE
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )	//ª≠œﬂ ‰≥ÅE
	{  
		//LCD_DrawPoint(uRow,uCol);//ª≠µÅE
		
		LCD_SetCursor(uRow,uCol);//…Ë÷√π‚±ÅEª÷√ 
		Write_Cmd(R34);//ø™ º–¥»ÅERAM
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
** ∫Ø ˝√˚≥∆: LCD_DrawLine
** π¶ƒ‹√Ë ÅE ‘⁄÷∏∂®Œª÷√ª≠“ª∏ˆ÷∏∂®¥Û–°µƒ‘≤
				(x,y):÷––ƒµÅE	 r    :∞ÅE∂
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
void Draw_Circle(uint8_t x0,uint16_t y0,uint8_t r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //≈–∂œœ¬∏ˆµ„Œª÷√µƒ±ÅEæ
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
		// π”√BresenhamÀ„∑®ª≠‘≤     
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
** ∫Ø ˝√˚≥∆: LCD_Fill
** π¶ƒ‹√Ë ÅE ‘⁄÷∏∂®«¯”Úƒ⁄Ã˚œ‰÷∏∂®—’…´
				«¯”Ú¥Û–°:  (xend-xsta)*(yend-ysta) 
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
void LCD_Fill(uint8_t xsta,uint16_t ysta,uint8_t xend,uint16_t yend,uint16_t color)
{                    
    uint32_t n;
	//…Ë÷√¥∞ø⁄										
	LCD_WriteReg(R80, xsta); //ÀÆ∆Ω∑ΩœÚGRAM∆ ºµÿ÷∑
	LCD_WriteReg(R81, xend); //ÀÆ∆Ω∑ΩœÚGRAMΩ· ¯µÿ÷∑
	LCD_WriteReg(R82, ysta); //¥π÷±∑ΩœÚGRAM∆ ºµÿ÷∑
	LCD_WriteReg(R83, yend); //¥π÷±∑ΩœÚGRAMΩ· ¯µÿ÷∑	
	LCD_SetCursor(xsta,ysta);//…Ë÷√π‚±ÅEª÷√  
	LCD_WriteRAM_Prepare();  //ø™ º–¥»ÅERAM	 	   	   
	n=(uint32)(yend-ysta+1)*(xend-xsta+1);    
	while(n--){Write_Dat(color);}//œ‘ æÀ˘Ã˚œ‰µƒ—’…´. 
	//ª÷∏¥…Ë÷√
	LCD_WriteReg(R80, 0x0000); //ÀÆ∆Ω∑ΩœÚGRAM∆ ºµÿ÷∑
	LCD_WriteReg(R81, DISPLAY_HEIGHT - 1); //ÀÆ∆Ω∑ΩœÚGRAMΩ· ¯µÿ÷∑
	LCD_WriteReg(R82, 0x0000); //¥π÷±∑ΩœÚGRAM∆ ºµÿ÷∑
	LCD_WriteReg(R83, DISPLAY_WIDTH - 1); //¥π÷±∑ΩœÚGRAMΩ· ¯µÿ÷∑	    
}
/*****************************************************************************
** ∫Ø ˝√˚≥∆: LCD_Delay
** π¶ƒ‹√Ë ÅE ”√”⁄LCD«˝∂Ø—” ±
** ◊ÅE °°’ﬂ: Dream
** »’°°  ∆⁄: 2010ƒÅE2‘¬06»’
*****************************************************************************/
#if SHARD_RTOS_ENABLED == 0
void LCD_Delay (uint32_t nCount)
{
	__IO uint16_t i;	 	
	for (i=0;i<nCount*100;i++);
}
#endif
/*********************************************************************************************************
** End of File
*********************************************************************************************************/
#endif
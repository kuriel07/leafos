/**************************************************************
** 	火牛开发皝E
**	LCD驱动代聛E
**  版本：V1.0  
**	论坛：www.openmcu.com
**	淘宝：http://shop36995246.taobao.com/   
**  技术支持群：121939788 
***************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "..\..\defs.h"
#include "..\..\config.h"
#if (SHARD_LCD_TYPE & 0xFFF0) == 0x9340
#include "..\..\inc\ili932x\ili932x.h"
#include "..\..\inc\ili932x\fonts.h"	 //字库文件縼E
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

uint16_t POINT_COLOR = BLUE, BACK_COLOR = WHITE;  /* 分别设置点的颜色和底色	*/

/*****************************************************************************
** 函数名称: LCD_Write_Reg
** 功能描蕘E 写指聋匕数据
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
*****************************************************************************/
void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_Dat)
{
	Write_Cmd(LCD_Reg);
	Write_Dat(LCD_Dat);
}
/*****************************************************************************
** 函数名称: Write_Cmd
** 功能描蕘E 写指羴E
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
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
** 函数名称: Write_Dat
** 功能描蕘E 写数据
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
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
** 函数名称: LCD_ReadReg
** 功能描蕘E 读指羴E
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
*****************************************************************************/
uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	uint16_t temp;
	Write_Cmd(LCD_Reg);  //写葋E恋募拇嫫骱�  

	//GPIOB->CRH = (GPIOB->CRH & 0x00000000) | 0x44444444; //将端口高8位配置成输�
	GPIO_InitStructure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;					//pin 2
  	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;			//peripheral (alternate function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;	
	GPIO_InitStructure.Pull = GPIO_NOPULL;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	//GPIOC->CRL = (GPIOC->CRL & 0x00000000) | 0x44444444; //将端口低8位配置成输葋E 
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
	temp = ((GPIOB->IDR&0xff00)|(GPIOC->IDR&0x00ff)); //读取数据(读寄存器时,并不需要读2次) 
	LCD_CS = 1; 
	//GPIOB->CRH = (GPIOB->CRH & 0x00000000) | 0x33333333; //释放端口高8位为输硜E
	//GPIOC->CRL = (GPIOC->CRL & 0x00000000) | 0x33333333; //释放端口低8位为输硜E

	GPIO_InitStructure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;					//pin 2
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (output function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	//GPIOC->CRL = (GPIOC->CRL & 0x00000000) | 0x44444444; //将端口低8位配置成输葋E 
	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;					//pin 2
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (output function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);	
	return temp;   
}
/*****************************************************************************
** 函数名称: LCD_ReadDat
** 功能描蕘E 读数据
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
*****************************************************************************/
uint16_t LCD_ReadDat()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	uint16_t temp;

	//GPIOE->CRH = (GPIOE->CRH & 0x00000000) | 0x44444444;  //将端口高8位配置成输葋E
	//GPIOE->CRL = (GPIOE->CRL & 0x00000000) | 0x44444444;  //将端口低8位配置成输葋E
	
	//GPIOB->CRH = (GPIOB->CRH & 0x00000000) | 0x44444444; //将端口高8位配置成输�
	GPIO_InitStructure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;					//pin 2
  	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;			//peripheral (alternate function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;	
	GPIO_InitStructure.Pull = GPIO_NOPULL;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	//GPIOC->CRL = (GPIOC->CRL & 0x00000000) | 0x44444444; //将端口低8位配置成输葋E 
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
	temp = ((GPIOB->IDR&0xff00)|(GPIOC->IDR&0x00ff));	//读取数据(读寄存器时,并不需要读2次)
	LCD_CS = 1;
	//GPIOE->CRH = (GPIOE->CRH & 0x00000000) | 0x33333333;  //释放端口高8位为输硜E
	//GPIOE->CRL = (GPIOE->CRL & 0x00000000) | 0x33333333;  //释放端口低8位为输硜E
	
	GPIO_InitStructure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;					//pin 2
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (output function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	//GPIOC->CRL = (GPIOC->CRL & 0x00000000) | 0x44444444; //将端口低8位配置成输葋E 
	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;					//pin 2
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (output function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);		

	return temp;   
}
/*****************************************************************************
** 函数名称: LCD_Configuration
** 功能描蕘E LCD_IO口配置
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
*****************************************************************************/
void LCD_Configuration()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC,ENABLE);
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	/* 配置数据IO 连接到GPIOB *********************/	
	GPIO_InitStructure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 
								| GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (output function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* 配置控制IO 连接到PD12.PD13.PD14.PD15 *********************/	
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
** 函数名称: LCD_Init
** 功能描蕘E LCD初始化
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
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
** 函数名称: LCD_DrawPoint
** 功能描蕘E 写一个祦E
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
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
** 函数名称: LCD_WriteRAM_Prepare
** 功能描蕘E 些准备
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
*****************************************************************************/
void LCD_WriteRAM_Prepare()
{
	//Write_Cmd(R34);	
	Write_Cmd(ILI9341_RAMWR); // write to RAM
}
/*****************************************************************************
** 函数名称: LCD_SetCursor
** 功能描蕘E 设置光眮E�
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
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
** 函数名称: LCD_SetDisplayWindow
** 功能描蕘E 设置窗口函数 (将Width和Height调换一下，前提是)
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
*****************************************************************************/
void LCD_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width)
{
  	//LCD_WriteReg(R80, Ypos);	   	   	//水平方向GRAM起始地址
  	//LCD_WriteReg(R81, Ypos+Width); 	//水平方向GRAM结束地址 
  	//LCD_WriteReg(R82, Xpos);		  	//垂直方向GRAM起始地址
  	//LCD_WriteReg(R83, Xpos+Height);  	//垂直方向GRAM结束地址

  	//LCD_SetCursor(Xpos, Ypos);			//设置光眮E恢�
	
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
** 函数名称: LCD_ShowString
** 功能描蕘E 显示字符串函数
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
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
** 函数名称: LCD_ShowChar
** 功能描蕘E 显示一个字符函数
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
*****************************************************************************/
void LCD_ShowChar(uint8_t x,uint16_t y,uint8_t chars,uint8_t size,uint8_t mode)
{
	uint8_t temp;
    uint8_t pos,t;      
    if(x>MAX_CHAR_POSX||y>MAX_CHAR_POSY) return;	    
											
	LCD_SetDisplayWindow(x,y,(size/2-1),size-1);  //设置窗口

	LCD_WriteRAM_Prepare();        //开始写葋ERAM	   
	if(!mode) 						//非叠加方式
	{
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=ASCII_1206[chars-0x20][pos];//调用1206字虂E
			else temp=ASCII_1608[chars-0x20][pos];		 //调用1608字虂E
			for(t=0;t<size/2;t++)
		    {     
		        //if(temp&0x01)            	 			//先传低位，取模有关系
		        if((temp<<t)&0x80)						//先传高位
				{
					Write_Dat(RED);
				}
				else 
				{
					Write_Dat(WHITE);	        
		        }
				//temp>>=1; 	   							//如果先传低位，去掉屏蔽线
		    }
		}	
	}
	else//叠加方式
	{
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=ASCII_1206[chars-0x20][pos];	//调用1206字虂E
			else temp=ASCII_1608[chars-0x20][pos];		 	//调用1608字虂E
			for(t=0;t<size/2;t++)
		    {                 
		        if((temp<<t)&0x80)LCD_DrawPoint(x+t,y+pos);	//画一个祦E    
		        //temp>>=1; 								//如果先传低位，去掉屏蔽线
		    }
		}
	}	    
	/* 恢复窗体大小	*/
	//LCD_WriteReg(R80, 0x0000); //水平方向GRAM起始地址
	//LCD_WriteReg(R81, DISPLAY_HEIGHT - 1); //水平方向GRAM结束地址
	//LCD_WriteReg(R82, 0x0000); //垂直方向GRAM起始地址
	//LCD_WriteReg(R83, DISPLAY_WIDTH - 1); //垂直方向GRAM结束地址	
}
/*****************************************************************************
** 函数名称: LCD_Clear
** 功能描蕘E 清屏幕函数
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
*****************************************************************************/
void LCD_Clear(uint16_t Color)
{
	uint32_t index=0;      
	LCD_SetCursor(0x00,0x0000); //设置光眮E恢� 
	LCD_WriteRAM_Prepare();     //开始写葋ERAM	 	  
	for(index=0;index<DISPLAY_SIZE;index++)
	{
		Write_Dat(Color);    
	}
}
/*****************************************************************************
** 函数名称: WriteString
** 功能描蕘E 在指定位置开始显示一个字符串和一串汉字
				支持自动换行
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
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
            break;                                     /* 字符串结蕘E           */
        }      
        x0 = x0 + (usWidth);                           /* 调节字符串显示松紧度         */
        if(*pcStr > 0x80)                              /* 判断为汉字                   */
        {
		    if((x0 + 16) > LCD_W)                      /* 紒E槭Ｓ嗫占涫欠褡愎�         */
            {
			    x0 = 0;
                y0 = y0 + 16;                          /* 改变显示坐眮E                */
                if(y0 > LCD_H)                         /* 纵坐眮E丒                  */
                {
				    y0 = 0;
                }
            }
            usIndex = findHzIndex(pcStr);
            usWidth = WriteOneHzChar((uint8_t *)&(ptGb16[usIndex].Msk[0]), x0, y0, color);
                                                       /* 显示字穪E                    */
            pcStr += 2;
        }
		else 
		{                                               /* 判断为非汉字                 */
            if (*pcStr == '\r')                         /* 换行                         */
            { 
			    x0 = sta_x;
                pcStr++;
                usWidth = 0;
                continue;
            } 
			else if (*pcStr == '\n')                    /* 对苼E狡鸬丒                  */
            {
			    x0 = sta_x;
			    y0 = y0 + 16;                           /* 改变显示坐眮E                */
                if(y0 > LCD_H)                          /* 纵坐眮E丒                  */
                {
				    y0 = 0;
                }
                pcStr++;
                usWidth = 0;
                continue;
            } 
			else 
			{
                if((x0 + 8) > LCD_W)                     /* 紒E槭Ｓ嗫占涫欠褡愎�         */
                {
				    x0 = 0;
                    y0 = y0 + 16;                        /* 改变显示坐眮E                */
                    if(y0 > LCD_H)                       /* 纵坐眮E丒                  */
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
** 函数名称: WriteOneHzChar
** 功能描蕘E 显示一个指定大小的汉字
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
*****************************************************************************/
uint16_t WriteOneHzChar(uint8_t *pucMsk,
                               uint16_t x0,
                               uint16_t y0,
                               uint16_t color)
{
    uint16_t i,j;
    uint16_t mod[16];                                      /* 当前字模                     */
    uint16_t *pusMsk;                                      /* 当前字库地址                 */
    uint16_t y;
    
    pusMsk = (uint16_t *)pucMsk;
    for(i=0; i<16; i++)                                    /* 保存当前汉字点阵式字模       */
    {
        mod[i] = *pusMsk++;                                /* 取得当前字模，皝E侄云丒梦�   */
        mod[i] = ((mod[i] & 0xff00) >> 8) | ((mod[i] & 0x00ff) << 8);/* 字模交换高低字节（为了显示   */
                                                           /* 需要）                       */
    }
    y = y0;
    for(i=0; i<16; i++)                                    /* 16行                         */
    { 
	    #ifdef __DISPLAY_BUFFER                            /* 使用显存显示                 */
        for(j=0; j<16; j++)                                /* 16列                         */
        {
		    if((mod[i] << j)& 0x8000)                      /* 显示字模                     */
            {
			    DispBuf[320*(y0+i) + x0+j] = color;
            }
        }
        #else                                              /* 直接显示                     */
        
		LCD_SetCursor(x0, y);                              /* 设置写数据地址指諄E          */
		LCD_WriteRAM_Prepare();        					   /*开始写葋ERAM	*/   
        for(j=0; j<16; j++)                                /* 16列                         */
        {
		    if((mod[i] << j) & 0x8000)                     /* 显示字模                     */
            { 
			    Write_Dat(color);
            } 
			else 
			{
                //Write_Dat(BACK_COLOR);                     /* 用读方式跳过写空白点的像素   */
            	//LCD_ReadDat();
			}
        }
        y++;
        #endif
    }
    return (16);                                          /* 返回16位列縼E                */
}
/*****************************************************************************
** 函数名称: WriteOneASCII
** 功能描蕘E 显示一个指定大小的字穪E
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
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
    for(i=0; i<16; i++) {                                 /* 16行                         */
        ucChar = *pucMsk++;
        #ifdef __DISPLAY_BUFFER                           /* 使用显存显示                 */
        for(j=0; j<8; j++) {                              /* 8列                          */
            if((ucChar << j)& 0x80) {                     /* 显示字模                     */
                DispBuf[320*(y0+i) + x0+j] = color;
            }
        }
        #else                                             /* 直接显示                     */
        bit = 0;
        //LCD_SetCursor(x0, y);                             /* 设置写数据地址指諄E          */
		LCD_WriteRAM_Prepare();        					  /* 开始写葋ERAM	    		  */
        for(j=0; j<8; j++) {                              /* 8列                          */
            if((ucChar << j) & 0x80) {                    /* 显示字模                     */
                //Write_Dat(color);
				//LCD_SetCursor(x0 + j, y);//设置光眮E恢� 
				//Write_Cmd(R34);//开始写葋ERAM
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
    return (8);                                           /* 返回16位列縼E                */
}
/*****************************************************************************
** 函数名称: Num_power
** 功能描蕘E M的N次方
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
*****************************************************************************/
uint32_t Num_power(uint8_t m,uint8_t n)
{
	uint32 result=1;	 
	while(n--)result*=m;    
	return result;
}
/*****************************************************************************
** 函数名称: LCD_ShowNum
** 功能描蕘E 在指定位置显示一串数字
				num:数值(0~4294967295);
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
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
** 函数名称: LCD_WriteBMP
** 功能描蕘E 在指定的位置显示一幅图片
				Xpos和Ypos为图片显示地址，Height和Width 为图片的窥胰和高度
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
*****************************************************************************/
void LCD_WriteBMP(uint8_t Xpos, uint16_t Ypos, uint8_t Height, uint16_t Width, uint8_t *bitmap)
{
  	uint32_t index;
  	uint32_t size = Height * Width;
  	uint16_t *bitmap_ptr = (uint16_t *)bitmap;

  	LCD_SetDisplayWindow(Xpos, Ypos, Width-1, Height-1);

  	//LCD_WriteReg(0x03, 0x1038);	//如果需要横向显示图片，将此去掉屏蔽 ，同时将Width和Hight调换一下就可以

  	LCD_WriteRAM_Prepare();

  	for(index = 0; index < size; index++)
  	{
    	Write_Dat(*bitmap_ptr++);
  	}
	//恢复窗体大小	 
	//LCD_WriteReg(R80, 0x0000); //水平方向GRAM起始地址
	//LCD_WriteReg(R81, DISPLAY_HEIGHT - 1); //水平方向GRAM结束地址
	//LCD_WriteReg(R82, 0x0000); //垂直方向GRAM起始地址
	//LCD_WriteReg(R83, DISPLAY_WIDTH - 1); //垂直方向GRAM结束地址	
	LCD_SetDisplayWindow(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
}
/*****************************************************************************
** 函数名称: LCD_DrawLine
** 功能描蕘E 在指定的两侧位置画一条线
				x1,y1:起点坐眮E�  x2,y2:终点坐眮E
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
*****************************************************************************/
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 

	delta_x=x2-x1; 				//计算坐眮E隽� 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; 		//设置单步方蟻E
	else if(delta_x==0)incx=0;	//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;	//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐眮E丒
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )	//画线输硜E
	{  
		//LCD_DrawPoint(uRow,uCol);//画祦E
		
		LCD_SetCursor(uRow,uCol);//设置光眮E恢� 
		Write_Cmd(R34);//开始写葋ERAM
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
** 函数名称: LCD_DrawLine
** 功能描蕘E 在指定位置画一个指定大小的圆
				(x,y):中心祦E	 r    :皝E�
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
*****************************************************************************/
void Draw_Circle(uint8_t x0,uint16_t y0,uint8_t r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //判断下个点位置的眮E�
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
		//使用Bresenham算法画圆     
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
** 函数名称: LCD_Fill
** 功能描蕘E 在指定区域内帖箱指定颜色
				区域大小:  (xend-xsta)*(yend-ysta) 
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
*****************************************************************************/
void LCD_Fill(uint8_t xsta,uint16_t ysta,uint8_t xend,uint16_t yend,uint16_t color)
{                    
    uint32_t n;
	//设置窗口										
	LCD_WriteReg(R80, xsta); //水平方向GRAM起始地址
	LCD_WriteReg(R81, xend); //水平方向GRAM结束地址
	LCD_WriteReg(R82, ysta); //垂直方向GRAM起始地址
	LCD_WriteReg(R83, yend); //垂直方向GRAM结束地址	
	LCD_SetCursor(xsta,ysta);//设置光眮E恢�  
	LCD_WriteRAM_Prepare();  //开始写葋ERAM	 	   	   
	n=(uint32)(yend-ysta+1)*(xend-xsta+1);    
	while(n--){Write_Dat(color);}//显示所帖箱的颜色. 
	//恢复设置
	//LCD_WriteReg(R80, 0x0000); //水平方向GRAM起始地址
	//LCD_WriteReg(R81, DISPLAY_HEIGHT - 1); //水平方向GRAM结束地址
	//LCD_WriteReg(R82, 0x0000); //垂直方向GRAM起始地址
	//LCD_WriteReg(R83, DISPLAY_WIDTH - 1); //垂直方向GRAM结束地址	    
	
	//LCD_SetDisplayWindow(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
}
/*****************************************************************************
** 函数名称: LCD_Delay
** 功能描蕘E 用于LCD驱动延时
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
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
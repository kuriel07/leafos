#ifndef __TOUCH_H
#define __TOUCH_H	
#include "..\..\defs.h"
#include "..\..\config.h"
#include <stdint.h>

/*´¥ÃşÆÁĞ¾Æ¬Á¬½ÓÒı½ÅÅäÖÃ	*/   
#define PEN  PCin(13)   //PC13  INT
#define DOUT PAin(6)   //PA6  MISO
#define TDIN PAout(7)  //PA7  MOSI
#define TCLK PAout(5)  //PA5  SCLK
#define TCS  PAout(4)  //PA4  CS  
/* °´¼E´Ì¬	*/ 
//#define Key_Down 0x01
//#define Key_Up   0x00  


/* ADS7843/7846/UH7843/7846/XPT2046/TSC2046 Ö¸Áî¼¯ */
#define CMD_RDY  0XD0 //0B10010000¼´ÓÃ²î·Ö·½Ê½¶ÁX×ø±ê
#define CMD_RDX	0X90  //0B11010000¼´ÓÃ²î·Ö·½Ê½¶ÁY×ø±ê  
/* Ê¹ÄÜ24LC02 */
//#define ADJ_SAVE_ENABLE	    
/* ±Ê¸Ë½á¹¹ÌE*/
typedef struct 
{
	uint16 X0;//Ô­Ê¼×ø±E
	uint16 Y0;
	uint16 X; //×ûòÕ/Ôİ´æ×ø±E
	uint16 Y;						   	    
	uint8  Key_Sta;//±ÊµÄ×´Ì¬			  
	//´¥ÃşÆÁĞ£×¼²ÎÊı
	float xfac;
	float yfac;
	short xoff;
	short yoff;
}Pen_Holder;

extern Pen_Holder Pen_Point;

int8 if_touch_init(void * display);
uint8 if_touch_get(void * display, uint16 * x, uint16 * y) ;
void if_touch_wake(void * display) ;
void if_touch_sleep(void * display) ;

void Touch_Configuration(void);
void ADS_Write_Byte(uint8_t num);
uint16_t ADS_Read_AD(uint8_t CMD);
uint16_t ADS_Read_XY(uint8_t xy);
uint8_t Read_TP_Once(void);
uint8_t Read_ADS2(uint16_t *x,uint16_t *y);
uint8_t Read_ADS(uint16_t *x,uint16_t *y);
void Drow_Touch_Point(uint8_t x,uint16_t y);
uint8_t Get_Adjdata(void);
#endif

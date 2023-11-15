#ifndef __TOUCH_H
#define __TOUCH_H	
#include "..\..\defs.h"
#include "..\..\config.h"
#include <stdint.h>

/*������оƬ������������	*/   
#define PEN  PCin(13)   //PC13  INT
#define DOUT PAin(6)   //PA6  MISO
#define TDIN PAout(7)  //PA7  MOSI
#define TCLK PAout(5)  //PA5  SCLK
#define TCS  PAout(4)  //PA4  CS  
/* ����E�̬	*/ 
//#define Key_Down 0x01
//#define Key_Up   0x00  


/* ADS7843/7846/UH7843/7846/XPT2046/TSC2046 ָ� */
#define CMD_RDY  0XD0 //0B10010000���ò�ַ�ʽ��X����
#define CMD_RDX	0X90  //0B11010000���ò�ַ�ʽ��Y����  
/* ʹ��24LC02 */
//#define ADJ_SAVE_ENABLE	    
/* �ʸ˽ṹ́E*/
typedef struct 
{
	uint16 X0;//ԭʼ����E
	uint16 Y0;
	uint16 X; //����/�ݴ�����E
	uint16 Y;						   	    
	uint8  Key_Sta;//�ʵ�״̬			  
	//������У׼����
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

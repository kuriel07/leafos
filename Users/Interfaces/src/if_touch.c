/**************************************************************
** 	��ţ������
**	���ܽ��ܣ� ��������������
**  �汾��V1.0  
**	��̳��www.openmcu.com
**	�Ա���http://shop36995246.taobao.com/   
**  ����֧��Ⱥ��121939788 
***************************************************************/ 
/* Includes ------------------------------------------------------------------*/
#include "..\..\defs.h"
#include "..\..\config.h"
#include "..\inc\if_apis.h"
#include "..\..\gui\inc\ui_core.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
//#include "stm32f4xx_hal.h"
//#include "stm32f4xx_hal_pwr.h"
//#include "stm32f4xx_hal_gpio.h"

#if SHARD_TS_TYPE == 0x0000
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//Pen_Holder Pen_Point;	/* �����ʵ�� */ 
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//EXTI_InitTypeDef EXTI_InitStructure;
static ui_config * g_cur_tconfig = NULL;

extern void Delay(__IO uint32_t nTime);

/*****************************************************************************
** ��������: Pen_Int_Set
** ������ʁE PEN�ж�����
				EN=1�������ж�
					EN=0: �ر��ж�
** ׁE ����: Dream
** �ա�  ��: 2010āE2��06��
*****************************************************************************/	 
void Pen_Int_Set(uint8_t en)
{
	if(en)EXTI->IMR|=1<<13;   //����line13�ϵ��ж�	  	
	else EXTI->IMR&=~(1<<13); //�ر�line13�ϵ��ж�	   
}
//SPIд����
//��7843д��1byte����   
void ADS_Write_Byte(uint8_t num)    
{  
	uint8_t count=0;   
	for(count=0;count<8;count++)  
	{ 	  
		if(num&0x80)TDIN=1;  
		else TDIN=0;   
		num<<=1;    
		TCLK=0;//��������Ч	   	 
		TCLK=1;      
	} 			    
} 		 
//SPI������ 
//��7846/7843/XPT2046/UH7843/UH7846��ȡadcֵ	   
uint16_t ADS_Read_AD(uint8_t CMD)	  
{ 	 
	GPIO_InitTypeDef GPIO_InitStructure;
	uint8_t i;
	uint8_t count=0; 	  
	uint16_t Num=0;
	GPIO_TypeDef GPIO_Temp;
	//save GPIOA context
	memcpy(&GPIO_Temp, GPIOA, sizeof(GPIO_TypeDef));
	//re-initialize GPIOA context
	GPIO_InitStructure.Pin = GPIO_PIN_4; 
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP; 	//ͨ����́E䳁E
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH; 
	HAL_GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	TCLK=0;//������ʱ�� 	 
	TCS=0; //ѡ��ADS7843	 
	ADS_Write_Byte(CMD);//����������
	for(i=100;i>0;i--);
//	delay_us(6);//ADS7846��ת��ʱ���Ϊ6us
	TCLK=1;//��1��ʱ�ӣ����BUSY   	    
	TCLK=0; 	 
	for(count=0;count<16;count++)  
	{ 				  
		Num<<=1; 	 
		TCLK=0;//�½�����Ч  	    	   
		TCLK=1;
		if(DOUT)Num++; 		 
	}  	
	Num>>=4;   //ֻ�и�12λ��Ч.
	TCS=1;//�ͷ�ADS7843	
	//restore GPIOA context
	memcpy(GPIOA, &GPIO_Temp, sizeof(GPIO_TypeDef));
	//GPIOA->ODR = ODR;
	return(Num);   
}
//��ȡһ������ֵ
//������ȡREAD_TIMES������,����Щ������������,
//Ȼ��ȥ����ͺ����LOST_VAL����,ȡƽ��ֵ 
#define READ_TIMES 15 //��ȡ����
#define LOST_VAL 5	  //����ֵ
uint16_t ADS_Read_XY(uint8_t xy)
{
	uint16_t i, j;
	uint16_t buf[READ_TIMES];
	uint16_t sum=0;
	uint16_t temp;
	for(i=0;i<READ_TIMES;i++)
	{				 
		buf[i]=ADS_Read_AD(xy);	    
	}				    
	for(i=0;i<READ_TIMES-1; i++)//����
	{
		for(j=i+1;j<READ_TIMES;j++)
		{
			if(buf[i]>buf[j])//��������
			{
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}	  
	sum=0;
	for(i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum+=buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	return temp;   
} 
//���˲��������ȡ
//��Сֵ��������100.
uint8_t Read_ADS(uint16_t *x,uint16_t *y)
{
	uint16_t xtemp,ytemp;			 	 		  
	xtemp=ADS_Read_XY(CMD_RDX);
	ytemp=ADS_Read_XY(CMD_RDY);	  												   
	if(xtemp<100||ytemp<100) return 0;//����ʧ��
	*x=xtemp;
	*y=ytemp;
	return 1;//�����ɹ�
}
//2�ζ�ȡADS7846,������ȡ2����Ч��ADֵ,�������ε�ƫ��ܳ���
//50,��������,����Ϊ������ȷ,�����������.	   
//�ú����ܴ�����׼ȷ��
#define ERR_RANGE 50 //��Χ 
uint8_t Read_ADS2(uint16_t *x,uint16_t *y) 
{
	uint16_t x1,y1;
 	uint16_t x2,y2;
 	uint8_t flag;    
    flag=Read_ADS(&x1,&y1);   
    if(flag==0)return(0);
    flag=Read_ADS(&x2,&y2);	   
    if(flag==0)return(0);   
    if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//ǰ�����β�����+-50��
    &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    }else return 0;	  
} 
//��ȡһ������ֵ	
//������ȡһ��,֪��PEN�ɿ��ŷ���!					   
uint8_t Read_TP_Once(void)
{
	uint8_t t=0;	    
	Pen_Int_Set(0);//�ر��ж�
	g_cur_tconfig->t_state = UI_KEY_UP;
	Read_ADS2(&g_cur_tconfig->x, &g_cur_tconfig->y);
	while(PEN==0&&t<=250)
	{
		t++;
		//Delay(10);
	};
	Pen_Int_Set(1);//�����ж�		 
	if(t>=250)return 0;//����2.5s ��Ϊ��Ч
	else return 1;	
}

//////////////////////////////////////////////////
//��LCD�����йصĺ���  
//��һ��������
//����У׼�õ�
static void if_touch_draw_point(gui_handle_p display, uint16 x, uint16 y)
{
	display->fill_area(display, UI_COLOR_WHITE, display->set_area(display, x - 4, y - 4, 8, 8));
}	  

//������У׼����
//�õ��ĸ�У׼����
void if_touch_adjust(void * handle)
{								 
	uint16_t pos_temp[4][2];//���껺��ֵ
	uint8_t  cnt=0;	
	uint16_t d1,d2;
	uint32_t tem1,tem2;
	ui_config * conf = ((gui_handle_p)handle)->touch_config;
	float fac; 	   
	gui_handle_p display = (gui_handle_p)handle;
	cnt=0;				
	display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
	if_touch_draw_point(display, 20, 20);
	conf->t_state = UI_KEY_UP;//���������ź� 
	conf->xfac = 0;//xfac��������Ƿ�У׼��,����У׼֮ǰ�������!�������	 
	while(1)
	{
		if(conf->t_state==UI_KEY_DOWN)//����������
		{
			if(Read_TP_Once())//�õ����ΰ���ֵ
			{  								   
				pos_temp[cnt][0]=conf->x;
				pos_temp[cnt][1]=conf->y;
				cnt++;
			}			 
			switch(cnt)
			{			   
				case 1:
					//LCD_Clear(WHITE);//���� 
					//Drow_Touch_Point(220,20);//����2
					display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
					if_touch_draw_point(display, (display->width - 20), 20);
					//display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, display->width - , 16, 8, 8));	
					break;
				case 2:
					//LCD_Clear(WHITE);//���� 
					///Drow_Touch_Point(20,300);//����3
					display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
					if_touch_draw_point(display, 20, (display->height - 20));
					break;
				case 3:
					//LCD_Clear(WHITE);//���� 
					//Drow_Touch_Point(220,300);//����4
					display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
					if_touch_draw_point(display, (display->width - 20), (display->height - 20));
					break;
				case 4:	 //ȫ���ĸ����Ѿ��õ�
	    		    	//�Ա����
					tem1=abs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2
					tem2=abs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//�õ�1,2�ľ���
					
					tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
					tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//�õ�3,4�ľ���
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05||d1==0||d2==0)//���ϸ�
					{
						cnt=0;
						display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
						if_touch_draw_point(display, 20, 20);
						continue;
					}
					tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//�õ�1,3�ľ���
					
					tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//�õ�2,4�ľ���
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//���ϸ�
					{
						cnt=0;
						display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
						if_touch_draw_point(display, 20, 20);
						continue;
					}//��ȷ��
								   
					//�Խ������
					tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//�õ�1,4�ľ���
	
					tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//�õ�2,3�ľ���
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//���ϸ�
					{
						cnt=0;
						display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
						if_touch_draw_point(display, 20, 20);
						continue;
					}//��ȷ��
					//������
					conf->yfac=(float)(display->height - 40)/(pos_temp[0][1]-pos_temp[3][1]);//�õ�xfac		 
					conf->yoff=(display->height - conf->yfac*(pos_temp[0][1]+pos_temp[3][1]))/2;//�õ�xoff
						  
					conf->xfac=(float)(display->width - 40)/(pos_temp[3][0]-pos_temp[2][0]);//�õ�yfac
					conf->xoff=(display->width - conf->xfac*(pos_temp[3][0]+pos_temp[2][0]))/2;//�õ�yoff  
					return;//У�����				 
			}
		}
	} 
}

static uint8 if_touch_read(gui_handle * display, uint16 * x, uint16 * y) {
	uint8 ret = 0;
	ui_config * conf = display->touch_config;
	conf->t_state = UI_KEY_UP;
	NVIC_DisableIRQ(EXTI15_10_IRQn);
	if(Read_ADS2(&conf->x,&conf->y))
	{
		if_touch_get(display, x, y);
		//*x = g_uconfig.xfac*g_uconfig.x+g_uconfig.xoff;
		//*y = display->height - (g_uconfig.yfac*g_uconfig.y+g_uconfig.yoff);  
	} else ret = -1;
	
    //EXTI_ClearITPendingBit(EXTI_IMR_MR13);	 //����жϹ���λ
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);
	NVIC_EnableIRQ(EXTI15_10_IRQn);
	return ret;
}

static uint8 if_touch_state(gui_handle * display) {
	ui_config * conf = display->touch_config;
	return conf->t_state;
}
/*****************************************************************************
** ��������: Touch_Init
** ��������: ��������ʼ��
				��У׼���
** ��  ����: Dream
** �ա�  ��: 2010��12��06��
*****************************************************************************/
int8 if_touch_init(void * display)
{
	ui_config * conf = ((gui_handle_p)display)->touch_config;
	g_cur_tconfig = conf;
	GPIO_InitTypeDef GPIO_InitStructure;
	Touch_Configuration();
 	Read_ADS(&conf->x,&conf->y);//��һ�ζ�ȡ��ʼ��			 

  	GPIO_InitStructure.Pin = GPIO_PIN_13;							//touch interrupt
  	GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;			//peripheral (alternate function push pull)
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.Pull = GPIO_PULLUP;
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;	
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);	

  	/* Generate software interrupt: simulate a falling edge applied on PEN EXTI line */
  	//EXTI_GenerateSWInterrupt(EXTI_Line13);
	__HAL_GPIO_EXTI_GENERATE_SWIT(GPIO_PIN_13);

	if(conf->magic != 0xEC) {
		memset((uint8 *)conf, 0, sizeof(ui_config));
		if_touch_adjust(display);
		conf->magic = 0xEC;
		if_config_write(conf, sizeof(ui_config));
	}
	((gui_handle_p)display)->touch_state = if_touch_state;
	((gui_handle_p)display)->touch_read = if_touch_read;
	return 0;
}

uint8 if_touch_get(void * display, uint16 * x, uint16 * y) {
	uint16 xx, yy, t;
	ui_config * conf = ((gui_handle_p)display)->touch_config;
	xx = conf->xfac * conf->x + conf->xoff;
	yy = (conf->yfac * conf->y + conf->yoff);
	switch(((gui_handle_p)display)->orientation) {
		case UI_DISP_ORIENTATION_0:		//default orientation
			xx = xx;
			yy = ((gui_handle_p)display)->height - yy;
			break;
		case UI_DISP_ORIENTATION_1:
			t = xx;
			xx = ((gui_handle_p)display)->width - yy;
			yy = ((gui_handle_p)display)->height - t;
			break;
		case UI_DISP_ORIENTATION_2:
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
** ��������: Touch_Configuration
** ��������: ������IO����
** ��  ����: Dream
** �ա�  ��: 2010��12��06��
*****************************************************************************/
void Touch_Configuration()
{
  	//NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC
	//						, ENABLE );  //��Ҫ����
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	//������SPI���GPIO��ʼ��
	GPIO_InitStructure.Pin = GPIO_PIN_5 | GPIO_PIN_7;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;  //ͨ���������
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA,&GPIO_InitStructure);

	//������SPI���GPIO��ʼ��
	GPIO_InitStructure.Pin = GPIO_PIN_6;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;  //������ȁE
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA,&GPIO_InitStructure);

	//Configure PA4 pin: TP_CS pin 
	//GPIO_InitStructure.GPIO_Pin = GPIO_PIN_4; 
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//ͨ���������
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	//GPIO_Init(GPIOA,&GPIO_InitStructure);

	//Configure PC13 pin: TP_INT pin 
	GPIO_InitStructure.Pin = GPIO_PIN_13; 
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT; 	//��������
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOC,&GPIO_InitStructure);
	/* Enable the EXTI15_10_IRQn Interrupt */
	//NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;	// �����ж���4
	//NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // ����ռ�����ȼ�Ϊ2
	//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  // ���ø����ȼ�Ϊ0
	//NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // ʹ���ж���4
	//NVIC_Init(&NVIC_InitStructure);							  // ��ʼ���ж���4
	NVIC_EnableIRQ(EXTI15_10_IRQn);
}
/*****************************************************************************
** ��������: EXTI15_10_IRQHandler
** ��������: �жϴ�����
				�ж�,��⵽PEN�ŵ�һ���½���.
					��λPen_Point.Key_StaΪ����״̬
						�ж���4���ϵ��жϼ��
** ��  ����: Dream
** �ա�  ��: 2010��12��06��
*****************************************************************************/
void EXTI15_10_IRQHandler()
{
	uint16_t i;
  	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_13) != RESET)
	{
    	//EXTI_ClearITPendingBit(EXTI_IMR_MR13);	 //����жϹ���λ
		__HAL_GPIO_EXTI_CLEAR_FLAG(GPIO_PIN_13);
		for(i=700;i>0;i--); 			//��ʱȥ����
		if(g_cur_tconfig == NULL) return;
		g_cur_tconfig->t_state=UI_KEY_DOWN;//�������� 	
		if_pwr_set_interrupt_source(IF_INT_WAKE | IF_INT_TOUCH);
	}
}
#ifdef ADJ_SAVE_ENABLE
#define SAVE_ADDR_BASE 40
/*****************************************************************************
** ��������: Save_Adjdata
** ��������: ����У׼������EEPROAM�е� ��ַ40
				�˲����漰��ʹ���ⲿEEPROM,���û���ⲿEEPROM,���δ˲��ּ���
				������EEPROM����ĵ�ַ�����ַ,ռ��13���ֽ�(RANGE:SAVE_ADDR_BASE~SAVE_ADDR_BASE+12)
** ��  ����: Dream
** �ա�  ��: 2010��12��06��
*****************************************************************************/
#if 0
void Save_Adjdata(void)
{
	uint8_t temp[4];
	//����У�����!		   							  
	temp[0]=(uint32_t)(g_uconfig.xfac*100000000)&0xff;	//����xУ������    
	temp[1]=(uint32_t)(g_uconfig.xfac*100000000)>>8&0xff;//����xУ������    
	temp[2]=(uint32_t)(g_uconfig.xfac*100000000)>>16&0xff;//����xУ������        
	temp[3]=(uint32_t)(g_uconfig.xfac*100000000)>>24&0xff;//����xУ������        
   	I2C_Write(&temp[0],SAVE_ADDR_BASE,4);

	temp[0]=(uint32_t)(g_uconfig.yfac*100000000)&0xff;//����xУ������    
	temp[1]=(uint32_t)(g_uconfig.yfac*100000000)>>8&0xff;//����xУ������    
	temp[2]=(uint32_t)(g_uconfig.yfac*100000000)>>16&0xff;//����xУ������        
	temp[3]=(uint32_t)(g_uconfig.yfac*100000000)>>24&0xff;//����xУ������ 

   	I2C_Write(&temp[0],SAVE_ADDR_BASE+4,4);
	temp[0]=(uint32_t)(g_uconfig.xoff*100000000)&0xff;//����xУ������    
	temp[1]=(uint32_t)(g_uconfig.xoff*100000000)>>8&0xff;//����xУ������    
   	I2C_Write(&temp[0],SAVE_ADDR_BASE+8,2);
	//����xƫ����
	//����yƫ����
	temp[0]=(uint32_t)(g_uconfig.yoff*100000000)&0xff;//����xУ������    
	temp[1]=(uint32_t)(g_uconfig.yoff*100000000)>>8&0xff;//����xУ������    
   	I2C_Write(&temp[0],SAVE_ADDR_BASE+10,2);
	I2C_Read(&temp[0],SAVE_ADDR_BASE+12,1);
	temp[0]&=0XF0;
	temp[0]|=0X0A;//���У׼����
	I2C_Write(&temp[0],SAVE_ADDR_BASE+12,1);			 
}
/*****************************************************************************
** ��������: Get_Adjdata
** ��������: �õ�������EEPROM�����У׼ֵ
				����ֵ��1���ɹ���ȡ����
						 0����ȡʧ�ܣ�Ҫ����У׼
** ��  ����: Dream
** �ա�  ��: 2010��12��06��
*****************************************************************************/       
uint8_t Get_Adjdata(void)
{	
	uint8_t temp[4];
	uint32_t tempfac=0;
	I2C_Read(&temp[0],52,1); //����ʮ���ֽڵĵ���λ��������Ƿ�У׼���� 		 
	if((temp[0]&0X0F)==0X0A)		//�������Ѿ�У׼����			   
	{   
		I2C_Read(&temp[0],40,4);
		tempfac = temp[0]|(temp[1]<<8)|(temp[2]<<16)|(temp[3]<<24);	   
		g_uconfig.xfac=(float)tempfac/100000000;//�õ�xУ׼����
		I2C_Read(&temp[0],44,4);
		tempfac = temp[0]|(temp[1]<<8)|(temp[2]<<16)|(temp[3]<<24);	   
			          
		g_uconfig.yfac=(float)tempfac/100000000;//�õ�yУ׼����
	    //�õ�xƫ����
		I2C_Read(&temp[0],48,2);		
		tempfac = temp[0]|(temp[1]<<8);	   		   	  
		g_uconfig.xoff=tempfac;					 
	    //�õ�yƫ����
		I2C_Read(&temp[0],50,2);
		tempfac = temp[0]|(temp[1]<<8);	   
						 	  
		g_uconfig.yoff=tempfac;					 
		return 1;	 
	}
	return 0;
}
#endif

#endif
/*********************************************************************************************************
** End of File
*********************************************************************************************************/
#endif
/**************************************************************
** 	火牛开发板
**	功能介绍： 触摸屏驱动代码
**  版本：V1.0  
**	论坛：www.openmcu.com
**	淘宝：http://shop36995246.taobao.com/   
**  技术支持群：121939788 
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
//Pen_Holder Pen_Point;	/* 定义笔实体 */ 
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//EXTI_InitTypeDef EXTI_InitStructure;
static ui_config * g_cur_tconfig = NULL;

extern void Delay(__IO uint32_t nTime);

/*****************************************************************************
** 函数名称: Pen_Int_Set
** 功能描蕘E PEN中断设置
				EN=1：开启中断
					EN=0: 关闭中断
** 讈E 　者: Dream
** 日　  期: 2010膩E2月06日
*****************************************************************************/	 
void Pen_Int_Set(uint8_t en)
{
	if(en)EXTI->IMR|=1<<13;   //开启line13上的中断	  	
	else EXTI->IMR&=~(1<<13); //关闭line13上的中断	   
}
//SPI写数据
//向7843写入1byte数据   
void ADS_Write_Byte(uint8_t num)    
{  
	uint8_t count=0;   
	for(count=0;count<8;count++)  
	{ 	  
		if(num&0x80)TDIN=1;  
		else TDIN=0;   
		num<<=1;    
		TCLK=0;//上升沿有效	   	 
		TCLK=1;      
	} 			    
} 		 
//SPI读数据 
//从7846/7843/XPT2046/UH7843/UH7846读取adc值	   
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
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP; 	//通用推蛠E涑丒
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH; 
	HAL_GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	TCLK=0;//先拉低时钟 	 
	TCS=0; //选中ADS7843	 
	ADS_Write_Byte(CMD);//发送命令字
	for(i=100;i>0;i--);
//	delay_us(6);//ADS7846的转换时间最长为6us
	TCLK=1;//给1个时钟，清除BUSY   	    
	TCLK=0; 	 
	for(count=0;count<16;count++)  
	{ 				  
		Num<<=1; 	 
		TCLK=0;//下降沿有效  	    	   
		TCLK=1;
		if(DOUT)Num++; 		 
	}  	
	Num>>=4;   //只有高12位有效.
	TCS=1;//释放ADS7843	
	//restore GPIOA context
	memcpy(GPIOA, &GPIO_Temp, sizeof(GPIO_TypeDef));
	//GPIOA->ODR = ODR;
	return(Num);   
}
//读取一个坐标值
//连续读取READ_TIMES次数据,对这些数据升序排列,
//然后去掉最低和最高LOST_VAL个数,取平均值 
#define READ_TIMES 15 //读取次数
#define LOST_VAL 5	  //丢弃值
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
	for(i=0;i<READ_TIMES-1; i++)//排序
	{
		for(j=i+1;j<READ_TIMES;j++)
		{
			if(buf[i]>buf[j])//升序排列
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
//带滤波的坐标读取
//最小值不能少于100.
uint8_t Read_ADS(uint16_t *x,uint16_t *y)
{
	uint16_t xtemp,ytemp;			 	 		  
	xtemp=ADS_Read_XY(CMD_RDX);
	ytemp=ADS_Read_XY(CMD_RDY);	  												   
	if(xtemp<100||ytemp<100) return 0;//读数失败
	*x=xtemp;
	*y=ytemp;
	return 1;//读数成功
}
//2次读取ADS7846,连续读取2次有效的AD值,且这两次的偏差不能超过
//50,满足条件,则认为读数正确,否则读数错误.	   
//该函数能大大提高准确度
#define ERR_RANGE 50 //误差范围 
uint8_t Read_ADS2(uint16_t *x,uint16_t *y) 
{
	uint16_t x1,y1;
 	uint16_t x2,y2;
 	uint8_t flag;    
    flag=Read_ADS(&x1,&y1);   
    if(flag==0)return(0);
    flag=Read_ADS(&x2,&y2);	   
    if(flag==0)return(0);   
    if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//前后两次采样在+-50内
    &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    }else return 0;	  
} 
//读取一次坐标值	
//仅仅读取一次,知道PEN松开才返回!					   
uint8_t Read_TP_Once(void)
{
	uint8_t t=0;	    
	Pen_Int_Set(0);//关闭中断
	g_cur_tconfig->t_state = UI_KEY_UP;
	Read_ADS2(&g_cur_tconfig->x, &g_cur_tconfig->y);
	while(PEN==0&&t<=250)
	{
		t++;
		//Delay(10);
	};
	Pen_Int_Set(1);//开启中断		 
	if(t>=250)return 0;//按下2.5s 认为无效
	else return 1;	
}

//////////////////////////////////////////////////
//与LCD部分有关的函数  
//画一个触摸点
//用来校准用的
static void if_touch_draw_point(gui_handle_p display, uint16 x, uint16 y)
{
	display->fill_area(display, UI_COLOR_WHITE, display->set_area(display, x - 4, y - 4, 8, 8));
}	  

//触摸屏校准代码
//得到四个校准参数
void if_touch_adjust(void * handle)
{								 
	uint16_t pos_temp[4][2];//坐标缓存值
	uint8_t  cnt=0;	
	uint16_t d1,d2;
	uint32_t tem1,tem2;
	ui_config * conf = ((gui_handle_p)handle)->touch_config;
	float fac; 	   
	gui_handle_p display = (gui_handle_p)handle;
	cnt=0;				
	display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
	if_touch_draw_point(display, 20, 20);
	conf->t_state = UI_KEY_UP;//消除触发信号 
	conf->xfac = 0;//xfac用来标记是否校准过,所以校准之前必须清掉!以免错误	 
	while(1)
	{
		if(conf->t_state==UI_KEY_DOWN)//按键按下了
		{
			if(Read_TP_Once())//得到单次按键值
			{  								   
				pos_temp[cnt][0]=conf->x;
				pos_temp[cnt][1]=conf->y;
				cnt++;
			}			 
			switch(cnt)
			{			   
				case 1:
					//LCD_Clear(WHITE);//清屏 
					//Drow_Touch_Point(220,20);//画点2
					display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
					if_touch_draw_point(display, (display->width - 20), 20);
					//display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, display->width - , 16, 8, 8));	
					break;
				case 2:
					//LCD_Clear(WHITE);//清屏 
					///Drow_Touch_Point(20,300);//画点3
					display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
					if_touch_draw_point(display, 20, (display->height - 20));
					break;
				case 3:
					//LCD_Clear(WHITE);//清屏 
					//Drow_Touch_Point(220,300);//画点4
					display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
					if_touch_draw_point(display, (display->width - 20), (display->height - 20));
					break;
				case 4:	 //全部四个点已经得到
	    		    	//对边相等
					tem1=abs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2
					tem2=abs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//得到1,2的距离
					
					tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
					tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//得到3,4的距离
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05||d1==0||d2==0)//不合格
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
					d1=sqrt(tem1+tem2);//得到1,3的距离
					
					tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//得到2,4的距离
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//不合格
					{
						cnt=0;
						display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
						if_touch_draw_point(display, 20, 20);
						continue;
					}//正确了
								   
					//对角线相等
					tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//得到1,4的距离
	
					tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//得到2,3的距离
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//不合格
					{
						cnt=0;
						display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, 0, 0, display->width, display->height));
						if_touch_draw_point(display, 20, 20);
						continue;
					}//正确了
					//计算结果
					conf->yfac=(float)(display->height - 40)/(pos_temp[0][1]-pos_temp[3][1]);//得到xfac		 
					conf->yoff=(display->height - conf->yfac*(pos_temp[0][1]+pos_temp[3][1]))/2;//得到xoff
						  
					conf->xfac=(float)(display->width - 40)/(pos_temp[3][0]-pos_temp[2][0]);//得到yfac
					conf->xoff=(display->width - conf->xfac*(pos_temp[3][0]+pos_temp[2][0]))/2;//得到yoff  
					return;//校正完成				 
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
	
    //EXTI_ClearITPendingBit(EXTI_IMR_MR13);	 //清楚中断挂起位
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);
	NVIC_EnableIRQ(EXTI15_10_IRQn);
	return ret;
}

static uint8 if_touch_state(gui_handle * display) {
	ui_config * conf = display->touch_config;
	return conf->t_state;
}
/*****************************************************************************
** 函数名称: Touch_Init
** 功能描述: 触摸屏初始化
				带校准检测
** 作  　者: Dream
** 日　  期: 2010年12月06日
*****************************************************************************/
int8 if_touch_init(void * display)
{
	ui_config * conf = ((gui_handle_p)display)->touch_config;
	g_cur_tconfig = conf;
	GPIO_InitTypeDef GPIO_InitStructure;
	Touch_Configuration();
 	Read_ADS(&conf->x,&conf->y);//第一次读取初始化			 

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
** 函数名称: Touch_Configuration
** 功能描述: 触摸屏IO配置
** 作  　者: Dream
** 日　  期: 2010年12月06日
*****************************************************************************/
void Touch_Configuration()
{
  	//NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC
	//						, ENABLE );  //重要！！
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	//下面是SPI相关GPIO初始化
	GPIO_InitStructure.Pin = GPIO_PIN_5 | GPIO_PIN_7;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;  //通用推挽输出
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA,&GPIO_InitStructure);

	//下面是SPI相关GPIO初始化
	GPIO_InitStructure.Pin = GPIO_PIN_6;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;  //上拉输葋E
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA,&GPIO_InitStructure);

	//Configure PA4 pin: TP_CS pin 
	//GPIO_InitStructure.GPIO_Pin = GPIO_PIN_4; 
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//通用推挽输出
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	//GPIO_Init(GPIOA,&GPIO_InitStructure);

	//Configure PC13 pin: TP_INT pin 
	GPIO_InitStructure.Pin = GPIO_PIN_13; 
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT; 	//上拉输入
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOC,&GPIO_InitStructure);
	/* Enable the EXTI15_10_IRQn Interrupt */
	//NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;	// 配置中断线4
	//NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // 设置占先优先级为2
	//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  // 设置副优先级为0
	//NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // 使能中断线4
	//NVIC_Init(&NVIC_InitStructure);							  // 初始化中断线4
	NVIC_EnableIRQ(EXTI15_10_IRQn);
}
/*****************************************************************************
** 函数名称: EXTI15_10_IRQHandler
** 功能描述: 中断处理函数
				中断,检测到PEN脚的一个下降沿.
					置位Pen_Point.Key_Sta为按下状态
						中断线4线上的中断检测
** 作  　者: Dream
** 日　  期: 2010年12月06日
*****************************************************************************/
void EXTI15_10_IRQHandler()
{
	uint16_t i;
  	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_13) != RESET)
	{
    	//EXTI_ClearITPendingBit(EXTI_IMR_MR13);	 //清楚中断挂起位
		__HAL_GPIO_EXTI_CLEAR_FLAG(GPIO_PIN_13);
		for(i=700;i>0;i--); 			//延时去抖动
		if(g_cur_tconfig == NULL) return;
		g_cur_tconfig->t_state=UI_KEY_DOWN;//按键按下 	
		if_pwr_set_interrupt_source(IF_INT_WAKE | IF_INT_TOUCH);
	}
}
#ifdef ADJ_SAVE_ENABLE
#define SAVE_ADDR_BASE 40
/*****************************************************************************
** 函数名称: Save_Adjdata
** 功能描述: 保存校准参数到EEPROAM中的 地址40
				此部分涉及到使用外部EEPROM,如果没有外部EEPROM,屏蔽此部分即可
				保存在EEPROM里面的地址区间基址,占用13个字节(RANGE:SAVE_ADDR_BASE~SAVE_ADDR_BASE+12)
** 作  　者: Dream
** 日　  期: 2010年12月06日
*****************************************************************************/
#if 0
void Save_Adjdata(void)
{
	uint8_t temp[4];
	//保存校正结果!		   							  
	temp[0]=(uint32_t)(g_uconfig.xfac*100000000)&0xff;	//保存x校正因素    
	temp[1]=(uint32_t)(g_uconfig.xfac*100000000)>>8&0xff;//保存x校正因素    
	temp[2]=(uint32_t)(g_uconfig.xfac*100000000)>>16&0xff;//保存x校正因素        
	temp[3]=(uint32_t)(g_uconfig.xfac*100000000)>>24&0xff;//保存x校正因素        
   	I2C_Write(&temp[0],SAVE_ADDR_BASE,4);

	temp[0]=(uint32_t)(g_uconfig.yfac*100000000)&0xff;//保存x校正因素    
	temp[1]=(uint32_t)(g_uconfig.yfac*100000000)>>8&0xff;//保存x校正因素    
	temp[2]=(uint32_t)(g_uconfig.yfac*100000000)>>16&0xff;//保存x校正因素        
	temp[3]=(uint32_t)(g_uconfig.yfac*100000000)>>24&0xff;//保存x校正因素 

   	I2C_Write(&temp[0],SAVE_ADDR_BASE+4,4);
	temp[0]=(uint32_t)(g_uconfig.xoff*100000000)&0xff;//保存x校正因素    
	temp[1]=(uint32_t)(g_uconfig.xoff*100000000)>>8&0xff;//保存x校正因素    
   	I2C_Write(&temp[0],SAVE_ADDR_BASE+8,2);
	//保存x偏移量
	//保存y偏移量
	temp[0]=(uint32_t)(g_uconfig.yoff*100000000)&0xff;//保存x校正因素    
	temp[1]=(uint32_t)(g_uconfig.yoff*100000000)>>8&0xff;//保存x校正因素    
   	I2C_Write(&temp[0],SAVE_ADDR_BASE+10,2);
	I2C_Read(&temp[0],SAVE_ADDR_BASE+12,1);
	temp[0]&=0XF0;
	temp[0]|=0X0A;//标记校准过了
	I2C_Write(&temp[0],SAVE_ADDR_BASE+12,1);			 
}
/*****************************************************************************
** 函数名称: Get_Adjdata
** 功能描述: 得到保存在EEPROM里面的校准值
				返回值：1，成功获取数据
						 0，获取失败，要重新校准
** 作  　者: Dream
** 日　  期: 2010年12月06日
*****************************************************************************/       
uint8_t Get_Adjdata(void)
{	
	uint8_t temp[4];
	uint32_t tempfac=0;
	I2C_Read(&temp[0],52,1); //第五十二字节的第四位用来标记是否校准过！ 		 
	if((temp[0]&0X0F)==0X0A)		//触摸屏已经校准过了			   
	{   
		I2C_Read(&temp[0],40,4);
		tempfac = temp[0]|(temp[1]<<8)|(temp[2]<<16)|(temp[3]<<24);	   
		g_uconfig.xfac=(float)tempfac/100000000;//得到x校准参数
		I2C_Read(&temp[0],44,4);
		tempfac = temp[0]|(temp[1]<<8)|(temp[2]<<16)|(temp[3]<<24);	   
			          
		g_uconfig.yfac=(float)tempfac/100000000;//得到y校准参数
	    //得到x偏移量
		I2C_Read(&temp[0],48,2);		
		tempfac = temp[0]|(temp[1]<<8);	   		   	  
		g_uconfig.xoff=tempfac;					 
	    //得到y偏移量
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
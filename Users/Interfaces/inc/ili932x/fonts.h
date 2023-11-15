/**************************************************************
** 	火牛开发板
**	字库代码
**  版本：V1.0  
**	论坛：www.openmcu.com
**	淘宝：http://shop36995246.taobao.com/   
**  技术支持群：121939788 
***************************************************************/
#ifndef __FONT_LIB_H
#define __FONT_LIB_H

#include "hz16.h"


extern const FNT_GB16 GBHZ_16[];

extern const unsigned char ASCII_1206[95][12];
extern const unsigned char ASCII_1608[][16] ;

extern const unsigned char g_font_8x16[];
extern const unsigned char g_font_6x8[] ;
extern const unsigned char g_font_8x8[] ;
extern const unsigned char g_font_5x8[];
extern const unsigned char g_font_7x12[];
extern const unsigned char g_font_11x16[];
extern const unsigned char g_font_14x20[];
extern const unsigned char g_font_17x24[];

/*********************************************************************************************************
汉字字模表
汉字库: 宋体16.dot 横向取模,数据排列:从左到右从上到下
*********************************************************************************************************/

#endif
/*********************************************************************************************************
** End of File
*********************************************************************************************************/

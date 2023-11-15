#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "..\..\defs.h"
#include "..\..\config.h"
#include "..\inc\if_apis.h"
#include "..\inc\ili932x\ili932x.h"
#include "..\inc\ili932x\fonts.h"
#include "..\inc\ssd1351\ssd1351.h"
#include "fonts\fonts.h"

#ifdef DMA2D_ENABLED

#endif

static uint16_t if_gui_putc_24(struct gui_handle * handle, uint8 *pucMsk, uint16 fw, uint16 fh,
                              uint16_t x0,
                              uint16_t y0,
                              uint32 color)
{
    uint16_t i,j;
    uint16_t y;
    uint16 ucChar;
	uint8 bit = 0;
	uint8 last_bit;
	uint32 half_col = (color >> 1) & 0x7F7F7F;
    y = y0;
    for(i=0; i<fh; i++) {     
        ucChar = *(uint32 *)pucMsk++;        
        bit = 0;
        for(j=0; j<fw; j++) {                 
            if((ucChar << j) & 0x00800000) {  
							handle->fill_area(handle, color, handle->set_area(handle, x0 + j, y, 1, 1));
							bit = 1;
						} else {
							//if(bit) handle->fill_area(handle, color, handle->set_area(handle, x0 + j, y, 1, 1));
							//bit = 0;
            }
        }
        y++;
    }
    return (fw);   
}

static uint16_t if_gui_putc_16(struct gui_handle * handle, uint8 *pucMsk, uint16 fw, uint16 fh,
                              uint16_t x0,
                              uint16_t y0,
                              uint32 color)
{
    uint16_t i,j;
    uint16_t y;
    uint16 ucChar;
	uint8 bit = 0;
	uint8 last_bit;
	uint32 half_col = (color >> 1) & 0x7F7F7F;
    y = y0;
    for(i=0; i<fh; i++) {     
        ucChar = *(uint16 *)pucMsk++;        
        bit = 0;
        for(j=0; j<fw; j++) {                 
            if((ucChar << j) & 0x8000) {   
				//if(bit) 
				handle->fill_area(handle, color, handle->set_area(handle, x0 + j, y, 1, 1));
				//else handle->fill_area(handle, half_col, handle->set_area(handle, x0 + j, y, 2, 1));
				//if(((ucChar << (j +1)) & 0x80) == 0 && j < 7) {
				//	if((color & 0xFFFFFF) != 0) handle->fill_area(handle, half_col, handle->set_area(handle, x0 + j + 1, y, 1, 1));
				//}
				bit = 1;
			} else {
				//if(bit) handle->fill_area(handle, color, handle->set_area(handle, x0 + j, y, 1, 1));
				//bit = 0;
            }
        }
        y++;
    }
    return (fw);   
}


static uint16_t if_gui_putc_8(struct gui_handle * handle, uint8_t *pucMsk, uint16 fw, uint16 fh,
                              uint16_t x0,
                              uint16_t y0,
                              uint32 color)
{
    uint16_t i,j;
    uint16_t y;
    uint8_t ucChar;
	uint8 bit = 0;
    y = y0;
    for(i=0; i<fh; i++) {     
        ucChar = *pucMsk++;        
        bit = 0;
        for(j=0; j<fw; j++) {                 
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
    return fw;   
}

extern unsigned _dma_xfer_completed;
static void if_gui_fill(struct gui_handle * handle, uint32 color, uint32 size) {
	
	uint32 rgb = color;
	switch(handle->fb_pxfmt) {
		case UI_RGB565:
			rgb = IF_COLOR(color);
			break;
		case UI_RGB888:
			break;
		case UI_ARGB8888:
			break;
	}
	while(size--){ handle->put_pixel(handle, rgb); }
}

static void if_gui_rectangle(struct gui_handle * handle, uint16 x, uint16 y, uint16 w, uint16 h, uint32 color, uint8 radius) {
	uint16 x1 = x;
	uint16 x2 = x+w;
	uint16 y1 = y;
	uint16 y2 = y+h;
	if(radius > 4) radius = 4;
	switch(radius) {
		case 0:
		case 1:
			if_gui_fill(handle, color, handle->set_area(handle, x1+radius, y1, (x2 - x1) - (radius << 1), 1));
			if_gui_fill(handle, color, handle->set_area(handle, x1, y1+radius, 1, (y2 - y1) - (radius << 1)));
			if_gui_fill(handle, color, handle->set_area(handle, x2, y1+radius, 1, (y2 - y1) - (radius << 1)));
			if_gui_fill(handle, color, handle->set_area(handle, x1+radius, y2-1, (x2 - x1) - (radius << 1), 1));
			break;
		case 2:
			if_gui_fill(handle, color, handle->set_area(handle, x1+radius, y1, (x2 - x1) - (radius << 1), 1));
			if_gui_fill(handle, color, handle->set_area(handle, x1, y1+radius, 1, (y2 - y1) - (radius << 1)));
			if_gui_fill(handle, color, handle->set_area(handle, x2, y1+radius, 1, (y2 - y1) - (radius << 1)));
			if_gui_fill(handle, color, handle->set_area(handle, x1+radius, y2, (x2 - x1) - (radius << 1), 1));
			//top left
			if_gui_fill(handle, color, handle->set_area(handle, x1 + (radius - 1), y1 + 1, 1, 1));		//horizontal
			if_gui_fill(handle, color, handle->set_area(handle, x1 + 1, y1 + (radius - 1), 1, 1));		//vertical
			//top right
			if_gui_fill(handle, color, handle->set_area(handle, x2 - (radius - 1), y1 + 1, 1, 1));		//horizontal
			if_gui_fill(handle, color, handle->set_area(handle, x2 - 1, y1 + (radius - 1), 1, 1));		//vertical
			//bottom left
			if_gui_fill(handle, color, handle->set_area(handle, x1 + (radius - 1), y2 - 1, 1, 1));
			if_gui_fill(handle, color, handle->set_area(handle, x1 + 1, y2 - (radius - 1), 1, 1));
			//top right
			if_gui_fill(handle, color, handle->set_area(handle, x2 - (radius - 1), y2 - 1, 1, 1));
			if_gui_fill(handle, color, handle->set_area(handle, x2  - 1, y2 - (radius -1), 1, 1));
			break;
	}
}

static void if_gui_line(struct gui_handle * handle, uint16 x1, uint16 y1, uint16 x2, uint16 y2, uint32 color) {
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 

	delta_x=x2-x1; 				 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; 		
	else if(delta_x==0)incx=0;	
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;	
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )	
	{  	
		//if_gui_rectangle(handle, uRow, uCol, 1, 1, color, 0);
		handle->fill_area(handle, color, handle->set_area(handle, uRow, uCol, 1, 1));
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


uint32_t dmap[96];
uint32_t i=0, b=0, j=0, z=0, f=0, d=0, ct = 0, v=0, cnt=0,t=0,h;
//get char data from ST7735 drawstring//
static void if_gui_putchar(struct gui_handle * handle, uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor, uint16_t wth)
{
	if((wth>1)&&(wth%8!=0)){
		handle->set_area(handle, x, y, (wth+1), ((font.data[dmap[0]-4]<<8)|(font.data[dmap[0]-5])));
	}else if(wth%8==0){
		handle->set_area(handle, x, y, (wth), ((font.data[dmap[0]-4]<<8)|(font.data[dmap[0]-5])));
	}	else handle->set_area(handle, x, y, 7, ((font.data[dmap[0]-4]<<8)|(font.data[dmap[0]-5])));



    f = (font.data[dmap[ch-32]]<<8)|(font.data[dmap[ch-32]-1]);
    f = (f<<8)|(font.data[dmap[ch-32]-2]);

    z = (font.data[dmap[(ch+1)-32]]<<8)|(font.data[dmap[(ch+1)-32]-1]);
    z = (z<<8)|(font.data[dmap[(ch+1)-32]-2]);
	
    f = (font.data[dmap[ch]]<<8)|(font.data[dmap[ch]-1]);
    f = (f<<8)|(font.data[dmap[ch]-2]);

    z = (font.data[dmap[(ch+1)]]<<8)|(font.data[dmap[(ch+1)]-1]);
    z = (z<<8)|(font.data[dmap[(ch+1)]-2]);

    for(i = f; i < z; i++)
    {
       b = font.data[i];//array adress
       if(wth>1){
       if(((wth-v)>=8)&&(wth%8!=0)){
            ct=8;
            v+=8;
            cnt++;
        }else if(((wth-v)<8)&&(cnt==(wth/8))){
            ct=(wth-v)+1;
            v=0;
            cnt=0;
        }
       }else ct=8;
       if(wth%8==0)ct=8;
        for(j = 0; j < ct; j++)
        {

        	////////////////// plot  //////////////////////////////
            if((b >> j) & 0x01)// if logic 1 in bitmap plot a glyph
            {
                //uint8_t data[] = { color >> 8, color & 0xFF };
                //ST7735_WriteData(data, sizeof(data));
								//handle->fill_area(handle, color, 1);
								handle->put_pixel(handle, color);
            }
            else// if logic 0 in bitmap plot a background
            {
                //uint8_t data[] = { bgcolor >> 8, bgcolor & 0xFF };
                //ST7735_WriteData(data, sizeof(data));
								//handle->fill_area(handle, bgcolor, 1);
								handle->skip_pixel(handle);
            }
            /////////////////////////////////////////////////////////

        }



    }
}


uint16 if_gui_drawstring(struct gui_handle * handle, uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor)
{
	//TFT_CS_L();//cs pin low
	uint16 startx  =x ;
    if(t==0){
	for(z=11;z<=387;z+=4)
      {
        dmap[f] = z;
	    f++;
	  }
	f=0;
	z=0;
	t++;
    }

    while(*str)
    {
       if(*str!=32){
    	d = font.data[dmap[(*str)]-3];
        b=d;
    	if(d==1){
    		b=8;
    	}

    	/* limit and newline check */
        if(x+b  >= handle->width)
        {
            x = 0;//set x = 0
            y += ((font.data[dmap[0]-4]<<8)|(font.data[dmap[0]-5]));//enter newline

            ///////////////////////////////////////////////////////////////////////////////////////
            if(y + ((font.data[dmap[0]-4]<<8)|(font.data[dmap[0]-5])) >= handle->height)
            {
                break;
            }

        }


        if_gui_putchar(handle, x, y, *str, font, color, bgcolor, d);//plot
        if(d==1){
          d=7;
        }
        if(d%8!=0){
          x += (d+1);//
        }else{
          x += d;
        }
       }else{
    	  handle->set_area(handle, x, y, ((font.data[dmap[55]-3])/2) +1, y+((font.data[dmap[0]-4]<<8)|(font.data[dmap[0]-5])));
          for(z=0;z<(((font.data[dmap[55]-3])/2)*((font.data[dmap[0]-4]<<8)|(font.data[dmap[0]-5])));z++){
        	  //uint8_t data[] = { bgcolor >> 8, bgcolor & 0xFF };
        	  //ST7735_WriteData(data, sizeof(data));
						//handle->fill_area(handle, bgcolor, 1);
							handle->skip_pixel(handle);
          }

           x+=((font.data[dmap[55]-3])/2);

       }
        str++;//nextchar
    }
    if_gui_putchar(handle, x+b, y, 32, font, color, bgcolor,font.data[dmap[0]-3]);
    //TFT_CS_H();//cs pin high
	return x - startx;
}


static uint16 if_gui_print(struct gui_handle * handle, uint8 size, uint16 x, uint16 y, uint8 * str, uint32 color) {
	uint16_t usIndex;
	char temp[2] = {0, 0};
    uint16_t usWidth = 0;
	uint16 sta_x = x;
    uint16 x0 = x;
	uint16 y0 = y;
	uint8 * pcStr = str;
	uint16 total_width = 0;
	if(size == 38) {
		//large font (34x38)
		return if_gui_drawstring(handle, x, y, str, g_font_verdana_34x38, color, UI_COLOR_BLACK);
	}
	if(size == 29) {
		//large font (34x38)
		return if_gui_drawstring(handle, x, y, str, g_font_lcd_mono_17x29, color, UI_COLOR_BLACK);
	}
	while(1)
    {
        if(*pcStr == 0) 
		{
            break;                                     /* ﾗﾖｷ逸ｮｽ睫・           */
        }      
        x0 = x0 + (usWidth);                           /* ｵﾚﾗﾖｷ逸ｮﾏﾔﾊｾﾋﾉｽﾈ         */
        if(*pcStr > 0x80)                              /* ﾅﾐｶﾏﾎｪｺｺﾗﾖ                   */
        {
		    if((x0 + size) > handle->width)                      /* ｼ・鯡｣ﾓ狒ﾕｼ萍ﾇｷ羯ｻ         */
            {
			    x0 = 0;
                y0 = y0 + size;                          /* ｸﾄｱ蔆ﾔﾊｾﾗ・                */
                if(y0 > handle->height) y0 = 0;
            }
                                                       /* ﾏﾔﾊｾﾗﾖｷ・                    */
            pcStr += 2;
        }
		else 
		{                                               /* ﾅﾐｶﾏﾎｪｷﾇｺｺﾗﾖ                 */
            if (*pcStr == '\r')                         /* ｻｻﾐﾐ                         */
            { 
			    x0 = sta_x;
                pcStr++;
                usWidth = 0;
                continue;
            } 
			else if (*pcStr == '\n')                    /* ｶﾔﾆ・ｽﾆ・                  */
            {
			    x0 = sta_x;
			    y0 = y0 + size;                           /* ｸﾄｱ蔆ﾔﾊｾﾗ・                */
                if(y0 > handle->height) y0 = 0;
                pcStr++;
                usWidth = 0;
                continue;
            } 
			else 
			{
                if((x0 + 8) > handle->width)                     /* ｼ・鯡｣ﾓ狒ﾕｼ萍ﾇｷ羯ｻ         */
                {
				    x0 = 0;
                    y0 = y0 + size;                        /* ｸﾄｱ蔆ﾔﾊｾﾗ・                */
                    if(y0 > handle->height) y0 = 0;
                }
				switch(size) {
					case 38:
						//large font (34x38)
						temp[0] = *pcStr;
						if_gui_drawstring(handle, x0, y0, temp, g_font_verdana_34x38, color, UI_COLOR_BLACK);
					usWidth = 34;
						break;
					case 24:
						//large font (17x24)
						usWidth = if_gui_putc_24(handle, (uint8 *)(g_font_17x24 + ((*pcStr * 24) * 3)), 17, 24, x0, y0, color);
						break;
					case 20:
						//large font (14x20)
						usWidth = if_gui_putc_16(handle, (uint8 *)(g_font_14x20 + ((*pcStr * 20) * 2)), 14, 20, x0, y0, color);
						break;
					case 16:
						//large font (8x16)
						usWidth = if_gui_putc_8(handle, (uint8_t *)(g_font_8x16 + (*pcStr * 16)), 8, 16, x0, y0, color);
						break;
					case 12:
						//small font (7x12)
						usWidth = if_gui_putc_8(handle, (uint8_t *)(g_font_7x12 + (*pcStr * 8)), 7, 12, x0, y0, color);
						break;
					case 8:
						//small font (6x8)
						usWidth = if_gui_putc_8(handle, (uint8_t *)(g_font_6x8 + (*pcStr * 8)), 6, 8, x0, y0, color);
						break;
					default: break;
				} 
				total_width += usWidth;
                pcStr += 1;
            }
		}
	}	
		return total_width;
}

uint16 if_gui_measurestring(struct gui_handle * handle, uint8 size, const char* str)
{
	FontDef font = g_font_verdana_34x38;
	if(size == 38) font = g_font_verdana_34x38;
	else if(size == 29) font = g_font_lcd_mono_17x29;
	uint16 x = 0, y = 0;
	//TFT_CS_L();//cs pin low
	uint16 startx  =x ;
    if(t==0){
	for(z=11;z<=387;z+=4)
      {
        dmap[f] = z;
	    f++;
	  }
	f=0;
	z=0;
	t++;
    }

    while(*str)
    {
       if(*str!=32){
    	d = font.data[dmap[(*str)]-3];
        b=d;
    	if(d==1){
    		b=8;
    	}

    	/* limit and newline check */
        if(x+b  >= handle->width)
        {
            x = 0;//set x = 0
            y += ((font.data[dmap[0]-4]<<8)|(font.data[dmap[0]-5]));//enter newline

            ///////////////////////////////////////////////////////////////////////////////////////
            if(y + ((font.data[dmap[0]-4]<<8)|(font.data[dmap[0]-5])) >= handle->height)
            {
                break;
            }

        }


        //if_gui_measurechar(handle, x, y, *str, font, color, bgcolor, d);//plot
        if(d==1){
          d=7;
        }
        if(d%8!=0){
          x += (d+1);//
        }else{
          x += d;
        }
       }else{
    	  //handle->set_area(handle, x, y, ((font.data[dmap[55]-3])/2) +1, y+((font.data[dmap[0]-4]<<8)|(font.data[dmap[0]-5])));
          for(z=0;z<(((font.data[dmap[55]-3])/2)*((font.data[dmap[0]-4]<<8)|(font.data[dmap[0]-5])));z++){
        	  //uint8_t data[] = { bgcolor >> 8, bgcolor & 0xFF };
        	  //ST7735_WriteData(data, sizeof(data));
						//handle->fill_area(handle, bgcolor, 1);
          }

           x+=((font.data[dmap[55]-3])/2);

       }
        str++;//nextchar
    }
    //if_gui_measurechar(handle, x+b, y, 32, font, color, bgcolor,font.data[dmap[0]-3]);
    //TFT_CS_H();//cs pin high
	return x - startx;
}

static void if_gui_bitmap(struct gui_handle * handle, uint32 forecolor, uint32 backcolor, uint8 * map, uint32 size) {
	//LCD_WriteBMP(xsta, ysta, xend - xsta, yend - ysta, uint8_t *bitmap);
	uint32_t index;
  	//uint32_t size = h * w;
	uint16_t fcolor = IF_COLOR(forecolor);
	uint16_t bcolor = IF_COLOR(backcolor);
  	//uint16_t *bitmap_ptr = (uint16_t *)bitmap;
  	//LCD_WriteRAM_Prepare();
	handle->begin_burst(handle);

  	for(index = 0; index < size; index++)
  	{
		if(map[index>>3] & (1 << (index & 0x7))) handle->put_pixel(handle, fcolor);
		else handle->put_pixel(handle, bcolor);
  	}
}

void if_display_wake(gui_handle_p handle) {
	if(handle == NULL) return;
	if(handle->init == NULL) return;
	handle->wake(handle);
}

void if_display_sleep(gui_handle_p handle) {
	if(handle == NULL) return;
	if(handle->init == NULL) return;
	handle->sleep(handle); 
}

uint8 if_gui_init(gui_handle_p handle, uint8 orientation, uint8 brightness) {
	if_delay(1000);
	memset(handle, 0, sizeof(gui_handle));
	if(ili934x_init(handle) == 0) goto init_gui;
	if(ssd1351_init(handle) == 0) goto init_gui;
	if(rk043fn48h_init(handle) == 0) goto init_gui;
	return -1;
	init_gui:
	handle->body = NULL;
	handle->header = NULL;

	handle->print_string = if_gui_print;
	handle->measure_string = if_gui_measurestring;
	handle->draw_rectangle = if_gui_rectangle;
	handle->draw_line = if_gui_line;
	handle->fill_area = if_gui_fill;
	handle->fill_bitmap = if_gui_bitmap;
	handle->status = 0;
	//init default user interface
	ui_display_init(handle);
	handle->brightness = brightness;					//set current screen brightness
	ui_switch_orientation(handle, orientation);			//change orientation based configuration (2017.10.22)
	if_display_wake(handle);
	if_gui_fill(handle, UI_COLOR_BLACK, handle->set_area(handle, 0, 0, ((gui_handle_p)handle)->width , ((gui_handle_p)handle)->height)) ;
	handle->present(handle);
	//activate backlight
	if_delay(300);
	handle->set_backlight(handle, 1);
	return 0;
}

void * if_gui_present(gui_handle_p display) {
#if SHARD_RTOS_ENABLED
	ui_present(display);
	if(display->present != NULL) display->present(display);
	return NULL;
#else
	uint16 counter = 0;
	void * obj = ui_process_events(display);
	if_delay(20);
	ui_present(display);
	if(display->present != NULL) display->present(display);
	return obj;
#endif
}


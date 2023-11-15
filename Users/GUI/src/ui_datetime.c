#ifdef WIN32
//#include "stdafx.h"
#endif
#include "..\..\defs.h"
#include "..\..\config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\inc\ui_resources.h"
#include "..\inc\ui_datetime.h"
#include "..\inc\ui_item.h"
#include "..\inc\ui_button.h"
#include "..\..\interfaces\inc\if_touch.h"
#include "..\..\interfaces\inc\if_apis.h"

CONST dt_object gt_dtymfield[] = {
	//hour field
	{ 0, 0, 64, 32, UI_DTSHOW_YEAR, DT_OBJ_YEAR },
	{ 64, 0, 32, 32, UI_DTSHOW_YEAR, DT_OBJ_YEAR_UP },
	{ 96, 0, 32, 32, UI_DTSHOW_YEAR, DT_OBJ_YEAR_DOWN },
	//month field
	{ 128, 0, 64, 32, UI_DTSHOW_MONTH, DT_OBJ_MONTH },
	{ 192, 0, 32, 32, UI_DTSHOW_MONTH, DT_OBJ_MONTH_UP },
	{ 224, 0, 32, 32, UI_DTSHOW_MONTH, DT_OBJ_MONTH_DOWN },
	//date field
	{ 0, 32, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 1 },
	{ 32, 32, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 2 },
	{ 64, 32, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 3 },
	{ 96, 32, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 4 },
	{ 128, 32, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 5 },
	{ 160, 32, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 6 },
	{ 192, 32, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 7 },
	{ 224, 32, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 8 },
	
	{ 0, 64, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 9 },
	{ 32, 64, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 10 },
	{ 64, 64, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 11 },
	{ 96, 64, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 12 },
	{ 128, 64, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 13 },
	{ 160, 64, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 14 },
	{ 192, 64, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 15 },
	{ 224, 64, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 16 },
	
	{ 0, 96, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 17 },
	{ 32, 96, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 18 },
	{ 64, 96, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 19 },
	{ 96, 96, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 20 },
	{ 128, 96, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 21 },
	{ 160, 96, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 22 },
	{ 192, 96, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 23 },
	{ 224, 96, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 24 },
	
	{ 0, 128, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 25 },
	{ 32, 128, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 26 },
	{ 64, 128, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 27 },
	{ 96, 128, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 28 },
	{ 128, 128, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 29 },
	{ 160, 128, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 30 },
	{ 192, 128, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 31 },
	//hour field
	{ 0, 160, 24, 32, UI_DTSHOW_HOUR, DT_OBJ_HOUR },
	{ 24, 160, 30, 32, UI_DTSHOW_HOUR, DT_OBJ_HOUR_UP },
	{ 54, 160, 30, 32, UI_DTSHOW_HOUR, DT_OBJ_HOUR_DOWN },
	//month field
	{ 88, 160, 24, 32, UI_DTSHOW_MINUTE, DT_OBJ_MINUTE },
	{ 112, 160, 30, 32, UI_DTSHOW_MINUTE, DT_OBJ_MINUTE_UP },
	{ 142, 160, 30, 32, UI_DTSHOW_MINUTE, DT_OBJ_MINUTE_DOWN },
	//second field
	{ 176, 160, 24, 32, UI_DTSHOW_SECOND, DT_OBJ_SECOND },
	{ 200, 160, 30, 32, UI_DTSHOW_SECOND, DT_OBJ_SECOND_UP },
	{ 230, 160, 30, 32, UI_DTSHOW_SECOND, DT_OBJ_SECOND_DOWN },
	//cancel button
	{ 0, 192, 130, 48, UI_DTSHOW_SUBMIT, DT_OBJ_CANCEL },
	//exit button
	{ 130, 192, 130, 48, UI_DTSHOW_SUBMIT, DT_OBJ_DONE },
	//end mark
	{ 0, 0, 0, 0, 0, DT_OBJ_END },
};


///YYYYMMDD
CONST dt_object gt_dymfield[] = {
	//hour field
	{ 0, 0, 64, 32, UI_DTSHOW_YEAR, DT_OBJ_YEAR },
	{ 64, 0, 32, 32, UI_DTSHOW_YEAR, DT_OBJ_YEAR_UP },
	{ 96, 0, 32, 32, UI_DTSHOW_YEAR, DT_OBJ_YEAR_DOWN },
	//month field
	{ 128, 0, 64, 32, UI_DTSHOW_MONTH, DT_OBJ_MONTH },
	{ 192, 0, 32, 32, UI_DTSHOW_MONTH, DT_OBJ_MONTH_UP },
	{ 224, 0, 32, 32, UI_DTSHOW_MONTH, DT_OBJ_MONTH_DOWN },
	//date field
	{ 0, 32, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 1 },
	{ 32, 32, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 2 },
	{ 64, 32, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 3 },
	{ 96, 32, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 4 },
	{ 128, 32, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 5 },
	{ 160, 32, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 6 },
	{ 192, 32, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 7 },
	{ 224, 32, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 8 },
	
	{ 0, 64, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 9 },
	{ 32, 64, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 10 },
	{ 64, 64, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 11 },
	{ 96, 64, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 12 },
	{ 128, 64, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 13 },
	{ 160, 64, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 14 },
	{ 192, 64, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 15 },
	{ 224, 64, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 16 },
	
	{ 0, 96, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 17 },
	{ 32, 96, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 18 },
	{ 64, 96, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 19 },
	{ 96, 96, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 20 },
	{ 128, 96, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 21 },
	{ 160, 96, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 22 },
	{ 192, 96, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 23 },
	{ 224, 96, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 24 },
	
	{ 0, 128, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 25 },
	{ 32, 128, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 26 },
	{ 64, 128, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 27 },
	{ 96, 128, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 28 },
	{ 128, 128, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 29 },
	{ 160, 128, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 30 },
	{ 192, 128, 32, 32, UI_DTSHOW_DATE, DT_OBJ_DATE_XX | 31 },
	//cancel button
	{ 0, 160, 130, 48, UI_DTSHOW_SUBMIT, DT_OBJ_CANCEL },
	//exit button
	{ 130, 160, 130, 48, UI_DTSHOW_SUBMIT, DT_OBJ_DONE },
	//end mark
	{ 0, 0, 0, 0, 0, DT_OBJ_END },
};

CONST dt_object gt_ymfield[] = {
	//hour field
	{ 0, 0, 64, 32, UI_DTSHOW_YEAR, DT_OBJ_YEAR },
	{ 64, 0, 32, 32, UI_DTSHOW_YEAR, DT_OBJ_YEAR_UP },
	{ 96, 0, 32, 32, UI_DTSHOW_YEAR, DT_OBJ_YEAR_DOWN },
	//month field
	{ 128, 0, 64, 32, UI_DTSHOW_MONTH, DT_OBJ_MONTH },
	{ 192, 0, 32, 32, UI_DTSHOW_MONTH, DT_OBJ_MONTH_UP },
	{ 224, 0, 32, 32, UI_DTSHOW_MONTH, DT_OBJ_MONTH_DOWN },
	//cancel button
	{ 0, 32, 130, 48, UI_DTSHOW_SUBMIT, DT_OBJ_CANCEL },
	//exit button
	{ 130, 32, 130, 48, UI_DTSHOW_SUBMIT, DT_OBJ_DONE },
	//end mark
	{ 0, 0, 0, 0, 0, DT_OBJ_END },
};

//time only (HHMMSS)
CONST dt_object gt_tfield[] = {
	//hour field
	{ 0, 0, 24, 32, UI_DTSHOW_HOUR, DT_OBJ_HOUR },
	{ 24, 0, 30, 32, UI_DTSHOW_HOUR, DT_OBJ_HOUR_UP },
	{ 54, 0, 30, 32, UI_DTSHOW_HOUR, DT_OBJ_HOUR_DOWN },
	//month field
	{ 88, 0, 24, 32, UI_DTSHOW_MINUTE, DT_OBJ_MINUTE },
	{ 112, 0, 30, 32, UI_DTSHOW_MINUTE, DT_OBJ_MINUTE_UP },
	{ 142, 0, 30, 32, UI_DTSHOW_MINUTE, DT_OBJ_MINUTE_DOWN },
	//second field
	{ 176, 0, 24, 32, UI_DTSHOW_SECOND, DT_OBJ_SECOND },
	{ 200, 0, 30, 32, UI_DTSHOW_SECOND, DT_OBJ_SECOND_UP },
	{ 230, 0, 30, 32, UI_DTSHOW_SECOND, DT_OBJ_SECOND_DOWN },
	//cancel button
	{ 0, 32, 130, 48, UI_DTSHOW_SUBMIT, DT_OBJ_CANCEL },
	//exit button
	{ 130, 32, 130, 48, UI_DTSHOW_SUBMIT, DT_OBJ_DONE },
	//end mark
	{ 0, 0, 0, 0, 0, DT_OBJ_END },
};

//ok/cancel only (HHMMSS)
CONST dt_object gt_field[] = {
	//cancel button
	{ 0, 0, 130, 48, UI_DTSHOW_SUBMIT, DT_OBJ_CANCEL },
	//exit button
	{ 130, 0, 130, 48, UI_DTSHOW_SUBMIT, DT_OBJ_DONE },
	//end mark
	{ 0, 0, 0, 0, 0, DT_OBJ_END },
};

uint8_c * g_str_months[] = { 
	(uint8_c *)"",
	(uint8_c *)"Jan", 
	(uint8_c *)"Feb", 
	(uint8_c *)"Mar", 
	(uint8_c *)"Apr", 
	(uint8_c *)"May", 
	(uint8_c *)"Jun", 
	(uint8_c *)"Jul", 
	(uint8_c *)"Agt", 
	(uint8_c *)"Sep", 
	(uint8_c *)"Oct", 
	(uint8_c *)"Nov", 
	(uint8_c *)"Dec" 
};

uint8_c * g_str_days[] = {
	(uint8_c *)"",
	(uint8_c *)"Mon",
	(uint8_c *)"Tue",
	(uint8_c *)"Wed",
	(uint8_c *)"Thu",
	(uint8_c *)"Fri",
	(uint8_c *)"Sat",
	(uint8_c *)"Sun",
};

static uint8_c g_str_year_table[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static uint8_c g_str_leap_year_table[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };


static uint8 ui_is_leap_year(uint16 year) {
	if((year % 400) == 0) return 1;
	if((year % 100) == 0) return 0;
	return ((year % 4) == 0);
}

uint32 ui_datetime_2_longtime(ui_datetime_value * val) {
	uint16 year;
	uint32 longtime = 0;
	uint8 m;
	uint16 days = 0;
	uint8 * m_table = (uint8 *)g_str_year_table;
	if(val == NULL) return longtime;
	if(val->year < 1970) return longtime;
	if(val->year > 2100) return longtime;
	year = val->year - 1970;
	longtime += year * (31556952);
	if(ui_is_leap_year(val->year)) m_table = (uint8 *)g_str_leap_year_table;
	for(m =0;m < 12;m++) {
		if((m + 1) == val->month) {
			longtime += (days * 0x00015180);
			break;
		}
		days += m_table[m];
	}
	val->hour %= 24;
	val->minute %= 60;
	val->second %= 60;
	longtime += ((val->date - 1) * 0x00015180);
	longtime += (val->hour * 3600);
	longtime += (val->minute * 60);
	longtime += val->second;
	return longtime;
}

void ui_longtime_2_datetime(ui_datetime_value * val, uint32 longtime) {
	uint16 year;
	uint16 max_days;
	uint32 total_days;
	uint8 * m_table = (uint8 *)g_str_year_table;
	uint8 month;
	uint8 date;
	uint32 starttime;
	uint32 shorttime = (longtime % 0x00015180);
	if(val == NULL) return;
	
	year = longtime / (31556952);		//daily tick * average yearly day
	year += 1970;
	//total_days = longtime / 0x00015180;						//total days since 1 january 1970
	total_days = (uint32)(longtime % 31556952);		//total days within one year
	total_days /= 0x00015180;
	if(ui_is_leap_year(year)) m_table = (uint8 *)g_str_leap_year_table;
	for(month = 1;month<=12;month++) {
		if(total_days < m_table[month - 1]) {
			date = (total_days + 1);
			break;
		} 
		total_days -= m_table[month - 1];
	}
	val->year = year;
	val->month = month;
	val->date = date;
	val->hour = 0;
	val->minute = 0;
	val->second = 0;
	starttime = ui_datetime_2_longtime(val);
	shorttime = longtime - starttime;
	//daily tick
	val->hour = shorttime / 3600;
	val->minute = (shorttime % 3600) / 60;
	val->second = shorttime % 60;
}

static uint8 ui_check_consecutive_char(uint8 * buffer, uint8 c) {
	uint8 i = 0, d;
	while((d = buffer[i++]) != 0 && d == c);
	return i;
}

static uint8 ui_print_padded_string(uint8 * buffer, uint8 plen, uint32 value) {
	uint8 i;
	for(i=plen;i>0;i--) {
		buffer[i-1] = 0x30 | (value % 10);
		value /= 10;
	}
	return plen;
}

static void ui_datetime_to_text(ui_datetime_value * val, uint8 * format, uint8 * text) {
	uint8 i;
	uint8 len;
	uint16 mode = 0;
	uint8 tlen;
	uint8 tbuf[40];
	len = strlen((const char *)format);
	for(i=0;i<len;) {
		switch(format[i]) {
			case 'Y':
			case 'y':
				tlen = ui_check_consecutive_char(format +i, format[i]);
				ui_print_padded_string(text + i, tlen, val->year);
				i+= tlen;
				break;
			case 'M':
				tlen = ui_check_consecutive_char(format +i, format[i]);
				ui_print_padded_string(text + i, tlen, val->month);
				i+= tlen;
				break;
			case 'D':
				tlen = ui_check_consecutive_char(format +i, format[i]);
				ui_print_padded_string(text + i, tlen, val->date);
				i+= tlen;
				break;
			case 'H':
			case 'h':
				tlen = ui_check_consecutive_char(format +i, format[i]);
				ui_print_padded_string(text + i, tlen, val->hour);
				i+= tlen;
				break;
			case 'm':
				tlen = ui_check_consecutive_char(format +i, format[i]);
				ui_print_padded_string(text + i, tlen, val->minute);
				i+= tlen;
				break;
			case 'S':
			case 's':
				tlen = ui_check_consecutive_char(format +i, format[i]);
				ui_print_padded_string(text + i, tlen, val->second);
				i+= tlen;
				break;
			default: i++; break;
		}
	}
	
}

static uint16 ui_datetime_format_mode(uint8 * format) {
	uint8 i;
	uint8 len;
	uint16 mode = 0;
	len = strlen((const char *)format);
	for(i=0;i<len;i++) {
		switch(format[i]) {
			case 'Y':
			case 'y':
				mode |= UI_DTSHOW_YEAR;
				break;
			case 'M':
				mode |= UI_DTSHOW_MONTH;
				break;
			case 'D':
				mode |= UI_DTSHOW_DATE;
				break;
			case 'H':
			case 'h':
				mode |= UI_DTSHOW_HOUR;
				break;
			case 'm':
				mode |= UI_DTSHOW_MINUTE;
				break;
			case 'S':
			case 's':
				mode |= UI_DTSHOW_SECOND;
				break;
		}				
	}
	return mode;
}

static void ui_datetime_draw_item(gui_handle_p display, uint16 x, uint16 y, uint16 w, uint16 h, uint8 * text, uint32 backcolor) {
	uint16 nc, wtxt, xx, yy;
	nc = strlen((const char *)text);
	wtxt = nc * 8;
	xx = ((w - wtxt) >> 1);		//x = width - text_width / 2;
	yy = (h - UI_FONT_DEFAULT) >> 1;
	display->fill_area(display, backcolor, display->set_area(display, x, y, w, h));
	display->print_string(display, UI_FONT_DEFAULT, x + xx, y + yy, text, UI_COLOR_WHITE);
	display->draw_rectangle(display, x+1, y+1, w-2, h-2, UI_COLOR_WHITE, 0);
}

static void ui_datetime_draw_text(gui_handle_p display, uint16 x, uint16 y, uint16 w, uint16 h, uint8 * text, uint32 backcolor) {
	uint16 nc, wtxt, xx, yy;
	nc = strlen((const char *)text);
	wtxt = nc * 8;
	xx = ((w - wtxt) >> 1);		//x = width - text_width / 2;
	yy = (h - UI_FONT_DEFAULT) >> 1;
	display->fill_area(display, backcolor, display->set_area(display, x, y, w, h));
	display->print_string(display, UI_FONT_DEFAULT, x + xx, y + yy, text, UI_COLOR_WHITE);
	//display->draw_rectangle(display, x+1, y+1, w-2, h-2, UI_COLOR_WHITE, 0);
}

static void ui_datetime_draw_button(gui_handle_p display, uint16 x, uint16 y, uint16 w, uint16 h, uint8 * text, uint8 mode) {
	uint16 nc, wtxt, xx, yy;
	ui_rect rect = { x, y, w, h };
	nc = strlen((const char *)text);
	wtxt = nc * 8;
	xx = ((w - wtxt) >> 1);		//x = width - text_width / 2;
	yy = (h - UI_FONT_DEFAULT) >> 1;
	//display->fill_area(display, backcolor, display->set_area(display, x, y, w, h));
	ui_draw_button(display, &rect, mode);
	display->print_string(display, UI_FONT_DEFAULT, x + xx, y + yy, text, UI_COLOR_WHITE);
	//display->draw_rectangle(display, x+1, y+1, w-2, h-2, UI_COLOR_WHITE, 0);
}

static void ui_datetime_field_render(gui_handle_p display, uint16 xx, uint16 yy, dt_object * obj, ui_datetime_value * val, uint32 backcolor, uint8 state) {
	dt_object * iterator = obj;
	uint8 buffer[19];
	buffer[0] = 0;			//clear text buffer
	switch(iterator->type & 0xF0) {
		case 0x00:
			//field type
			switch(iterator->type & 0x0F) {
				case 0x00:			//year
					sprintf((char *)buffer, "%04d", val->year); 
					break;
				case 0x01:			//month
					sprintf((char *)buffer, "%s", g_str_months[val->month % 14]); 
					break;
				case 0x04:			//hour
					sprintf((char *)buffer, "%02d", val->hour % 24); 
					break;
				case 0x05:			//minute
					sprintf((char *)buffer, "%02d", val->minute % 60); 
					break;
				case 0x06:			//second
					sprintf((char *)buffer, "%02d", val->second % 60); 
					break;
				case 0x07:		//days
					sprintf((char *)buffer, "%s", g_str_days[val->day % 8]); 
					break;
				default: break;
			}
			ui_datetime_draw_item(display, iterator->x + xx, iterator->y + yy, iterator->w, iterator->h, buffer, backcolor);
			break;
		case 0x10: 
			sprintf((char *)buffer, "+"); 
			ui_datetime_draw_button(display, iterator->x + xx, iterator->y + yy, iterator->w, iterator->h, buffer, state);
			break;
		case 0x20: 
			sprintf((char *)buffer, "-"); 
			ui_datetime_draw_button(display, iterator->x + xx, iterator->y + yy, iterator->w, iterator->h, buffer, state);
			break;
		case 0x70:
		case 0x60:
		case 0x50:
		case 0x40:
			sprintf((char *)buffer, "%02d", iterator->type & 0x3F); 
			if((iterator->type & 0x3F) == val->date) backcolor = UI_COLOR_QUEEN_BLUE;
			ui_datetime_draw_item(display, iterator->x + xx, iterator->y + yy, iterator->w, iterator->h, buffer, backcolor);
			break;
		case 0xF0:			//exit button
			if(iterator->type & 0x01) ui_datetime_draw_button(display, iterator->x + xx, iterator->y + yy, iterator->w, iterator->h, (uint8 *)"Cancel", state);
			else ui_datetime_draw_button(display, iterator->x + xx, iterator->y + yy, iterator->w, iterator->h, (uint8 *)"Done", state);
			break;
		default: break;
	}
}

static void ui_datetime_input_render(ui_object * obj, gui_handle_p display) {
	uint16 i = 0;
	ui_datetime * dtime;
	uint16 xx,yy;
	uint16 cx,cy;
	dt_object * iterator;
	uint32 backcolor = UI_COLOR_RGB(23,24,36);
	ui_datetime_value * val;
	uint8 max_date = 31;
	if(obj == NULL) return;
	dtime = (ui_datetime *)obj->target;
	if(dtime == NULL) return;
	iterator = (dt_object *)dtime->field;
	xx = ((ui_rect *)obj)->x;
	yy = ((ui_rect *)obj)->y;
	//get current time configuration
	val = &dtime->value;
	if((val->month % 2) == 0) max_date = 30;
	if(val->month == 2) {
		if(ui_is_leap_year(val->year)) max_date = 29;
		else max_date = 28;
	}
	//initialize render
	if(obj->state & UI_STATE_INIT) {
		ui_panel_render(obj, display);
	}
	iterator = (dt_object *)dtime->field;
	while(iterator != NULL) {
		if((iterator->state & dtime->state) == iterator->state) {		//check if mode matched
			switch(iterator->type & 0xF0) {
				case 0x50:
				case 0x40:
					//render date field (if exist, checked state)
					if((iterator->type & 0x1F) > max_date) { 
						display->fill_area(display, obj->backcolor, display->set_area(display, xx + iterator->x, yy + iterator->y, iterator->w, iterator->h));
						break;
					}
				default:
					//render datetime item
					ui_datetime_field_render(display, xx, yy, iterator, val, UI_COLOR_RGB(58,60,66), UI_BUTTON_STATE_NONE);
					break;
			}
		}
		iterator++;
		if(iterator->type == DT_OBJ_END) break;
	}
	if(obj->state & (UI_STATE_KEYUP | UI_STATE_KEYDOWN)) {
		if_touch_get(display, &cx, &cy);
		cy = cy - yy;
		cx = cx - xx;
		iterator = (dt_object *)dtime->field;
		while(iterator->type != DT_OBJ_END) {
			if(cx > (iterator->x) && cx <= (iterator->x + iterator->w)) {
				if(cy > (iterator->y) && cy <= (iterator->y + iterator->h)) {
					//render datetime click item
					if((iterator->state & dtime->state) == iterator->state) {		//check if mode matched
						if(obj->state & UI_STATE_KEYDOWN) backcolor = UI_COLOR_QUEEN_BLUE;
						else backcolor = UI_COLOR_RGB(58,60,66);
						if(obj->state & UI_STATE_KEYDOWN) {
							ui_datetime_field_render(display, xx, yy, iterator, val, backcolor, UI_BUTTON_STATE_PRESSED);
						} else {
							ui_datetime_field_render(display, xx, yy, iterator, val, backcolor, UI_BUTTON_STATE_NONE);
						}
					}
				}
			}
			iterator++;
		}
	}
}

static void ui_datetime_input_handler(ui_object * obj, void * params) {	
	dt_object * iterator;
	ui_datetime_value * val;
	ui_datetime * dtime;
	uint16 xx,yy;
	uint16 cx,cy;
	uint8 max_date = 31;
	if(obj == NULL) return;
	dtime = (ui_datetime *)obj->target;
	if(dtime == NULL) return;
	xx = ((ui_rect *)obj)->x;
	yy = ((ui_rect *)obj)->y;
	//get current time configuration
	val = &dtime->value;
	if((val->month % 2) == 0) max_date = 30;
	if(val->month == 2) {
		if(ui_is_leap_year(val->year)) max_date = 29;
		else max_date = 28;
	}
	iterator = (dt_object *)dtime->field;
	if_touch_get(params, &cx, &cy);
	cy = cy - yy;
	cx = cx - xx;
	while(iterator != NULL) {
		if(cx > (iterator->x) && cx <= (iterator->x + iterator->w)) {
			if(cy > (iterator->y) && cy <= (iterator->y + iterator->h)) {
				if((iterator->state & dtime->state) == iterator->state) {
					switch(iterator->type & 0xF0) {
						case 0x00:
							break;
						case 0x10: 			//up button
							switch(iterator->type & 0x0F) {
								case 0x00:			//year
									if(val->year >= 2100) break;
									val->year++;
									break;
								case 0x01:			//month
									val->month++;
									if(val->month > 12) val->month = 1;
									break;
								case 0x04:			//hour
									val->hour++;
									if(val->hour >= 24) val->hour = 0;
									break;
								case 0x05:			//minute
									val->minute++;
									if(val->minute >= 60) val->minute = 0;
									break;
								case 0x06:			//second
									val->second++;
									if(val->second >= 60) val->second = 0;
									break;
								case 0x07:		//days
									//sprintf((char *)buffer, "%s", g_str_days[val->day % 8]); 
									break;
								default: break;
							}
							//ui_reinitialize_object(obj);
							break;
						case 0x20: 
							switch(iterator->type & 0x0F) {
								case 0x00:			//year
									if(val->year <= 1970) break;
									val->year--;
									break;
								case 0x01:			//month
									val->month--;
									if(val->month <= 0) val->month = 12;
									break;
								case 0x04:			//hour
									val->hour--;
									if(val->hour > 24) val->hour = 23;
									break;
								case 0x05:			//minute
									val->minute--;
									if(val->minute > 60) val->minute = 59;
									break;
								case 0x06:			//second
									val->second--;
									if(val->second > 60) val->second = 59;
									break;
								case 0x07:		//days
									//sprintf((char *)buffer, "%s", g_str_days[val->day % 8]); 
									break;
								default: break;
							}
							//ui_reinitialize_object(obj);
							break;
						case 0x70:
						case 0x60:
						case 0x50:
						case 0x40:
							if((iterator->type & 0x1F) > max_date) break;
							val->date = (iterator->type & 0x3F);
							//ui_reinitialize_object(obj);
							break;
						case 0xF0:			//exit button
							if_time_set((datetime_p)val);
							ui_pop_screen((gui_handle_p)params);
							//remove keydown state from previous operation
							((ui_object *)dtime)->state &= ~(UI_STATE_KEYDOWN | UI_STATE_SELECTED);
							ui_reinitialize_object((ui_object *)dtime);
							break;
						default: break;
					}
				}
			}
		}
		iterator++;
		if(iterator->type == DT_OBJ_END) break;
	}
	//ui_reinitialize_object(obj);
}


static void ui_datetime_item_handler(ui_object * obj, void * params) {	
	dt_object * iterator;
	ui_datetime_value * val;
	ui_datetime_item * dtime;
	uint16 xx,yy;
	uint16 cx,cy;
	uint8 max_date;
	if(obj == NULL) return;
	dtime = (ui_datetime_item *)obj->target;
	if(dtime == NULL) return;
	xx = ((ui_rect *)obj)->x;
	yy = ((ui_rect *)obj)->y;
	//get current time configuration
	val = &dtime->base.value;
	if((val->month % 2) == 0) max_date = 30;
	if(val->month == 2) {
		if(ui_is_leap_year(val->year)) max_date = 29;
		else max_date = 28;
	}
	iterator = (dt_object *)dtime->base.field;
	if_touch_get(params, &cx, &cy);
	cy = cy - yy;
	cx = cx - xx;
	while(iterator != NULL) {
		if(cx > (iterator->x) && cx <= (iterator->x + iterator->w)) {
			if(cy > (iterator->y) && cy <= (iterator->y + iterator->h)) {
				if((iterator->state & ((ui_datetime *)dtime)->state) == iterator->state) {		//process input only when iterator->state matched
					switch(iterator->type & 0xF0) {
						case 0x00:
							break;
						case 0x10: 			//up button
							switch(iterator->type & 0x0F) {
								case 0x00:			//year
									if(val->year >= 2100) break;
									val->year++;
									break;
								case 0x01:			//month
									val->month++;
									if(val->month > 12) val->month = 1;
									break;
								case 0x04:			//hour
									val->hour++;
									if(val->hour >= 24) val->hour = 0;
									break;
								case 0x05:			//minute
									val->minute++;
									if(val->minute >= 60) val->minute = 0;
									break;
								case 0x06:			//second
									val->second++;
									if(val->second >= 60) val->second = 0;
									break;
								case 0x07:		//days
									//sprintf((char *)buffer, "%s", g_str_days[val->day % 8]); 
									break;
								default: break;
							}
							//ui_reinitialize_object(obj);
							break;
						case 0x20: 
							switch(iterator->type & 0x0F) {
								case 0x00:			//year
									if(val->year <= 1970) break;
									val->year--;
									break;
								case 0x01:			//month
									val->month--;
									if(val->month <= 0) val->month = 12;
									break;
								case 0x04:			//hour
									val->hour--;
									if(val->hour > 24) val->hour = 23;
									break;
								case 0x05:			//minute
									val->minute--;
									if(val->minute > 60) val->minute = 59;
									break;
								case 0x06:			//second
									val->second--;
									if(val->second > 60) val->second = 59;
									break;
								case 0x07:		//days
									//sprintf((char *)buffer, "%s", g_str_days[val->day % 8]); 
									break;
								default: break;
							}
							//ui_reinitialize_object(obj);
							break;
						case 0x70:
						case 0x60:
						case 0x50:
						case 0x40:
							if((iterator->type & 0x1F) > max_date) break;
							val->date = (iterator->type & 0x3F);
							//ui_reinitialize_object(obj);
							break;
						case 0xF0:			//exit button
							//remove keydown state from previous operation
							((ui_object *)dtime)->state &= ~(UI_STATE_KEYDOWN | UI_STATE_SELECTED);
							//ui_reinitialize_object((ui_object *)dtime);
							break;
						default: break;
					}
				}
			}
		}
		iterator++;
		if(iterator->type == DT_OBJ_END) break;
	}
	//change text to matched current datetime value according to spceified format
	ui_datetime_to_text(&dtime->base.value, dtime->format, dtime->dtval);
}

static void ui_datetime_button_render(ui_object * obj, gui_handle_p display) {
	uint16 nc, wtxt, xx, yy;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint8 * tbuffer[48];
	uint8 * fbuffer[19];
	ui_datetime * dt = (ui_datetime *)obj;
	if(obj->state & UI_STATE_INIT) {
		memset(tbuffer, 0, sizeof(tbuffer));
		ui_item_render(obj, display);
		display->set_area(display, x, y, w, h);
		if(((ui_object *)obj)->image.buffer != NULL) xx = 52;
		else xx = 4;		//x = width - text_width / 2;
		yy = ((h - UI_FONT_DEFAULT) >> 1);		//y = height - text_height /2;
		if(dt->state & UI_DTSHOW_MONTH) {
			sprintf((char *)fbuffer, "%s ", g_str_months[dt->value.month]);
			strcat((char *)tbuffer, (const char *)fbuffer);
		}
		if(dt->state & UI_DTSHOW_DATE) {
			sprintf((char *)fbuffer, "%02d ", dt->value.date);
			strcat((char *)tbuffer, (const char *)fbuffer);
		}
		if(dt->state & UI_DTSHOW_YEAR) {
			sprintf((char *)fbuffer, "%04d ", dt->value.year);
			strcat((char *)tbuffer, (const char *)fbuffer);
		}
		if(dt->state & UI_DTSHOW_DAY) {
			sprintf((char *)fbuffer, "%s, ", g_str_days[dt->value.day]);
			strcat((char *)tbuffer, (const char *)fbuffer);
		}
		if(dt->state & UI_DTSHOW_HOUR) {
			if(dt->state & (UI_DTSHOW_MINUTE | UI_DTSHOW_SECOND)) sprintf((char *)fbuffer, "%02d:", dt->value.hour % 24);
			else sprintf((char *)fbuffer, "%02d", dt->value.hour % 24);			//hour only
			strcat((char *)tbuffer, (const char *)fbuffer);
		}
		if(dt->state & UI_DTSHOW_MINUTE) {
			if(dt->state & UI_DTSHOW_SECOND) sprintf((char *)fbuffer, "%02d:", dt->value.minute % 60);
			else sprintf((char *)fbuffer, "%02d", dt->value.minute % 60);
			strcat((char *)tbuffer, (const char *)fbuffer);
		}
		if(dt->state & UI_DTSHOW_SECOND) {
			sprintf((char *)fbuffer, "%02d", dt->value.second % 60);
			strcat((char *)tbuffer, (const char *)fbuffer);
		}
		display->print_string(display, UI_FONT_DEFAULT, x + xx, y + yy, (uint8 *)tbuffer, ((ui_object *)obj)->forecolor);
		display->fill_area(display, UI_COLOR_RGB(79,81,89), display->set_area(display, x, y + h-2, w, 1));
		display->fill_area(display, UI_COLOR_RGB(60,62,70), display->set_area(display, x, y + h-1, w, 1));
	}
}

static void ui_datetime_button_handler(ui_object * obj, void * params) {
	ui_datetime_show((gui_handle_p)params, NULL, (ui_datetime *)obj);
}

ui_object * ui_datetime_button_create(uint8 * name, uint16 mode, datetime_p value) {
	ui_datetime_button * obj = (ui_datetime_button *)ui_create_object(sizeof(ui_datetime_button), UI_TYPE_BUTTON | UI_ALIGN_VERTICAL);
	((ui_object *)obj)->rect.h = 48;
	((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
	((ui_object *)obj)->backcolor = UI_COLOR_RGB(23,24,36);
	((ui_object *)obj)->render = ui_datetime_button_render;
	((ui_object *)obj)->handler = ui_datetime_button_handler;
	strcpy((char *)(((ui_object *)obj)->name), (const char *)name);
	//obj->bitmap = (uint8 *)image_png_calendar;
	//((ui_object *)obj)->bitmap = (uint8 *)image_png_calendar;
	//((ui_object *)obj)->bmpsize = sizeof(image_png_calendar);
	((ui_object *)obj)->image.buffer = NULL;
	((ui_object *)obj)->image.size = 0;
	((ui_item *)obj)->id = 0;
	//ui_longtime_2_datetime(&obj->base.value, longtime);
	memcpy(&obj->base.value, value, sizeof(datetime));
	obj->base.state = mode;
	obj->base.field = (dt_object *)gt_dtymfield;
	return (ui_object *)obj;
}

ui_object * ui_datetime_item_create(uint8 * name, uint8 * format, uint8 * value) {
	uint8 i;
	uint16 mode;
	ui_datetime_item * obj;
	dt_object * field = (dt_object *)gt_dtymfield;
	obj = (ui_datetime_item *)ui_create_object(sizeof(ui_datetime_item), UI_TYPE_BUTTON | UI_ALIGN_VERTICAL);
	((ui_object *)obj)->render = ui_datetime_input_render;
	((ui_object *)obj)->handler = ui_datetime_item_handler;
	((ui_object *)obj)->target = obj;
	//ui_longtime_2_datetime(&obj->base.value, longtime);
	mode = ui_datetime_format_mode(format);		//set mode
	((ui_object *)obj)->rect.h = 192;
	if((mode & UI_DTSHOW_FULLDATE) == 0) {			//check if no date on format
		field = (dt_object *)gt_tfield;
		((ui_object *)obj)->rect.h = 32;
		if((mode & UI_DTSHOW_FULLTIME) == 0) { 
			((ui_object *)obj)->rect.h = 0;
			field = (dt_object *)gt_field;
		}
	} else {
		if((mode & UI_DTSHOW_FULLTIME) == 0) {
			field = (dt_object *)gt_dymfield;
			((ui_object *)obj)->rect.h = 160;
			if((mode & UI_DTSHOW_DATE) == 0) {
				field = (dt_object *)gt_ymfield;
				((ui_object *)obj)->rect.h = 32;
			}
		}
	}
	if(mode & UI_DTSHOW_SUBMIT) ((ui_object *)obj)->rect.h += 48;
	strncpy((char *)obj->format, (const char *)format, 24);
	ui_datetime_to_text(&obj->base.value, format, obj->dtval);
	obj->base.field = field;
	obj->base.state = mode;
	return (ui_object *)obj;
}

void ui_datetime_show(gui_handle_p display, ui_panel * panel, ui_datetime * target) {
	uint8 i;
	ui_object * board;
	if(panel != NULL) board = (ui_object *)panel;
	else {
		board = ui_window_create(display, (uint8 *)"datetime");
		ui_push_screen(display, board);
	}
	board->render = ui_datetime_input_render;
	board->handler = ui_datetime_input_handler;
	board->target = target;
	ui_reinitialize_object(board);
}

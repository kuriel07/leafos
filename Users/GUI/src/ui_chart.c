
#include "..\..\defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\inc\ui_keyboard.h"
#include "..\inc\ui_chart.h"
#include <math.h>


void ui_series_bar_render(gui_handle_p display, ui_chart_area * area, ui_series * series) {
	uint16 i=0;
	uint16 h;
	uint16 x, y;
	uint16 min_x = area->min_x;
	uint16 max_x = area->max_x;
	uint16 min_y = area->min_y;
	uint16 max_y = area->max_y;
	int32 wval;
	uint16 w;
	float step_x = (area->max_x - area->min_x) / series->length;
	int32 max = ui_series_get_max(series);				//get max()
	int32 min = ui_series_get_min(series);				//get min()
	if(area->mode == UI_CHART_MODE_ABSOLUTE) {
		min = 0;		//set min to zero
		if(area->axis[1].max != 0) max = area->axis[1].max;		//set axis max value to fixed point
	}
	
	for(i=0;i<series->length;i++) {
		switch(series->type) {
			case sizeof(int8):
						wval  = ((int8 *)series->elements)[i];
						break;
			case sizeof(int16):
						wval  = ((int16 *)series->elements)[i];
						break;
			case sizeof(int32):
						wval  = ((int32 *)series->elements)[i];
						break;
			case sizeof(double):
						wval  = ((double *)series->elements)[i];
						break;
		}
		w = step_x;
		if(step_x > 10) w = (int)ceil(step_x);
		h = (wval - min) * (max_y - min_y) / (max - min);
		x = min_x + (i * step_x) + 1;
		y = max_y - h;
		if(y < area->min_y) {
			y = area->min_y;
			h = area->max_y - area->min_y;
		}
		if(y > area->max_y) {
			//negative value
			y = area->max_y;
			h = abs((int16)h);
			if(h > (area->max_y - area->min_y)) h = area->max_y - area->min_y;
		}
		display->fill_area(display, series->colors[0], display->set_area(display, x, y, w, h));
	}
}

void ui_series_line_render(gui_handle_p display, ui_chart_area * area, ui_series * series) {
	uint16 i=0;
	uint16 h;
	uint16 x1, y1, x2, y2;
	uint16 min_x = area->min_x;
	uint16 max_x = area->max_x;
	uint16 min_y = area->min_y;
	uint16 max_y = area->max_y;
	int32 wval;
	uint16 w;
	float step_x = (area->max_x - area->min_x) / series->length;
	int32 max = ui_series_get_max(series);				//get max()
	int32 min = ui_series_get_min(series);				//get min()
	if(area->mode == UI_CHART_MODE_ABSOLUTE) {
		min = 0;		//set min to zero
		if(area->axis[1].max != 0) max = area->axis[1].max;		//set axis max value to fixed point
	}
	
	switch(series->type) {
		case sizeof(int8):
					wval  = ((int8 *)series->elements)[0];
					break;
		case sizeof(int16):
					wval  = ((int16 *)series->elements)[0];
					break;
		case sizeof(int32):
					wval  = ((int32 *)series->elements)[0];
					break;
		case sizeof(double):
					wval  = ((double *)series->elements)[0];
					break;
	}
	//w = step_x;
	h = (wval - min) * (max_y - min_y) / (max - min);
	x1 = min_x + (i * step_x) + 1;
	y1 = max_y - h;
	
	for(i=1;i<series->length;i++) {
		switch(series->type) {
			case sizeof(int8):
						wval  = ((int8 *)series->elements)[i];
						break;
			case sizeof(int16):
						wval  = ((int16 *)series->elements)[i];
						break;
			case sizeof(int32):
						wval  = ((int32 *)series->elements)[i];
						break;
			case sizeof(double):
						wval  = ((double *)series->elements)[i];
						break;
		}
		//w = step_x;
		h = (wval - min) * (max_y - min_y) / (max - min);
		x2 = min_x + (i * step_x) + 1;
		y2 = max_y - h;
		//generate line from previous node to current node (bresenham algorithm)
		display->draw_line(display, x1, y1, x2, y2, series->colors[0]);
		x1 = x2;
		y1 = y2;
	}
}

void ui_series_spectral_render(gui_handle_p display, ui_chart_area * area, ui_series * series) {
	uint16 i=0;
	uint16 h;
	uint16 x, y;
	uint16 min_x = area->min_x;
	uint16 max_x = area->max_x;
	uint16 min_y = area->min_y;
	uint16 max_y = area->max_y;
	int32 wval;
	uint16 w;
	uint16 next_y;
	uint8 ci = 0;
	uint16 step_h = (area->max_y - area->min_y) / 32;				//histogram color gradient
	float step_x = (area->max_x - area->min_x) / series->length;
	int32 max = ui_series_get_max(series);				//get max()
	int32 min = ui_series_get_min(series);				//get min()
	if(area->mode == UI_CHART_MODE_ABSOLUTE) {
		min = 0;		//set min to zero
		if(area->axis[1].max != 0) max = area->axis[1].max;		//set axis max value to fixed point
	}
	
	for(i=0;i<series->length;i++) {
		switch(series->type) {
			case sizeof(int8):
						wval  = ((int8 *)series->elements)[i];
						break;
			case sizeof(int16):
						wval  = ((int16 *)series->elements)[i];
						break;
			case sizeof(int32):
						wval  = ((int32 *)series->elements)[i];
						break;
			case sizeof(double):
						wval  = ((double *)series->elements)[i];
						break;
		}
		w = (int)ceil(step_x);
		if(step_x > 10) w = (int)ceil(step_x) - 1;
		h = (wval - min) * (max_y - min_y) / (max - min);
		x = min_x + (i * step_x) + 1;
		y = max_y - h;
		step_h = (area->max_y - area->min_y) / 32;
		ci = 0;
		if(y > area->max_y) {
			//negative value
			y = area->max_y;
			h = abs((int16)h);
			if(h > (area->max_y - area->min_y)) h = area->max_y - area->min_y;
			//ci = ((y - area->max_y) / step_h);
			//fill with gradient
			for(ci=0,next_y=0;next_y<h;next_y+=step_h,ci++) {
				if((next_y + step_h) > h) step_h = h - next_y;
				if((next_y + step_h) > display->height) step_h = display->height - next_y;
				if(next_y > display->height) break;			//skip operation (out of bounds)
				display->fill_area(display, series->colors[ci], display->set_area(display, x, y + next_y, w, step_h));
			}
		} else {
			
			if(y < area->min_y) {
				y = area->min_y;
				h = area->max_y - area->min_y;
			}
			//fill with gradient
			//ci = ((y - area->max_y) / step_h);
			for(ci=0,next_y=(y+h);next_y>y;next_y-=step_h,ci++) {
				if((next_y - step_h) < y) step_h = next_y-y;
				if((next_y - step_h) < 0) step_h = next_y-0;
				if(next_y < 0) break;			//skip operation (out of bounds)
				display->fill_area(display, series->colors[ci], display->set_area(display, x, next_y - step_h, w, step_h));
			}
		}
	}
}


int ui_series_get_max(ui_series * series) {
	uint16 i;
	int32 wval;
	int32 max = 0;
	for(i=0;i<series->length;i++) {
		switch(series->type) {
			case sizeof(int8):
						wval  = ((int8 *)series->elements)[i];
						break;
			case sizeof(int16):
						wval  = ((int16 *)series->elements)[i];
						break;
			case sizeof(int32):
						wval  = ((int32 *)series->elements)[i];
						break;
			case sizeof(double):
						wval  = ((double *)series->elements)[i];
						break;
		}
		if(wval > max) max = wval;
	}
	return max;
}

int ui_series_get_min(ui_series * series) {
	uint16 i;
	int32 wval;
	int32 min = 0x7FFFFFFF;
	for(i=0;i<series->length;i++) {
		switch(series->type) {
			case sizeof(int8):
						wval  = ((int8 *)series->elements)[i];
						break;
			case sizeof(int16):
						wval  = ((int16 *)series->elements)[i];
						break;
			case sizeof(int32):
						wval  = ((int32 *)series->elements)[i];
						break;
			case sizeof(double):
						wval  = ((double *)series->elements)[i];
						break;
		}
		if(wval < min) min = wval;
	}
	return min;
}


static void ui_chart_render(ui_object * obj, gui_handle_p display) {
	uint8 buffer[UI_MAX_TEXT];
	uint16 i = 0; size_t j;
	uint16 cx, cy;
	int32 max = 0, min = 0;
	double y_scale, y_temp;
	char * str_unit = "";
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint32 step_x, step_y;
	uint16 num_grid_y = 0, num_grid_x = 0;
	uint16 total_width = 0;
	uint8 nc;
	ui_series * iterator = NULL;
	uint8 charline = (w / 8) - 1;			//number of character perline
	uint16 chartotal = strlen((const char *)((ui_textbox *)obj)->content);		//calculate total characters
	ui_chart * chart = (ui_chart *)obj;
	uint8 padding = chart->area.padding;
	uint16 start_x = x + padding + 20;
	uint16 start_y = y + 8;
	uint16 end_x =  x + (w - padding);
	uint16 end_y = y + (h - (padding + 8));
	ui_chart_area temp_area;
	chart->area.max_x = end_x;
	chart->area.max_y = end_y;
	chart->area.min_x = start_x;
	chart->area.min_y = start_y;
	uint16 center_x = start_x;
	uint16 center_y = end_y;
	if(obj->state & UI_STATE_INIT) {
		display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, x, y, w, h));
		if(chart->grid.width != 0) {
			//draw grid line (vertical grid lines)
			for(i=start_x;i<end_x;i+= chart->grid.width) {
				//display->draw_line(display, i, end_y, i, start_y, UI_COLOR_WHITE);					//y axis
				display->fill_area(display, UI_COLOR_GREY, display->set_area(display, i, start_y, 1, end_y - start_y));
				num_grid_x++;
			}
		} else {
			//display->draw_line(display, start_x, end_y, start_x, start_y, UI_COLOR_WHITE);					//y axis
			//display->fill_area(display, UI_COLOR_WHITE, display->set_area(display, start_x, start_y, 1, end_y - start_y));
		}
		if(chart->grid.height != 0) {
			//draw grid line (vertical grid lines)
			for(i=end_y;i>start_y;i-= chart->grid.height) {
				//display->draw_line(display, start_x, i, end_x, i, UI_COLOR_WHITE);					//x axis
				display->fill_area(display, UI_COLOR_GREY, display->set_area(display, start_x, i, end_x - start_x, 1));
				num_grid_y++;
			}
		} else {
			//display->draw_line(display, start_x, end_y, end_x, end_y, UI_COLOR_WHITE);					//x axis
			//display->fill_area(display, UI_COLOR_WHITE, display->set_area(display, start_x, end_y, end_x - start_x, 1));
		}
		
		//start draw series (draw each series respectively)
		iterator = chart->series;
		memcpy(&temp_area, &chart->area, sizeof(ui_chart_area));
		while(iterator != NULL) {
			switch(chart->area.mode) {
				
				case UI_CHART_MODE_AUTO:
				case UI_CHART_MODE_RELATIVE:
					if(iterator->render != NULL) iterator->render(display, &temp_area, iterator);
					break;
				
				case UI_CHART_MODE_ABSOLUTE:	
					switch(chart->area.type) {
						case 0: 				//single quadrant
							if(iterator->render != NULL) iterator->render(display, &temp_area, iterator);
							break;
						case UI_CHART_AREA_TYPE_H2: 				//double quadrant, horizontal split
							temp_area.max_y = temp_area.min_y + ((temp_area.max_y - temp_area.min_y) / 2);
							if(iterator->render != NULL) iterator->render(display, &temp_area, iterator);
							break;
						case UI_CHART_AREA_TYPE_V2: 				//double quadrant, vertical split
							temp_area.min_x = temp_area.min_x + ((temp_area.max_x - temp_area.min_x) / 2);
							if(iterator->render != NULL) iterator->render(display, &temp_area, iterator);
							break;
						case UI_CHART_AREA_TYPE_Q4: 				//four quadrant, horizontal split+vertical split
							temp_area.max_y = temp_area.min_y + ((temp_area.max_y - temp_area.min_y) / 2);
							temp_area.min_x = temp_area.min_x + ((temp_area.max_x - temp_area.min_x) / 2);
							if(iterator->render != NULL) iterator->render(display, &temp_area, iterator);
							break;
						
					}
					break;
			}
			iterator = iterator->next;
		}
		
		//draw y scale
		iterator = chart->series;
		while(iterator != NULL) {
			max = ui_series_get_max(iterator);				//get max()
			min = ui_series_get_min(iterator);				//get min()
			if(chart->area.mode == UI_CHART_MODE_ABSOLUTE) {
				min = 0;		//set min to zero
				if(chart->area.axis[1].max != 0) max = chart->area.axis[1].max;		//set axis max value to fixed point
			}
			if(chart->area.type & UI_CHART_AREA_TYPE_H2) min = 0- max;			//set min to negative max
			if((max - min) > 0) {
				step_x = (chart->area.max_x - chart->area.min_x) / iterator->length;
				step_y = (max - min) / num_grid_y;
				y_scale = min;
				for(i=end_y;i>start_y;i-= chart->grid.height) {
					//display->draw_line(display, start_x, i, end_x, i, UI_COLOR_WHITE);					//x axis
					//display->fill_area(display, UI_COLOR_WHITE, display->set_area(display, start_x, i, end_x - start_x, 1));
					//num_grid_y++;
					if(y_scale > 1000000) { y_temp = y_scale/1000000; str_unit = "M"; }
					else if(y_scale > 1000) { y_temp = y_scale/1000; str_unit = "K"; }
					else y_temp = y_scale;
					if((max - min) > 10) {
						snprintf((char *)buffer, UI_MAX_TEXT, "%d%s", (int32)y_temp, str_unit);
					} else {
						snprintf((char *)buffer, UI_MAX_TEXT, "%0.1f%s", y_scale, str_unit);
					}
					display->print_string(display, UI_FONT_SMALL, start_x - (strlen((const char*)buffer) * 6) - 4 , i - UI_FONT_SMALL, buffer, UI_COLOR_WHITE);
					y_scale += step_y;
				}
			}
			iterator = iterator->next;
		}
		//calculate center axis for both x and y
		if(chart->area.type & UI_CHART_AREA_TYPE_V2) center_x = start_x + ((end_x - start_x) / 2);			//split vertical
		if(chart->area.type & UI_CHART_AREA_TYPE_H2) center_y = start_y + ((end_y - start_y) / 2);			//split horizontal
		//draw y axis
		display->fill_area(display, UI_COLOR_WHITE, display->set_area(display, center_x, start_y, 1, end_y - start_y));
		//draw x axis
		display->fill_area(display, UI_COLOR_WHITE, display->set_area(display, start_x, center_y, end_x - start_x, 1));
		
		//print x axis name (bottom right corner)
		nc = strlen(chart->area.axis[0].name);
		display->print_string(display, UI_FONT_SMALL, (x + w) - (nc* UI_FONT_SMALL_WIDTH) - 4 , (y + h) - UI_FONT_SMALL, (uint8 *)chart->area.axis[0].name, UI_COLOR_WHITE);
		//pint y axis name (top left corner)
		display->print_string(display, UI_FONT_SMALL, x + 4 , y, (uint8 *)chart->area.axis[1].name, UI_COLOR_WHITE);
	}
}

void ui_chart_handler(ui_object * obj, void * display) {
	if(obj->state & UI_STATE_SELECTED) {
		//nothing to do
	}
}

ui_series * ui_series_create(char * name, uint8 mode, uint8 type, uint32 color1, uint32 color2, uint16 length, void * elements ) {
	ui_series * series = (ui_series *)os_alloc(sizeof(ui_series) + (length * type));
	series->length = length;
	series->next = NULL;
	strncpy(series->name, name, UI_MAX_OBJECT_NAME);
	series->colors[0] = color1;
	series->colors[1] = color2;
	series->type= type;
	series->mode = mode;
	series->render = NULL;
	if(elements != NULL) memcpy(series->elements, elements, (type * length));
	else memset(series->elements, 0, (type * length));
	return series;
}

ui_series * ui_series_create_bar(char * name, uint8 type, uint32 color1, uint32 color2, uint16 length, void * elements) {
	ui_series * series = ui_series_create(name, UI_SERIES_MODE_BAR, type, color1, color2, length, elements);
	series->render = ui_series_bar_render;
	return series;
}


ui_series * ui_series_create_line(char * name, uint8 type, uint32 color1, uint32 color2, uint16 length, void * elements) {
	ui_series * series = ui_series_create(name, UI_SERIES_MODE_LINE, type, color1, color2, length, elements);
	series->render = ui_series_line_render;
	return series;
}

ui_series * ui_series_create_spectral(char * name, uint8 type, uint32 color1, uint32 color2, uint16 length, void * elements) {
	ui_series * series = ui_series_create(name, UI_SERIES_MODE_SPECTRUM, type, color1, color2, length, elements);
	series->colors[0] = UI_COLOR_RGB(16, 255, 0) ;
	series->colors[1] = UI_COLOR_RGB(32, 255, 0) ;
	series->colors[2] = UI_COLOR_RGB(48, 255, 0) ;
	series->colors[3] = UI_COLOR_RGB(64, 255, 0) ;
	series->colors[4] = UI_COLOR_RGB(80, 255, 0) ;
	series->colors[5] = UI_COLOR_RGB(96, 255, 0) ;
	series->colors[6] = UI_COLOR_RGB(112, 255, 0) ;
	series->colors[7] = UI_COLOR_RGB(128, 255, 0) ;
	series->colors[8] = UI_COLOR_RGB(128, 255, 0) ;
	series->colors[9] = UI_COLOR_RGB(144, 255, 0) ;
	series->colors[10] = UI_COLOR_RGB(160, 255, 0) ;
	series->colors[11] = UI_COLOR_RGB(176, 255, 0) ;
	series->colors[12] = UI_COLOR_RGB(192, 255, 0) ;
	series->colors[13] = UI_COLOR_RGB(208, 255, 0) ;
	series->colors[14] = UI_COLOR_RGB(224, 255, 0) ;
	series->colors[15] = UI_COLOR_RGB(240, 255, 0) ;
	series->colors[16] = UI_COLOR_RGB(255, 255, 0) ;
	series->colors[17] = UI_COLOR_RGB(255, 240, 0) ;
	series->colors[18] = UI_COLOR_RGB(255, 224, 0) ;
	series->colors[19] = UI_COLOR_RGB(255, 208, 0) ;
	series->colors[20] = UI_COLOR_RGB(255, 192, 0) ;
	series->colors[21] = UI_COLOR_RGB(255, 176, 0) ;
	series->colors[22] = UI_COLOR_RGB(255, 160, 0) ;
	series->colors[23] = UI_COLOR_RGB(255, 144, 0) ;
	series->colors[24] = UI_COLOR_RGB(255, 128, 0) ;
	series->colors[25] = UI_COLOR_RGB(255, 112, 0) ;
	series->colors[26] = UI_COLOR_RGB(255, 96, 0) ;
	series->colors[27] = UI_COLOR_RGB(255, 80, 0) ;
	series->colors[28] = UI_COLOR_RGB(255, 64, 0) ;
	series->colors[29] = UI_COLOR_RGB(255, 48, 0) ;
	series->colors[30] = UI_COLOR_RGB(255, 32, 0) ;
	series->colors[31] = UI_COLOR_RGB(255, 16, 0) ;
	series->render = ui_series_spectral_render;
	return series;
}

ui_series * ui_chart_get_series_by_name(ui_chart * chart, char * name) {
	ui_series * iterator;
	if(chart == NULL) return NULL;
	iterator = chart->series;
	while(iterator != NULL) {
		if(strcmp(iterator->name, name) == 0) return iterator;
		iterator = iterator->next;
	}
	return NULL;
}

void ui_chart_delete_series(ui_chart * chart, ui_series * series) {
	ui_series * iterator;
	if(chart == NULL) return ;
	iterator = chart->series;
	if(iterator == series ) {						//check if direct descendant
		chart->series = series->next;
		os_free(series);
	} else {
		while(iterator != NULL) {
			if(iterator->next == series) {
				iterator->next = series->next;
				os_free(series);
				return;
			}
			iterator = iterator->next;
		}
	}
}

void ui_chart_release(ui_chart * chart) {
	ui_series * iterator = NULL;
	ui_series * to_delete = NULL;
	if(chart == NULL) return ;
	iterator = chart->series;
	while(iterator != NULL) {
		to_delete = iterator;
		iterator = iterator->next;
		os_free(to_delete);
	}
}

ui_object * ui_chart_create(char * name, char * title, ui_series * series) {
	ui_chart * obj = (ui_chart *)ui_create_object(sizeof(ui_textbox), UI_TYPE_GRIDITEM | UI_TYPE_PANEL | UI_TYPE_VISIBLE) ;
	((ui_object *)obj)->rect.h = UI_FONT_DIGITAL + 8;		//(UI_FONT_DEFAULT * numline) + 32;
	((ui_object *)obj)->render = ui_chart_render;
	((ui_object *)obj)->forecolor = UI_COLOR_BLACK;
	((ui_object *)obj)->backcolor = UI_COLOR_WHITE;
	((ui_object *)obj)->handler = ui_chart_handler;
	obj->area.min_x = 0;
	obj->area.min_y = 0;
	obj->area.max_x = 0;
	obj->area.max_y = 0;
	obj->area.padding = 10;
	obj->area.mode = UI_CHART_MODE_ABSOLUTE;
	obj->area.type = UI_CHART_AREA_TYPE_S1;
	obj->area.axis[0].max = 0;
	obj->area.axis[1].max = 0;
	sprintf(obj->area.axis[0].name, "%s", "X");
	sprintf(obj->area.axis[1].name, "%s", "Y");
	obj->area.axis[0].min = 0;
	obj->area.axis[1].min = 0;
	obj->grid.height = 32;
	obj->grid.width = 32;
	obj->series = series;
	strncpy(obj->title, title, UI_MAX_OBJECT_NAME);
	ui_set_object_name((ui_object *)obj, name);
	//obj->mode = UI_TEXTBOX_NUMERIC;
	//strncpy((char *)((ui_textbox *)obj)->content, (const char *)default_text, UI_MAX_TEXT);
	//((ui_textbox *)obj)->curidx = strlen((const char *)default_text);
	return (ui_object *)obj;
}
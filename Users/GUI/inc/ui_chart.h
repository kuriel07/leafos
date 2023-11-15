#ifndef UI_CHART__H
#define UI_CHART__H

#ifndef UI_CORE__H
#include "ui_core.h"
#endif
#ifndef UI_TEXTBOX__H
#include "ui_textbox.h"
#endif


#define UI_SERIES_MODE_BAR					0
#define UI_SERIES_MODE_LINE					1
#define UI_SERIES_MODE_SPECTRUM			2

#define UI_CHART_MODE_AUTO					0
#define UI_CHART_MODE_ABSOLUTE			1
#define UI_CHART_MODE_RELATIVE			2

#define UI_CHART_AREA_TYPE_S1						0				//single quadrant (positive y, positive x)
#define UI_CHART_AREA_TYPE_H2						1				//double quadrant horizontal split
#define UI_CHART_AREA_TYPE_V2						2				//double quadrant vertical split
#define UI_CHART_AREA_TYPE_Q4						3				//four quadrant


typedef struct ui_series ui_series;
typedef struct ui_chart ui_chart;
typedef struct ui_grid ui_grid;
typedef struct ui_chart_area ui_chart_area;

typedef struct ui_series {
	ui_series * next;								//next series
	uint8 mode;											//series type bar or line
	uint8 type;											//data type
	char name[UI_MAX_OBJECT_NAME ];
	uint32 colors[32];									//color pallete (max 8)
  void (* render)(gui_handle_p handle, ui_chart_area * area, ui_series * series)	;
	uint16 length;								//total elements
	uint8 elements[0];								//elements (use array for easy memory copy)
} ui_series;

typedef struct ui_grid {
	uint16 width;
	uint16 height;
} ui_grid;

typedef struct ui_axis {
	double min;
	double max;
	char name[UI_MAX_OBJECT_NAME];
} ui_axis;

typedef struct ui_chart_area {
	double min_x;
	double min_y;
	double max_x;
	double max_y;
	ui_axis axis[2];					//0=axis x, 1=axis y
	uint8 type;									
	uint8 padding;
	uint8 mode;										//auto, relative or absolute
} ui_chart_area;

typedef struct ui_chart {
	ui_object base;
	ui_chart_area area;
	ui_grid grid;								//grid configuration
	ui_series * series;
	char title[UI_MAX_OBJECT_NAME];
} ui_chart;



ui_series * ui_series_create_bar(char * name, uint8 type, uint32 color1, uint32 color2, uint16 length, void * elements);
ui_series * ui_series_create_line(char * name, uint8 type, uint32 color1, uint32 color2, uint16 length, void * elements);
ui_series * ui_series_create_spectral(char * name, uint8 type, uint32 color1, uint32 color2, uint16 length, void * elements);
ui_series * ui_series_create(char * name, uint8 mode, uint8 type, uint32 color1, uint32 color2, uint16 length, void * elements);
ui_object * ui_chart_create(char * name, char * title, ui_series * series);
ui_series * ui_chart_get_series_by_name(ui_chart * chart, char * name);
void ui_chart_delete_series(ui_chart * chart, ui_series * series) ;
void ui_chart_release(ui_chart * chart) ;

int ui_series_get_max(ui_series * series);
int ui_series_get_min(ui_series * series) ;
#endif

#include "..\..\defs.h"
#include "..\..\config.h"

#ifndef UI_CORE__H
#include "ui_core.h"
#endif
#ifndef UI_PANEL__H
#include "ui_panel.h"
#endif
#ifndef UI_ITEM__H
#include "ui_item.h"
#endif

#define DT_OBJ_YEAR						0x00
#define DT_OBJ_YEAR_UP					0x10
#define DT_OBJ_YEAR_DOWN				0x20
#define DT_OBJ_MONTH					0x01
#define DT_OBJ_MONTH_UP				0x11
#define DT_OBJ_MONTH_DOWN			0x21
#define DT_OBJ_DATE_XX					0x40

#define DT_OBJ_HOUR						0x04
#define DT_OBJ_HOUR_UP					0x14
#define DT_OBJ_HOUR_DOWN				0x24

#define DT_OBJ_MINUTE					0x05
#define DT_OBJ_MINUTE_UP				0x15
#define DT_OBJ_MINUTE_DOWN			0x25

#define DT_OBJ_SECOND					0x06
#define DT_OBJ_SECOND_UP				0x16
#define DT_OBJ_SECOND_DOWN			0x26

#define DT_OBJ_DAY						0x07
#define DT_OBJ_DAY_UP					0x17
#define DT_OBJ_DAY_DOWN				0x27

#define DT_OBJ_DONE						0xF0
#define DT_OBJ_CANCEL					0xF1
#define DT_OBJ_END						0xFF


#ifndef UI_DATETIME__H
#define UI_DATETIME__H

#define UI_DTMODE_12H			0x80
#define UI_DTMODE_24H			0x00
#define UI_DTMODE_PM				0x01
#define UI_DTMODE_AM				0x00

#define UI_DTSHOW_SUBMIT		0x8000
#define UI_DTSHOW_YEAR			0x4000
#define UI_DTSHOW_MONTH		0x2000
#define UI_DTSHOW_DATE			0x1000
#define UI_DTSHOW_DAY			0x0800
#define UI_DTSHOW_HOUR			0x0004
#define UI_DTSHOW_MINUTE		0x0002
#define UI_DTSHOW_SECOND		0x0001

#define UI_DTSHOW_FULLDATE	(UI_DTSHOW_YEAR | UI_DTSHOW_MONTH | UI_DTSHOW_DATE)
#define UI_DTSHOW_FULLTIME	(UI_DTSHOW_HOUR | UI_DTSHOW_MINUTE | UI_DTSHOW_SECOND)

typedef struct dt_object {
	uint16 x;
	uint16 y;
	uint16 w;
	uint16 h;
	uint16 state;
	uint8 type;
} dt_object;

typedef struct ui_datetime_value {
	uint8 second;
	uint8 minute;
	uint8 hour;
	uint8 day;
	uint8 date;
	uint8 month;
	uint16 year;
	uint16 days;			//days of year
	int8 tz;				//timezone
	uint8 mode;
} ui_datetime_value;

typedef struct ui_datetime {
	ui_item base;
	uint16 state;
	ui_datetime_value value;
	dt_object * field;
} ui_datetime;

typedef struct ui_datetime_button {
	ui_datetime base;
} ui_datetime_button;

typedef struct ui_datetime_item {
	ui_datetime base;
	uint8 result;
	uint8 format[28];
	uint8 dtval[28];
} ui_datetime_item;

ui_object * ui_datetime_button_create(uint8 * name, uint16 mode, datetime_p value);
ui_object * ui_datetime_item_create(uint8 * name, uint8 * format, uint8 * value);
void ui_datetime_show(gui_handle_p display, ui_panel * panel, ui_datetime * target);
void ui_longtime_2_datetime(ui_datetime_value * val, uint32 longtime);
uint32 ui_datetime_2_longtime(ui_datetime_value * val);

#endif

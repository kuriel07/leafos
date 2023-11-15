#ifndef UI_GAUGE__H
#define UI_GAUGE__H

#ifndef UI_CORE__H
#include "ui_core.h"
#endif

#define UI_GAUGE_HORIZONTAL			2
#define UI_GAUGE_VERTICAL				1

typedef struct ui_gauge {
	ui_text base;
	uint8 orientation;
	uint8 percent;
} ui_gauge;

typedef struct ui_infostat {
	ui_gauge base;
	uint16 tick;
	uint8 cpu_util;
	float fps;
	uint8 status;
} ui_infostat;

ui_object * ui_gauge_create(DWORD color, uint8 percent, uchar * text);
ui_object * ui_infostat_create(void * ctx);
#endif

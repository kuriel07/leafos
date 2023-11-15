#ifndef UI_LABEL__H
#define UI_LABEL__H

#ifndef UI_CORE__H
#include "ui_core.h"
#endif

typedef struct ui_label {
	ui_object base;
	uint16 font_size;
	uint8 content[1];
} ui_label;

ui_object * ui_label_create(DWORD color, uint8 numline, uint16 fontsize,  uchar * text);
ui_object * ui_dynamic_label_create(DWORD color, uint8 numline, uint16 fontsize,  uchar * text);
#endif

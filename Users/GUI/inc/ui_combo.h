#ifndef UI_COMBO__H
#define UI_COMBO__H

#ifndef UI_CORE__H
#include "ui_core.h"
#endif
#include <stdarg.h>

typedef struct ui_combo {
	ui_text base;
	uint8 index;
	uint8 total;
	void (* selected)(ui_object * obj, void * params);
	uint8 content[1];
} ui_combo;

ui_object * ui_combo_create(uint8 index, uint8 total, uint8 * text, void (* selected)(ui_object *, void *), ...);
#endif

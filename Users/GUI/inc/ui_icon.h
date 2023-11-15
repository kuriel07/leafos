#ifndef UI_ICON__H
#define UI_ICON__H
#ifndef UI_CORE__H
#include "ui_core.h"
#endif

typedef struct ui_icon {
	ui_text base;
	uint8 aidlen;
	uint8 aid[16];
	uint16 icolen;
	uint8 bitmap[1];
} ui_icon;

ui_object * ui_icon_create(uchar * text, uint8 aidlen, uint8 * aid, uint16 size, uint8 * bitmap, void (* click)(ui_object *, void *));
#endif


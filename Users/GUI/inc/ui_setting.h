#ifndef UI_SETTING__H
#define UI_SETTING__H

#ifndef UI_CORE__H
#include "ui_core.h"
#endif
#ifndef UI_ITEM__H
#include "..\inc\ui_item.h"
#endif


typedef struct ui_state_item {
	uint8 * bitmap;
	uint32 bmpsize;
	uint8 * text;
} ui_state_item;

typedef struct ui_tristate_item {
	//ui_toggle base;
	ui_item base;
	uint8 state;
	ui_state_item items[3];
} ui_tristate_item;


ui_object * ui_autobtn_create(uint8 * name, void * target, void (* click)(ui_object *, void *));

#endif

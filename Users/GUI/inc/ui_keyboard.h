#ifndef UI_KEYBOARD__H
#define UI_KEYBOARD__H

#ifndef UI_CORE__H
#include "ui_core.h"
#endif
#ifndef UI_TEXTBOX__H
#include "ui_textbox.h"
#endif

#define UI_KEYBOARD_UPPERCASE		0
#define UI_KEYBOARD_LOWERCASE		1
#define UI_KEYBOARD_NUMSYM			2
#define UI_KEYBOARD_NUMERIC			3
#define UI_KEYBOARD_NUMERIC_DOT	4

#define UI_TEXTBOX_PRESS_DURATION		5

typedef struct ui_keybtn {
	uint8 x;
	uint8 y;
	uint8 w;
	uint8 h;				
	uint16 code;
} ui_keybtn;

typedef struct ui_keyboard {
	ui_object base;
	uint8 idx;
	uint8 duration;
	uint8 last_key;
} ui_keyboard;

typedef struct ui_textbox ui_textkeybox;
typedef struct ui_keyboard ui_keyboard;

ui_object * ui_keybtn_create(DWORD color, uint8 id, ui_object * target, void (* click)(ui_object *, void *));
ui_object * ui_keyboard_create(ui_object * target, uint16 x, uint16 y, uint16 w, uint16 h, uint8 keyset);
ui_object * ui_textkeybox_create(ui_textbox * target, uint16 width, uint16 height);
void ui_keyboard_show(gui_handle_p display, ui_textbox * textbox, uint8 keyset) ;
#endif

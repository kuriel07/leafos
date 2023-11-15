#ifndef UI_TEXTBOX__H
#define UI_TEXTBOX__H

#ifndef UI_CORE__H
#include "ui_core.h"
#endif

#define UI_TEXTBOX_BLINK_ENABLED		0x80

#define UI_TEXTBOX_ALPHANUM				0x01
#define UI_TEXTBOX_NUMERIC				0x00
#define UI_TEXTBOX_PASSWORD			0x05
#define UI_TEXTBOX_PIN						0x04
//special textbox
#define UI_TEXTBOX_IP						0x84
#define UI_TEXTBOX_EMAIL					0x85

typedef struct ui_textbox {
	ui_object base;
	uint8 content[UI_MAX_TEXT];
	uint8 mode;
	uint8 duration;
	uint8 blinked;
	uint8 curidx;
	uint8 oldidx;
	uint8 numline;
	uint8 maxlen;
} ui_textbox;

typedef struct ui_searchbox {
	ui_textbox base;
	void (* submit)(ui_object *, void *);
} ui_searchbox;


void ui_set_content(ui_object * obj, char * content) ;
ui_object * ui_textbox_create(uint8 mode, uchar * default_text, uint8 maxlen, uint16 numline);
ui_object * ui_numbox_create(uchar * default_text, uint8 maxlen);
#endif


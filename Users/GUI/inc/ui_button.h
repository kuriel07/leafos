#ifndef UI_BUTTON__H
#define UI_BUTTON__H

#ifndef UI_CORE__H
#include "ui_core.h"
#endif

#define UI_BUTTON_STYLE_NORMAL			0x00
#define UI_BUTTON_STYLE_BLUE				0x10

#define UI_BUTTON_STATE_PRESSED			1
#define UI_BUTTON_STATE_NONE				0

#define UI_BUTTON_STATE_SUBMIT				0x10
#define UI_BUTTON_STATE_CANCEL			0x20

#define UI_BUTTON_SET_ABORT					0x30
#define UI_BUTTON_SET_NO						0x20
#define UI_BUTTON_SET_CANCEL				0x10
#define UI_BUTTON_SET_CONTINUE			0x03	
#define UI_BUTTON_SET_YES						0x02				
#define UI_BUTTON_SET_OK						0x01


typedef struct ui_button {
	ui_text base;
	uint8 style;
	uint8 id;
} ui_button;

typedef struct ui_tool_button {
	ui_toggle base;
	ui_resource normal_image;
	ui_resource clicked_image;
} ui_tool_button;

typedef struct ui_buttonset {
	ui_button base;
	uint16 type;
	uint8 button_state;
	void (* submit)(ui_object *, void *);
	void (* cancel)(ui_object *, void *);
} ui_buttonset;

void ui_draw_button(gui_handle_p display, ui_rect * rect, uint8 mode);
ui_object * ui_button_create(DWORD color, uchar * text, uint8 id, void (* click)(ui_object *, void *));
ui_object * ui_buttonset_create(uint16 type, uint8 id, void (* submit)(ui_object *, void *), void (* cancel)(ui_object *, void *));
ui_object * ui_custom_button_create(uint8 * text, ui_rect * rect, uint8 mode, uint8 id, void (* click)(ui_object *, void *));
ui_object * ui_tool_button_create(ui_rect * rect, ui_resource * normal_image, ui_resource * clicked_image, void (* click)(ui_object *, void *));
void ui_button_set_style(ui_object * btn, uint8 style);
#endif

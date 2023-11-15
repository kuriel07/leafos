#ifndef UI_PANEL__H
#define UI_PANEL__H

#ifndef UI_CORE__H
#include "ui_core.h"
#endif

typedef struct ui_panel {
	ui_text base;
	ui_image * thumbnail;		//thumbnail screenshot
	uint8 border;
	uint8 radius;
} ui_panel;

ui_object * ui_create_panel(size_t objsize, uint16 x, uint16 y, uint16 w, uint16 h, uint8 * text, uint8 border, uint8 radius);
ui_object * ui_window_create(gui_handle_p display, uint8 * text);
ui_object * ui_panel_create(uint32 align, uint16 w, uint16 h) ;
ui_object * ui_toolbar_create(uint16 x, uint16 y, uint16 w, uint16 h);
void ui_panel_render(ui_object * obj, gui_handle_p display);
#endif

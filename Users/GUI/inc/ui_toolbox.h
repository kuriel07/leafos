#ifndef UI_TOOLBOX__H
#define UI_TOOLBOX__H

#ifndef UI_CORE__H
#include "ui_core.h"
#endif

typedef struct ui_progress {
	ui_object base;
	uint8 percent;
} ui_progress;

typedef struct ui_card {
	ui_object base;
} ui_card;

typedef struct ui_icon_bitmap {
	uint8 * bitmap;
	uint32 bmpsize;
} ui_icon_bitmap;

typedef struct ui_icon_tool {
	ui_toggle base;
	uint8 show_marker;
	ui_icon_bitmap bitmap[2];
} ui_icon_tool;

void ui_icon_draw_marker(ui_object * obj, gui_handle_p display);
void ui_launcher_draw_marker(ui_object * obj, gui_handle_p display);

ui_object * ui_battery_create(uint8 * name, uint8 percent);
ui_object * ui_progress_create(uint8 * name, uint8 percent) ;
ui_object * ui_card_create(uint8 *name, uint8 *text, uint16 width, uint16 height, void (* click)(ui_object *, void *));
ui_object * ui_action_create(uint8 *name, uint16 w, uint16 h) ;
ui_object * ui_header_create(uint8 *name, uint8* text, uint16 w, uint16 h, uint16 bmpsize, uint8 * bitmap, void (* click)(ui_object *, void *));
ui_object * ui_launcher_create(uint8 *name, uint8* text, uint16 w, uint16 h, uint16 bmpsize, uint8 * bitmap, void (* click)(ui_object *, void *));
ui_object * ui_config_create(uint8 *name, uint8 * text, uint16 width, uint16 height, void (* click)(ui_object *, void *)) ;

#endif

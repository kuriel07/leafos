#ifndef UI_WINDOW__H
#define UI_WINDOW__H
#ifndef _DEFS__H
#include "..\..\defs.h"
#endif
#ifndef UI_CORE__H
#include "ui_core.h"
#endif
#include "ui_panel.h"

typedef struct ui_window ui_window;
typedef struct ui_window * ui_window_p;

struct ui_window {
	ui_panel base;
	uint16 mode;					//alert mode
	uint8 icdx;						//icon index
	ui_object * panel;
};

ui_window * ui_window_show(gui_handle_p display, uint8 * text, uint16 mode, uint8 icon) ;
void ui_window_close(gui_handle_p display, ui_window * window);
#endif

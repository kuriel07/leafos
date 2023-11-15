#ifndef UI_SCROLL__H
#define UI_SCROLL__H

#ifndef UI_CORE__H
#include "ui_core.h"
#endif
#include "ui_button.h"

#define UI_LIST_MODE_GRID		0x01
#define UI_LIST_MODE_LIST		0x00

typedef struct ui_list_scroll {
	ui_container base;
	uint8 mode;
	uint8 scroll_state;
	uint16 page_length;			//total items per-page
	uint16 cur_index;			//current page index
	uint16 num_index;			//total item on list
} ui_list_scroll;

typedef struct ui_list_view {
	ui_container base;
	ui_list_scroll * list_area;
	ui_object * view_btn;
	ui_object * next_btn;
	ui_object * prev_btn;
	uint8 scroll_state;
	uint16 cur_index;			//current page index
	uint16 num_index;			//total item on list
} ui_list_view;


ui_object * ui_list_create(uint16 x, uint16 y, uint16 w, uint16 h);
void ui_list_add(ui_object * list, ui_object * obj);
void ui_list_clear(gui_handle_p display, ui_list_scroll * list ) ;

#endif

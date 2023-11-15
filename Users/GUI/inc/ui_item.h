#ifndef UI_ITEM__H
#define UI_ITEM__H
#ifndef UI_CORE__H
#include "ui_core.h"
#endif
#include "..\..\config.h"

#define UI_ITEM_EVENT_CLICK			0x00
#define UI_ITEM_EVENT_DETAIL		0x01

#define UI_ISTAT_ICON_RENDERED		0x01
#define UI_ISTAT_BORDER_RENDERED	0x02


typedef struct ui_item {
	ui_text base;		
	uint8 id;				
	ui_image * thumbnail;
} ui_item;

typedef struct ui_app_item {
	ui_item base;
	uint8 aidlen;
	uint8 aid[16];
} ui_app_item;

typedef struct ui_orbriver_item {
	ui_item base;
	uint8 event;
	uint8 oid[16];
	uint16 size;
	uint32 nod;
	uint16 bnum;
	uint8 hash[SHARD_HASH_SIZE];
	uint8 desc[SHARD_MAX_DESC + 1];
} ui_orbriver_item;

typedef struct ui_select_item_list {
	int16 value;
	char * text;
} ui_select_item_list;

typedef struct ui_select_item {
	ui_item based;
	uint8 index;
	uint8 total;
	uint8 duration;
	void (* changed)(ui_object *, void * params);
	ui_select_item_list * list;
} ui_select_item;

uint8 ui_item_render(ui_object * obj, gui_handle_p display);
void ui_item_text_render(ui_object * obj, gui_handle_p display, uint8 stat);
void ui_item_show_detail(gui_handle_p display, ui_orbriver_item * item);
void * ui_create_item(size_t objsize, uint32 mode, char * text);

ui_object * ui_searchbox_create(uchar * text, uint8 * bitmap, uint32 bmpsize, uint8 id, void (* click)(ui_object *, void *)) ;
ui_object * ui_item_create(uchar * text, uint8 * bitmap, uint32 bmpsize, uint8 id, void (* click)(ui_object *, void * params)) ;
ui_object * ui_app_item_create(uchar * text, uint8 aidlen, uint8 * aid, void (* click)(ui_object *, void * params)) ;
ui_object * ui_orbriver_item_create(uchar * text, uint8 * oid, uint16 size, uint32 nod, uint16 bnum, uint8 * hash, uint8 * desc, void (* click)(ui_object *, void * params));
ui_object * ui_select_item_create(uchar * text, uint8 index, uint8 total, ui_select_item_list * list, void (* changed)(ui_object *, void * params));
#endif

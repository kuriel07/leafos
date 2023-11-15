#ifndef UI_SIGNAL__H
#define UI_SIGNAL__H

#ifndef UI_CORE__H
#include "ui_core.h"
#endif
#ifndef UI_TOOLBOX__H
#include "ui_toolbox.h"
#endif
#ifndef IF_NET__H
#include "..\..\interfaces\inc\if_net.h"
#endif

typedef struct ui_ssidbtn {
	ui_object base;
	net_context_p netctx;
	ui_object * password_field;
	int16 percent;
	int16 att;
	uint8 sec;
	uint8 ssid_name[1];
} ui_ssidbtn;

typedef struct ui_signal {
	ui_icon_tool base;
	net_context_p netctx;
	void * cur_screen;			
	int16 percent;				//wifi percent
	int16 batt;					//battery percent
	uint8 is_charging;
	uint8 date;
	uint8 month;
	uint8 hour;
	uint8 min;
} ui_signal;

ui_object * ui_signal_create(uint8 * name, uint16 width, uint16 height, uint8 percent, uint8 batt, void * netctx) ;
#endif

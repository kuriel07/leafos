#ifndef UI_TASKER__H
#define UI_TASKER__H

#ifndef UI_CORE__H
#include "ui_core.h"
#endif

typedef struct ui_tasker {
	ui_toggle base;
} ui_tasker;

ui_object * ui_tasker_create(void) ;
#endif


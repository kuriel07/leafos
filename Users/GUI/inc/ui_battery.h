#ifndef UI_BATTERY__H
#define UI_BATTERY__H

#ifndef UI_CORE__H
#include "ui_core.h"
#endif

typedef struct ui_battery {
	ui_object base;
	BYTE percent;
} ui_battery;

#endif
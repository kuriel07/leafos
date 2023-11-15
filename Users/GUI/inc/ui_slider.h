#ifndef UI_SLIDER__H
#define UI_SLIDER__H

#ifndef UI_CORE__H
#include "ui_core.h"
#endif
#ifndef UI_TEXTBOX__H
#include "ui_textbox.h"
#endif
#ifndef UI_GAUGE__H
#include "ui_gauge.h"
#endif

#define UI_SLIDER_HORIZONTAL		UI_GAUGE_HORIZONTAL
#define UI_SLIDER_VERTICAL 		UI_GAUGE_VERTICAL		

typedef struct ui_slider {
	ui_gauge base;
	uint8 id;
	void (* changed)(ui_object * , void  *);
} ui_slider;

ui_object * ui_slider_create(char * text, uint8 id, uint8 orientation, uint16 value, void (* changed)(ui_object *, void *)) ;

#endif
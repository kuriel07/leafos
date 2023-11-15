#ifndef UI_PICTURE__H
#define UI_PICTURE__H

#ifndef UI_CORE__H
#include "ui_core.h"
#endif

typedef struct ui_picture {
	ui_text base;
} ui_picture;


ui_object * ui_picture_create(uchar * text, uint16 pxfmt, uint16 bmsize, uint8 * bitmap, void (* click)(ui_object *, void * params));
#endif

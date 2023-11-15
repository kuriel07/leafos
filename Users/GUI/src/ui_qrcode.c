#ifdef WIN32
//#include "stdafx.h"
#endif
#include "..\..\defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\inc\ui_qrcode.h"
#include "..\inc\ui_core.h"
#include "qrencode.h"

static void ui_qrcode_render(ui_object * obj, gui_handle_p display) {
	uint16 xx, yy;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint16 unWidth, unHeight, pgrid;
	uint16 i,j;
	QRcode*			pQRC;
	if(obj->state & UI_STATE_INIT) {
		display->set_area(display, x, y, w, h);
		xx = 2;		//x = width - text_width / 2;
		yy = 2;
		display->fill_area(display, obj->backcolor, display->set_area(display, x + xx, y + yy, (((ui_rect *)obj)->h - 4) , (((ui_rect *)obj)->h - 4) ));
		if ((pQRC = QRcode_encodeString((char *)((ui_text *)obj)->text, 0, QR_ECLEVEL_H, QR_MODE_8, 1))) {
			unWidth = pQRC->width;
			if(((ui_rect *)obj)->h < 10) return;
			pgrid = (((ui_rect *)obj)->h - 4) / (unWidth + 2);
			for(j = 0; j < unWidth; j++)
			{
				//pDestData = pRGBData + unWidthAdjusted * y * OUT_FILE_PIXEL_PRESCALER;
				for(i = 0; i < unWidth; i++) {
					// Allocate pixels buffer
					if(pQRC->data[(j * unWidth) + i] & 1) {
						display->fill_area(display, obj->forecolor, display->set_area(display, x + xx + ((i + 1) * pgrid), y + yy + ((j + 1) * pgrid), pgrid, pgrid));
					} else { 
						//display->fill_area(display, obj->backcolor, display->set_area(display, x + xx + ((i + 1) * pgrid), y + yy + ((j + 1) * pgrid), pgrid, pgrid));
					}
				}
				//printf("\n");
			}
			QRcode_free(pQRC);
		}
	}
}

ui_object * ui_qrcode_create(DWORD color, uint16 height, uchar * text) {
	ui_label * obj = (ui_label *)ui_create_text(sizeof(ui_label) + UI_MAX_TEXT, UI_TYPE_LABEL | UI_ALIGN_VERTICAL, (char *)text) ;
	((ui_object *)obj)->rect.h = height;
	((ui_object *)obj)->forecolor = color;
	((ui_object *)obj)->backcolor = UI_COLOR_WHITE;
	((ui_object *)obj)->render = ui_qrcode_render;
	((ui_object *)obj)->handler = NULL;
	return (ui_object *)obj;
}
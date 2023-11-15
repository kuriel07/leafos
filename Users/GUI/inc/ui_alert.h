#ifndef UI_ALERT__H
#define UI_ALERT__H
#ifndef _DEFS__H
#include "..\..\defs.h"
#endif
#ifndef UI_CORE__H
#include "ui_core.h"
#endif

#define UI_ALERT_INFO						0x00
#define UI_ALERT_BUTTON_OK			0x01
#define UI_ALERT_BUTTON_CANCEL	0x02
#define UI_ALERT_PROGRESS_BAR		0x08

#define UI_ALERT_ICON_INFO				0x00
#define UI_ALERT_ICON_WARNING		0x01
#define UI_ALERT_ICON_ERROR			0x02
#define UI_ALERT_ICON_BUSY				0x03
#define UI_ALERT_ICON_NETWORK		0x04
#define UI_ALERT_ICON_SECURE			0x05


#define UI_ALERT_RESULT_OK				0x00
#define UI_ALERT_RESULT_CANCEL		-1
#define UI_ALERT_RESULT_WAIT			0x88

typedef struct ui_alert ui_alert;
typedef struct ui_alert * ui_alert_p;

struct ui_alert {
	ui_text base;
	uint16 mode;					//alert mode
	uint16 percent;					//percent completed (progress bar)
	uint8 icdx;						//icon index
	uint8 content[1];				//text content
};

ui_alert * ui_alert_show(gui_handle_p display, uint8 * text, uint8 * content, uint16 mode, uint8 icon) ;
void ui_alert_set_progress_bar(gui_handle_p display, ui_alert * alert, uint16 percent) ;
uint8 ui_alert_result(gui_handle_p display) ;
void ui_alert_close(gui_handle_p display, ui_alert * alert);
#endif

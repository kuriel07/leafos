#ifndef UI_QRCODE__H
#define UI_QRCODE__H

#ifndef UI_CORE__H
#include "ui_core.h"
#endif
#include "ui_label.h"

ui_object * ui_qrcode_create(DWORD color, uint16 height, uchar * text);

#endif

#ifndef IF_USB__H
#define IF_USB__H
#ifndef _DEFS__H
#include "..\..\defs.h"
#endif
#include "usb_device.h"
#ifndef IF_CARD__H
#include "if_card.h"
#endif
//finished USB Smart Card Reader CCID driver implementation (2016.03.30)

#define USB_STATE_INITIALIZED 	0x8000

typedef struct usb_context * usb_context_p;
typedef struct usb_context {
	USBD_HandleTypeDef * instance;
	uint16 (* read)(struct usb_context *, uint8 * buffer);
	uint16 (* write)(struct usb_context *, uint8 * buffer, uint16 length);
	void (* status)(struct usb_context *, uint8);
	uint16 (* debug)(struct usb_context *, uint8 *, uint16);
	uint16 state;
	scard_context_p shandle;
	uint16 (* handler)(scard_context_p, uint8 * bufin, uint16 length, uint8 * bufout);
} usb_context;

//uint8 gba_usb_buffer[268];
void if_usb_init(usb_context_p uctx, void * handle, uint16 (* handler)(void *, uint8 *, uint16, uint8 *));
void if_usb_handler(usb_context_p uctx, uint32 event);
#endif

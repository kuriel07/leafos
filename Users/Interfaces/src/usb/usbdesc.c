/*----------------------------------------------------------------------------
 *      U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 *      Name:    USBDESC.C
 *      Purpose: USB Descriptors
 *      Version: V1.10
 *----------------------------------------------------------------------------
 *      This file is part of the uVision/ARM development tools.
 *      This software may only be used under the terms of a valid, current,
 *      end user licence from KEIL for a compatible version of KEIL software
 *      development tools. Nothing else gives you the right to use it.
 *
 *      Copyright (c) 2005-2007 Keil Software.
 *---------------------------------------------------------------------------*/

#include "..\..\..\defs.h"
#include "..\..\..\config.h"

#include "..\..\inc\usb\usb.h"
#include "..\..\inc\usb\hid.h"
#include "..\..\inc\usb\usbcfg.h"
#include "..\..\inc\usb\usbdesc.h"


/* HID Report Descriptor */
const BYTE HID_ReportDescriptor[] = {
  HID_UsagePageVendor(0x00),
  HID_Usage(0x01),
  HID_Collection(HID_Application),
    HID_UsagePage(HID_USAGE_PAGE_BUTTON),
    HID_UsageMin(1),
    HID_UsageMax(2),
    HID_LogicalMin(0),
    HID_LogicalMax(1),
    HID_ReportCount(2),
    HID_ReportSize(1),
    HID_Input(HID_Data | HID_Variable | HID_Absolute),
    HID_ReportCount(1),
    HID_ReportSize(6),
    HID_Input(HID_Constant),
    HID_UsagePage(HID_USAGE_PAGE_LED),
    HID_Usage(HID_USAGE_LED_GENERIC_INDICATOR),
    HID_LogicalMin(0),
    HID_LogicalMax(1),
    HID_ReportCount(8),
    HID_ReportSize(1),
    HID_Output(HID_Data | HID_Variable | HID_Absolute),
  HID_EndCollection,
};

const WORD HID_ReportDescSize = sizeof(HID_ReportDescriptor);


/* USB Standard Device Descriptor */
const BYTE USB_DeviceDescriptor[] = {
  USB_DEVICE_DESC_SIZE,              /* bLength */
  USB_DEVICE_DESCRIPTOR_TYPE,        /* bDescriptorType */
  WBVAL(0x0200), /* 1.10 */          /* bcdUSB */
  0x00,                              /* bDeviceClass */
  0x00,                              /* bDeviceSubClass */
  0x00,                              /* bDeviceProtocol */
  USB_MAX_PACKET0,                   /* bMaxPacketSize0 */
#if SHARD_USB_DRIVER == SHARD_DRIVER_OKEY
  WBVAL(0x076B),                     /* idVendor */
  WBVAL(0x1021),                     /* idProduct */
#endif
#if SHARD_USB_DRIVER == SHARD_DRIVER_KRON
  WBVAL(0x7EAF),                     /* idVendor */
  WBVAL(0xE001),                     /* idProduct */
#endif
  WBVAL(0x0100), /* 1.00 */          /* bcdDevice */
  0x01,                              /* iManufacturer */
  0x02,                              /* iProduct */
  0x00,                              /* iSerialNumber */
  0x01                               /* bNumConfigurations */
};

/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
const BYTE USB_ConfigDescriptor[] = {
/* Configuration 1 */
  USB_CONFIGUARTION_DESC_SIZE,       /* bLength */
  USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType */
  WBVAL(                             /* wTotalLength */
    USB_CONFIGUARTION_DESC_SIZE +
    USB_INTERFACE_DESC_SIZE     +
    0x36               +
    USB_ENDPOINT_DESC_SIZE +
    USB_ENDPOINT_DESC_SIZE + USB_ENDPOINT_DESC_SIZE
  ),
  0x01,                              /* bNumInterfaces */
  0x01,                              /* bConfigurationValue */
  0x03,                              /* iConfiguration */
  USB_CONFIG_BUS_POWERED |       /* bmAttributes */
USB_CONFIG_REMOTE_WAKEUP ,
  USB_CONFIG_POWER_MA(100),          /* bMaxPower */
/* Interface 0, Alternate Setting 0, HID Class */
  USB_INTERFACE_DESC_SIZE,           /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
  0x00,                              /* bInterfaceNumber */
  0x00,                              /* bAlternateSetting */
  0x03,                              /* bNumEndpoints */
#if SHARD_USB_DRIVER == SHARD_DRIVER_OKEY
  USB_DEVICE_CLASS_SMART_CARD,  /* bInterfaceClass */
#endif
#if SHARD_USB_DRIVER == SHARD_DRIVER_KRON
  USB_DEVICE_CLASS_VENDOR_SPECIFIC,  /* bInterfaceClass */
#endif
  HID_SUBCLASS_NONE,                 /* bInterfaceSubClass */
  HID_PROTOCOL_NONE,                 /* bInterfaceProtocol */
  0x00,                              /* iInterface */
/* HID Class Descriptor */
/* HID_DESC_OFFSET = 0x0012 */
  //HID_DESC_SIZE,                     /* bLength */
  //HID_HID_DESCRIPTOR_TYPE,           /* bDescriptorType */
  //WBVAL(0x0100), /* 1.00 */          /* bcdHID */
  //0x00,                              /* bCountryCode */
  //0x01,                              /* bNumDescriptors */
  //HID_REPORT_DESCRIPTOR_TYPE,        /* bDescriptorType */
  //WBVAL(HID_REPORT_DESC_SIZE),       /* wDescriptorLength */
  0x36, 0x21, 
  0x00, 0x01, 0x00, 0x07, 0x03, 0x00, 0x00, 0x00,
  0x6C, 0x0E, 0x00, 0x00, 0x6C, 0x0E, 0x00, 0x00, 
  0x01, 0xC4, 0x26, 0x00, 0x00, 0x97, 0xD8, 0x04,
  0x00, 0x13, 0xFE, 0x00, 0x00, 0x00, 0x07, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB1, 0x03,
  0x01, 0x00, 0x0F, 0x01, 0x00, 0x00, 0xFF, 0xFF,
  0x00, 0x00, 0x00, 0x01,
/* Endpoint, HID Interrupt In (EP3) */
  USB_ENDPOINT_DESC_SIZE,            /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
  USB_ENDPOINT_IN(3),                /* bEndpointAddress */
  USB_ENDPOINT_TYPE_INTERRUPT,       /* bmAttributes */
  WBVAL(0x0008),                     /* wMaxPacketSize */
  0x18,          /* 32ms */          /* bInterval */
/* Endpoint, HID Bulk In (EP4) */
  USB_ENDPOINT_DESC_SIZE,            /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
  USB_ENDPOINT_IN(4),                /* bEndpointAddress */
  USB_ENDPOINT_TYPE_BULK,       /* bmAttributes */
  WBVAL(0x0040),                     /* wMaxPacketSize */
  0x00,          /* 32ms */          /* bInterval */
/* Endpoint, HID Bulk out (EP5) */
  USB_ENDPOINT_DESC_SIZE,            /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
  USB_ENDPOINT_OUT(5),                /* bEndpointAddress */
  USB_ENDPOINT_TYPE_BULK,       /* bmAttributes */
  WBVAL(0x0040),                     /* wMaxPacketSize */
  0x00,          /* 32ms */          /* bInterval */
/* Terminator */
  0                                  /* bLength */
};


/* Index 0x00: LANGID Codes */
const BYTE USB_StringLangId[] = 
  { 0x04,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  WBVAL(0x0409) }; /* US English */    /* wLANGID */
 /* Index 0x04: Manufacturer */
 const BYTE USB_StringManufacturer[] = 
  { 0x10,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'O',0,
  'r',0,
  'b',0,
  'l',0,
  'e',0,
  'a',0,
  'f',0 };
 /* Index 0x20: Product */
 const BYTE USB_StringProduct[] = 
  { 0x10,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'E',0,
  't',0,
  'h',0,
  'e',0,
  'r',0,
  'o',0,
  'n',0 };
/* Index 0x44: Serial Number */
 const BYTE USB_StringSerialNum[] = 
  { 0x2C,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'S',0,
  'm',0,
  'a',0,
  'r',0,
  't',0,
  ' ',0,
  'C',0,
  'a',0,
  'r',0,
  'd',0,
  ' ',0,
  'R',0,
  'e',0,
  'a',0,
  'd',0,
  'e',0,
  'r',0,
  ' ',0,
  'U',0,
  'S',0,
  'B',0 };
/* Index 0x5E: Interface 0, Alternate Setting 0 */
 const BYTE USB_StringInterface[] = 
  { 0x0A,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'C',0,
  'C',0,
  'I',0,
  'D',0 };

  
/* USB String Descriptor (optional) */
const BYTE * USB_StringDescriptor[] = {
	USB_StringLangId,
	USB_StringManufacturer,
	USB_StringProduct,
	USB_StringSerialNum,
	USB_StringInterface,
};

#ifndef _KRON_H

#include "os_config.h"
#include "os_mach.h"
#include "os_core.h" 

//os_object default tag
#define OS_OBJECT_TAG			0xA6

//os_object type
#define OS_OBJECT_TYPE_DEVICE		0xDD			//device object class 
#define OS_OBJECT_TYPE_IMAGE		0x32			//ui object class
#define OS_OBJECT_TYPE_UI			0x4E			//ui object class
#define OS_OBJECT_TYPE_MESSAGE		0x70
#define OS_OBJECT_TYPE_MAILBOX		0x7b			  
#define OS_OBJECT_TYPE_SEMAPHORE	0x72
#define OS_OBJECT_TYPE_UNKNOWN		0xff

//supported mode of os_object
#define OS_OBJECT_MODE_DEVAPI		0x0080			//contain init,open,read,write,close method
#define OS_OBJECT_MODE_RECT  		0x8000		  	//have x, y, w, h (ui_rect)
#define OS_OBJECT_MODE_IMAGE 		0x4000		  	//have bitmap, layout, image key (ui_image, ui_object)
#define OS_OBJECT_MODE_STYLE		0x2000			//support style margin (ui_object)
#define OS_OBJECT_MODE_VIEW			0x0800			//have backimage, backcolor property (ui_container)
#define OS_OBJECT_MODE_TEXT			0x0400			//have text, forecolor property (ui_label, ui_button)

typedef struct os_object os_object;
_ALIGN1_ struct os_object{  
	uchar tag;										//object tag
	uchar type;										//type of object (device, ui)
	uint16 mode; 									//supported mode
	uint32 size;
	uint32 clsid;									//class id of current object
	os_task * owner;								//
	os_thread * observer;							//current waiting thread								
};   

typedef struct os_event os_event;
typedef struct os_event {
	uchar pip;
} os_event;

#define OS_NO_ERR				0
#define OS_SEMAPHORE_BUSY		6
#define OS_TASK_BUSY			4
#define OS_MESSAGE_FULL			7
typedef uint16 os_status;

#define OS_ASSERT_TAG(x) (((os_object *)x)->tag == OS_OBJECT_TAG)
#define OS_ASSERT_TYPE(x, t) (((os_object *)x)->type == t)
#define OS_ASSERT_MODE(x, m) ((((os_object *)x)->mode & m) == m)
#define OS_ASSERT_CLSID(x, id) (((os_object *)x)->clsid == id)

void os_init_object(os_object * object, uchar type, uint16 mode, uint32 size); 
void os_destroy_object(os_object * object);
void os_wait_object(os_object * object, uint32 timeout) ;
void os_signal_object(os_object * object);


#define _KRON_H
#endif

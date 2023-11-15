#ifndef OS_MSG__H
#include "..\..\defs.h"
#include "os_core.h"
											
#define OS_MESSAGE_QUEUED			   	1
#define OS_MESSAGE_SERVICED 			2
#define OS_MESSAGE_TIMEOUT				4

typedef struct os_message os_message;
typedef struct os_message {
	os_message * next;		//pointer to next message
	os_task * sender;			
	void * context;
	uint16 flag;
	void * request;
	uint32 reqlen;
	void * response;
	uint32 reslen;
} os_message;

//system apis
os_message * os_create_message(void * context, void * request, uint16 length, void * response);
os_message * os_send_message(os_task * to, os_message * msg);
os_message * os_queue_message(os_task * to, os_message * msg) ;
//user apis
os_message * os_peek_message(void);
os_message * os_dequeue_message_s(os_task *);
os_message * os_dequeue_message(void);
void os_delete_message(os_message * msg);
void os_dispatch_reply(os_message * msg);

#define OS_MSG__H
#endif
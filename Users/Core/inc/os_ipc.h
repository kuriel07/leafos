#ifndef _OS_IPC__H
#include "..\..\defs.h"
#include "os.h"
											
#define OS_MESSAGE_QUEUED			   	1
#define OS_MESSAGE_SERVICED 			2
#define OS_MESSAGE_TIMEOUT				4

#define OS_MAILBOX_SIZE					6

typedef struct os_message os_message;
typedef struct os_message {
	os_object base;
	uint16 type;
	uint16 status;
	uint32 handle;
	uint32 param;
	uint32 sender;
	void * context;
	os_message * next;		//pointer to next message
} os_message;

typedef struct os_mailbox os_mailbox;
typedef struct os_mailbox {
	os_object base;
	void * msg[OS_MAILBOX_SIZE];
	uint32 head, tail;
	os_mailbox * next;
} os_mailbox;

//system apis
os_message * os_create_message(os_task * task, uint16 type, uint32 sender, uint32 handle, uint32 param);
os_mailbox * os_create_mailbox(void);
os_mailbox * os_task_create_mailbox(os_task * owner);
void os_wait_mailbox(os_mailbox * mailbox, uint32 timeout);
os_mailbox * os_post_mailbox(os_mailbox * mailbox, void * msg);
void * os_fetch_mailbox(os_mailbox * mailbox);
void os_delete_mailbox(os_mailbox * mailbox);

//user apis
os_message * os_peek_message(void);
os_message * os_dequeue_message(void);
void os_delete_message(os_message * msg);
#define _OS_IPC__H
#endif
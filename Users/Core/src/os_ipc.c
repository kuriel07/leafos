#include "..\..\defs.h"
#include "..\inc\os.h"
#include "..\inc\os_ipc.h"

extern os_task * _active_task;

//////////////////////////////MESSAGE CREATION (SYSTEM MODE)///////////////////////////////////
void os_enqueue_message(os_task * task, os_message * msg) {
	os_message * iterator = task->messages;
	if(iterator == NULL) {
		task->messages = msg;
	} else {
		while(iterator->next != NULL) {
			iterator = iterator->next;
		}
		iterator->next = msg;
	}
}

//create a message and automatically enqueue it to specified task
os_message * os_create_message(os_task * task, uint16 type, uint32 sender, uint32 handle, uint32 param) {
	//allocate in user/system shared memory
	os_message * msg = (os_message *)os_task_alloc(task, sizeof(os_message));
	
	os_init_object((os_object *)msg, OS_OBJECT_TYPE_MESSAGE, 0, sizeof(os_message));
	msg->type = type;
	msg->status = OS_MESSAGE_QUEUED;
	msg->sender = sender;
	msg->handle = handle;
	msg->param = param;
	msg->next = NULL;
	os_enqueue_message(task, msg);
	return msg;
}

///////////////////////////END OF MESSAGE CREATION (SYSTEM MODE)///////////////////////////////


///////////////////////////////MESSAGE POOLING (USER MODE)/////////////////////////////////////
os_message * os_peek_message(void) {
	if(_active_task != NULL) {
		if(_active_task->messages != NULL) {
			_active_task = _active_task;
		}
		return _active_task->messages;
	}
	return NULL;
}

os_message * os_dequeue_message(void) {
	os_message * msg = NULL;
	if(_active_task->messages != NULL) {
		msg = _active_task->messages;
		_active_task->messages = msg->next;
	}
	return msg;
}

void os_delete_message(os_message * msg) {
	//os_task_free(_active_task, msg);
	os_destroy_object((os_object *)msg);
}

////////////////////////////END OF MESSAGE POOLING (USER MODE)//////////////////////////////////

////////////////////////////////////MAIL BOX////////////////////////////////////////////////////
static os_mailbox * _mailbox_list = NULL;  	  

static void os_enqueue_mailbox(os_mailbox * mailbox) {
	os_mailbox * iterator = _mailbox_list;
	if(iterator == NULL) {
		_mailbox_list = mailbox;
	} else {
		while(iterator->next != NULL) {
			iterator = iterator->next;
		}
		iterator->next = mailbox;
	}
}

static os_mailbox * os_dequeue_mailbox(os_mailbox * mailbox) {
	os_mailbox * iterator = _mailbox_list;
	if(iterator == NULL) return NULL;
	if(iterator == mailbox) {
		_mailbox_list = NULL;
		return mailbox;
	} else {
		while(iterator->next != NULL) {
			if(iterator->next == mailbox) {
			 	iterator->next = mailbox->next;
				return mailbox;
			}
			iterator = iterator->next;
		}
	}
	return NULL;
}

os_mailbox * os_create_mailbox(void) {
 	return os_task_create_mailbox(os_get_active_task());
}

os_mailbox * os_task_create_mailbox(os_task * owner) {
	os_mailbox * mbox = (os_mailbox *)os_task_alloc(owner, sizeof(os_mailbox));
	
	os_init_object((os_object *)mbox, OS_OBJECT_TYPE_MAILBOX, 0, sizeof(os_mailbox));
	mbox->head = 0;
	mbox->tail = 0;
	mbox->next = NULL;
	os_enqueue_mailbox(mbox);
	return mbox;	
}

void os_wait_mailbox(os_mailbox * mailbox, uint32 timeout) {
	os_wait_object((os_object *)mailbox, timeout);	
}

os_mailbox * os_post_mailbox(os_mailbox * mailbox, void * msg) {
	mailbox->msg[mailbox->head++] = msg;
	if(mailbox->head == OS_MAILBOX_SIZE) {
		mailbox->head = 0;
	}
	os_signal_object((os_object *)mailbox);				//signal object to waiting observer
	return mailbox;
}

void * os_fetch_mailbox(os_mailbox * mailbox) {
	void * msg = NULL;
	if(mailbox->head == mailbox->tail) return NULL;		//no message available
	msg = mailbox->msg[mailbox->tail++];
	if(mailbox->tail == OS_MAILBOX_SIZE) {
		mailbox->tail = 0;
	}
	return msg;		
}

void os_delete_mailbox(os_mailbox * mailbox) {
	mailbox = os_dequeue_mailbox(mailbox);
	if(mailbox != NULL) {
		os_destroy_object((os_object *)mailbox); 	
	}
}


////////////////////////////////END OF MAIL BOX/////////////////////////////////////////////////
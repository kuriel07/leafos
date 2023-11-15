#include "..\..\defs.h"
#include "..\inc\os.h"
#include "..\inc\os_core.h"
#include "..\inc\os_config.h"
#include "..\inc\os_mach.h"
#include "..\inc\os_msg.h"

//system apis
os_message * os_create_message(void * context, void * request, uint16 length, void * response) {
	os_message * msg = os_alloc(sizeof(os_message));
	msg->next = NULL;
	msg->sender = os_get_active_task();
	msg->context = context;
	msg->request = request;
	msg->reqlen = length;
	msg->response = response;
	msg->reslen = 0;
	msg->flag = OS_MESSAGE_QUEUED;
	return msg;
}

os_message * os_send_message(os_task * to, os_message * msg) {
	os_task * current = os_get_active_task();
	uint32 psw;
	os_message * iterator;
	if(to == NULL) return msg;
	if(to == current) {		//check if sending to oneself
		return NULL;
	}
	psw = os_enter_critical();
	iterator = to->messages;
	if(iterator == NULL) {
		to->messages = msg;
	} else {
		while(iterator->next != NULL) {
			iterator = iterator->next;
		}
		iterator->next = msg;
	}
	os_exit_critical(psw);
	if(msg->response != NULL) {
		//halt current task until message responded
		os_suspend();
	}
	return msg;
}

os_message * os_queue_message(os_task * to, os_message * msg) {
	//os_task * current = os_get_active_task();
	uint32 psw;
	os_message * iterator;
	psw = os_enter_critical();
	iterator = to->messages;
	if(iterator == NULL) {
		to->messages = msg;
	} else {
		while(iterator->next != NULL) {
			iterator = iterator->next;
		}
		iterator->next = msg;
	}
	os_exit_critical(psw);
	return msg;
}

void os_dispatch_reply(os_message * msg) {
	msg->flag = OS_MESSAGE_SERVICED;
	os_resume(msg->sender);
}

//user apis
os_message * os_peek_message(void) {
	os_task * current = os_get_active_task();
	return current->messages;
}

os_message * os_dequeue_message(void) {
	return os_dequeue_message_s(os_get_active_task());
}

os_message * os_dequeue_message_s(os_task * current) {
	OS_DEBUG_ENTRY(os_dequeue_message);
	//os_task * current = os_get_active_task();
	os_message * msg = current->messages;
	if(msg != NULL) {
		current->messages = msg->next;
		msg->next = NULL;
	}
	OS_DEBUG_EXIT();
	return msg;
}

void os_delete_message(os_message * msg) {
	os_free(msg);
}
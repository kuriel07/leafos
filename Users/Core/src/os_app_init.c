#include "..\..\defs.h"
//#include "..\apis\services.h"
#include "..\..\core\inc\os.h"
#include "..\..\core\inc\os_ipc.h"

#if 0
void os_user_event_dispatcher() {
	uint16 i=0;
	os_message * msg = NULL;
	while(1) {
		msg = (os_message *)peek_message();
		if(msg != NULL) {
			msg = (os_message *)dequeue_message();
			//msg->handle(msg->sender, (void *)msg->param); 
			((void (*)(w_handle, void *))msg->handle)((w_handle)msg->sender, (void *)msg->param);
			delete_message((lp_system_message)msg);
		}
	}	
}

void os_init_user_app(void) {
	create_thread(os_user_event_dispatcher, 1, 16384);	
}
#endif
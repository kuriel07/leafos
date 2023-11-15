#include "..\..\crypto\inc\cr_apis.h"
#include "..\..\drivers\inc\if_apis.h"
#include "..\..\toolkit\inc\tk_apis.h"
#include "..\..\gp\inc\gp_apis.h"
#include "..\..\defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "os.h"

void task1(void) {
	uint32 i;
	while(1) {
		os_wait(200);
		for(i=0;i<2000;i++);
	}
}

void task2(void) {
	uint32 i;
	while(1) {
		os_wait(200);
		for(i=0;i<2000;i++);
	}
}

int main(void)
{
	gui_handle ghandle;
	//if_sys_init(NULL);					//system init (must be called first)
	os_init();
	if_flash_init();
	
	os_create_task(0, task1, 0x24, 0x200);
	os_create_task(1, task2, 0x24, 0x200);
	os_start();
	while(1) {
		
	}
	return 0;
}
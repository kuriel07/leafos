#include "..\..\crypto\inc\cr_apis.h"
#include "..\..\interfaces\inc\if_apis.h"
#include "..\..\toolkit\inc\tk_apis.h"
#include "..\..\gui\inc\ui_resources.h"
#include "..\..\gp\inc\gp_apis.h"
#include "..\..\orbweaver\inc\VMStackApis.h"
#include "..\..\defs.h"
#include "..\..\config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jansson.h"

extern uint8 gb_tk_state;
#define gba_apdu_data_field (gba_apdu_buffer + 5)  
extern uint8 gba_net_buffer[NET_BUFFER_SIZE];
extern uint16 gba_orc_size;
uint8_c sample23[1795];		//http://orbleaf.com/apis/river/dl.php?oid=23
extern uint8_c * g_strErrorMsg[];
#define TK_KERNEL_MAX_CODESIZE		(IF_FLASH_SECTORSIZE - 0x200)

static vm_context g_exec_handle = { {0,0,0,0,0,0,0,0}, NULL, -1};
void tk_kernel_exec(tk_context_p ctx) {
	vm_instance * instance  = NULL;
	if(g_exec_handle.offset != (uint16)-1) {
		instance = vm_new_instance(ctx->config->devlet_name);
		if(instance != NULL) {
			vm_init(instance, &g_exec_handle.handle, g_exec_handle.offset);
			ctx->cos_status |= TK_COS_STAT_VM_STARTED;		//only worker allowed to execute vm_decode
			//start_execute_kernel:
			vm_exec_instance(instance);
		}
	}
}

void tk_kernel_kill(tk_context_p ctx) {
	os_task * task;
	if(strlen((const char *)ctx->config->devlet_name) != 0) {
		//find all task with specified name
		while((task = os_find_task_by_name((char *)ctx->config->devlet_name)) != NULL) {
			os_kill_task(task);
			os_wait(20);
		}
	}
}

void tk_kernel_init(tk_context_p ctx) {
	//initialize kernel framework/application
	OS_DEBUG_ENTRY(tk_kernel_init);
	uint16 code_offset;
	vm_instance * instance = NULL;
	os_task * task;
	vf_handle orcfile;  
	uint8 tag, hlen;	
	uint16 tsize, size;
	uint8 hdr_size;
	uint8 * icon_buffer;
	uint8 * ptr;
	uint8 event = VM_EVENT_STARTUP;
	uint8_c t_alias[] = "test server";
	ui_icon_tool * mycard;
	vr_init_global(ctx);
	mycard = (ui_icon_tool *)ui_get_object_by_name((gui_handle_p)ctx->display, "myCard");
	if(mycard != NULL) {
		//if(mycard->bitmap[0].bitmap != image_png_owvr_off) {	//check for default icon, release if any
		//	os_free(mycard->bitmap[0].bitmap);
		//	mycard->bitmap[0].bmpsize = 0;
		//}
		//set to default icon
		//mycard->bitmap[0].bitmap = (uint8 *)image_png_owvr_off;
		//mycard->bitmap[0].bmpsize = sizeof(image_png_owvr_off);
	}
	//start registering program.main (if any)
	g_exec_handle.offset = -1;
	if(tk_kernel_framework_load(ctx, &orcfile) != (uint8)-1) {
		code_offset = vr_load_script(VM_SCRIPT_BY_CLASS, &orcfile, 7, (uint8 *)"program");
		if(code_offset != (uint16)-1) {
			code_offset = vr_load_script(VM_SCRIPT_BY_METHOD, &orcfile, 6, (uint8 *)"main?0");
			if(code_offset != (uint16)-1) {
				//program.main found, register current handle
				memcpy(&g_exec_handle, &orcfile, sizeof(vf_handle));
				g_exec_handle.vars = NULL;
				g_exec_handle.offset = code_offset;
			}
		}
	}
	//search for app icon
	if(tk_kernel_framework_load(ctx, &orcfile) != (uint8)-1) {
		vf_first_handle(&orcfile);
		while((hdr_size = vf_next_handle(&orcfile, &tag, &size)) != 0) {
			if(tag != 0x99) continue;
			icon_buffer = os_alloc(size);
			if(icon_buffer != NULL) {
				vf_read_handle(&orcfile, orcfile.file_offset, icon_buffer, size);
				if(g_exec_handle.offset != (uint16)-1) {		//program.main available
					mycard->bitmap[0].bitmap = icon_buffer;
					mycard->bitmap[0].bmpsize = size;
					if((ptr = (uint8 *)strstr((const char *)ctx->config->devlet_name, ".")) != NULL) {
						ui_set_text((ui_text *)mycard, ptr + 1);
					}
				} else {
					os_free(icon_buffer);
				}
				break;
			}
		}
		
	}
	//start re-init my-card object
	if(mycard != NULL) ui_reinitialize_object((ui_object *)mycard);
	//close active application
	task = os_find_task_by_name((const char *)ctx->app_name);
	if(task != NULL && vm_is_running(task->context)) {
		vm_abort(task->context);
		while(vm_is_running(task->context)) {
			os_wait(40);
		}
	}
	//start executing startup event (if any)
	if(tk_kernel_framework_load(ctx, &orcfile) != (uint8)-1) {
		code_offset = vr_load_script(VM_SCRIPT_BY_EVENT, &orcfile, 1, &event);
		if(code_offset == (uint16)-1) goto exit_invoke_kernel;
		//start executing startup event
		instance = vm_new_instance(ctx->config->devlet_name);
		if(instance != NULL) {
			vm_init(instance, &orcfile, code_offset);
			ctx->cos_status |= TK_COS_STAT_VM_STARTED;
			vm_exec_instance(instance);
		}
	}
	exit_invoke_kernel:
	OS_DEBUG_EXIT();
}

uint8 tk_kernel_framework_load(tk_context_p ctx, void * handle) {
	OS_DEBUG_ENTRY(tk_kernel_framework_load);
	uint8 hash[SHARD_HASH_SIZE];
	uint8 ret = -1;
	//load kernel application framework
	uint8 * code_buffer = (uint8 *)0x8008000;				//default framework address
	uint32 code_length = *((uint32 *)code_buffer);
	tk_gen_hash(code_buffer + sizeof(uint32), code_length, hash);
	//check hash first for file integrity
	if(memcmp(ctx->config->devlet_hash, hash, SHARD_HASH_SIZE) ==0) {
		if(code_length != (uint32)-1) {			//no data allocated (node code allocated)
			//[code_len:uint32][code][data_len:uint32][data]
			ret = vr_init_handle(handle, 0, sizeof(uint32), code_length);
		}
	}
	OS_DEBUG_EXIT();
	return ret;
}

uint8 tk_kernel_data_load(tk_context_p ctx, void * handle) {
	OS_DEBUG_ENTRY(tk_kernel_data_load);
	uint8 ret = -1;
	//load kernel application data
#if 0
	uint8 * code_buffer = (uint8 *)0x8008000;				//default framework address
	uint32 code_length = *((uint32 *)code_buffer);
	uint8 * data_buffer = code_buffer + code_length + sizeof(uint32);
	uint32 data_length = *((uint32 *)data_buffer);
	if(code_length != (uint32)-1) {
		//[code_len:uint32][code][data_len:uint32][data]
		ret = vr_init_handle(handle, 0, code_length + sizeof(uint32) + sizeof(uint32), data_length);
	}
#else
	//uint8 * code_buffer = (uint8 *)0x8007000;				//default framework address
	//uint32 code_length = *((uint32 *)code_buffer);
	uint8 * data_buffer = (uint8 *)0x8007000;
	uint32 data_length = *((uint32 *)data_buffer);
	if(data_length != (uint32)-1) {
		//[code_len:uint32][code][data_len:uint32][data]
		ret = vr_init_handle(handle, 1, 0x3000 + sizeof(uint32), data_length);
	}
#endif
	OS_DEBUG_EXIT();
	return ret;
}

static void tk_kernel_menu_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(tk_kernel_menu_click);
	vf_handle ofile;
	vm_instance * instance = NULL;
	uint32 code_offset;
	uint16 size, tsize;
	tk_context_p ctx = obj->target;
	ui_item * item = (ui_item *)obj;
	if(tk_kernel_framework_load(ctx, &ofile) != (uint8)-1) {
		//unable to load framework
		code_offset = vr_load_script(VM_SCRIPT_BY_ALIAS|VM_SCRIPT_BY_INDEX, &ofile, 1, &item->id);
		if(code_offset != (uint16)-1) {
			instance = vm_new_instance(ctx->config->devlet_name);
			if(instance != NULL) {
				vm_init(instance, &ofile, code_offset);
				ctx->cos_status |= TK_COS_STAT_VM_STARTED;
				vm_exec_instance(instance);
			}
		}
	}
	OS_DEBUG_EXIT();
}

uint8 tk_kernel_trigger_event(tk_context_p ctx, uint8 event, uint8 argLen, uint8 * arg) {
	//initialize kernel framework/application
	OS_DEBUG_ENTRY(tk_kernel_trigger_event);
	vf_handle ofile;
	vm_instance * instance = NULL;
	uint8 ret = 0;
	uint32 code_offset;
	uint16 size, tsize;
	if((ret = tk_kernel_framework_load(ctx, &ofile)) != (uint8)-1) {
		//unable to load framework
		ret = -1;
		code_offset = vr_load_script(VM_SCRIPT_BY_EVENT, &ofile, 1, &event);
		if(code_offset != (uint16)-1) {
			instance = vm_new_instance(ctx->config->devlet_name);
			if(instance != NULL) {
				vm_init(instance, &ofile, code_offset);
				if(argLen != 0) vm_push_argument(instance, argLen, arg);
				ctx->cos_status |= TK_COS_STAT_VM_STARTED;
				vm_exec_instance(instance);
			}
			ret = 0;
		}
	} 
	OS_DEBUG_EXIT();
	return ret;
}

uint8 tk_kernel_list_menu(tk_context_p ctx) { 
	OS_DEBUG_ENTRY(tk_kernel_list_menu);
	vf_handle orcfile;
	uint8 tbuf[255];
	uint8 tlen = 255;
	uint16 size, tsize;
	ui_item * obj;
	uint8 tag;
	uint8 ret = -1;
	uint16 i, j;
	tk_clear_body(ctx);
	if(tk_kernel_framework_load(ctx, &orcfile) != (uint8)-1) {
		//display all registered menu on framework file
		size = vr_load_script(VM_SCRIPT_BY_ALIAS | VM_SCRIPT_LIST, &orcfile, tlen, tbuf);
		if(tlen != 0) {
			for(i = 0, j = 0; i < size; j++) {
				i += tk_pop(tbuf + i, &tag, &tsize, tbuf);
				tbuf[tsize] = 0;
				ui_add_body(ctx->display, (obj = (ui_item *)ui_item_create(tbuf, NULL, 0, j, tk_kernel_menu_click)));
				((ui_object *)obj)->target = ctx;
			}
			ret = 0;		
		}
	}
	OS_DEBUG_EXIT();
	return ret;
}

uint8 tk_kernel_execute(tk_context_p ctx) {
	OS_DEBUG_ENTRY(tk_kernel_execute);
	uint8 ret = -1;
	if(tk_kernel_list_menu(ctx) == 0) {		//no menu/app installed
		//while(if_rfid_present(ctx->rctx) == 0) {
		//clear all unused objects
		//tk_clear_body(ctx);
		ret = 0;
	}
	OS_DEBUG_EXIT();
	return ret;				//success
}

uint16 tk_kernel_invoke(tk_context_p ctx, uint8 mode, uint8 len, uint8 * tags) {
	//invoke method/function from kernel framework
	OS_DEBUG_ENTRY(tk_kernel_invoke);
	ui_item_object * obj;
	vm_instance * instance = NULL;
	uint8 tlen;
	uint16 rlen = 0;
	uint16 hlen;
	uint8 tbuf[255];
	uint8 hbuf[64];
	uint8 lbuf[64];
	uint8 hash[SHARD_HASH_SIZE];
	uint16 dev_address;
	uint16 certsz = 0;
	ui_alert_p alert = NULL;
	uint8 tag, i;
	uint16 size;
	uint8 num_arg = 0;
	uint16 code_offset;
	gba_orc_size = 0;
	vf_handle orcfile;
	vm_object * retObj = NULL;
	struct ow_context {
		uint8 magic;
		vf_handle handle;
	} w_context;
	strcpy((char *)tbuf, (const char *)"Invoking...");
	switch(mode & 0x07) {
		case 0:			//execute function by alias, parameter already passed on STK_TAG_PARAM
			//passed parameters STK_TAG_ALIAS, STK_TAG_PARAM[1]
			//get function alias
			if((hlen = tk_pop_by_tag(tags, len, STK_TAG_ALIAS, hbuf)) == (uint16)-1) goto exit_invoke_kernel;
			//init script file
			if(tk_kernel_framework_load(ctx, &orcfile) == (uint8)-1) goto exit_invoke_kernel;
			code_offset = vr_load_script(VM_SCRIPT_BY_ALIAS, &orcfile, hlen, hbuf);
			if(code_offset == (uint16)-1) goto exit_invoke_kernel;
			//initialize virtual machine
			instance = vm_new_instance(ctx->config->devlet_name);
			if(instance != NULL) {
				vm_init(instance, &orcfile, code_offset);
				if((hlen = tk_pop_by_tag(tags, len, STK_TAG_PARAM, tbuf)) != (uint16)-1)  {
					//push new argument
					vm_push_argument(instance, hlen, tbuf);
				}
				start_execute_kernel:
				ctx->cos_status |= TK_COS_STAT_VM_STARTED;
				//start executing
				vm_decode(instance);
				//set returning value
				retObj = vm_pop_stack_arc(instance);
				rlen = retObj->len;
				if(retObj != NULL) {	
					memcpy(tags, retObj->bytes, retObj->len);
				}
				vm_close(instance);
				vm_release_instance(instance);
			}
			exit_invoke_kernel:
			break;
		case 1:			//load instance by class
			//passed parameters STK_TAG_VENDOR, STK_TAG_APPNAME, STK_TAG_CLASS
			if((hlen = tk_pop_by_tag(tags, len, STK_TAG_CLASS, hbuf)) == (uint16)-1) goto exit_invoke_kernel;
			//vr_init_script(&orcfile, (uint8 *)code_buffer + sizeof(uint32), code_length);
			//tk_kernel_framework_load(ctx, &orcfile);	
			if(tk_kernel_framework_load(ctx, &orcfile) == (uint8)-1) goto exit_invoke_kernel;
			code_offset = vr_load_script(VM_SCRIPT_BY_CLASS, &orcfile, hlen, hbuf);
			if(code_offset == (uint16)-1) goto exit_invoke_kernel;
			//retObj = vm_create_object(sizeof(code_offset), &code_offset);
			w_context.magic = 0xAF;
			memcpy(&w_context.handle, &orcfile, sizeof(vf_handle));
			memcpy(tags, &w_context, sizeof(w_context));
			rlen = sizeof(w_context);
			break;
		case 2:			//execute method on instance
			//passed parameters STK_TAG_INSTANCE, STK_TAG_METHOD, STK_TAG_PARAM[n]
			if((hlen = tk_pop_by_tag(tags, len, STK_TAG_INSTANCE, hbuf)) == (uint16)-1) goto exit_invoke_kernel;
			if(hlen != sizeof(w_context)) goto exit_invoke_kernel;
			memcpy(&w_context, hbuf, sizeof(w_context));
			if(w_context.magic != 0xAF) { rlen = -1; goto exit_invoke_kernel; }
			//calculate number of parameters
			for(i=0;i<len;) {
				i += tk_pop(tags + i, &tag, &size, tbuf);
				if(tag == STK_TAG_PARAM) num_arg++;
			}
			//load method for execution
			if((hlen = tk_pop_by_tag(tags, len, STK_TAG_METHOD, hbuf)) == (uint16)-1) goto exit_invoke_kernel;
			memcpy(&orcfile, &w_context.handle, sizeof(vf_handle));
			code_offset = vr_load_script(VM_SCRIPT_BY_METHOD, &orcfile, hlen, hbuf);
			if(code_offset == (uint16)-1) goto exit_invoke_kernel;
			instance = vm_new_instance(ctx->config->devlet_name);
			if(instance != NULL) {
				vm_init(instance, &w_context.handle, code_offset);
				//push arguments
				for(i=0;i<len;) {
					i += tk_pop(tags + i, &tag, &size, tbuf);
					if(tag == STK_TAG_PARAM) vm_push_argument(instance, size, tbuf);
				}
				goto start_execute_kernel;
			}
			break;
	}
	OS_DEBUG_EXIT();
	return rlen;
}

uint8 tk_kernel_install(tk_context_p ctx, uint8 * name, uint8 * oid, uint8 * sign, uint16 size, uint16 bnum, uint16 mode) {
	OS_DEBUG_ENTRY(tk_kernel_install);
	uint8 esid_string[33];
	uint8 url_buffer[256];
	uint8 hash[SHARD_HASH_SIZE];
	uint32 code_length, data_length;
	uint8 * data_ptr;
	ssl_cert_p cert = NULL;
	net_request req;
	uint8 error_code = 0;
	
	tk_bin2hex(ctx->esid, SHARD_ESID_SIZE, esid_string);
	sprintf((char *)url_buffer, "http://orbleaf.com/apis/river/dld.php?oid=%s&esid=%s", oid, esid_string);
	//device wake
	if_net_wake(ctx->netctx);
#if 0	//SHARD_SSL_SUPPORT
	code_length = https_send(ctx->netctx, IF_HTTP_TYPE_GET, url_buffer, 443, NULL, NULL, 0, gba_net_buffer + sizeof(uint32));
#else			
	code_length = http_send(ctx->netctx, net_request_struct(&req, IF_HTTP_TYPE_GET, url_buffer, 80, cert), NULL, NULL, 0, gba_net_buffer + sizeof(uint32));
#endif
	//device sleep
	if_net_sleep(ctx->netctx);
	//ui_alert_close(ctx->display, alert);
	if(code_length == 0) {
		error_code = TK_ERR_ORC_DATA;
		//gb_tk_state = TK_STATE_RIVER_ERROR;
		goto show_error_msg;
	}
	code_length = tk_base64_decode(gba_net_buffer + sizeof(uint32), code_length);
	if(code_length > TK_KERNEL_MAX_CODESIZE) {
		//ORC file too large
		error_code = TK_ERR_ORC_DATA;
		goto show_error_msg;
	}
	//[CODE_LENGTH:4][CODE:n][DATA_LENGTH:4][DATA:n]
	code_length = size;
	tk_gen_hash(gba_net_buffer + sizeof(uint32), code_length, hash);		//save generated hash
	if(memcmp(hash, sign, SHARD_HASH_SIZE) == 0) {
		//kill previous application if running
		tk_kernel_kill(ctx);
		//start code installation
		*((uint32 *)gba_net_buffer) = code_length;													//set length
#if 0
		data_ptr = (gba_net_buffer + code_length + sizeof(uint32));
		data_length = IF_FLASH_SECTORSIZE - (code_length + 0x100);											//calculate free data size
		*((uint32 *)data_ptr) = data_length;
		memset(data_ptr + sizeof(uint32), 0xFF, 8);													//write data end mark
		if_flash_code_write(NULL, 0, gba_net_buffer, code_length + sizeof(uint32) + 12);				//write length and data altogether
#else
		//data and code separation (2017.08.31)
		if_flash_code_write(NULL, 0, gba_net_buffer, code_length + sizeof(uint32));				//write length and data altogether
		//formating data area for fresh installation
		if(mode == 0) {		//install new
			memset(gba_net_buffer + sizeof(uint32), 0xFF, 0x1000);									//delete previous data
			*((uint32 *)gba_net_buffer) = 0x1000 - sizeof(uint32);													//set data length (fixed size)
			if_flash_data_write(NULL, 0x3000, gba_net_buffer, 0x1000);				//write length and data altogether
		}
#endif
		//alert = ui_alert_show(ctx->display, msg_title, (uint8 *)g_strErrorMsg[error_code], UI_ALERT_BUTTON_OK, msg_icon);
		//save current devlet name
		strncpy((char *)ctx->config->devlet_name, (char *)name, 16);							//save devlet name (AID)
		ctx->config->devlet_size = code_length;																	//save devlet size
		strncpy((char *)ctx->config->devlet_id, (const char *)oid, 16);															//save oid
		memcpy(ctx->config->devlet_hash, sign, SHARD_HASH_SIZE); 							//save hash
		ctx->config->devlet_build = bnum;																//save build number
		if_config_write(ctx->config, sizeof(tk_config));
		//start re-init kernel application, automatically releasing any prev installed handles
		tk_kernel_init(ctx);
	} else {
		error_code = TK_ERR_HASH_INVALID;
	}
	show_error_msg:
	OS_DEBUG_EXIT();
	return error_code;
}

//synchronize current devlet with cloud, in-case devlet already updated
uint8 tk_kernel_devlet_sync(tk_context_p ctx) {
	OS_DEBUG_ENTRY(tk_kernel_devlet_sync);
	json_t * json;
    json_error_t error;
	int rv;
	uint8 resbuf[0x200];
	uint16 len;
	uint8 ret = -1;
	uint8 * oid;
	uint8 * hash;
	ssl_cert_p cert = NULL;
	net_request req;
	int size;
	int bnum;
	int8 tz;
	uint8 hash_bytes[SHARD_HASH_SIZE];
#if SHARD_SSL_SUPPORT
	ssl_handle_p ssdl;
#endif
	uint8 url_buffer[256];
	if_net_wake(ctx->netctx);
#if 0
	len = http_send(ctx->netctx, IF_HTTP_TYPE_GET, (uint8 *)"http://orbleaf.com/updates/kron/v10/alive.php", 80, NULL, 0, resbuf);
	if(len == 0) goto exit_check_update;
	sprintf((char *)url_buffer, "https://orbleaf.com/apis/river/chkd.php?oid=%s", ctx->config->devlet_id);
	if(if_ssl_init(ctx->netctx, (uint8 *)orbleaf_cert_der_1024, sizeof(orbleaf_cert_der_1024)) != 0) goto exit_check_update;
	len = https_send(ctx->netctx, IF_HTTP_TYPE_GET, url_buffer, 443, NULL, NULL, 0, resbuf);
	if_ssl_release(ctx->netctx);
#else
	sprintf((char *)url_buffer, "http://orbleaf.com/apis/river/chkd.php?oid=%s", ctx->config->devlet_id);
	len = http_send(ctx->netctx, net_request_struct(&req, IF_HTTP_TYPE_GET, url_buffer, 80, cert), NULL, NULL, 0, resbuf);
#endif
	//network sleep
	if_net_sleep(ctx->netctx);
	resbuf[len] = 0;
	if(len != 0) {
		//{"oid":"45","hash":"bd13a3d675a4de608fcb531bf87497cbc0fe145522416eaf52598605fcc54f21","size":"1047","build_number":"20"}
		json = json_loads((const char *)resbuf, JSON_DECODE_ANY, &error);
		rv = json_unpack(json, "{s:s, s:s, s:i, s:i}", "oid", &oid, "hash", &hash, "size", &size, "build_number", &bnum);
		//check for build number
		if(bnum > ctx->config->devlet_build) {
			tk_hex2bin(hash, hash_bytes);
			if(tk_kernel_install(ctx, ctx->config->devlet_name, oid, hash_bytes, size, bnum, 1) == 0) {
				tk_console_print(ctx, "framework updated");
			} else {
				tk_console_print(ctx, "sync failed");
			}
		}
		json_object_clear(json);
		ret = 0;
	}
	exit_check_update:
	OS_DEBUG_EXIT();
	return ret;
}

//download new devlet from cloud store
void tk_devlet_item_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(tk_devlet_item_click);
	ui_orbriver_item item;
	ui_alert * alert;
	uint8 * msg_title = (uint8 *)"Error";
	uint8 msg_icon = UI_ALERT_ICON_ERROR;
	uint8 err;
	ui_object * btn;
	uint8 app_name[80];
	uint8 text_conf_buffer[256];
	tk_context_p ctx = obj->target;
	//use container to save object properties
	memcpy(&item, obj, sizeof(ui_orbriver_item));
	switch(item.event) {
		case UI_ITEM_EVENT_CLICK:
			sprintf((char *)text_conf_buffer, "Do you want to install %s?", ((ui_text *)&item)->text);
			alert = ui_alert_show(ctx->display, (uint8 *)"Confirmation", (uint8 *)text_conf_buffer, UI_ALERT_BUTTON_OK | UI_ALERT_BUTTON_CANCEL, UI_ALERT_ICON_INFO);
			//start ui rendering presentation (enter loop)	
			btn = ui_wait_user_input(ctx->display);
			ui_alert_close(ctx->display, alert);
			if(ui_alert_result(ctx->display) == UI_ALERT_RESULT_OK) {
				start_download:
				alert = ui_alert_show(ctx->display, (uint8 *)"Information", (uint8 *)"Downloading...", UI_ALERT_INFO, UI_ALERT_ICON_NETWORK);
				err = tk_kernel_install(ctx, (uint8 *)((ui_text *)&item)->text, item.oid, item.hash, item.size, item.bnum, 0);
				ui_alert_close(ctx->display, alert);
				if(err != 0) {
					alert = ui_alert_show(ctx->display, msg_title, (uint8 *)g_strErrorMsg[err], UI_ALERT_BUTTON_OK, msg_icon);
				} else {
					msg_title = (uint8 *)"Info";
					msg_icon = UI_ALERT_ICON_INFO;
					alert = ui_alert_show(ctx->display, msg_title, (uint8 *)g_strErrorMsg[err], UI_ALERT_BUTTON_OK, msg_icon);
				}
			}
			break;
		case UI_ITEM_EVENT_DETAIL:
			ui_item_show_detail(params, (ui_orbriver_item *)&item);
			break;
	}
	OS_DEBUG_EXIT();
	return;
}

//list all devlet available for current user from cloud store
void tk_kernel_devlet_list(tk_context_p ctx) {
	OS_DEBUG_ENTRY(tk_kernel_devlet_list);
	//load store application
	uint8 url_buffer[256];
	uint8 search_buffer[40] = { 0, 0 };
	uint16 j;
	uint16 size, isize;
	uint8 tag;
	uint8 oid[16];
	uint32 nod;
	uint16 bnum;
	uint8 name[16];
	uint8 desc[SHARD_MAX_DESC + 1];
	uint8 hash_buffer[SHARD_HASH_SIZE];
	uint8 elfBuffer[17];
	uint8 appBuffer[17];
	uint8 sdBuffer[17];
	uint8 esid_string[33];
	gui_handle_p display;
	ssl_handle_p ssdl;
	ssl_cert_p cert = NULL;
	net_request req;
	uint16 hh;
	uint16 i, sz;
	uint8 error_code = 0;
	ui_textbox * textbox;
	ui_list_scroll * listbox;
	ui_alert * alert;
	ui_object * cbbox;
	ui_object * scbox;
	ui_object * obj;
	tk_config * conf = ctx->config;
	ui_object * screen;
	tk_bin2hex(ctx->esid, SHARD_ESID_SIZE, esid_string);
	//list top most orblet
	restart_display:
	//ui_clear_body(ctx->display, UI_COLOR_WHITE);
	sprintf((char *)url_buffer, "http://orbleaf.com/apis/river/lsd.php?uid=%s&esid=%s&q=%s", conf->uid, esid_string, search_buffer);
	alert = ui_alert_show(ctx->display, (uint8 *)"Information", (uint8 *)"Connecting...", UI_ALERT_INFO, UI_ALERT_ICON_NETWORK);
	//wake wifi
	if_net_wake(ctx->netctx);
	//sending reqeuest to river service (list application)
#if 0	//SHARD_SSL_SUPPORT
	//gba_orc_size = https_send(ctx->netctx, IF_HTTP_TYPE_GET, url_buffer, 443, NULL, 0, gba_net_buffer);
	gba_orc_size = https_send(ctx->netctx, IF_HTTP_TYPE_GET, url_buffer, 443, NULL, NULL, 0, gba_net_buffer);
#else			
	gba_orc_size = http_send(ctx->netctx, net_request_struct(&req, IF_HTTP_TYPE_GET, url_buffer, 80, cert), NULL, NULL, 0, gba_net_buffer);
#endif
	//network sleep
	if_net_sleep(ctx->netctx);
	//decode base64 charset
	if(gba_orc_size == 0) {
		//not connected
		ui_alert_close(ctx->display, alert);
		ui_alert_show(ctx->display, (uint8 *)"Error", (uint8 *)"Unable to connect", UI_ALERT_BUTTON_OK, UI_ALERT_ICON_ERROR);
		goto exit_devlet_list;
	}
	gba_orc_size = tk_base64_decode(gba_net_buffer, gba_orc_size);
	ui_alert_close(ctx->display, alert);
	//print_hex(gba_net_buffer, gba_orc_size);
	//ui_add_body(ctx->display, (cbbox = ui_combo_create(category, 2, (uchar *)"", tk_orbriver_category_select, "Latest", "Most Downloads")));
	//obj->aidlen = 128;
	//obj->aidlen = 0;
	i = 0;
	screen = ui_push_screen(ctx->display, NULL);
	ui_set_object_name(screen, "devletForm");
	display = (gui_handle_p)ctx->display;
	hh = ((ui_rect *)display->body)->h;// - ((ui_rect *)cbbox)->h;
	listbox = (ui_list_scroll *)ui_list_create(0, 0, ((ui_rect *)display->body)->w, hh);
	ui_add_body(ctx->display, listbox);
	//ui_list_add((ui_object *)listbox, (ui_object *)ui_item_create((uchar *)"Search", NULL, 0, 0, tk_orbriver_search_click));

	while(i < gba_orc_size) {
		i+= tk_pop(gba_net_buffer + i, &tag, &size, gba_net_buffer);
		if(tag == 0x30 && size != 0) {		//valid sequence tag
			for(j=0;j<size;) {
				j += tk_pop(gba_net_buffer + j, &tag, &isize, gba_net_buffer);
				gba_net_buffer[isize] = 0;
				switch(tag) {
					case 0x02: 		//OID
						strncpy((char *)oid, (const char *)gba_net_buffer, sizeof(oid));
						break;
					case 0x08:		//orblet size
						sz = atoi((const char *)gba_net_buffer);
						break;
					case 0x0A:		//NOD
						nod = atoi((const char *)gba_net_buffer);
						break;
					case 0x05: 		//Name
						strncpy((char *)name, (const char *)gba_net_buffer, sizeof(name));
						break;
					case 0x0B:		//build number
						bnum = atoi((const char *)gba_net_buffer);
						break;
					case 0x0C:		//hash
						memcpy(hash_buffer, gba_net_buffer, isize);
						break;
					case 0x0D:		//description
						strncpy((char *)desc, (const char *)gba_net_buffer, sizeof(desc));
						break;
				}
			}
#if 0
			j = 0;
			//oid
			j += tk_pop(gba_net_buffer + j, &tag, &size, gba_net_buffer);
			memcpy(oid, gba_net_buffer, size);
			oid[size] = 0;
			//size
			j += tk_pop(gba_net_buffer + j, &tag, &size, gba_net_buffer);
			gba_net_buffer[size] = 0;
			sz = atoi((const char *)gba_net_buffer);
			//number of downloads
			j += tk_pop(gba_net_buffer + j, &tag, &size, nod);
			nod[size] = 0;
			//name
			j += tk_pop(gba_net_buffer + j, &tag, &size, gba_net_buffer);
			gba_net_buffer[size] = 0;
			//build number
			j += tk_pop(gba_net_buffer + j, &tag, &size, bnum);
			bnum[size] =0;
			//hash
			j += tk_pop(gba_net_buffer + j, &tag, &size, hash);
#endif
			ui_list_add((ui_object *)listbox, (obj = (ui_object *)ui_orbriver_item_create(name, oid, sz, nod, bnum, hash_buffer, desc, tk_devlet_item_click)));
			if(obj == NULL) break;
			obj->target = ctx;
		}
	}
	exit_devlet_list:
	OS_DEBUG_EXIT();
	return;
}
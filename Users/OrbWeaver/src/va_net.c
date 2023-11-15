#include "..\..\defs.h"
#include "..\..\config.h"
#include "..\inc\MMApis.h"	
#include "..\inc\VMStackApis.h"	
#include "..\crypto\inc\cr_apis.h"
#include "..\toolkit\inc\tk_apis.h"
#include "..\gui\inc\ui_core.h"
#include "..\core\inc\os_core.h"
#include "..\interfaces\inc\if_apis.h"
#include "..\inc\vm_framework.h"
#include "..\..\ether\inc\smtp.h"
#include "..\..\ether\inc\netbios.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//extern vm_object * g_pVaRetval;
//extern sys_context g_sVaSysc;
extern tk_context_p g_pfrmctx;

//////////////////////////////////////NETWORK APIS//////////////////////////////////////////
typedef struct va_netbuf {
	net_buffer * netbuf;
	struct va_netbuf * next;
} va_netbuf;

static va_netbuf * g_netconn = NULL;

static void va_net_add_conn(net_buffer * netbuf) {
	va_netbuf * iterator = g_netconn;
	va_netbuf * nbuf = (va_netbuf *)os_alloc(sizeof(va_netbuf));
	if(nbuf != NULL) {
		nbuf->netbuf = netbuf;
		nbuf->next = NULL;
		if(g_netconn == NULL) {
			g_netconn = nbuf;
		} else {
			while(iterator->next != NULL) {
				iterator = iterator->next;
			}
			iterator->next = nbuf;
		}
	}
}

static void va_net_remove_conn(net_buffer * netbuf) {
	va_netbuf * prev = NULL;
	va_netbuf * iterator = g_netconn;
	while(iterator != NULL) {
		if(iterator->netbuf == netbuf) {
			if(prev == NULL) g_netconn = iterator->next;
			else prev->next = iterator->next;
			break;
		}
		prev = iterator;
		iterator = iterator->next;
	}
	os_free(iterator);
}

void va_net_init() {			//should be called one time during context initialization or new installation
	va_netbuf * iterator = g_netconn;
	va_netbuf * nbuf;
	while(iterator != NULL) {
		nbuf = iterator;
		iterator = iterator->next;
		switch(nbuf->netbuf->mode & 0x07) {
			case (NET_TYPE_SSL | NET_TYPE_TCP): 
				if_net_ssl_close((ssl_handle_p)nbuf->netbuf);
				break;
			default:
			case (NET_TYPE_SSL | NET_TYPE_UDP): 
				//if_net_ssl_close((ssl_handle_p)nbuf->netbuf);
				break;
			case NET_TYPE_TCP: 
				if_net_tcp_close((net_conn_p)nbuf->netbuf);
				break;
			case NET_TYPE_UDP:
				if_net_udp_close((net_conn_p)nbuf->netbuf);
				break;
		}
		nbuf->netbuf = NULL;
		os_free(nbuf);
	}
	g_netconn = NULL;
}

//network APIs
static void va_net_flush(VM_DEF_ARG) {			//param1 = context
	OS_DEBUG_ENTRY(va_net_flush);
	//net_context_p nctx;
	if(g_pfrmctx == NULL) goto exit_net_close;
	//nctx = g_pfrmctx->netctx;
	vm_object * vctx = vm_get_argument(VM_ARG, 0);
	if(vctx->len != 0) {		//return;
		if(vm_imemcmp(((va_net_context *)vctx->bytes)->type, "TCP", 3) == 0) {
			//if_net_flush(((va_net_context *)vctx->bytes)->base.ctx);
		} else if(vm_imemcmp(((va_net_context *)vctx->bytes)->type, "UDP", 3) == 0) {
			//if_net_flush(((va_net_context *)vctx->bytes)->base.ctx);
		} else if(vm_imemcmp(((va_net_context *)vctx->bytes)->type, "SSL", 3) == 0) {
			//close ssl handle
			//if_net_flush(((va_net_context *)vctx->bytes)->base.ctx);
		} else if(vm_imemcmp(((va_net_context *)vctx->bytes)->type, "TLS", 3) == 0) {
			//close ssl handle
			//if_net_flush(((va_net_context *)vctx->bytes)->base.ctx);
		}		
	}
	exit_net_close:
	OS_DEBUG_EXIT();
	return;
}

static void va_net_close(VM_DEF_ARG) {			//param1 = context
	OS_DEBUG_ENTRY(va_net_close);
	//net_context_p ctx;
	if(g_pfrmctx == NULL) goto exit_net_close;
	//ctx = g_pfrmctx->netctx;
	vm_object * vctx = vm_get_argument(VM_ARG, 0);
	if(vctx->len != 0) {		//return;
		if(vm_imemcmp(((va_net_context *)vctx->bytes)->type, "TCP", 3) == 0) {
			if_net_tcp_close(((va_net_context *)vctx->bytes)->base.ctx);
		} else if(vm_imemcmp(((va_net_context *)vctx->bytes)->type, "UDP", 3) == 0) {
			if_net_udp_close(((va_net_context *)vctx->bytes)->base.ctx);
		} else if(vm_imemcmp(((va_net_context *)vctx->bytes)->type, "SSL", 3) == 0) {
			//close ssl handle
			if_net_ssl_close(((va_net_context *)vctx->bytes)->base.ctx);
		} else if(vm_imemcmp(((va_net_context *)vctx->bytes)->type, "TLS", 3) == 0) {
			//close ssl handle
			if_net_ssl_close(((va_net_context *)vctx->bytes)->base.ctx);
		}		
		//remove from connection list
		va_net_remove_conn(((va_net_context *)vctx->bytes)->base.ctx);
	}
	exit_net_close:
	OS_DEBUG_EXIT();
	return;
}

static void va_net_recv(VM_DEF_ARG) {			//param1 = context, param2=timeout, return response
	OS_DEBUG_ENTRY(va_net_recv);
	vm_object * vctx = vm_get_argument(VM_ARG, 0);
	uint16 timeout = 5000;
	uint16 len = 0;
	if(vm_get_argument_count(VM_ARG) > 1) timeout = va_o2f(vm_get_argument(VM_ARG, 1));
	if(vctx->len != 0) {
		if(vm_imemcmp(((va_net_context *)vctx->bytes)->type, "TCP", 3) == 0) {
			len = if_net_tcp_recv(((va_net_context *)vctx->bytes)->base.ctx, gba_net_buffer, 1024);
		} else if(vm_imemcmp(((va_net_context *)vctx->bytes)->type, "UDP", 3) == 0) {
			len = if_net_udp_recv(((va_net_context *)vctx->bytes)->base.ctx, gba_net_buffer, 1024);
		} else if(vm_imemcmp(((va_net_context *)vctx->bytes)->type, "SSL", 3) == 0) {
			len = if_net_ssl_recv(((va_net_context *)vctx->bytes)->base.ctx, gba_net_buffer, 1024);
		} else if(vm_imemcmp(((va_net_context *)vctx->bytes)->type, "TLS", 3) == 0) {
			len = if_net_ssl_recv(((va_net_context *)vctx->bytes)->base.ctx, gba_net_buffer, 1024);
		} 
		if(len != 0) {
			vm_set_retval(vm_create_object(len, gba_net_buffer));		//set response as result
		}
	}
	OS_DEBUG_EXIT();
	return;
}

static void va_net_send(VM_DEF_ARG) {			//param1 = context, param2=payload
	OS_DEBUG_ENTRY(va_net_send);
	vm_object * vctx = vm_get_argument(VM_ARG, 0);
	vm_object * payload =  vm_get_argument(VM_ARG, 1);
	uint16 len = 0;
	if(vctx->len != 0) {		//return;
		if(vm_imemcmp(((va_net_context *)vctx->bytes)->type, "TCP", 3) == 0) {
			if_net_tcp_send(((va_net_context *)vctx->bytes)->base.ctx, payload->bytes, payload->len);
		} else if(vm_imemcmp(((va_net_context *)vctx->bytes)->type, "UDP", 3) == 0) {
			if_net_udp_send(((va_net_context *)vctx->bytes)->base.ctx, payload->bytes, payload->len, NULL, 0);
		} else if(vm_imemcmp(((va_net_context *)vctx->bytes)->type, "SSL", 3) == 0) {
			if_net_ssl_send(((va_net_context *)vctx->bytes)->base.ctx, payload->bytes, payload->len);
		} else if(vm_imemcmp(((va_net_context *)vctx->bytes)->type, "TLS", 3) == 0) {
			if_net_ssl_send(((va_net_context *)vctx->bytes)->base.ctx, payload->bytes, payload->len);
		}
	}
	OS_DEBUG_EXIT();
}

void va_net_listen_callback(void * vctx, net_buffer_p netbuf) {
	vm_object * vsock;
	vm_object * vbuffer;
	vm_instance * instance = NULL;
	va_net_context defctx;
	char name[16];
	vm_function * func = (vm_function *)vctx;
	if(func == NULL) return;
	((va_default_context *)&defctx)->ctx = netbuf;
	((va_default_context *)&defctx)->close = va_net_flush;
	((va_default_context *)&defctx)->read = va_net_recv;
	((va_default_context *)&defctx)->write = va_net_send;
	((va_default_context *)&defctx)->offset =  0;
	((va_default_context *)&defctx)->seek = NULL;
	switch(netbuf->mode & 0x07) {
		case (NET_TYPE_SSL | NET_TYPE_TCP): memcpy(defctx.type, "SSL", 3);		//copy type
			break;
		default:
		case (NET_TYPE_SSL | NET_TYPE_UDP): memset(defctx.type, 0, 3);		//copy type
			break;
		case NET_TYPE_TCP: memcpy(defctx.type, "TCP", 3);		//copy type
			break;
		case NET_TYPE_UDP: memcpy(defctx.type, "UDP", 3);		//copy type
			break;
	}
	//start executing
	//this should create a new task and execute the newly created instance
	sprintf(name, "net%d", ((net_conn_p) netbuf->ctx)->port);
	instance = vm_new_instance((uint8 *)name);
	if(instance != NULL) {
		vm_init(instance, &func->base.handle, -1);		//set current PC offset to -1
		vm_push_argument(instance, sizeof(va_net_context), (uint8 *)&defctx);
		vm_push_argument(instance, netbuf->buflen, netbuf->buffer);
		start_execute_callback:
		vm_exec_function(instance, func);
		vm_exec_instance(instance);
	}
	//os_resume(os_find_task_by_name("app"));
}

void va_net_open(VM_DEF_ARG) {			//param1 = type, param2 = address, param3 = port, param4=callback, return context
	OS_DEBUG_ENTRY(va_net_open);
	net_context_p netctx;
	vm_object * vtype = vm_get_argument(VM_ARG, 0);
	vm_object * vaddr = vm_get_argument(VM_ARG, 1);
	vm_object * vport = vm_get_argument(VM_ARG, 2);
	vm_object * vfunc = vm_get_argument(VM_ARG, 3);
	uint8 url[256];
	vf_handle orcfile;
	uint8 tag;
	uint16 size;
	ssl_cert_p cert = NULL;
	ssl_keys_p keys = NULL;
	net_request req;
	uint16 mode = NET_TRANSMIT;
	net_buffer * ret = NULL;
	vm_function * func = NULL;
	va_net_context defctx;
	uint16 port = va_o2f(vport);
	if(g_pfrmctx == NULL) goto exit_net_open;
	netctx = g_pfrmctx->netctx;
	if(vm_get_argument_count(VM_ARG) < 3) goto exit_net_open;
	//process listen callback first
	if(vm_get_argument_count(VM_ARG) >= 4) {
		mode = NET_LISTEN;			//switch to listen mode
		func = (vm_function *)os_alloc(sizeof(vm_function));
		memcpy(func, vfunc->bytes, sizeof(vm_function));
		if(func->arg_count > 2) {	//check for function argument, must be one parameter
			vm_invoke_exception(VM_ARG, VX_ARGUMENT_MISMATCH);
			goto exit_net_open;
		}
	}
	memset(url, 0, sizeof(url));
	memcpy(url, vaddr->bytes, vaddr->len);
	if(vm_imemcmp(vtype->bytes, "TCP", 3) == 0) {
		ret = (net_buffer *)if_net_tcp_open(netctx, url, port, mode);
		if(func != NULL) {
			if_net_set_listen_callback((void *)ret, func, va_net_listen_callback);
		}
	} else if(vm_imemcmp(vtype->bytes, "UDP", 3) == 0) {
		ret = (net_buffer *)if_net_udp_open(netctx, url, port, mode);
		if(func != NULL) {
			if_net_set_listen_callback((void *)ret, func, va_net_listen_callback);
		}
	} 
	#if SHARD_SSL_SUPPORT
	else if(vm_imemcmp(vtype->bytes, "SSL", 3) == 0 || vm_imemcmp(vtype->bytes, "TLS", 3) == 0) {
		//should init SSL first here, load current devlet certificate
		if(tk_kernel_framework_load(g_pfrmctx, &orcfile) == 0) {
			vf_first_handle(&orcfile);
			while(vf_next_handle(&orcfile, &tag, &size) != 0) {
				if(tag == VA_ORC_CERT_TAG) {
					if(size > 2192) continue;			//max certificate size = 2K
					vf_read_handle(&orcfile, orcfile.file_offset, gba_net_buffer, size);
					//initialize SSL certificate
					if((cert = if_ssl_create_cert(gba_net_buffer, size)) == NULL) continue;
					break;
				}
				if(tag == VA_ORC_KEYS_TAG) {
					if(size > 2192) continue;			//max certificate size = 2K
					vf_read_handle(&orcfile, orcfile.file_offset, gba_net_buffer, size);
					//initialize SSL keys 
					if((keys = if_ssl_create_keys(gba_net_buffer, size)) == NULL) continue;
					break;
				}
			}
			if(cert == NULL) {
				//use orbleaf certificate
				cert = if_ssl_create_cert((uint8 *)server_cert_der_1024, sizeof(server_cert_der_1024));
			}
			if(keys == NULL) {
				//use orbleaf certificate
				keys = if_ssl_create_keys((uint8 *)server_keys_der_1024, sizeof(server_keys_der_1024));
			}
			if(cert != NULL && keys != NULL) {
				//try connecting to remote server
				ret = (net_buffer *)if_net_ssl_open(netctx, url, port, mode, cert, keys);
				if(func != NULL && ret != NULL) {
					if_ssl_set_listen_callback((void *)ret, func, va_net_listen_callback);
				}
			}
		}
	}
	#endif
	if(ret != NULL) {			//operation clear
		((va_default_context *)&defctx)->ctx = ret;
		((va_default_context *)&defctx)->close = va_net_close;
		((va_default_context *)&defctx)->read = va_net_recv;
		((va_default_context *)&defctx)->write = va_net_send;
		((va_default_context *)&defctx)->offset =  0;
		((va_default_context *)&defctx)->seek = NULL;
		vm_set_retval(vm_create_object(sizeof(va_net_context), &defctx));
		memcpy(defctx.type, vtype->bytes, 3);		//copy type
		va_net_add_conn(ret);
	}
	exit_net_open:
	OS_DEBUG_EXIT();
	return;
}

static uint16 va_load_framework_certificate(tk_context_p ctx, uint8 * certbuf) {
	OS_DEBUG_ENTRY(va_load_framework_certificate);
	vf_handle handle;
	uint8 tag;
	uint16 size;
	uint8 hdr_size;
	uint16 ret = 0;
	if(tk_kernel_framework_load(ctx, &handle) == 0) {	//return 0;
		vf_first_handle(&handle);
		while((hdr_size = vf_next_handle(&handle, &tag, &size)) != 0) {
			if(tag == 0x9C) {
				//certificate file
				vf_read_handle(&handle, 0, certbuf, size);		//read direct onto certificate buffer
			}
		}
	}
	OS_DEBUG_EXIT();
	return ret;
}

static uint8 va_array2string(uint8 * array, uint8 * bytes) {
	uint8 i = 0, k = 0;
	uint8 wtag, wlen;
	uint8 tag, len;
	wtag = array[i++];
	wlen = array[i++];
	bytes[0] = 0;		//end of string
	if(wtag != ASN_TAG_SET) return k;
	for(;i<(wlen + 2);) {
		tag = array[i++];
		len = array[i++];
		if(tag == ASN_TAG_OCTSTRING) {
			memcpy(bytes + k, array + i, len);
			k += len;
			bytes[k++] = '\r';
			bytes[k++] = '\n';
		}
		i+=len;
	}
	bytes[k] = 0;
	return k;
}

void va_net_transmit(VM_DEF_ARG) {		//param1 = url, param2 = parameters, param3 = method, param4 = type, param5= headers, param6=payload, return response
	OS_DEBUG_ENTRY(va_net_transmit);
	vm_object * vaddr =  vm_get_argument(VM_ARG, 0);
	vm_object * vparams =  vm_get_argument(VM_ARG, 1);
	vm_object * vmethod = vm_get_argument(VM_ARG, 2);
	vm_object * vtype = vm_get_argument(VM_ARG, 3);
	vm_object * vheaders = vm_get_argument(VM_ARG, 4);
	vm_object * vpayload = vm_get_argument(VM_ARG, 5);
	uint8 type = 0;
	ui_item_object * obj;
	uint8 tlen;
	uint8 ulen;
	ssl_cert_p cert = NULL;
	net_request req;
	uint8 * tbuf = (gba_net_buffer + 0x2000);		//payload buffer
	uint8 ubuf[210];				//url buffer
	uint8 * hbuf[512];				//header buffer
	uint8 * hptr = NULL;			//no additional headers
	uint16 port;
	uint8 mode;
	uint16 certsz = 0;
	ui_alert_p alert = NULL;
	uint8 tag;
	uint16 len = 0;
	net_protocol_p netp;
	tk_context_p wctx = g_pfrmctx;
	strcpy((char *)tbuf, (const char *)"Requesting...");
	if(g_pfrmctx == NULL) goto exit_net_transmit;
	//get URL
	ulen = vaddr->len;
	memcpy(ubuf, vaddr->bytes, ulen);
	ubuf[ulen] = 0;
	if(vm_get_argument_count(VM_ARG) < 1) goto exit_net_transmit;		//at least URL must be specified
	//transport layer type decode
	if(vm_get_argument_count(VM_ARG) > 3 && vtype->len != 0) {
		if(vm_memcmp(vtype->bytes, "TCP", 3) == 0) {
			type |= IF_TRANSPORT_TCP;
		} else if(vm_memcmp(vtype->bytes, "UDP", 3) == 0) {
			type |= IF_TRANSPORT_UDP;
		} else 
			goto exit_net_transmit;		//invalid type
	} else { type |= IF_TRANSPORT_TCP; }		//default transport mode TCP
	//method argument decode
	if(vm_get_argument_count(VM_ARG) > 2 && vmethod->len != 0) {
		if(vm_memcmp(vmethod->bytes, "GET", 3) == 0) {
			type |= IF_HTTP_TYPE_GET;
		} else if(vm_memcmp(vmethod->bytes, "POST", 4) == 0) {
			type |= IF_HTTP_TYPE_POST;
		}  else if(vm_memcmp(vmethod->bytes, "PUT", 3) == 0) {
			type |= IF_HTTP_TYPE_PUT;
		}  else if(vm_memcmp(vmethod->bytes, "DELETE", 6) == 0) {
			type |= IF_HTTP_TYPE_DELETE;
		} else 
			goto exit_net_transmit;		//invalid method
	} else { type |= IF_HTTP_TYPE_GET; }		//default method GET
	//get payload
	start_send_request:
	netp = net_get_protocol(ubuf);		//get underlying application protocol by URI, http, coap, etc...
	port = netp->port;						//default port
	if(netp == NULL) goto exit_net_transmit; 			//invalid protocol
	//mode[0:2]  	=> 8 type of request methods (HEADER,GET,POST,PUT,DELETE,....)
	//mode[3]  	=> transport protocol (1=UDP,0=TCP)
	if(netp->type & NETP_USE_SSL) {
		alert = ui_alert_show(wctx->display, (uint8 *)"Information", (uint8 *)tbuf, UI_ALERT_INFO, UI_ALERT_ICON_SECURE);
	} else {
		alert = ui_alert_show(wctx->display, (uint8 *)"Information", (uint8 *)tbuf, UI_ALERT_INFO, UI_ALERT_ICON_NETWORK);
	}
	if_net_wake(wctx->netctx);
	tbuf[0] = 0;
	//check for any parameters
	if(vparams->len != 0) {
		//decode http/coap parameters list Key-Value list
		len = tk_decode_params(vparams->bytes, tbuf);
	}
	//check for any additional payload
	if(vpayload->len != 0) {
		memcpy(tbuf + len, vpayload->bytes, vpayload->len);
		len += vpayload->len;
	}
	
#if SHARD_SSL_SUPPORT
	//if SSL used, initialize certificate and ssl context
	if(netp->type & NETP_USE_SSL) {
		//load certificate from card
		certsz = va_load_framework_certificate(wctx, gba_net_buffer);	//--> this function use apdu_buffer
		if(certsz != 0) {
			//if(if_net_ssl_init(ctx->netctx, gba_net_buffer, certsz) != 0) goto exit_send_request;
			if((cert = if_ssl_create_cert(gba_net_buffer, certsz)) == NULL) goto exit_send_request;
		}
	}	
#endif
	//check for any additional headers
	if(vheaders->len != 0) {
		if((vheaders->mgc_refcount & VM_OBJ_MAGIC) == VM_OBJ_MAGIC) {
			//array type
			va_array2string(vheaders->bytes, (uint8 *)hbuf);
		} else {
			//string type
			memcpy(hbuf, vheaders->bytes, vheaders->len);
			hbuf[vheaders->len] = 0;
		}
		hptr = (uint8 *)hbuf;
	}
	//start request
	len = netp->send(wctx->netctx, net_request_struct(&req, type, ubuf, port, cert), 
						hptr, tbuf, len, gba_net_buffer);
	
#if SHARD_SSL_SUPPORT
	if(netp->type & NETP_USE_SSL) {
		//release secure socket layer
		//if(certsz != 0) if_net_ssl_release(ctx->netctx);
		if(certsz != 0) if_ssl_release_cert(cert);
	}
#endif
	exit_send_request:
	if(alert != NULL) ui_alert_close(wctx->display, alert);
	//return gba_orc_size;
	vm_set_retval(vm_create_object(len, gba_net_buffer));		//set response as result
	if_net_sleep(wctx->netctx);
	exit_net_transmit:
	OS_DEBUG_EXIT();
	return;
}

void va_net_list(VM_DEF_ARG) {
	OS_DEBUG_ENTRY(va_net_transmit);
	vm_object * vtype =  vm_get_argument(VM_ARG, 0);
	netbios_ns * ns = g_netbios_svc;
	netbios_ns_entry  *iter;
	uint8 bbuf[VA_OBJECT_MAX_SIZE];
	uint16 len = 0;
	if(ns == NULL) return;
    //assert(ns != NULL);
    TAILQ_FOREACH(iter, &ns->entry_queue, next)
    {
		if (iter->flag & NS_ENTRY_FLAG_VALID_NAME) {
			len += tk_push(bbuf + len, ASN_TAG_OCTSTRING, strlen(iter->name), (uint8 *)iter->name);
		}
    }
	len = tk_push(bbuf, ASN_TAG_SET, len, (uint8 *)bbuf);
	vm_set_retval(vm_create_object(len, bbuf)); 
	if(vm_get_retval() != VM_NULL_OBJECT) vm_get_retval()->mgc_refcount |= VM_OBJ_MAGIC;				//set to object
	exit_net_list:
	OS_DEBUG_EXIT();
	return;
}

//////////////////////////////////////NETWORK APIS//////////////////////////////////////////


void va_net_mail_create(VM_DEF_ARG) {		//param1 = server, param2 = port, param3 = username, param4 = password, return smtp_handle
	OS_DEBUG_ENTRY(va_net_mail_create);
	vm_object * vserver = vm_get_argument(VM_ARG, 0);
	vm_object * vusrname =  vm_get_argument(VM_ARG, 1);
	vm_object * vpasswd =  vm_get_argument(VM_ARG, 2);
	uint16 port = 25;
	uint8 server[64], username[64], password[64];
	smtp_context smtp;
	tk_context_p wctx = g_pfrmctx;
	if(ctx == NULL) goto exit_mail_create;
	if(vm_get_argument_count(VM_ARG) < 3) goto exit_mail_create;
	if(vm_get_argument_count(VM_ARG) > 3) port = va_o2f(vm_get_argument(VM_ARG, 3));
	memset(server, 0, sizeof(server));
	memset(username, 0, sizeof(username));
	memset(password, 0, sizeof(password));
	//server name
	if(vserver->len != 0) strncpy((char *)server, (char *)vserver->bytes, sizeof(server));
	if(vserver->len < sizeof(server)) server[vserver->len] = 0;
	//username
	strncpy((char *)username, (char *)vusrname->bytes, sizeof(username));
	if(vusrname->len < sizeof(username)) username[vusrname->len] = 0;
	//password
	strncpy((char *)password, (char *)vpasswd->bytes, sizeof(password));
	if(vpasswd->len < sizeof(password)) password[vpasswd->len] = 0;
	if(smtp_init(wctx->netctx, server, port, username, password, &smtp) == 0) {
		vm_set_retval(vm_create_object(sizeof(smtp_context), &smtp));
	}
	exit_mail_create:
	OS_DEBUG_EXIT();
	return;
}

void va_net_mail_send(VM_DEF_ARG) {
	OS_DEBUG_ENTRY(va_net_mail_send);
	uint16 ret;
	vm_object * vsmtp = vm_get_argument(VM_ARG, 0);		//smtp_handle
	vm_object * vto = vm_get_argument(VM_ARG, 1);		//to
	vm_object * vsubj =  vm_get_argument(VM_ARG, 2);	//subject
	vm_object * vmsg =  vm_get_argument(VM_ARG, 3);	//msg
	vm_object * vfrom =  vm_get_argument(VM_ARG, 4);	//from
	uint8 from[64], to[64], subject[128];
	uint8 msg[VA_OBJECT_MAX_SIZE];
	tk_context_p wctx = g_pfrmctx;
	if(wctx == NULL) goto exit_mail_send;
	if(vm_get_argument_count(VM_ARG) < 5) goto exit_mail_send;
	memset(from, 0, sizeof(from));
	memset(to, 0, sizeof(to));
	memset(subject, 0, sizeof(subject));
	memset(msg, 0, sizeof(msg));
	strncpy((char *)to, (char *)vto->bytes, sizeof(to));
	if(vto->len < sizeof(to)) to[vto->len] = 0;
	strncpy((char *)from, (char *)vfrom->bytes, sizeof(from));
	if(vfrom->len < sizeof(from)) from[vfrom->len] = 0;
	strncpy((char *)subject, (char *)vsubj->bytes, sizeof(subject));
	if(vsubj->len < sizeof(subject)) subject[vsubj->len] = 0;
	//if(smtp_init(ctx->netctx, server, port, username, password, &smtp) == 0) {
	memcpy((char *)msg, (char *)vmsg->bytes, (vmsg->len > (sizeof(msg) -1))?sizeof(msg) - 1:vmsg->len );
	ret = smtp_send((smtp_context_p)vsmtp->bytes, from, subject, to, msg);
	va_return_word(VM_ARG, ret);
	exit_mail_send:
	OS_DEBUG_EXIT();
	return;
}
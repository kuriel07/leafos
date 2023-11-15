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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//extern vm_object * g_pVaRetval;
//extern sys_context g_sVaSysc;
extern tk_context_p g_pfrmctx;

#if 0
///////////////////////////////////////////////ISO - 8583//////////////////////////////////////////////////

#define DEM_TYPE_FIXED		0x00
#define DEM_TYPE_VAR		0x08
#define DEM_TYPE_LLEN(x)	(x & 0x07)
#define DEM_TYPE_BIN		0x10
#define DEM_TYPE_NUM		0x20
#define DEM_TYPE_ALPHA		0x40

typedef struct data_element_map {
	uint8 id;
	uint8 type;
	uint8 length;
} data_element_map;

CONST data_element_map g_vtable[] = {
	{ 1, DEM_TYPE_BIN | DEM_TYPE_FIXED, 8 },
	{ 2, DEM_TYPE_NUM | DEM_TYPE_LLEN(2) | DEM_TYPE_VAR, 0 },
	{ 3, DEM_TYPE_ALPHA | DEM_TYPE_NUM | DEM_TYPE_FIXED, 6 },
	{ 4, DEM_TYPE_NUM | DEM_TYPE_FIXED, 16 },
	{ 5, DEM_TYPE_NUM | DEM_TYPE_FIXED, 16 },
	{ 6, DEM_TYPE_NUM | DEM_TYPE_FIXED, 16 },
	{ 7, DEM_TYPE_NUM | DEM_TYPE_FIXED, 10 },
	{ 8, DEM_TYPE_NUM | DEM_TYPE_FIXED, 12 },
	{ 9, DEM_TYPE_NUM | DEM_TYPE_FIXED, 8 },
	{ 10, DEM_TYPE_NUM | DEM_TYPE_FIXED, 8 },
	{ 11, DEM_TYPE_NUM | DEM_TYPE_FIXED, 12 },
	{ 12, DEM_TYPE_NUM | DEM_TYPE_FIXED, 14 },
	{ 13, DEM_TYPE_NUM | DEM_TYPE_FIXED, 6 },
	{ 14, DEM_TYPE_NUM | DEM_TYPE_FIXED, 4 },
	{ 15, DEM_TYPE_NUM | DEM_TYPE_FIXED, 8 },
	{ 16, DEM_TYPE_NUM | DEM_TYPE_FIXED, 4 },
	{ 17, DEM_TYPE_NUM | DEM_TYPE_FIXED, 4 },
	{ 18, DEM_TYPE_ALPHA | DEM_TYPE_NUM | DEM_TYPE_BIN | DEM_TYPE_LLEN(3) | DEM_TYPE_VAR, 0 },
	{ 19, DEM_TYPE_NUM | DEM_TYPE_FIXED, 3 },
	{ 20, DEM_TYPE_NUM | DEM_TYPE_FIXED, 3 },
	{ 21, DEM_TYPE_ALPHA | DEM_TYPE_NUM | DEM_TYPE_FIXED, 22 },
	{ 22, DEM_TYPE_BIN | DEM_TYPE_FIXED, 16 },
	{ 23, DEM_TYPE_NUM | DEM_TYPE_FIXED, 3 },
	{ 24, DEM_TYPE_NUM | DEM_TYPE_FIXED, 3 },
	{ 25, DEM_TYPE_NUM | DEM_TYPE_FIXED, 4 },
	{ 26, DEM_TYPE_NUM | DEM_TYPE_FIXED, 4 },
	{ 27, DEM_TYPE_ALPHA | DEM_TYPE_NUM | DEM_TYPE_BIN  | DEM_TYPE_FIXED, 4 },
	{ 28, DEM_TYPE_NUM | DEM_TYPE_FIXED, 8 },
	{ 29, DEM_TYPE_NUM | DEM_TYPE_FIXED, 3 },
	{ 30, DEM_TYPE_NUM | DEM_TYPE_FIXED, 32 },
	{ 31, DEM_TYPE_NUM | DEM_TYPE_FIXED, 23 },
	{ 32, DEM_TYPE_NUM | DEM_TYPE_LLEN(2) | DEM_TYPE_VAR, 11 },
	{ 33, DEM_TYPE_NUM | DEM_TYPE_LLEN(2) | DEM_TYPE_VAR, 11 },
	{ 34, DEM_TYPE_BIN | DEM_TYPE_LLEN(4) | DEM_TYPE_VAR, 0 },
	{ 35, DEM_TYPE_BIN | DEM_TYPE_LLEN(2) | DEM_TYPE_VAR, 0 },
	{ 36, DEM_TYPE_BIN | DEM_TYPE_LLEN(3) | DEM_TYPE_VAR, 0 },
	{ 37, DEM_TYPE_ALPHA | DEM_TYPE_NUM | DEM_TYPE_FIXED, 12 },
	{ 38, DEM_TYPE_ALPHA | DEM_TYPE_NUM | DEM_TYPE_FIXED, 6 },
	{ 39, DEM_TYPE_NUM | DEM_TYPE_FIXED, 4 },
	{ 40, DEM_TYPE_NUM | DEM_TYPE_FIXED, 3 },
	{ 0, 0, 0}		//end mark
}; 
CONST uint8 g_b2h[] = { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 'A', 'B', 'C', 'D', 'E', 'F' };	
CONST uint8 g_bmask[] = { 0x08, 0x04, 0x02, 0x01 };	
CONST uint16 g_wMod[] = { 1, 10, 100, 1000, 10000 };
	 
void va_iso_create_message(VM_DEF_ARG) _REENTRANT_ {
	OS_DEBUG_ENTRY(va_iso_create_message);
	vm_object * mti;
	//uint8 dbuf[250];
	uint8 i,j,dlen;
	mti = vm_get_argument(VM_ARG, 0);
	if(mti != VM_NULL_OBJECT) {
		mmMemSet(g_baOrbBuffer, '0', 20);
		for(i=4,j=1;i>0 && j<=mti->len;i--,j++) {
			g_baOrbBuffer[i-1] = mti->bytes[mti->len - j];
		}
		vm_set_retval(vm_create_object(20, g_baOrbBuffer));
	}
	OS_DEBUG_EXIT();
}

static data_element_map * iso_get_map(uint8 tag) {
	data_element_map * iterator = g_vtable;
	while(iterator->id != 0) {
		if(iterator->id == tag) return iterator;
		iterator++;
	}
	return NULL;
}

static uint8 iso_h2bin(uint8 hex) {
	if(hex>=0x30 && hex <=0x39) return hex - 0x30;
	if(hex>='A' && hex <='F') return (hex - 'A') + 10;
	if(hex>='a' && hex <='f') return (hex - 'a') + 10;
	return 0;
}

static vm_object * iso_push(vm_object * a, uint8 tag, uint8 * b, uint8 len) {
	vm_object * obj = VM_NULL_OBJECT;
	uint8 bmap;
	if(tag == 0) return obj;
	tag -=1;
	//obj = (vm_object *)malloc(sizeof(vm_object) + a->len + len);
	obj = vm_create_object(a->len + len, NULL);
	obj->len = a->len + len;
	vm_memcpy(obj->bytes, a->bytes, a->len);
	bmap = iso_h2bin(obj->bytes[4 + (tag>>2)]);
	obj->bytes[4 + (tag>>2)] = g_b2h[(bmap | g_bmask[tag % 4])];
	vm_memcpy(obj->bytes + a->len, b, len);
	return obj;
}		  

void va_iso_push_element(VM_DEF_ARG) _REENTRANT_ {
	OS_DEBUG_ENTRY(va_iso_push_element);
	//uint8 dbuf[250];
	uint8 i,j,dlen;
	uint16 wlen;
	vm_object * hdr = vm_get_argument(VM_ARG, 0);
	uint8 tag = va_o2f(vm_get_argument(VM_ARG, 1));
	vm_object * dem = vm_get_argument(VM_ARG, 2);  
	data_element_map * map = iso_get_map(tag);
	if(vm_get_argument_count(VM_ARG) < 3) goto exit_push_element;
	if(map == NULL) {
		g_pVaRetval = iso_push(hdr, tag, dem->bytes, dem->len);
	} else {
		if(map->type & DEM_TYPE_VAR) {
			dlen = (map->type & 0x07);
			wlen = dem->len;
			for(i=dlen,j=0;i>0;i--,j++) {
				g_baOrbBuffer[j] = g_b2h[wlen /g_wMod[i-1]];
				wlen = dem->len % g_wMod[i-1];
			}
			vm_memcpy(g_baOrbBuffer + dlen, dem->bytes, dem->len);
			vm_set_retval(iso_push(hdr, tag, g_baOrbBuffer, dlen + dem->len));
		} else {
			mmMemSet(g_baOrbBuffer, '0', map->length);
			for(i=map->length,j=1;i>0 && j<=dem->len;i--,j++) {
				g_baOrbBuffer[i-1] = dem->bytes[dem->len - j];
			}
			vm_set_retval(iso_push(hdr, tag, g_baOrbBuffer, map->length));
		}
	}
	if(vm_get_retval() != VM_NULL_OBJECT) {
		vm_get_retval()->mgc_refcount = (hdr->mgc_refcount + 1);		//copy header bytes
		vm_update_mutator(VM_ARG, hdr, g_pVaRetval);						//update mutator
		hdr->mgc_refcount &= 0xF0;									//clear refcount
		vm_release_object(hdr);										//release header
	}
	exit_push_element:
	OS_DEBUG_EXIT();
	return;
}

void va_iso_get_element(VM_DEF_ARG) _REENTRANT_ {
	OS_DEBUG_ENTRY(va_iso_get_element);
	vm_object * obj = VM_NULL_OBJECT;
	uint8 bmap;
	uint8 i,k,l,n;
	uint16 j = 20;
	uint8 dlen;
	vm_object * msg = vm_get_argument(VM_ARG, 0);
	uint8 tag = va_o2f(vm_get_argument(VM_ARG, 1));
	if(vm_get_argument_count(VM_ARG) < 2) goto exit_get_element;
	if(msg == VM_NULL_OBJECT) goto exit_get_element ;
	tag -= 1;
	bmap = iso_h2bin(msg->bytes[4 + (tag>>2)]);
	if((bmap & g_bmask[tag % 4]) == 0) goto exit_get_element;
	for(i=0;i<40;i++) {
		bmap = iso_h2bin(msg->bytes[4 + (i>>2)]);
		if((bmap & g_bmask[i % 4])) {
			if(g_vtable[i].type & DEM_TYPE_VAR) {
				dlen =0;
				l = g_vtable[i].type & 0x03;
				for(k=0,n=(l-1);k<l;k++,n--) {
					dlen += iso_h2bin(msg->bytes[j+k]) * g_wMod[n]; 
				}
				j += l;
				if(i == tag) {
return_element:
					vm_set_retval(vm_create_object(dlen, msg->bytes + j));
					goto exit_get_element;
				}
				j += dlen;
			} else {
				dlen = g_vtable[i].length;
				if(i == tag) goto return_element;
				j += dlen;
			}
		}
	}
	exit_get_element:
	OS_DEBUG_EXIT();
	return;
}

////////////////////////////////////////////END OF ISO - 8583//////////////////////////////////////////////
#endif
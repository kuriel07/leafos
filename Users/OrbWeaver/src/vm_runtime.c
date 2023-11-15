#include "..\..\defs.h"
#include "..\..\config.h"
#include "..\inc\MMApis.h"	
#include "..\inc\VMStackApis.h"
#include "..\..\interfaces\inc\if_apis.h"
#include <string.h>

uint16 vf_read_handle(vf_handle_p handle, uint16 offset, uint8 * buffer, uint16 size) {
	offset = ((BSAllocHandle *)handle)->start + offset;			
	if(offset > ((BSAllocHandle *)handle)->end) return 0;
	if((offset + size) > ((BSAllocHandle *)handle)->end) size = ((BSAllocHandle *)handle)->end - offset;
	return bsHandleReadW(handle, offset, buffer, size);
}

uint16 vf_write_handle(vf_handle_p handle, uint16 offset, uint8 * buffer, uint16 size) {
	offset = ((BSAllocHandle *)handle)->start + offset;			//physical offset
	if(offset > ((BSAllocHandle *)handle)->end) return 0;
	if((offset + size) > ((BSAllocHandle *)handle)->end) size = ((BSAllocHandle *)handle)->end - offset;
	return bsHandleWriteW(handle, offset, buffer, size);
}

uint8 vf_pop_handle(vf_handle_p handle, uint16 offset, uint8 * tag, uint16 * size) _REENTRANT_ {
	WORD i = 0; 
	WORD j;
	WORD lsize = 0;
	uint8 buffer[6];
	//vf_read_handle(handle, handle->offset, buffer, sizeof(buffer));
	if((offset + sizeof(buffer)) > handle->file_size) return 0;
	//memcpy(buffer, handle->base_address + handle->file_offset + offset, sizeof(buffer));
	vf_read_handle(handle, handle->file_offset + offset, buffer, sizeof(buffer));
	tag[0] = (uint8)buffer[i++];
	if(buffer[i] == 0x81) {
		i++;
	} else if(buffer[i] == 0x82) {
		i++;
		lsize = (WORD)buffer[i++];
		lsize <<= 8;
	} 
	lsize += (WORD)buffer[i++];
	size[0] = lsize;
	if((handle->file_offset + offset + i + lsize) > handle->size) return 0;
	return i;
}


void vf_first_handle(vf_handle_p handle) {
	handle->next_offset = 0;		//reset offset
}

uint8 vf_next_handle(vf_handle_p handle, uint8 * tag, uint16 * size) _REENTRANT_ {
	WORD i = 0; 
	WORD j;
	WORD lsize = 0;
	uint8 buffer[6];
	//vf_read_handle(handle, handle->offset, buffer, sizeof(buffer));
	//if((handle->next_offset + sizeof(buffer)) > handle->size) return 0;
	//memcpy(buffer, handle->base_address + handle->next_offset, sizeof(buffer));
	if(vf_read_handle(handle, handle->next_offset, buffer, sizeof(buffer)) == 0) return 0;
	
	tag[0] = (uint8)buffer[i++];
	if(buffer[i] == 0x81) {
		i++;
	} else if(buffer[i] == 0x82) {
		i++;
		lsize = (WORD)buffer[i++];
		lsize <<= 8;
	} 
	lsize += (WORD)buffer[i++];
	size[0] = lsize;
	if((handle->next_offset + i + lsize) > handle->size) return 0;
	handle->file_offset = handle->next_offset + i;
	handle->file_size = lsize;
	handle->next_offset += (i + lsize);
	return i;
}

void vr_init_global(tk_context_p ctx) {
	mmInit();					//initialize object heap
	va_init_context(ctx);	//init framework context (dev entity)
}

uint8 vr_init_handle(vf_handle_p handle, uint8 api, uint32 address, uint16 size) {
	uint8 tag;
	if(size == (uint16)-1) return -1;	//invalid size
	if(size == 0) return -1;				//invalid size
	//intialize bootstrap allocator
	((BSAbstractionHandle *)&handle->base)->handle = NULL;
	((BSAbstractionHandle *)&handle->base)->api_id = 0; 
	((BSAllocHandle *)&handle->base)->start = address;
	((BSAllocHandle *)&handle->base)->end = address + size;
	handle->size = size;
	handle->next_offset = 0;
	handle->file_offset = 0;
	handle->file_size = 0;
	//try reading magic tag
	vf_read_handle(handle, 0, &tag, 1);
	if(tag != 0x6F) return -1;
	return 0;
}

uint16 vr_load_script(uint8 mode, vf_handle_p handle, uint8 len, uint8 * buffer) {
	OS_DEBUG_ENTRY(vr_load_script);
	vf_handle local_handle;
	vf_handle temp;		   
	uint8 tag, hlen;	
	uint16 tsize, size;
	uint16 codestart, i, j, k; 
	uint16 l = -1 ;		//length of pushed buffer
	uint8 iterator = 0;
	int8 gid = -1;
	uint8 state = 0;
	uint8 lbuf[256];
	uint8 hdr_size;
	
	//FSFileParameters params;
	if(mode & VM_SCRIPT_LIST) l=0;		//reset index if list mode
	if(mode & VM_SCRIPT_BY_INDEX) gid = buffer[0];
	if((mode & 0x0F) == VM_SCRIPT_BY_METHOD) {
		hlen = vf_pop_handle(handle, 0, &tag, &tsize);
		if(tag != (ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED)) {
			OS_DEBUG_EXIT();
			return l;	//{		//check for constructed sequence
		}
		codestart = tsize + hlen;										//total sequence length
		i = hlen;
		for(;i<codestart;i+=tsize) {
			hlen = vf_pop_handle(handle, i, &tag, &tsize);				  
			i += hlen;
			if(tag != (ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED)) continue;	//{   		//check for matching class entry tag
			//check for class entry	 
			k = tsize;
			j = 0;
			//iterate method property
			for(; j < k; j+= size, iterator++) {
				hlen = vf_pop_handle(handle, i+j, &tag, &size);
				j += hlen;
				if(tag != ASN_TAG_OCTSTRING) continue;
				//size = (size > VR_MAX_ENTRY_LENGTH)?VR_MAX_ENTRY_LENGTH:size;
				vf_read_handle(handle, handle->file_offset + i +j, lbuf, size);

				if(mode & 0x80) { 		//list mode
					if((l + size + 3) >= len) {
						OS_DEBUG_EXIT();
						return l;					//not enough buffer
					}
					if(gid != -1) if( gid != iterator) continue;
					l += tk_push(buffer + l, VM_SCRIPT_BY_METHOD, size-3, lbuf+3); 		//use dbuffer+3 for method (offset[2], arg[1], name[n] ) (fixed : 2015.05.16)
					//break; 
				} else {
					if(gid != (int8)-1) { 
						if( gid == iterator) goto vr_method_return;
					} else if(memcmp(buffer, lbuf+3, len) == 0) {
						vr_method_return:
						codestart = end_swap16(*((WORD *)lbuf)) + codestart;
						OS_DEBUG_EXIT();
						return codestart;
					}
				}
				//iterator ++;					   //increment iterator (added 2015.05.16)
			}
		}	
	} else {
		memcpy(&temp, handle, sizeof(vf_handle));
		vf_first_handle(handle);
		while((hdr_size = vf_next_handle(handle, &tag, &size)) != 0) {
			if(tag != 0x90) continue;
			//decode ASN.1 structure for package and look for entry point for matching class id
			hlen = vf_pop_handle(handle, 0, &tag, &tsize);
			if(tag != (ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED)) continue;	//{		//check for constructed sequence
			codestart = tsize + hlen;										//total sequence length
			i = hlen;
			for(;i<codestart;i+=tsize) {
				hlen = vf_pop_handle(handle, i, &tag, &tsize);				  
				i += hlen;
				switch(mode & 0x0F) {
					case VM_SCRIPT_BY_CLASS:
						if(tag != (ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED)) continue;	//{   		//check for matching class entry tag
						//check for class entry	 
						k = tsize;
						j = 0;
						//iterate class property
						for(; j < k; j+= size) {
							hlen = vf_pop_handle(handle, i+j, &tag, &size);
							j += hlen;
							if(tag != ASN_TAG_IA5STRING) continue;
							//size = (size > VR_MAX_ENTRY_LENGTH)?VR_MAX_ENTRY_LENGTH:size;
							vf_read_handle(handle,  handle->file_offset + i+j, lbuf, size);
							vm_memcpy(lbuf + 2, lbuf, size);
							codestart = 0;				//an orb file could only contain one class, if this compare failed, skip this file
							vm_memset(lbuf, 0, 2);
							goto vr_load_compare;
						}
						break;
					case VM_SCRIPT_BY_ALIAS:
						if(tag != ASN_TAG_OCTSTRING) continue; //{					  //check for menu entry
						//size = (tsize > sizeof(lbuf))?sizeof(dbuffer):tsize;
						vf_read_handle(handle,  handle->file_offset + i, lbuf, tsize);
						size = tsize - 2;
						goto vr_load_compare;
						break; 
					case VM_SCRIPT_BY_EVENT:
						if(tag != ASN_TAG_INTEGER) continue;	//{					  //check for event entry
						vf_read_handle(handle,  handle->file_offset + i, lbuf, tsize);
						size = tsize - 2;
						vr_load_compare:
						if(mode & 0x80) { 		//list mode		  
							if((l + size + 3) >= len) {
								OS_DEBUG_EXIT();
								return l;			//not enough buffer	 
							}
							if(gid != -1) if( gid != iterator) break;
							l += tk_push(buffer + l, ((mode & 0x3F) | state), size, lbuf+2); 
							state++;
						} else {	
							if(gid != (int8)-1) { 
								if( gid == iterator) goto vr_event_return;
								iterator ++;						  //increment iterator (added 2015.05.16)
							} else if(vm_memcmp(buffer, lbuf + 2, size) == 0) {
								vr_event_return:
								codestart = end_swap16(*((WORD *)lbuf)) + codestart;
								OS_DEBUG_EXIT();
								return codestart;
							}
						}
						break;
					default: break;
				}
			}
		}
		restore_handle:
		vm_memcpy(handle, &temp, sizeof(vf_handle));		//restore handle
	}
	OS_DEBUG_EXIT();
	return l; 			//return list length (mode list)
}

uint8 vr_load_text(WORD fid, BYTE tag, BYTE * buffer, BYTE max_len) {
	//use default text
	return -1;
}

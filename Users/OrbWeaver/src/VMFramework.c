#include "..\..\..\defs.h"
#include "..\..\..\config.h"
#ifndef _RTAPIS__H
#include "..\..\..\RT\include\RTApis.h"
#endif
#ifndef _TKAPIS__H	  					   
#include "..\..\..\TK\include\TKApis.h"
#endif	   
#ifndef _CRAPIS__H
#include "..\..\..\CR\include\CRApis.h"
#endif
#ifndef _CMAPIS__H
#include "..\..\..\CM\include\CMApis.h"
#endif
#ifndef _FSAPIS__H  
#include "..\..\..\FS\include\FSApis.h"
#endif	   
#ifndef _MMAPIS__H
#include "..\..\..\MM\include\MMApis.h"
#endif
#ifndef _BSAPIS__H  
#include "..\..\..\BS\include\BSApis.h"
#endif	
#ifndef _VM_STACK__H
#include "..\..\include\VM\VMStackApis.h"
#endif
#if TK_WIB_ENABLED
#ifndef _WBAPIS__H 	   
#include "..\..\..\TK\include\WIB\WBApis.h"
#endif
#endif
#if COS_DEFAULT_SERVICE == COS_SERVICE_ORB
#include "..\..\..\GP\include\GPApis.h"
#endif

#if TK_STACK_ENABLED
extern BYTE g_baIOBuffer[];
#define g_baOrbBuffer  (g_baIOBuffer + 5)
extern vm_object * g_pVaRetval;
extern sys_context g_sVaSysc;

//global
void va_set_execution_context() _REENTRANT_ ;
//var
void va_set_var() _REENTRANT_ ;
void va_get_var() _REENTRANT_ ;
void va_delete_var() _REENTRANT_ ; 
//string
void va_index_of() _REENTRANT_ ;
void va_last_index_of() _REENTRANT_ ;
void va_replace() _REENTRANT_ ;
void va_substr() _REENTRANT_ ;
//bytes creation
void va_bytes() _REENTRANT_ ;
//file
void va_fopen() _REENTRANT_ ;  
void va_fread() _REENTRANT_ ;
void va_fwrite() _REENTRANT_ ;
void va_fclose() _REENTRANT_ ;
void va_fpopbytag() _REENTRANT_ ;  
//json				
void va_arg_findtag() _REENTRANT_ ;				
void va_arg_create() _REENTRANT_ ;
void va_arg_object() _REENTRANT_ ;
void va_arg_array() _REENTRANT_ ; 	 
void va_arg_at() _REENTRANT_ ;
void va_arg_get() _REENTRANT_ ;
void va_arg_serialize() _REENTRANT_ ;			// -> to json string
void va_arg_deserialize() _REENTRANT_ ;			// -> from json string

//toolkit (21-36)
void va_select_item() _REENTRANT_;
void va_set_timer() _REENTRANT_ ; 
//invoke external
void va_invoke_external() _REENTRANT_ ;
//iso8583 (37-39)
void va_iso_create_message() _REENTRANT_ ; 
void va_iso_push_element() _REENTRANT_ ;
void va_iso_get_element() _REENTRANT_ ;

//generic toolkit
void va_toolkit_create() _REENTRANT_ ;  
void va_toolkit_push_ext() _REENTRANT_ ;
void va_toolkit_push_raw() _REENTRANT_ ;
void va_toolkit_dispatch() _REENTRANT_ ;
void va_toolkit_get_result() _REENTRANT_ ;
//bit operation 
void va_check_bit() _REENTRANT_ ; 
void va_set_bit() _REENTRANT_ ;
void va_clear_bit() _REENTRANT_ ;
//converter
void va_bin2hex() _REENTRANT_;
void va_hex2bin() _REENTRANT_; 
void va_bin2dec() _REENTRANT_;
void va_dec2bin() _REENTRANT_;
void va_b64_encode() _REENTRANT_ ;
void va_b64_decode() _REENTRANT_ ;
//codec
void va_crypto_create() _REENTRANT_ ;
void va_crypto_encrypt() _REENTRANT_ ; 
void va_crypto_decrypt() _REENTRANT_ ;
void va_random() _REENTRANT_ ;
//security
void va_verify_pin() _REENTRANT_ ;
//toolkit manager			  
void va_terminal_profile() _REENTRANT_ ; 
//cross APIs
void va_wib_set_return_var() _REENTRANT_ ; 
void va_get_info() _REENTRANT_ ;
//default syscall return
vm_object * va_syscall_ret(BYTE size, BYTE * buffer) _REENTRANT_ ;

CONST vm_api_entry g_vaRegisteredApis[] = {
#if STACK_VAR_APIS
 	//var APIs
	{1, va_set_var, NULL},
 	{2, va_get_var, NULL},
 	{3, va_delete_var, NULL},
#endif
#if STACK_STRING_APIS
	//string APIs
	{4, va_index_of, NULL},
	{5, va_replace, NULL},	   
	{6, va_substr, NULL},
#endif
	{7, va_bytes, NULL},
#if STACK_FILE_APIS
	//file APIs
	{8, va_fopen, NULL},  
	{9, va_fclose, NULL},
	{10, va_fread, NULL},
	{11, va_fwrite, NULL}, 
	{12, va_fpopbytag, NULL},
#endif 
	{13, va_arg_findtag, NULL},
#if STACK_JSON_APIS	
	{14, va_arg_create, NULL},
	{15, va_arg_object, NULL},
	{16, va_arg_array, NULL}, 	 
	{17, va_arg_at, NULL},
	{18, va_arg_get, NULL},
	{19, va_arg_serialize, NULL},			// -> to json string
	{20, va_arg_deserialize, NULL},			// -> from json string
#endif
	//21 - 31 			//should be reserved for toolkit apis
	//// - 21			//toolkit apis 
#if STACK_TOOLKIT_SELECT_APIS
	{24, va_select_item, va_syscall_ret},
#endif	
#if STACK_TOOLKIT_TIMER_APIS
	{27, va_set_timer, va_syscall_ret},
#endif
	//32 - 63			//extended orb apis
	{32, va_invoke_external, NULL },	
	//// - 36			//end of toolkit apis
#if STACK_ISO8583_APIS
	{37, va_iso_create_message, NULL },
	{38, va_iso_push_element, NULL },
	{39, va_iso_get_element, NULL },
#endif

#if STACK_BIT_APIS
	//bit APIs
	{128, va_check_bit, NULL},
	{129, va_set_bit, NULL},	 
	{130, va_clear_bit, NULL},
#endif
#if STACK_CONVERTER_APIS
	//converter APIs
	{131, va_bin2hex, NULL},
	{132, va_hex2bin, NULL}, 
	{133, va_bin2dec, NULL},
	{134, va_b64_encode, NULL},
	{135, va_b64_decode, NULL},
#endif
#if STACK_CRYPTO_APIS
	//cryptography APIs
	{136, va_crypto_create, NULL},
	{137, va_crypto_encrypt, NULL },	 
	{138, va_crypto_decrypt, NULL },
	{139, va_random, NULL },
#endif							
	{160, va_toolkit_create, NULL},
	{161, va_toolkit_dispatch, va_syscall_ret},
	{162, va_toolkit_push_ext, NULL},
	{163, va_toolkit_push_raw, NULL},
	{164, va_toolkit_get_result, NULL},
	//terminal profile
	{240, va_terminal_profile, NULL},
#if STACK_WIB_APIS
#if TK_WIB_ENABLED
	{241, va_wib_set_return_var, NULL},
#endif					
	{243, va_verify_pin, NULL},
	{248, va_set_execution_context, NULL},
#endif
#if COS_DEFAULT_SERVICE == COS_SERVICE_ORB
	{252, va_get_info, NULL},
#endif
 	{0, NULL, NULL}
} ;

extern FSHandle _vm_file;
BYTE g_vaWaitTag = 0; 

static WORD va_o2f(vm_object * obj) _REENTRANT_ {
	BYTE buffer[7];
	BYTE offset = 0;
	BYTE len = obj->len;
	if(len == 0) return 0;		//NULL object
	if(obj->len > 6) len = 6;
	offset = obj->len - len;
	mmMemCpy(buffer, obj->bytes + offset, len);
	buffer[len] = 0;
	return (WORD)mmAtoi(buffer); 
}	  

static void va_return_word(WORD val) _REENTRANT_ {
	mmItoa(MM_ITOA_WORD, g_baOrbBuffer, (WORD)val);
	g_pVaRetval = vm_create_object(vm_strlen(g_baOrbBuffer), g_baOrbBuffer);
}

#define VA_DISP_TEXT 	STK_CMD_DISPLAY_TEXT 
#define VA_DISP_INKEY	STK_CMD_GET_INKEY
#define VA_DISP_IDLE	STK_CMD_SET_UP_IDLE_TEXT

#define VA_VAR_SET		1
#define VA_VAR_GET		2
#define VA_VAR_DELETE	3 

void va_verify_pin() _REENTRANT_ {	
	vm_object * param;
	WORD status;
	//BYTE pin_buffer[8];
	BYTE len = 0;
	BYTE keyref = va_o2f(vm_get_argument(0));
	param = vm_get_argument(1);	 
	mmMemSet(g_baOrbBuffer, 0xFF, 0x08);
	if(param->len != 0) {
		len = param->len;
		mmMemCpy(g_baOrbBuffer, param->bytes, len);
		len = 8;
	}
	if(((status = scAuthVerifyPasswordB(cmGetSelectedContext(), keyref, g_baOrbBuffer, len)) & 0xF0) != SC_S_OK) {
		switch(status & 0xF0) {
			default:
			case SC_ERR_BLOCKED: status = -1; break;
			case SC_ERR_STATE_INVALID: status = -2;	break;
			case SC_ERR_NOT_OK: if((status & 0x0F) == 0) status = -1; break;
		}
	}
	va_return_word((WORD)status);
	//mmSprintf(g_baOrbBuffer, "%d", (WORD)status);
	//g_pVaRetval = vm_create_object(strlen(g_baOrbBuffer), g_baOrbBuffer);
}
 
#if STACK_VAR_APIS
static void va_var_operation(BYTE mode) _REENTRANT_ {
   	vm_object * param;
	FSHandle fhandle;	
	BYTE i = 0, j;
	BYTE exist = 0;
#if 0	
	param = vm_get_argument(0);
	if(param == VM_NULL_OBJECT) return;// g_pVaRetval;
	mmMemCpy(&fhandle, &_vm_file, sizeof(FSHandle));
	if(fsSelectFileByFidW(&fhandle, FID_STACK_LOCAL, NULL) != FS_OK) return;			// g_pVaRetval;
	if(fsSelectFileByFidW(&fhandle, FID_STACK_DATA, &g_strFileParams) != FS_OK) return;	// g_pVaRetval; 
	i = 0;
	while((j = fsFileReadRecordW(&fhandle, i, g_baOrbBuffer, g_strFileParams.reclen)) == g_strFileParams.reclen) {
		if(param->len == g_baOrbBuffer[0]) { 
			if(mmMemCmp(g_baOrbBuffer + 1, param->bytes, (g_baOrbBuffer[0] > 9)?9:g_baOrbBuffer[0]) == 0) {	//matched
				exist = 1;
				if(mode == VA_VAR_GET) {
					g_pVaRetval = vm_create_object(g_baOrbBuffer[10], g_baOrbBuffer + 11);
					return;// g_pVaRetval;
				} else if (mode == VA_VAR_SET) {
				  	param = vm_get_argument(1);
					mmMemCpy(g_baOrbBuffer + 10, param->bytes - 1, param->len + 1);
				} else if(mode == VA_VAR_DELETE) {
					mmMemSet(g_baOrbBuffer, 0xFF, g_strFileParams.reclen); 
				}
				fsFileWriteRecordW(&fhandle, i, g_baOrbBuffer, g_strFileParams.reclen); 
				break;
			} 
		}
		i++;
	} 
	if(mode == VA_VAR_SET) {
		i = 0;
		if(exist == 0) {  			//create new key, current variable didn't exist
			while((j = fsFileReadRecordW(&fhandle, i, g_baOrbBuffer, g_strFileParams.reclen)) == g_strFileParams.reclen) {
				if(g_baOrbBuffer[0] == 0xFF) {
					mmMemCpy(g_baOrbBuffer, param->bytes - 1, param->len + 1);
					param = vm_get_argument(1);
					mmMemCpy(g_baOrbBuffer + 10, param->bytes - 1, param->len + 1);
					fsFileWriteRecordW(&fhandle, i, g_baOrbBuffer, g_strFileParams.reclen);
					break;	
				}
				i++;
			}	
		}  
	}
#else  
	BSIterator iterator;
	BYTE tag[3] = { 0xE0, 0x1F, 0x00 };
	WORD objLen;     				    
	param = vm_get_argument(0);
	if(param == VM_NULL_OBJECT) return;// g_pVaRetval;
	mmMemCpy(&fhandle, &_vm_file, sizeof(FSHandle));			//copy handle from current executed orblet	 
	fhandle.curFile = fhandle.curDir;							//set to current orblet application directory
#if	COS_DEFAULT_SERVICE == COS_SERVICE_ORB
	//CMContextP ctx = cmGetSelectedContext(); 
	//mmMemCpy(&fhandle, &ctx->rootHandle, sizeof(FSHandle)); 	 
	if(fsSelectFileParentW(&fhandle, NULL) != FS_OK) return;	//select orblet directory
#else
	//cmSelectCurrentMFW(ctx, &fHandle, NULL);		  
	//fsSelectFileByFidW(&fhandle, FID_STACK, NULL);
#endif	 
	if(fsSelectFileByFidW(&fhandle, FID_STACK_LOCAL, NULL) != FS_OK) return;				//configuration directory
	if(fsSelectFileByFidW(&fhandle, FID_STACK_DATA, &g_strFileParams) != FS_OK) return;		//configuration file	
	ftWrapContext(&iterator, &fhandle); 
	objLen = bsIteratorInit(&iterator, &iterator);
	while(objLen != 0) {
		objLen = bsIteratorRead(&iterator, 0, g_baOrbBuffer, objLen);
		if(objLen >= (param->len +1)) {
			if(mmMemCmp(g_baOrbBuffer, param->bytes, param->len) == 0 && g_baOrbBuffer[param->len] == ':') break;	//matched
		}
		objLen = bsIteratorNext(&iterator);
	}
	switch(mode) {
		case VA_VAR_GET:
			g_pVaRetval = VM_NULL_OBJECT;
			if(objLen == 0) return; 
			if(objLen == (param->len + 1)) return;
			g_pVaRetval = vm_create_object(objLen - (param->len + 1), g_baOrbBuffer + (param->len + 1));
			break;	
		default:
		case VA_VAR_DELETE:
	 	case VA_VAR_SET:  
			if(objLen != 0) {
				bsReleaseByOffset(&iterator, iterator.current);	
			}
			if(mode == VA_VAR_DELETE) break;
			mmMemCpy(g_baOrbBuffer, param->bytes, param->len); 
			j = param->len;
			g_baOrbBuffer[j++] = ':';
			param = vm_get_argument(1);	
			//if(param == VM_NULL_OBJECT) return; 
			//if(param->len == 0) return;		//do not allocate 
			mmMemCpy(g_baOrbBuffer + j, param->bytes, param->len);
			j += param->len;	 
			if(bsErr(iterator.current = bsAllocObject(&iterator, tag, j))) return;
			bsHandleWriteW(&iterator, iterator.current, g_baOrbBuffer, j);
			break;
	}
#endif
}
			
void va_set_var() _REENTRANT_ { 
	va_var_operation(VA_VAR_SET);	
} 
 		
void va_get_var() _REENTRANT_ { 
	va_var_operation(VA_VAR_GET);
}
		
void va_delete_var() _REENTRANT_ { 
	va_var_operation(VA_VAR_DELETE);
}
#endif 	//end of var apis

#define VA_STR_INDEX_OF			0
#define VA_STR_LAST_INDEX_OF	1
#define VA_STR_REPLACE			2

#if STACK_STRING_APIS
static void va_string_operation(BYTE mode) _REENTRANT_ {
	vm_object * pattern;
	vm_object * param;
	WORD index = 0xFFFF;
	WORD offset = 0;
	BYTE length;
	//BYTE buffer[10];
	if(vm_get_argument_count() < 2) return;// g_pVaRetval;
	if((param = vm_get_argument(0))	!= VM_NULL_OBJECT) { 		//source string
	  	mmMemCpy(g_baOrbBuffer, param->bytes, param->len);
		length = param->len;
	}
	if((param = vm_get_argument(1))	!= VM_NULL_OBJECT) {		//pattern
		if(length < param->len) return;// g_pVaRetval;
		pattern = param;
		//length -= param->len;
	}
	if((param = vm_get_argument(2))	!= VM_NULL_OBJECT) {  		//offset/new pattern
		if(mode < 2) { 		//check if index_of or last_index_of
			if(param->len > 10) return;// g_pVaRetval;	
			offset = va_o2f(param);	 
			if(offset > length) return;// g_pVaRetval;
		}  
	}
	switch(mode) {
	  	case VA_STR_INDEX_OF:
			for(offset;offset<length;offset++) {
			 	if(mmMemCmp(g_baOrbBuffer + offset, pattern->bytes, pattern->len) == 0) {
					mmItoa(MM_ITOA_WORD, g_baOrbBuffer + length, (WORD)offset);
					g_pVaRetval = vm_create_object(vm_strlen(g_baOrbBuffer + length), g_baOrbBuffer + length);
				 	break;
				}
			}
			break;
		case VA_STR_REPLACE:
			//for(offset;offset<length;offset++) {
			while(offset < length) {
			 	if(mmMemCmp(g_baOrbBuffer + offset, pattern->bytes, pattern->len) == 0) {
					length -= pattern->len;
					mmMemCpy(g_baOrbBuffer + offset + param->len, g_baOrbBuffer + offset + pattern->len, length - offset);
					mmMemCpy(g_baOrbBuffer + offset, param->bytes, param->len);
					length += param->len;
					offset += param->len;
				} else {
					offset ++;
				}
			}
			g_pVaRetval = vm_create_object(offset, g_baOrbBuffer);
			break;
	}
	return;// g_pVaRetval;
}

void va_index_of() _REENTRANT_ {
 	va_string_operation(VA_STR_INDEX_OF);	
}

void va_replace() _REENTRANT_ {
   	va_string_operation(VA_STR_REPLACE);
}

static uchar va_is_numeric(uchar * buffer, uchar len) _REENTRANT_ { 
	BYTE n;
	if(len == 0) return FALSE;
	while(len != 0 ) {
		n = buffer[--len];
		if(n > 0x39 || n < 0x30) { return FALSE; }
	} 
	return TRUE;
}

void va_substr() _REENTRANT_ {
	BYTE offset, len;	 
	BYTE * opd1, * opd2;
	vm_object * obj, * op1, * op2;
	if(vm_get_argument_count() < 2) return;// g_pVaRetval;
	obj = vm_get_argument(0);  
	op1 = vm_get_argument(1);
	op2 = vm_get_argument(2);
	opd1 = mmAllocMemP(op1->len + 1);
	opd2 = mmAllocMemP(op2->len + 1);
	mmMemCpy(opd1, op1->bytes, op1->len); opd1[op1->len] = 0;		//null terminated string
	mmMemCpy(opd2, op2->bytes, op2->len); opd2[op2->len] = 0;		//null terminated string
	if(va_is_numeric(op1->bytes, op1->len) == FALSE) { offset = 0; } else { offset = mmAtoi(opd1); }			//
	if(va_is_numeric(op2->bytes, op2->len) == FALSE) { len = obj->len; } else { len = mmAtoi(opd2); }
	if(len > (obj->len - offset)) len = (obj->len - offset);
	mmFreeMem(opd1);
	mmFreeMem(opd2);
	g_pVaRetval = vm_create_object(len, obj->bytes + offset);
}
#endif

#define VA_FILE_OPEN 	0
#define VA_FILE_READ 	1
#define VA_FILE_WRITE 	2
#define VA_FILE_CLOSE 	4
#define VA_FILE_BY_TAG	0x10

#if STACK_FILE_APIS
static void va_file_operation(BYTE mode) _REENTRANT_ {
	//vm_object * g_pVaRetval = (vm_object *)VM_NULL_OBJECT;
	vm_object * param;
	vm_object * temp;
	BYTE i,j;
	//FSFileParameters fparam;
	//WORD fid;
	BYTE fidbuf[4];
	WORD offset, length;
	FSHandle * file;// = vm_get_argument(0);
	BYTE isRead = 0;
	//fsGetPropertyB(file, &fparam, sizeof(FSFileParameters));
	switch(mode & 0x0F) {
		case VA_FILE_OPEN:
			if(vm_get_argument_count() < 1) return;// g_pVaRetval;
			param = vm_get_argument(0);		//path
			if(param->len == 0) return;// g_pVaRetval;
			if((param->len & 0x03) != 0x00) return;// g_pVaRetval;		//odd path
			//decode hex path
			temp = vm_create_object(sizeof(FSHandle) + 1, NULL);
			if(temp == VM_NULL_OBJECT) return;// g_pVaRetval;
			temp->bytes[0] = 0xF2;		//set handle tag
			file = (temp->bytes + 1);		//handle location pointer
			cmSelectCurrentMFW(cmGetSelectedContext(), file, NULL); 
			for(i=0;i<param->len;i+=4) {
				mmMemCpy(fidbuf, param->bytes + i, 4);
				tkHex2Bin(fidbuf, 4, 4);
				if(fsSelectFileByFidW(file, end_swap16(*((WORD *)fidbuf)), NULL) != FS_OK) {
					vm_release_object(temp);  		//delete allocated object
					return;// g_pVaRetval;	 		//return
				} 	
			}
			g_pVaRetval = temp;				//set return value
			break;
		case VA_FILE_READ: 
			isRead = 1;
		case VA_FILE_WRITE:
			if(vm_get_argument_count() < 3) return;// g_pVaRetval;
			temp = vm_get_argument(0);
			if(temp->len != (sizeof(FSHandle) + 1)) return;// g_pVaRetval;	//invalid handle
			if(temp->bytes[0] != 0xF2) return;// g_pVaRetval;		//invalid handle
			file = (temp->bytes + 1);
			fsGetPropertyB(file, &g_strFileParams);
			offset = va_o2f(vm_get_argument(1)); 		//offset
			if(isRead) {
				length = va_o2f(vm_get_argument(2));	//length
				switch(g_strFileParams.desc & 0x07) {
				  	case 0x01:		 //binary
						if(mode & VA_FILE_BY_TAG) {
							if((length = ftPopByTag(file, offset, (BYTE)length, g_baOrbBuffer, 255)) == -1) length = 0;
						} else { 
							length = fsFileReadBinaryW(file, offset, g_baOrbBuffer, length);
						}
						break;
					case 0x02:		 //linfix
					case 0x03:
					case 0x06:		 //cyclic
					case 0x07: 
						if(offset == 0) return;// g_pVaRetval;
						if(offset > g_strFileParams.recnum) return;// g_pVaRetval;  
						if(mode & VA_FILE_BY_TAG) {	
							fsFileReadRecordW(file, offset -1, g_baOrbBuffer, g_strFileParams.reclen);
							offset = 0;
							while(offset < g_strFileParams.reclen) {
							  	if(g_baOrbBuffer[offset++] == (BYTE)length) {
								 	length = g_baOrbBuffer[offset++];
									mmMemCpy(g_baOrbBuffer, g_baOrbBuffer + offset, length);
									offset = g_strFileParams.reclen;
									goto operation_finished;
								} else {
								 	offset += (g_baOrbBuffer[offset] + 1);
								}
							}
							length = 0; 	
						}  else {
							length = fsFileReadRecordW(file, offset -1, g_baOrbBuffer, length);	
						}
						break;
				}
				operation_finished:
				g_pVaRetval = vm_create_object((BYTE)length, g_baOrbBuffer);  		//set return to data readed
			} else {
			 	param = vm_get_argument(2);				//data
				switch(g_strFileParams.desc & 0x07) {
				  	case 0x01:		//binary
						length = fsFileWriteBinaryW(file, offset, param->bytes, param->len);
						break;
					case 0x02:		//linfix
					case 0x03:		  
						if(offset == 0) return;// g_pVaRetval;
						if(offset > g_strFileParams.recnum) return;// g_pVaRetval;
						length = fsFileWriteRecordW(file, offset - 1, param->bytes, param->len);
						break;
#if FS_API_APPEND_RECORD
					case 0x06:		//cyclic
					case 0x07:
						if(offset != 0) return;// g_pVaRetval;
						length = fsFileAppendRecordW(file, 0, param->bytes, param->len);
						break;
#endif
				} 
				mmItoa(MM_ITOA_WORD, g_baOrbBuffer, length);
				length = (BYTE)mmStrLen(g_baOrbBuffer);
				//g_pVaRetval = vm_create_object(, g_baOrbBuffer);		//set return to ammount of bytes wrote
				goto operation_finished;
			}
			break;
		case VA_FILE_CLOSE:
			if(vm_get_argument_count() < 1) return;// g_pVaRetval;
			temp = vm_get_argument(0);
			if(temp->len != (sizeof(FSHandle) + 1)) return;// g_pVaRetval;	//invalid handle
			file = (temp->bytes + 1);
			if(temp->bytes[0] == 0xF2) {	   
				temp->bytes[0] = 0x2F;			//set handle tag to closed handle
				//g_pVaRetval = vm_create_object(4, VM_TRUE_OBJECT);		//create new object (clone object)
			}// else {
				//g_pVaRetval = vm_create_object(5, VM_FALSE_OBJECT);		//create new object (clone object)
			//}
			//optional parameter (activate/deactivate)
			if((param = vm_get_argument(1))	!= VM_NULL_OBJECT) {
				fsChangeStateB(file, va_o2f(param));
			}
			break;
	 	
	}
	return;// g_pVaRetval;
}

void va_fopen() _REENTRANT_ {
	va_file_operation(VA_FILE_OPEN);
}

void va_fread() _REENTRANT_ {
	va_file_operation(VA_FILE_READ);
}

void va_fwrite() _REENTRANT_ { 
	va_file_operation(VA_FILE_WRITE);
}

void va_fclose() _REENTRANT_ { 
	va_file_operation(VA_FILE_CLOSE);
} 

void va_fpopbytag() _REENTRANT_ {  
	va_file_operation(VA_FILE_BY_TAG | VA_FILE_READ);
}
#endif	//file APIs

//invoke external instance
void va_invoke_external() _REENTRANT_ {
	//param[0] = vendor id
	//param[1] = application name
	//param[2] = class name
	vm_object * vid;		
	vm_object * app;
	//implemented 2016.07.05
	vm_context vctx;
	vm_object * clsobj;
	CMContext mctx;	   
	BYTE aid[0x10];
	BYTE aidlen;
 	if(vm_get_argument_count() < 3) goto invoke_exception;
	vid = vm_get_argument(0);					//vendor id	
	app = vm_get_argument(1);					//application name
	clsobj = vm_get_argument(2);				//class name
	if(vid->len > 7) return;									
	if(app->len > 6) return;	
	mmMemSet(&vctx, 0, sizeof(vm_context));		//clear context
	//try constructing AID (based on vendor identifier and application name)
	aid[0] = 0xAE;								//application executable
	aid[1] = ((vid->len << 4) | (app->len & 0x0F));
	mmMemCpy(aid + 2, vid->bytes, vid->len);	
	mmMemCpy(aid + 2 + vid->len, app->bytes, app->len);
	aidlen = 2 + vid->len + app->len;
	//aid[aidlen++] = 0;			//version number (should be ignored, always zero)
	//try selecting application	
	cmInitContext(&mctx, SCB_OWNER_USER, SCQ_UAK | SCQ_UAB);		
	if(cmSelectApplicationW(&mctx, aid, aidlen, NULL) == (BYTE)-1) goto invoke_exception;
	if(fsSelectFileByFidW(&mctx, FID_STACK, NULL) != FS_OK) goto invoke_exception;
	mmMemCpy(&vctx.handle, &mctx, sizeof(FSHandle)); 
	if(vrLoadScript(VM_SCRIPT_BY_CLASS, &vctx.handle, clsobj->len, clsobj->bytes) == 0) {
		g_pVaRetval = (vm_object *)vm_create_object(sizeof(vm_context), &vctx);		//return new context
		return;
	}
	invoke_exception:
	vm_invoke_exception(VX_UNRESOLVED_CLASS);
}

#if STACK_CONVERTER_APIS
void va_bin2hex() _REENTRANT_ {
	//vm_object * g_pVaRetval = (vm_object *)VM_NULL_OBJECT; 
	vm_object * param;
	BYTE i;
	//if((param = vm_get_argument(0)) != (vm_object *)VM_NULL_OBJECT) {
	param = vm_get_argument(0);
	if(param->len > 127) return;// g_pVaRetval;
	g_pVaRetval = vm_create_object(param->len * 2, param->bytes);
	//fill hexstring
	tkBin2Hex(param->bytes, param->len, g_pVaRetval->len);
	//}
	return;// g_pVaRetval;
}

void va_hex2bin() _REENTRANT_ { 
	//vm_object * g_pVaRetval = (vm_object *)VM_NULL_OBJECT;
	vm_object * param;
	BYTE i;
	BYTE b;
	BYTE len;
	param = vm_get_argument(0);
	//if((param = vm_get_argument(0)) != (vm_object *)VM_NULL_OBJECT) {
		//len = param->len / 2;
	mmMemCpy(g_baOrbBuffer, param->bytes, param->len);
	tkHex2Bin(g_baOrbBuffer, param->len, param->len);
	g_pVaRetval = vm_create_object(len, g_baOrbBuffer);
	//}
	return;// g_pVaRetval;
}

void va_bin2dec() _REENTRANT_ {
	//vm_object * g_pVaRetval = (vm_object *)VM_NULL_OBJECT; 
	vm_object * param;
	BYTE buffer[6];
	WORD d = 0;
	BYTE i;
	//if((param = vm_get_argument(0)) != (vm_object *)VM_NULL_OBJECT) {
	param = vm_get_argument(0);
	if(param->len > 2) return;// g_pVaRetval;
	for(i=param->len;i>0;i--) {
		d *= 0x100;
		d += param->bytes[i-1];
	}
	//mmSprintf(buffer, "%d", d);
	//g_pVaRetval = vm_create_object(vm_strlen(buffer), buffer);		//return decimal value	
	//}
	va_return_word((WORD)d);
	return;// g_pVaRetval;	
}

void va_b64_encode() _REENTRANT_ {		
	vm_object * param;
  	uint8 out_len = 0;
  	uint8 i = 0;
  	uint8 j = 0;
  	uint8 char_array_3[3];
  	uint8 char_array_4[4];	 
	uint8 * bytes_to_encode;
	uint8 in_len;

	CONST uint8 base64_chars[] = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/"; 
	param = vm_get_argument(0);
	bytes_to_encode = param->bytes;
	in_len = param->len;


  	while (in_len--) {
    	char_array_3[i++] = *(bytes_to_encode++);
    	if (i == 3) {
      	char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      	char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      	char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      	char_array_4[3] = char_array_3[2] & 0x3f;

      	for(i = 0; (i <4) ; i++)
      	  	g_baOrbBuffer[out_len++] = base64_chars[char_array_4[i]];
      	i = 0;
    	}
  	}

  	if (i)
  	{
    	for(j = i; j < 3; j++)
      		char_array_3[j] = '\0'; 
    	char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    	char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    	char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    	char_array_4[3] = char_array_3[2] & 0x3f;

    	for (j = 0; (j < i + 1); j++)
      		g_baOrbBuffer[out_len++] = base64_chars[char_array_4[j]];

    	while((i++ < 3))
      		g_baOrbBuffer[out_len++] = '=';
  	}
  	//g_baOrbBuffer[out_len] = 0;		//EOS
  	g_pVaRetval = vm_create_object(out_len, g_baOrbBuffer);		//return decimal value
}

static BYTE va_pr2six(BYTE ch) {
	if ((ch >= 'A') && (ch <= 'Z')) ch = ch - 'A';
	else if ((ch >= 'a') && (ch <= 'z')) ch = ch - 'a' + 26;	
	else if ((ch >= '0') && (ch <= '9')) ch = ch - '0' + 52;
	else if (ch == '+') ch = 62;	
	else if (ch == '=') ch = 64;
	else if (ch == '/') ch = 63;
	else ch = 64;
	return ch;
}

void va_b64_decode() _REENTRANT_ {	
	vm_object * param;
	uint8 nbytesdecoded;
    register uint8 *bufin;
    register uint8 *bufout;
    register uint8 nprbytes;

	param = vm_get_argument(0);
	  
    nprbytes = param->len;
	nbytesdecoded = ((nprbytes + 3) / 4) * 3; 

    bufout = (uint8 *) g_baOrbBuffer;
    bufin = (uint8 *) param->bytes;

    while (nprbytes > 4) {
		*(bufout++) = (uint8) (va_pr2six(*bufin) << 2 | va_pr2six(bufin[1]) >> 4);
		*(bufout++) = (uint8) (va_pr2six(bufin[1]) << 4 | va_pr2six(bufin[2]) >> 2);
		*(bufout++) = (uint8) (va_pr2six(bufin[2]) << 6 | va_pr2six(bufin[3]));
		bufin += 4;
		nprbytes -= 4;
    }

    /* Note: (nprbytes == 1) would be an error, so just ingore that case */
    if (nprbytes > 1) *(bufout++) = (uint8) (va_pr2six(*bufin) << 2 | va_pr2six(bufin[1]) >> 4);
    if (nprbytes > 2) *(bufout++) = (uint8) (va_pr2six(bufin[1]) << 4 | va_pr2six(bufin[2]) >> 2);
    if (nprbytes > 3) *(bufout++) = (uint8) (va_pr2six(bufin[2]) << 6 | va_pr2six(bufin[3]));

    *(bufout++) = '\0';
    nbytesdecoded -= (4 - nprbytes) & 3;
    //return nbytesdecoded;  
  	g_pVaRetval = vm_create_object(nbytesdecoded, g_baOrbBuffer);		//return decimal value
}
#endif		//converter APIs

#if STACK_CRYPTO_APIS
CONST struct VACryptoCodec {
 	BYTE * name;
	BYTE mode;
	BYTE keylen;
} g_vaRegisteredCodec[] = { 
	{ "DES", CR_MODE_DES, 8 }, 
	{ "DES2", CR_MODE_DES2, 16 },
	{ "DES3", CR_MODE_DES3, 24 },
	{ "AES", CR_MODE_AES, 16 },	
	{ NULL, CR_MODE_DES, 0 },
};

void va_crypto_create() _REENTRANT_ {	
	//param[0] = codec 0 = "DES", 1 = "DES2", 2 = "DES3", 3 = "AES"
	//param[1] = mode  "CBC" / null
	//param[2] = key		 
	vm_object * param;
	vm_object * key;
	BYTE len;	 
	BYTE mode = 0;			//default DES
	BYTE keylen = 8;		//8 bytes per block
	BYTE objLen;
	//VACryptoContext crCtx;
	CRContextP pctx;
	struct VACryptoCodec * c_iterator = &g_vaRegisteredCodec[0];  
 	if(vm_get_argument_count() < 3) return;	
	//check parameter 0, supported codec
	param = vm_get_argument(0);
	while(c_iterator->name != NULL) {	 
		if(mmMemCmp(param->bytes, c_iterator->name, param->len) == 0) { mode = c_iterator->mode; keylen=c_iterator->keylen; }
	 	c_iterator++;
	}
	//check codec mode ECB/CBC
	param = vm_get_argument(1);
	if(mmMemCmp(param->bytes, "CBC", param->len) == 0) mode |= CR_MODE_CBC;
	key = vm_get_argument(2);
	keylen = key->len;
	//mmMemSet(crCtx.icv, 0, CR_ICV_SIZE);		//clear icv
	//crInitContext(&crCtx, g_baOrbBuffer);		//changed to context on 2015.06.14
	//crCtx.mode = mode;	
	objLen = sizeof(CRContext) + keylen;
	g_pVaRetval = vm_create_object(objLen, NULL);	  
	if(g_pVaRetval == VM_NULL_OBJECT) return;		//not enough memory, cannot allocate object
	pctx = ((CRContext *)g_pVaRetval->bytes);
	mmMemSet(pctx, 0, objLen);					//clear context first before initializing
	crInitContext(pctx, g_baOrbBuffer);			//changed to context on 2015.06.14
	pctx->mode = mode;
	//check icv  
	param = vm_get_argument(3);
	if(param != (vm_object *)VM_NULL_OBJECT) mmMemCpy(pctx->icv, param->bytes, (param->len > CR_ICV_SIZE)?CR_ICV_SIZE:param->len);
	//set context key pointer
	pctx->key = (BYTE*)pctx + sizeof(CRContext);
	if(key->len < keylen) keylen = key->len;
	mmMemCpy(pctx->key, key->bytes, keylen); 	
}

static void va_crypto_do(BYTE mode) _REENTRANT_ { 			 
	vm_object * param;
	vm_object * ctxt;
	BYTE len;
	//CRContext crCtx;	
 	if(vm_get_argument_count() < 2) return;	
	//check parameter 0, crypto handler
	param = vm_get_argument(0);
	ctxt = vm_get_argument(1);
	if(ctxt->len > 224) return;								
	mmMemSet(g_baOrbBuffer, 0, (ctxt->len + 16) & 0xF0);
	mmMemCpy(g_baOrbBuffer, ctxt->bytes, ctxt->len);
	((CRContext *)param->bytes)->mode = mode | (((CRContext *)param->bytes)->mode & 0x7F);
	len = crDoCrypt(param->bytes, 0, ctxt->len);
	g_pVaRetval = vm_create_object(len, g_baOrbBuffer);		//return decimal value
}

void va_crypto_encrypt() _REENTRANT_ {
 	va_crypto_do(CR_MODE_ENCRYPT);
}

void va_crypto_decrypt() _REENTRANT_ { 
 	va_crypto_do(CR_MODE_DECRYPT);
}

void va_random() _REENTRANT_ {	   
	BYTE len = va_o2f(vm_get_argument(0));			//added 2016.06.08
	if(len == 0) return;
	crRandomize(g_baOrbBuffer, len);
	g_pVaRetval = vm_create_object(len, g_baOrbBuffer);		//return decimal value
}

#if 0 		//DEPRECATED APIS
static void va_crypto_operation(BYTE mode) _REENTRANT_ {
	//vm_object * g_pVaRetval = (vm_object *)VM_NULL_OBJECT; 
	vm_object * param;
	vm_object * key;
	BYTE len;	 
	//BYTE plen;
	CRContext crCtx;
	if(vm_get_argument_count() < 3) return;// g_pVaRetval;
	//if((param = vm_get_argument(0)) != (vm_object *)VM_NULL_OBJECT) {
	mode |= (va_o2f(vm_get_argument(0)) & 0x3F);		//0=DES,1=DES2key,2=DES3Key,3=AES			
	//}
	//if((key = vm_get_argument(1)) != (vm_object *)VM_NULL_OBJECT) {			//get cryptokey
	key = vm_get_argument(1);
	//mode = va_o2f(param);	
	//if((param = vm_get_argument(2)) == (vm_object *)VM_NULL_OBJECT) return;// g_pVaRetval;
	param = vm_get_argument(2);
	//start operation in memory	
	//len = param->len;
	//len = ((param->len + 16) & 0xF0);
	mmMemSet(g_baOrbBuffer, 0, ((param->len + 16) & 0xF0));		 
	mmMemCpy(g_baOrbBuffer, param->bytes, param->len);
	crInitContext(&crCtx, g_baOrbBuffer);			   //changed 2015.06.14
	crCtx.mode = mode;
	crCtx.key = key->bytes;
	//len = crMemOperation(mode, key->bytes, param->bytes, param->len, g_baOrbBuffer);
	len = crDoCrypt(&crCtx, 0, param->len);
	g_pVaRetval = vm_create_object(len, g_baOrbBuffer);		//return decimal value		
}

//codec
void va_encrypt_ecb() _REENTRANT_ {
 	va_crypto_operation(CR_MODE_ENCRYPT | CR_MODE_ECB);	
}

void va_decrypt_ecb() _REENTRANT_ {
	va_crypto_operation(CR_MODE_DECRYPT | CR_MODE_ECB);
} 

void va_encrypt_cbc() _REENTRANT_ {
 	va_crypto_operation(CR_MODE_ENCRYPT | CR_MODE_CBC);	
}

void va_decrypt_cbc() _REENTRANT_ {
	va_crypto_operation(CR_MODE_DECRYPT | CR_MODE_CBC);
}
#endif			//END DEPRECATED APIS
#endif

#if STACK_TOOLKIT_SELECT_APIS
void va_select_item() _REENTRANT_ {	   //SELECT ITEM
	vm_object * param;
	//vm_object * g_pVaRetval = (vm_object *)VM_NULL_OBJECT; 
	BYTE i = 0, j;
	BYTE len;					
	//check if qualifier contain DCS
	//i = tkPrintf("cd", STK_CMD_SELECT_ITEM, 0, STK_DEV_ME);
	i = tkPushHeaderB(STK_CMD_SELECT_ITEM, 0, STK_DEV_ME);
	if((param = vm_get_argument(0))	!= VM_NULL_OBJECT) {
		len = vrConvertText(param->bytes, param->len, g_baOrbBuffer + i, TK_BUFFER_SIZE - i);
		i = tkPushBufferB(i, STK_TAG_ALPHA, len, g_baOrbBuffer + i);	
	}
	for(j=1;j<vm_get_argument_count();j++) {
		param = vm_get_argument(j);	   
		g_baOrbBuffer[i] = j; 	//item id	
		len = 1;
		len += vrConvertText(param->bytes, param->len, g_baOrbBuffer + (i + 1), TK_BUFFER_SIZE - (i + 1));
		i = tkPushBufferB(i, STK_TAG_ITEM, len, g_baOrbBuffer + i);
	} 
	tkDispatchCommandW(NULL, i);
	g_vaWaitTag = STK_TAG_ITEM_ID;
	vm_set_state(VM_STATE_SUSPEND);
	//return g_pVaRetval;
}
#endif

#if STACK_TOOLKIT_TIMER_APIS
void va_set_timer() _REENTRANT_ {  
	//vm_object * g_pVaRetval = (vm_object *)VM_NULL_OBJECT; 
	FSHandle temp_fs;	   
	vm_object * param;
	//vm_context vctx; 
	TKApplicationContext actx;
	BYTE buffer[18];
	BYTE l, h;
	//BYTE dbuffer[3];
	BYTE tid;
	BYTE tag, hlen;
	WORD tsize;
	BYTE k;
	WORD index;
	WORD i, codestart;
	WORD tick;
	//set timer initial value  
	mmMemSet(buffer + 2, 0, 3);		//clear timer value to 000000
	if((param = vm_get_argument(1))	== VM_NULL_OBJECT) return;
	tick = va_o2f(param);	 
	for(k=3;k!=1;k--) {
		buffer[k+1] = (tick % 60);
		tick /= 60;
	}
	buffer[2] = tick;
	for(k=3;k!=0;k--) {
		l = buffer[k+1] % 10;
		h = buffer[k+1] / 10;
		buffer[k+1] = (h << 4) | (l & 0x0F);
	}
	//} else return g_pVaRetval;
	//timer id
	if((param = vm_get_argument(0))	!= VM_NULL_OBJECT) {
		if(param->len > 2) return;// g_pVaRetval;
		//mmMemCpy(buffer, param->bytes, param->len);
		index = param->bytes[0];
		index <<= 8;
		index |= param->bytes[1];
	} else return;// g_pVaRetval;
	mmMemCpy(&temp_fs, &_vm_file, sizeof(FSHandle));

	//decode ASN.1 structure for package and look for entry point for matching menu_id
	hlen = tkPopFile(&temp_fs, 0, &tag, &tsize);
	if(tag == (ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED)) {		//check for constructed sequence
		codestart = tsize + hlen;										//total sequence length
		//re-initialize vm to execute current selected menu
		mmMemCpy(&actx, &temp_fs, sizeof(FSHandle));		//set handle to current active bytecode where this api executed
		actx.offset = codestart + index;
#if TK_MANAGER_ENABLED
		tid = tkRegisterServiceToTimer(TK_SID_STACK, sizeof(TKApplicationContext), &actx);
#else
		
#endif
		if(tid != 0) {
			i = tkPushHeaderB(STK_CMD_TIMER_MANAGEMENT, 0, STK_DEV_ME);
			i = tkPushBufferB(i, STK_TAG_TIMER_IDENTIFIER, 1, &tid);
			i = tkPushBufferB(i, STK_TAG_TIMER_VALUE, 3, buffer + 2);
			tkDispatchCommandW(NULL, i);	
			g_vaWaitTag = STK_TAG_RESULT;
			vm_set_state(VM_STATE_SUSPEND);	
		}
	}
	//return g_pVaRetval;
}
#endif

#define VA_BIT_CHECK		3
#define VA_BIT_SET		   	1
#define VA_BIT_CLR			2

#if STACK_BIT_APIS
static void va_bit_operation(BYTE mode) {
	vm_object * param;
	WORD bnum;
	BYTE offset, mask;//, res;
	//if(vm_get_argument_count() <  2) return;// g_pVaRetval;
	param = vm_get_argument(0);
	bnum = va_o2f(vm_get_argument(1));		//bit number
	offset = bnum / 8;	//byte number
	mask = bnum % 8;	//mask
	mask = 1 << mask;
	if(param->len <= offset) return;// g_pVaRetval;
	switch(mode) {
		case VA_BIT_CHECK:
			bnum = '0';
			if(param->bytes[offset] & mask) {
				bnum = '1';	
			}
			g_pVaRetval = vm_create_object(1, &bnum);
			break;
		case VA_BIT_SET:
			param->bytes[offset] |= mask;
			break;
		case VA_BIT_CLR:
			param->bytes[offset] &= ~mask;
			break;
		default: break;
	}
	//return g_pVaRetval;
}

void va_check_bit() _REENTRANT_ {
	va_bit_operation(VA_BIT_CHECK);
}

void va_set_bit() _REENTRANT_ {
	va_bit_operation(VA_BIT_SET);
}

void va_clear_bit() _REENTRANT_ {
	va_bit_operation(VA_BIT_CLR);
} 
#endif

void va_terminal_profile() _REENTRANT_  {
	extern BYTE g_baTerminalProfile[];
	extern BYTE g_bTerminalProfileLength;
	//memset(g_baTerminalProfile, 0x00, TK_MAX_TERMINAL_PROFILE);				//added 2015.06.10
	g_pVaRetval = vm_create_object(g_bTerminalProfileLength, g_baTerminalProfile);
}

#if STACK_WIB_APIS && TK_WIB_ENABLED
//cross APIs
void va_wib_set_return_var() _REENTRANT_ {
	//vm_object * g_pVaRetval = (vm_object *)VM_NULL_OBJECT; 
	extern WORD g_wWibLastFID;
	vm_object * param;
	BYTE err;
	g_wWibLastFID = 0;
	err = va_o2f(vm_get_argument(0));
	param = vm_get_argument(1);
	wbSetReturnVariable(err, param->len, param->bytes);
	g_wWibLastFID = va_o2f(vm_get_argument(2));	 	
}

BYTE vm_exec_wib_plugin(BYTE result) _REENTRANT_ {
	BYTE ret = 0;
	switch(result) {
	 	case 0x10:
			ret = -1;
			goto exit_plugin_execution;
		case 0x11:
			ret = -2;
			exit_plugin_execution:
			wbSetReturnVariable(ret, 0, NULL);	
			vm_close();
			break; 
		default: break;
	}
	return ret;
}
#endif

void va_arg_findtag() _REENTRANT_ {
 	//find_by_tag(obj, tag)
	uint8 len;		 
	vm_object * obj = vm_get_argument(0);
	uint8 tag = va_o2f(vm_get_argument(1));
	len = mmPopTlvByTag(obj->bytes, obj->len, tag, g_baOrbBuffer);
	if(len == (uint8)-1) return;
	g_pVaRetval = vm_create_object(len, g_baOrbBuffer);
}

#if STACK_JSON_APIS	  
void va_arg_create() _REENTRANT_ {
	//uint16 len = strlen((const char *)key) + strlen((const char *)value) + 2;
	vm_object * obj;
	vm_object * key = vm_get_argument(0);
	vm_object * val = vm_get_argument(1);
	uint16 len = key->len + val->len + 3;
	if(len > 250) return;

	g_baOrbBuffer[0] = ASN_TAG_OBJDESC;	  
	g_baOrbBuffer[1] = len - 2;
	mmMemCpy(g_baOrbBuffer + 2, key->bytes, key->len);
	g_baOrbBuffer[key->len + 2] = ':'; 
	mmMemCpy(g_baOrbBuffer + key->len + 3, val->bytes, val->len);
	g_pVaRetval = vm_create_object(len, g_baOrbBuffer);
	if(g_pVaRetval != VM_NULL_OBJECT) g_pVaRetval->mgc_refcount |= VM_OBJ_MAGIC;				//set to object
}

void va_arg_object() _REENTRANT_ {
	//va_list ap;
    uint8 j;
	vm_object * obj, * ibj;
	uint16 len = 0;
    //double sum = 0;
	uint8 count =  vm_get_argument_count();
	//cid = va_o2f(vm_get_argument(0));

    //va_start(ap, count);					/* Requires the last fixed parameter (to get the address) */
    for (j = 0; j < count; j++) {
        //obj = va_arg(ap, vm_object *);		/* Increments ap to the next argument. */
		obj = vm_get_argument(j);
		len += obj->len;
	}
	if(len > 250) return;
	//va_start(ap, count);					/* Requires the last fixed parameter (to get the address) */
    //obj = (vm_object *)malloc(len + sizeof(vm_object));
	len = 2;
	for (j = 0; j < count; j++) {
        //ibj = va_arg(ap, vm_object *);   
		ibj = vm_get_argument(j);
		if((ibj->mgc_refcount & 0xF0) == VM_OBJ_MAGIC) {
			//check for OWB object
			mmMemCpy(g_baOrbBuffer + len, ibj->bytes, ibj->len);
			len += ibj->len;
		} else {
			//default string object
		 	mmMemCpy(g_baOrbBuffer + len + 2, ibj->bytes, ibj->len);
			g_baOrbBuffer[len] = ASN_TAG_OBJDESC;
			g_baOrbBuffer[len + 1] = ibj->len;
			len += (ibj->len + 2);
		}
	}
	g_baOrbBuffer[0]= ASN_TAG_SEQ;	//'*';		//object mark 
	g_baOrbBuffer[1]= len - 2;	//'*';		//actual object content length (not incuding header)
	//va_end(ap);
    g_pVaRetval = vm_create_object(len, g_baOrbBuffer);
}
							   
void va_arg_array() _REENTRANT_ {
    uint8 j;
	vm_object * obj, * ibj;
	uint16 len = 0;
    //double sum = 0;	   
	uint8 count =  vm_get_argument_count();

    //va_start(ap, count);					/* Requires the last fixed parameter (to get the address) */
    for (j = 0; j < count; j++) {
        //obj = va_arg(ap, vm_object *);		/* Increments ap to the next argument. */ 
		obj = vm_get_argument(j);
		len += obj->len;
	}
	if(len > 250) return;
	//va_start(ap, count);					/* Requires the last fixed parameter (to get the address) */
    //obj = (vm_object *)malloc(len + sizeof(vm_object));
	len = 2;
	for (j = 0; j < count; j++) {
        //ibj = va_arg(ap, vm_object *);	  
		ibj = vm_get_argument(j);
		if((ibj->mgc_refcount & 0xF0) == VM_OBJ_MAGIC) {
			//check for OWB object
			mmMemCpy(g_baOrbBuffer + len, ibj->bytes, ibj->len);
			len += ibj->len;
		} else {
			//default string object
		 	mmMemCpy(g_baOrbBuffer + len + 2, ibj->bytes, ibj->len);
			g_baOrbBuffer[len] = ASN_TAG_OBJDESC;
			g_baOrbBuffer[len + 1] = ibj->len;
			len += (ibj->len + 2);
		}
	}
	g_baOrbBuffer[0]= ASN_TAG_SET;	//'#';		//array mark
	g_baOrbBuffer[1]= len - 2;	//'*';		//actual object content length (not incuding header)
    //va_end(ap);
    g_pVaRetval = vm_create_object(len, g_baOrbBuffer);
}

void va_arg_at() _REENTRANT_ {
	uint8 i;
	uint8 len;
	uint8 j;
	uint8 tlen;
	uint8 tag;
	vm_object * ibj;		 
	vm_object * obj = vm_get_argument(0);
	uint8 index = va_o2f(vm_get_argument(1));
	if(obj->len == 0) return;
	if(obj->bytes[0] != ASN_TAG_SET) return;		//invalid array mark
	//obj = obj->bytes;
	tlen = obj->bytes[1];
	for(i=2,j=0;i<tlen;j++) {
		tag = obj->bytes[i];
		len = obj->bytes[i + 1];
		if(j == index) { 
    		g_pVaRetval = vm_create_object(len + 2, obj->bytes + i);
			if(g_pVaRetval != VM_NULL_OBJECT) g_pVaRetval->mgc_refcount |= VM_OBJ_MAGIC;
			break;
		} 
		i += (len + 2);
	}
	//return VM_NULL_OBJECT;		//index out of bounds
}
			  
void va_arg_get() _REENTRANT_ {
	uint8 i = 1;
	uint8 len;
	uint8 j;   
	uint8 tlen;
	uint8 tag;
	vm_object * ibj;		
	vm_object * obj = vm_get_argument(0);
	vm_object * key = vm_get_argument(1);
	if(obj->len == 0) return;
	if(obj->bytes[0] != ASN_TAG_SET && obj->bytes[0] != ASN_TAG_SEQ ) return;		//invalid array/object mark	  
	//obj = obj->bytes;
	tlen = obj->bytes[1];
	for(i=2;i<tlen;) {			 
		tag = obj->bytes[i];
		len = obj->bytes[i + 1];
		if(tag == ASN_TAG_OBJDESC && obj->bytes[i + 2 + key->len] == ':'  && mmMemCmp(&obj->bytes[i + 2], key->bytes, key->len) == 0 ) {
			len = len - (key->len + 1);
			g_pVaRetval = vm_create_object(len, &obj->bytes[i + 2] + key->len + 1);
			if(g_pVaRetval != VM_NULL_OBJECT) g_pVaRetval->mgc_refcount |= VM_OBJ_MAGIC;
			break;
		}
		i += (len + 2);
	}
	//return VM_NULL_OBJECT;		//index out of bounds
}

static uint8 va_arg_serialize_s(uint8 * buffer, uint8 ** strbuf, uint8 objlen) _REENTRANT_ {
	uint8 c;
	uint8 j = 0;
	uint8 len = 0;
	uint8 state =0;
	while(j++ < objlen && (c = *(strbuf[0])++)) {
		switch(c) {
			case ASN_TAG_SET:		//array
				state=0;
				buffer[len++]= '[';
				c = *(strbuf[0])++;
				j++;
				len += va_arg_serialize_s(buffer + len, strbuf, c);
				j += c;
				buffer[len++]= ']';
				break;
			case ASN_TAG_SEQ:		//object
				state=0;
				buffer[len++]= '{';
				c = *(strbuf[0])++;
				j++;
				len += va_arg_serialize_s(buffer + len, strbuf, c);
				j += c;
				buffer[len++]= '}';
				break;
			case ':':
				if(state == 1) {
					state++;
					buffer[len++]= '\"';
				}
				buffer[len++]= ':';
				break;
			case ASN_TAG_OBJDESC:
				if(state == 0) { 
					if(len != 0) buffer[len++]= ',';
					c = *(strbuf[0])++;
					j++;
					len += va_arg_serialize_s(buffer + len, strbuf, c);
					j += c;
				} 
				break;
			case ' ': if(state == 0) break;
			default:
				if(state == 2 || state == 0) {
					state++;
					buffer[len++]= '\"';
				}
				buffer[len++]= c;
				break;
		}
	}
	if((state & 0x01) != 0) buffer[len++]= '\"';
	return len;
}

void va_arg_serialize() _REENTRANT_ {			// -> to json string
	uint8 dlen;
	vm_object * obj = vm_get_argument(0);
	uint8 * tptr = obj->bytes;
	if((obj->mgc_refcount & 0xF0) != VM_OBJ_MAGIC) return;		//should be OWB object
	dlen = va_arg_serialize_s(g_baOrbBuffer, &tptr, obj->len);	 
	g_pVaRetval = vm_create_object(dlen, g_baOrbBuffer);
}

static uint8 va_arg_deserialize_s(uint8 * buffer, uint8 ** strbuf, uint8 slen, uint8 * index) _REENTRANT_ {
	uint8 c;
	uint8 len = 0;
	uint8 ilen;
	uint8 state =0;
	uint8 ldx;
	uint8 klen=0;
	//uint8 j=0;
	while(index[0]++ < slen && (c = *(strbuf[0])++)) {
		switch(c) {
			case '[':		//start array
				buffer[len++] = ASN_TAG_SET;
				ilen = va_arg_deserialize_s(buffer + len + 1, strbuf, slen, index);
				buffer[len++] = ilen;
				len += ilen;
				break;
			case '{':		//start object
				buffer[len++] = ASN_TAG_SEQ;
				ilen = va_arg_deserialize_s(buffer + len + 1, strbuf, slen, index);
				buffer[len++] = ilen;
				len += ilen;
				break;
			case '}':		//end object
			case ']':		//end array
			case 0:			//end string
				return len;
			case '\"':		//start key-value
				switch(state) {
					case 0: buffer[len++]=ASN_TAG_OBJDESC; ldx = len++; klen = 0; break;
					case 1: buffer[ldx] = klen; break;
					case 2: break;
					case 3: buffer[ldx] = klen; break;
				}
				state++;
				break;
			case ',':
				if(state != 1 || state != 3) { 			//don't skip comma when inside quotation mark
					state = 0;
				} else goto push_char;
				break;
			case ':':
			default:
			push_char:
				klen++;
				buffer[len++] = c;
				break;
			case ' ': break;	//skip white space
		}
	}
	return len;
}

void va_arg_deserialize() _REENTRANT_ {	//-> from json string
	uint8 dlen;
	uint8 t = 0;			  
	vm_object * obj = vm_get_argument(0);
	uint8 * tptr = obj->bytes;	   
	dlen = va_arg_deserialize_s(g_baOrbBuffer, &tptr, obj->len, &t);
	//obj = (vm_object *)malloc(sizeof(vm_object) + dlen);
	//obj->len = dlen;
	//memcpy(obj->bytes, dbuf, dlen);
	//return obj;
	g_pVaRetval = vm_create_object(dlen, g_baOrbBuffer);
	if(g_pVaRetval != VM_NULL_OBJECT) g_pVaRetval->mgc_refcount |= VM_OBJ_MAGIC;
}
#endif

BYTE g_bVtkLength;
BYTE g_bVtkMenuIndex;
void va_toolkit_create() _REENTRANT_ {
	//vm_object * g_pVaRetval = (vm_object *)VM_NULL_OBJECT;
	BYTE cmd, cq, dev;
	//if(vm_get_argument_count() <  3) return;// g_pVaRetval;
	cmd = va_o2f(vm_get_argument(0));		//proactive command	 
	cq = va_o2f(vm_get_argument(1));		//command qualifier
	dev = va_o2f(vm_get_argument(2));		//target device
	g_bVtkLength = tkPushHeaderB(cmd, cq, dev); 
	g_bVtkMenuIndex = 1;
	//return g_pVaRetval;
}

static void va_toolkit_push(BYTE is_raw) _REENTRANT_ { 
	//vm_object * g_pVaRetval = (vm_object *)VM_NULL_OBJECT;
	vm_object * param; 
	BYTE tag;
	BYTE len, tlen, i, ilen;
	WORD wtemp;
	//if(vm_get_argument_count() <  2) return;// g_pVaRetval;
	tag = va_o2f(vm_get_argument(0));		//tag
	param = vm_get_argument(1);
	if(is_raw) goto raw_operation;
	switch(tag & 0x7F) {
	 	case STK_TAG_ITEM:						  //added 2015.05.23, automatically add item identifier
			//g_baOrbBuffer[g_bVtkLength] = g_bVtkMenuIndex++;  	//add item identifier
			//len = 1;					
			//len += vrConvertText(param->bytes, param->len, g_baOrbBuffer + (g_bVtkLength + 1), 252 - (g_bVtkLength + 1));
			if(param->len == 0) return;
			if(((param->mgc_refcount & 0xF0) == VM_OBJ_MAGIC) && param->bytes[0] == ASN_TAG_SET) {
				//only if array
				tlen = param->bytes[1];
				for(i=2;i<tlen;) {
					//tag = param->bytes[i]; 		//skip tag
					ilen = param->bytes[i + 1];
					g_baOrbBuffer[g_bVtkLength] = g_bVtkMenuIndex++;  	//add item identifier
					len = 1;					
					len += vrConvertText(param->bytes + i + 2, len, g_baOrbBuffer + (g_bVtkLength + 1), 252 - (g_bVtkLength + 1));
					g_bVtkLength = tkPushBufferB(g_bVtkLength, STK_TAG_ITEM, len, g_baOrbBuffer + g_bVtkLength);	
					i += (ilen + 2);
				}
				return;
			} else {  
				g_baOrbBuffer[g_bVtkLength] = g_bVtkMenuIndex++;  	//add item identifier
				len = 1;					
				len += vrConvertText(param->bytes, param->len, g_baOrbBuffer + (g_bVtkLength + 1), 252 - (g_bVtkLength + 1));
			}
			break; 
		case STK_TAG_ALPHA:						 //added 2015.05.23, automatically add 0x80 in-case of ucs2
			len = vrConvertText(param->bytes, param->len, g_baOrbBuffer + g_bVtkLength, 253 - g_bVtkLength);
			if(g_baOrbBuffer[g_bVtkLength] & 0x80) tag |= 0x80;		//comprehensive tlv
			break;
		case STK_TAG_TEXT_STRING:				 //added 2015.05.23, automatically add dcs
			len = vrConvertTextDcs(param->bytes, param->len, g_baOrbBuffer + g_bVtkLength, 253 - g_bVtkLength);
			break;
		case STK_TAG_USSD_STRING:		   		//added 2015.05.27, input in binary
			g_baOrbBuffer[g_bVtkLength] = 0;		//DCS 7 bit, class 0
			len = tkEncode827(param->bytes, g_baOrbBuffer+ g_bVtkLength + 1, param->len, 0);
			len += 1;			//length of DCS
			break;
		case STK_TAG_DURATION:			  		//added 2015.05.27, input from decimal value in seconds
			wtemp = va_o2f(param);			//automatically convert to duration
			if(wtemp != 0) {
				if(wtemp < 256) { tag = 0x01; }  					  //use second
				else if(wtemp >= 2550) { tag = 0x00; wtemp = (wtemp/60); }		  //use minutes
				else { tag = 0x02; wtemp = (wtemp/10); }					 //use period
				g_baOrbBuffer[g_bVtkLength] = tag;
				g_baOrbBuffer[g_bVtkLength + 1] = wtemp;
				len = 2;
			}
			break;
		//case STK_TAG_ADDRESS:					//added 2015.05.27, using input from string
		//	len = tkFormatAddressField(g_baOrbBuffer + g_bVtkLength, (BYTE)param->len, param->bytes);
		//	break;
		default:
			raw_operation:
			mmMemCpy(g_baOrbBuffer + g_bVtkLength, param->bytes, param->len);
			len = param->len;
			break;
	}
	g_bVtkLength = tkPushBufferB(g_bVtkLength, tag, len, g_baOrbBuffer + g_bVtkLength);	 
	//return g_pVaRetval;
} 

void va_toolkit_push_raw() _REENTRANT_ {
   	va_toolkit_push(1);
}

void va_toolkit_push_ext() _REENTRANT_ {
   	va_toolkit_push(0);
}

void va_toolkit_dispatch() _REENTRANT_ {
	BYTE tag = STK_TAG_NONE; 
	if(vm_get_argument_count() > 0) {
		tag = va_o2f(vm_get_argument(0));		//tag to wait (if exist)
	}
	tkDispatchCommandW(NULL, g_bVtkLength);
	g_vaWaitTag = tag;
	vm_set_state(VM_STATE_SUSPEND);
	//return g_pVaRetval;
}

void va_toolkit_get_result() _REENTRANT_ {
	extern BYTE g_bVrResult;		//last result
	va_return_word((WORD)g_bVrResult);
}

void va_set_execution_context() _REENTRANT_ {
	extern BYTE g_vmExecContext;
 	if(vm_get_argument_count() > 0) {
		g_vmExecContext = va_o2f(vm_get_argument(0));		//tag to wait (if exist)
	}  	
}

void va_bytes() _REENTRANT_ {	   //SELECT ITEM  
	BYTE i, j, k;
	BYTE len;
	vm_object * param;
	j = vm_get_argument_count();
	//if(j == 0) return;// g_pVaRetval;
	for(i=0,k=0;i<j;i++) { 
		param = vm_get_argument(i);
		g_baOrbBuffer[k++] = va_o2f(param);	
	}
	g_pVaRetval = vm_create_object(k, g_baOrbBuffer); 
}

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
	 
void va_iso_create_message() _REENTRANT_ {
	vm_object * mti;
	//uint8 dbuf[250];
	uint8 i,j,dlen;
	mti = vm_get_argument(0);
	if(mti == VM_NULL_OBJECT) return;
	mmMemSet(g_baOrbBuffer, '0', 20);
	for(i=4,j=1;i>0 && j<=mti->len;i--,j++) {
		g_baOrbBuffer[i-1] = mti->bytes[mti->len - j];
	}
	g_pVaRetval = vm_create_object(20, g_baOrbBuffer);
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
	mmMemCpy(obj->bytes, a->bytes, a->len);
	bmap = iso_h2bin(obj->bytes[4 + (tag>>2)]);
	obj->bytes[4 + (tag>>2)] = g_b2h[(bmap | g_bmask[tag % 4])];
	mmMemCpy(obj->bytes + a->len, b, len);
	return obj;
}		  

void va_iso_push_element() _REENTRANT_ {
	//uint8 dbuf[250];
	uint8 i,j,dlen;
	uint16 wlen;
	vm_object * hdr = vm_get_argument(0);
	uint8 tag = va_o2f(vm_get_argument(1));
	vm_object * dem = vm_get_argument(2);  
	data_element_map * map = iso_get_map(tag);
	if(vm_get_argument_count() < 3) return;
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
			mmMemCpy(g_baOrbBuffer + dlen, dem->bytes, dem->len);
			g_pVaRetval = iso_push(hdr, tag, g_baOrbBuffer, dlen + dem->len);
		} else {
			mmMemSet(g_baOrbBuffer, '0', map->length);
			for(i=map->length,j=1;i>0 && j<=dem->len;i--,j++) {
				g_baOrbBuffer[i-1] = dem->bytes[dem->len - j];
			}
			g_pVaRetval = iso_push(hdr, tag, g_baOrbBuffer, map->length);
		}
	}
	if(g_pVaRetval != VM_NULL_OBJECT) {
		g_pVaRetval->mgc_refcount = (hdr->mgc_refcount + 1);		//copy header bytes
		vm_update_mutator(hdr, g_pVaRetval);						//update mutator
		hdr->mgc_refcount &= 0xF0;									//clear refcount
		vm_release_object(hdr);										//release header
	}
}

void va_iso_get_element() _REENTRANT_ {
	vm_object * obj = VM_NULL_OBJECT;
	uint8 bmap;
	uint8 i,k,l,n;
	uint16 j = 20;
	uint8 dlen;
	vm_object * msg = vm_get_argument(0);
	uint8 tag = va_o2f(vm_get_argument(1));
	if(vm_get_argument_count() < 2) return;
	if(msg == VM_NULL_OBJECT) return ;
	tag -= 1;
	bmap = iso_h2bin(msg->bytes[4 + (tag>>2)]);
	if((bmap & g_bmask[tag % 4]) == 0) return;
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
					g_pVaRetval = vm_create_object(dlen, msg->bytes + j);
					return;
				}
				j += dlen;
			} else {
				dlen = g_vtable[i].length;
				if(i == tag) goto return_element;
				j += dlen;
			}
		}
	}
	return;
}

#if COS_DEFAULT_SERVICE == COS_SERVICE_ORB
void va_get_info() _REENTRANT_ {
 	vm_object * obj = VM_NULL_OBJECT;
	uint8 div_data[26];
	uint8 div_len;
	uint8 tag = va_o2f(vm_get_argument(0));
	if(tag == 0)  {
		div_len = gpLoadData(cmGetSelectedContext(), GPC_TAG_DIVDATA, 26, div_data); 		//set diversification data
		g_pVaRetval = vm_create_object(div_len, div_data);
	}
	else g_pVaRetval = VM_NULL_OBJECT;
}
#endif

////////////////////////////////////////////END OF ISO - 8583//////////////////////////////////////////////


vm_object * va_syscall_ret(BYTE size, BYTE * buffer) _REENTRANT_ {
	BYTE i = 0, tag, j;
	WORD len;
	vm_object * obj = VM_NULL_OBJECT;
	g_vaWaitTag &= 0x7F;
	while(i < size) {
		i += tkPop(buffer + i, &tag, &len, buffer);
		tag &= 0x7F; 
		if(tag == g_vaWaitTag) {
			switch(tag) { 
				case STK_TAG_CHANNEL_DATA_LENGTH: 
					mmItoa(MM_ITOA_WORD, buffer + 1, *((WORD *)buffer));
					len = vm_strlen(buffer + 1); 
					break;
				case STK_TAG_CHANNEL_STATUS:
					if((buffer[0] & 0x80) == 0) break;		//check if link is established and return channel id
				case STK_TAG_RESULT: 
				case STK_TAG_ITEM_ID:
					mmItoa(MM_ITOA_WORD, buffer + 1, (WORD)buffer[0]);
					len = vm_strlen(buffer + 1); 
					break;
				case STK_TAG_TEXT_STRING:
					//len -= 1;		//ignore dcs
					len = tkGsm2Utf8(buffer + 1, len -1, (TK_BUFFER_SIZE - 1));
					break;	  
				case STK_TAG_DATETIMEZONE:
				case STK_TAG_LOCATION_INFO:	
				case STK_TAG_NMR:
				case STK_TAG_IMEI:
				case STK_TAG_LANGUAGE:
				default:
					mmMemCpy(buffer + 1, buffer, len);
					break;
			}
			obj = vm_create_object(len, buffer + 1);
			break;
		}
	}
	return obj;
}

#endif
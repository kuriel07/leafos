#include "..\..\..\defs.h"
#include "..\..\..\config.h"  
#ifndef _CMAPIS__H
#include "..\..\..\CM\include\CMApis.h"
#endif
#ifndef _VM_STACK__H
#include "..\..\include\VM\VMStackApis.h"
#endif		
#ifndef _RTAPIS__H
#include "..\..\..\RT\include\RTApis.h"
#endif
#ifndef _TKAPIS__H
#include "..\..\include\TKApis.h"
#endif
#ifndef _FSAPIS__H 
#include "..\..\..\FS\include\FSApis.h"
#endif	 	
#ifndef _CRAPIS__H 
#include "..\..\..\CR\include\CRApis.h"
#endif	 
#ifndef _MMAPIS__H	  
#include "..\..\..\MM\include\MMApis.h"	 
#endif	

#if TK_STACK_ENABLED

#define VR_MAX_ENTRY_LENGTH		24
#define VR_START_ENTRY_ID		TK_STATE_SELECT_MENU 		//
extern BYTE g_baTKBuffer[];

typedef struct VRProcessEventArgs {
 	BYTE event;
	BYTE tag;
} VRProcessEventArgs;

CONST VRProcessEventArgs g_baStackProcessEventArgs[] = {
 	{ TK_STATE_EVENT_MT_CALL, STK_TAG_ADDRESS },
 	{ TK_STATE_EVENT_MT_CALL, STK_TAG_SUBADDRESS },   
 	{ TK_STATE_EVENT_CALL_DISCONNECTED, STK_TAG_CAUSE },
 	{ TK_STATE_EVENT_CALL_DISCONNECTED, STK_TAG_DEV_ID },	
 	{ TK_STATE_EVENT_CALL_CONNECTED, STK_TAG_CAUSE },	
 	{ TK_STATE_EVENT_LOCATION_STATUS, STK_TAG_LOCATION_STATUS },	
 	{ TK_STATE_EVENT_LOCATION_STATUS, STK_TAG_LOCATION_INFO },
 	{ TK_STATE_EVENT_CARD_READER_STAT, STK_TAG_CARD_READER_STATUS },
 	{ TK_STATE_EVENT_LANGUAGE_SELECT, STK_TAG_LANGUAGE }, 
 	{ TK_STATE_EVENT_LANGUAGE_SELECT, STK_TAG_LANGUAGE },
 	{ TK_STATE_EVENT_BROWSER_TERMINATION, STK_TAG_BROWSER_TERMINATION },  
 	{ TK_STATE_EVENT_DATA_AVAILABLE, STK_TAG_CHANNEL_STATUS },
 	{ TK_STATE_EVENT_DATA_AVAILABLE, STK_TAG_CHANNEL_DATA_LENGTH },
 	{ TK_STATE_EVENT_CHANNEL_STATUS, STK_TAG_CHANNEL_STATUS },
	{ TK_STATE_EVENT_SMSPP, STK_TAG_ALL },	  
	{ TK_STATE_EVENT_CALLBACK, STK_TAG_ALL },
	{ 0, 0 }			//end of mark
};

//extern WORD vrLoadScriptW(BYTE mode, FSHandleP handle, BYTE len, BYTE * buffer) _REENTRANT_ ;
  
BYTE vrPreprocessResult(BYTE size, BYTE * buffer) _REENTRANT_ {
	//BYTE res_buffer[1];
	BYTE retval = 0;
	extern BYTE g_vmExecContext;
	if(tkPopByTag(buffer, size, STK_TAG_RESULT, g_baTKBuffer) != TK_TAG_NOT_FOUND) {
		//execute context
		retval = va_exec_context(g_baTKBuffer[0]);
		return retval;
	}
	return retval;
}

extern FSHandle _vm_file;
BYTE vrSelectConfigurationFile(FSHandleP handle, WORD fid) _REENTRANT_ {
	mmMemCpy(handle, &_vm_file, sizeof(FSHandle));
	handle->curFile = handle->curDir;							//set to current directory (STACK_BYTECODES)
	fsSelectFileParentW(handle, NULL);							//current application root
	if(fsSelectFileByFidW(handle, FID_STACK_LOCAL, NULL) != FS_OK) return FS_FILE_NOT_FOUND;			// g_pVaRetval;
	return fsSelectFileByFidW(handle, fid, NULL) ;
}

BYTE vrPreprocessEventArgs(BYTE id, BYTE size, BYTE * buffer) _REENTRANT_ {
	BYTE length = 0; 
	//BYTE arg_buffer[140];
	extern BYTE g_baTKBuffer[];
	BYTE i= 0;
	while(g_baStackProcessEventArgs[i].event != 0) {
	 	if(g_baStackProcessEventArgs[i].event == id) {
			//wbSetArgument(g_baProcessEventArgs[i].tag, g_baProcessEventArgs[i].vid, size, buffer);
			length = tkPopByTag(buffer, size, g_baStackProcessEventArgs[i].tag, g_baTKBuffer);
			if(length == TK_TAG_NOT_FOUND) length = 0;		//no length data, push null
			vm_push_argument(length, g_baTKBuffer);
		}
	}
	return 0;
}

#if (TK_DEFAULT_MENUTITLE == TK_SID_STACK)
void tkSetupMenuTitle() _REENTRANT_ {
	CONST BYTE defaultTitle[] = { 'O', 'r', 'b', 'W', 'e', 'a', 'v', 'e', 'r', ' ', (VM_MAJOR_VERSION >> 4) | 0x30, (VM_MAJOR_VERSION & 0x0F) | 0x30, '.', (VM_MINOR_VERSION >> 4) | 0x30, (VM_MINOR_VERSION & 0x0F) | 0x30 };	
   	BYTE dbuffer[VR_MAX_ENTRY_LENGTH + 1];
	BYTE ebuffer[3];
	FSHandle handle;
	BYTE tag, hlen;
	//from EF menutitle	
	if((hlen = vrLoadText(FID_STACK_CONFIG, 0x01, dbuffer + 1, VR_MAX_ENTRY_LENGTH)) != -1) {
		dbuffer[0] = hlen;
	} else {
		//OrbWeaver 1.0 beta
		//mmSprintf(dbuffer + 1, "OrbWeaver %d.%db", (WORD)VM_MAJOR_VERSION, (WORD)VM_MINOR_VERSION);
		mmMemCpy(dbuffer + 1, defaultTitle, sizeof(defaultTitle));
		//dbuffer[0] = vm_strlen(dbuffer + 1);
		dbuffer[0] = sizeof(defaultTitle);
	}
	dbuffer[0] = vrConvertText(dbuffer + 1, dbuffer[0], dbuffer + 1, VR_MAX_ENTRY_LENGTH);
	tkSetupMenuSetTitle(dbuffer + 1, dbuffer[0]);
}
#endif

/*!
******************************************************************************
\section  WORD vrTriggerEvent(BYTE eventid, BYTE size, BYTE * buffer)
\brief    stack virtual machine trigger event APIs
\param 	  eventid eventid for corresponding event (startup, sms-pp, etc)
\param	  size size of argument to pass
\param	  buffer pointer to corresponding event argument
\return   WORD status word (tkStatusW)
\author   AGP
\version  1.0
\date     2014.10.28

\verbatim

stack virtual machine trigger event APIs 

\endverbatim
*/	
WORD vrTriggerEvent(BYTE eventid, BYTE size, BYTE * buffer) _REENTRANT_ {
   	BYTE dbuffer[VR_MAX_ENTRY_LENGTH + 1];
	BYTE ebuffer[3];
	FSHandle handle;
	BYTE tag, hlen;
	BYTE icon[2] = {0, 0};
	WORD tsize;
	WORD codestart, i, j = 0, gid = 0;		   
#if	COS_DEFAULT_SERVICE == COS_SERVICE_ORB
	CMContextP cctx = cmGetSelectedContext();
	mmMemCpy(&handle, &cctx->rootHandle, sizeof(FSHandle));
#else
	cmSelectCurrentMFW(cmGetSelectedContext(), &handle, NULL);
#endif
	fsSelectFileByFidW(&handle, FID_STACK, NULL);
	switch(eventid & 0xF0) { 			//check group
   		case 0x40:			//SELECT MENU
			//eventid = buffer[0];		//selected menu
			if((codestart = vrLoadScript(VM_SCRIPT_BY_INDEX | VM_SCRIPT_BY_ALIAS, &handle, size, buffer)) != -1) {
				//close currently running virtual machine if any
				vm_close();
				//re-initialize vm to execute current selected menu
			 	if(vm_init(&handle, codestart) != 0) return SW_CE_TOOLKIT_OVERLOAD; 
				//start decode
				vm_decode();
			}
			break;
		case 0x80:			//proprietary event
		case 0x90:			//proprietary event
		case 0x00:			//ME event 
		case 0x10:			//ME event
			if((codestart = vrLoadScript(VM_SCRIPT_BY_EVENT, &handle, 1, &eventid)) != -1) {
				//re-initialize vm to execute current selected menu
			 	if(vm_init(&handle, codestart) != 0) return SW_CE_TOOLKIT_OVERLOAD;
				vrPreprocessEventArgs(eventid, size, buffer); 
				//start decode
				vm_decode();
			}
			break;
		case 0xF0:			//SYSTEM EVENT (INIT)	
			if(fsSelectFileFirstChildW(&handle, NULL) != FS_OK) break;
			//	case 0x0E:			//SETUP EVENT
			do {
				//decode ASN.1 structure for package and look for entry point
				hlen = tkPopFile(&handle, 0, &tag, &tsize);
				if(tag == (ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED)) {		//check for constructed sequence
					codestart = tsize + hlen;										//total sequence length
					i = hlen;
					tsize = 0;
					for(;i<codestart;i+=tsize) {
						hlen = tkPopFile(&handle, i, &tag, &tsize);
						if(eventid == 0xFF && tag == ASN_TAG_OCTSTRING) {					  //check for menu entry
							tsize -= 2;
							tsize = (tsize > VR_MAX_ENTRY_LENGTH)?VR_MAX_ENTRY_LENGTH:tsize;
							fsFileReadBinaryW(&handle, i+ hlen+ 2, dbuffer + 1, tsize);
							//generate corresponding menuitem
							dbuffer[0] = vrConvertText(dbuffer + 1, tsize, dbuffer + 1, VR_MAX_ENTRY_LENGTH);
							tkSetupMenuRegisterEntry(gid, dbuffer + 1, dbuffer[0], icon);
							gid ++;
							tsize += 2;
						}
						if(eventid == 0xFE && tag == ASN_TAG_INTEGER) {					  //check for menu entry
							fsFileReadBinaryW(&handle, i+ hlen, ebuffer, 3);
							if(ebuffer[2] == TK_STATE_EVENT_STARTUP) {	 	//initialize startup code
					 			vm_init(&handle, end_swap16(*((WORD *)ebuffer)) + codestart);
							}					 
							tkSetupEventRegisterEntry(ebuffer[2]);
						}
						tsize += hlen;				
					}	
				}
			} while(fsSelectFileNextSiblingW(&handle, NULL) == FS_OK);
				//	break;
				//default: break;
			//}
			break;
		case TK_STATE_EVENT_INCOMING_SM:

			break;	 
		case TK_STATE_EVENT_EXECUTE:							//execute code on the fly
		case TK_STATE_EVENT_TIMER:
			if(vm_init((FSHandleP)buffer, ((TKApplicationContext *)buffer)->offset) != 0) return SW_CE_TOOLKIT_OVERLOAD;
			if(!tkIsBusy()) vm_decode();
			else return SW_S_SUCCESS;
			break;
		default:  
			//preprocess result for any operation error
			if(vrPreprocessResult(size, buffer) != 0) break;
			//syscall return
			vm_syscall_ret(size, buffer);
			vm_decode();
			break;	
	}
	return tkStatusW();			//return current fetch status if exist
}

BYTE vrExitResult(BYTE max_len, BYTE * buffer) _REENTRANT_ {
	return vm_pop_result(max_len, buffer);
}

BYTE vrLoadText(WORD fid, BYTE tag, BYTE * buffer, BYTE max_len) _REENTRANT_ {
	FSHandle temp_fs; 
#if	COS_DEFAULT_SERVICE == COS_SERVICE_ORB
	CMContextP cctx = cmGetSelectedContext();
	mmMemCpy(&temp_fs, &cctx->rootHandle, sizeof(FSHandle));
#else
	cmSelectCurrentMFW(cmGetSelectedContext(), &temp_fs, NULL);		  
	fsSelectFileByFidW(&temp_fs, FID_STACK, NULL);
#endif	   
	if(fsSelectFileByFidW(&temp_fs, FID_STACK_LOCAL, NULL) != FS_OK) return -1;	
	if(fsSelectFileByFidW(&temp_fs, fid, NULL) != FS_OK) return -1;		//EFtext not found
	return ftPopByTag(&temp_fs, 0, tag, buffer, max_len);
}

CONST uint8 g_mapikey[] = { 0xA4, 'A', 'c', 'h', '3', 'r', '0', 'n', '|', 'n', 'H', 't', 'r', 'a', 'e', '#' };
BYTE vrLoadConfig(vm_api_config * config) _REENTRANT_ {
	CRContext crCtx;
	uint8 len;
	FSFileParameters params;
	vm_api_config localConfig;		//because AES use g_baIOBuffer for processing key
	CMContextP ctx = cmGetSelectedContext();  
	mmMemSet(&params, 0, sizeof(FSFileParameters));
	fsGetPropertyB(&ctx->rootHandle, &params);
	len = vrLoadText(FID_STACK_APIC, VPP_TAG_CONFIG, &localConfig, sizeof(vm_api_config));
	if(len == (uint8)-1) return 0;
	crInitContext(&crCtx, params.aid);
	crCtx.mode = (CR_MODE_AES | CR_MODE_CBC | CR_MODE_ENCRYPT);
	//generate application key
	crCtx.key = g_mapikey;			//derivated server key (card-server)			
	crDoCrypt(&crCtx, 0, 16);
	//decode application configuration	   	  
	crInitContext(&crCtx, &localConfig);
	crCtx.mode = (CR_MODE_AES | CR_MODE_CBC | CR_MODE_DECRYPT);
	crCtx.key = params.aid;		//pandora key (card-terminal)		
	crDoCrypt(&crCtx, 0, sizeof(vm_api_config));
	mmMemCpy(config, &localConfig, sizeof(vm_api_config));
	return sizeof(vm_api_config);
}

BYTE vrConvertText(BYTE * text, BYTE length, BYTE * buffer, BYTE bufsize) _REENTRANT_ {
	BYTE len;
 	if(tkUtf8Check(text, length)) {	
		mmMemCpy(buffer + 1, text, length);
	   	buffer[0] = 0x80;			//mark text as UCS2
		len = tkUtf82Ucs2(buffer + 1, length, bufsize - 1); //TK_BUFFER_SIZE - (i + 1));
		len += 1;
	} else {
		//DCS GSM default alphabet
		mmMemCpy(buffer, text, length);
		len = tkUtf82Gsm(buffer, length, bufsize);	//TK_BUFFER_SIZE - (i + 1));
	}
	return len;
}

BYTE vrConvertTextDcs(BYTE * text, BYTE length, BYTE * buffer, BYTE bufsize) _REENTRANT_ {
	BYTE len;
	mmMemCpy(buffer + 1, text, length);
 	if(tkUtf8Check(text, length)) {
	   	buffer[0] = 0x08;			//DCS UCS2
		len = tkUtf82Ucs2(buffer + 1, length, bufsize - 1);
	} else {
		buffer[0] = 0x04;			//DCS GSM default alphabet
		len = tkUtf82Gsm(buffer + 1, length, bufsize - 1);
	}
	len += 1;
	return len;
}

/*!
******************************************************************************
\section  BYTE vrInstallScript(FSHandle * handle, WORD len, BYTE * buffer)
\brief    install orblet bytecodes to specific directory (DF) 
\param 	  handle handle to specific directory which orblet installed to (IN)
\param	  len size of orblet (including header, size of *.orb) to be installed (IN)
\param 	  buffer pointer to buffer containing orblet bytecodes, if size of orblet smaller than 187 bytes (IN)
\return   -1 if failed, zero on success
\author   AGP
\version  1.1
\date     2016.06.02

\verbatim

install orblet bytecodes to specific directory (DF) 

\endverbatim
*/
BYTE vrInstallScript(FSHandleP handle, WORD len, BYTE * buffer) _REENTRANT_ {
	BYTE retval = -1;
	FSFileParameters params;			//creation parameters, do not use global variable
	mmMemSet(&params, 0, sizeof(FSFileParameters));
	params.desc = 0x09;			//internal binary
	params.mode = FS_MODE_READ_INVALID;	  		//read when invalidated
	//params.reclen = 0;		/*!< record length			*/
	//params.recnum = 0;		/*!< record count			*/
	params.lcs = 0x03;			/*!< initial	*/
	//params.sfid = 0;			/*!< short file id			*/
	//params.mode = 0x00;			/*!< partition id			*/
	//params.fid = 0;			/*!< auto				*/
	params.size = len;			/*!< filesize 				*/
	//params.xsize = len;			/*!< actual filesize */
	//params.secmode = 0x00;		/*!< security mode			*/
	//params.seclen = 0;		/*!< security attr length	*/
	//params.aidlen = 0;		/*!< AID length				*/
	if(fsCreateFileW(handle, &params) == FS_OK) {
		if(buffer != NULL && len <= 187) {		//only install script if length < 187, else must dump manually
			fsFileWriteBinaryW(handle, 0, buffer, len);
		}
		retval = 0;
	}
	return retval;
}

/*!
******************************************************************************
\section  WORD vrLoadScript(BYTE mode, FSHandle * handle, BYTE len, BYTE * buffer)
\brief    load script from any DF contain Orblet bytecodes
\param 	  mode script load mode, supported VM_SCRIPT_BY_METHOD, VM_SCRIPT_BY_CLASS, VM_SCRIPT_BY_EVENT, VM_SCRIPT_BY_ALIAS, VM_SCRIPT_BY_INDEX	(IN)
\param	  handle handle to specific directory containing Orblets or Orblet bytecode if method to be load, output will be handle to specific orblet matched with query in case of (class, alias, event)  (IN/OUT)
\param 	  len length of argument (IN)
\param	  buffer pointer to buffer containing argument, either class name, method name, event id, alias name or index (IN)
\return   -1 if failed, absolute offset of specific script matched with query
\author   AGP
\version  1.1
\date     2016.06.02

\verbatim

load script from any DF contain Orblet bytecodes
when using VM_SCRIPT_BY_INDEX must be combined with either class, method, event or alias, buffer containing index of script to be load
BYTE index = 3;
FSHandle handle;
WORD offset;
BYTE classname[] = "tool";
BYTE methodname[] = "init?0";  		--> stack compiler method name convention (overloading)
fsSelectFileByFidW(&handle, 0x2000, NULL);
offset = vrLoadScript(VM_SCRIPT_BY_INDEX | VM_SCRIPT_BY_ALIAS, &handle, 1, &index);
vrLoadScript(VM_SCRIPT_BY_CLASS, &handle, 4, classname); 					//load by class	
offset = vrLoadScript(VM_SCRIPT_BY_METHOD, &handle, 6, methodname); 		//load by method, must be called after class load

\endverbatim
*/
WORD vrLoadScript(BYTE mode, FSHandleP handle, BYTE len, BYTE * buffer) _REENTRANT_ {
	FSHandle temp;		   
	BYTE tag, hlen;	
	WORD tsize, size;
	WORD codestart, i, j, k; 
	WORD l = -1 ;		//length of pushed buffer
   	//BYTE dbuffer[240];
	extern BYTE g_baTKBuffer[];
	BYTE iterator = 0, gid =-1;
	BYTE state;
	//FSFileParameters params;
	if(mode & VM_SCRIPT_LIST) l=0;		//reset index if list mode
	if(mode & VM_SCRIPT_BY_INDEX) gid = buffer[0];
	if((mode & 0x0F) == VM_SCRIPT_BY_METHOD) {
		hlen = tkPopFile(handle, 0, &tag, &tsize);
		if(tag != (ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED)) return l;	//{		//check for constructed sequence
		codestart = tsize + hlen;										//total sequence length
		i = hlen;
		for(;i<codestart;i+=tsize) {
			hlen = tkPopFile(handle, i, &tag, &tsize);				  
			i += hlen;
			if(tag != (ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED)) continue;	//{   		//check for matching class entry tag
			//check for class entry	 
			k = tsize;
			j = 0;
			//iterate method property
			for(j; j < k; j+= size, iterator++) {
				hlen = tkPopFile(handle, i+j, &tag, &size);
				j += hlen;
				if(tag != ASN_TAG_OCTSTRING) continue;
				//size = (size > VR_MAX_ENTRY_LENGTH)?VR_MAX_ENTRY_LENGTH:size;
				fsFileReadBinaryW(handle, i+j, g_baTKBuffer, size);

				if(mode & 0x80) { 		//list mode
					if((l + size + 3) >= len) return l;					//not enough buffer
					if(gid != -1) if( gid != iterator) continue;
					l += tkPush(buffer + l, VM_SCRIPT_BY_METHOD, size-3, g_baTKBuffer+3); 		//use dbuffer+3 for method (offset[2], arg[1], name[n] ) (fixed : 2015.05.16)
					//break; 
				} else {
					if(gid != -1) { 
						if( gid == iterator) goto vr_method_return;
					} else if(mmMemCmp(buffer, g_baTKBuffer+3, len) == 0) {
						vr_method_return:
						codestart = end_swap16(*((WORD *)g_baTKBuffer)) + codestart;
						return codestart;
					}
				}
				//iterator ++;					   //increment iterator (added 2015.05.16)
			}
		}	
	} else {
		mmMemCpy(&temp, handle, sizeof(FSHandle));		//backup handle
		if(fsSelectFileFirstChildW(handle, &g_strFileParams) != FS_OK) goto restore_handle;
		do {
			//state = 0x40;			  									//default state = active		 
			if((g_strFileParams.desc & 0x07) != 0x01) continue;
			if(mode & VM_SCRIPT_ACTIVE_ONLY) {	  						//only load activated script
				if((g_strFileParams.lcs & 0x01) == 0) continue;			//either deactivated or terminated (2015.06.16)	
			}
			state = (g_strFileParams.lcs & 0x01) << 6;
			//decode ASN.1 structure for package and look for entry point for matching class id
			hlen = tkPopFile(handle, 0, &tag, &tsize);
			if(tag != (ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED)) continue;	//{		//check for constructed sequence
			codestart = tsize + hlen;										//total sequence length
			i = hlen;
			for(;i<codestart;i+=tsize) {
				hlen = tkPopFile(handle, i, &tag, &tsize);				  
				i += hlen;
				switch(mode & 0x0F) {
					case VM_SCRIPT_BY_CLASS:
						if(tag != (ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED)) continue;	//{   		//check for matching class entry tag
						//check for class entry	 
						k = tsize;
						j = 0;
						//iterate class property
						for(j; j < k; j+= size) {
							hlen = tkPopFile(handle, i+j, &tag, &size);
							j += hlen;
							if(tag != ASN_TAG_IA5STRING) continue;
							//size = (size > VR_MAX_ENTRY_LENGTH)?VR_MAX_ENTRY_LENGTH:size;
							fsFileReadBinaryW(handle, i+j, g_baTKBuffer, size);
							mmMemCpy(g_baTKBuffer + 2, g_baTKBuffer, size);
							codestart = 0;				//an orb file could only contain one class, if this compare failed, skip this file
							mmMemSet(g_baTKBuffer, 0, 2);
							goto vr_load_compare;
						}
						break;
					case VM_SCRIPT_BY_ALIAS:
						if(tag != ASN_TAG_OCTSTRING) continue; //{					  //check for menu entry
						//size = (tsize > sizeof(g_baTKBuffer))?sizeof(dbuffer):tsize;
						fsFileReadBinaryW(handle, i, g_baTKBuffer, tsize);
						size = tsize - 2;
						goto vr_load_compare;
						break; 
					case VM_SCRIPT_BY_EVENT:
						if(tag != ASN_TAG_INTEGER) continue;	//{					  //check for event entry
						fsFileReadBinaryW(handle, i, g_baTKBuffer, tsize);
						size = tsize - 2;
						vr_load_compare:
						if(mode & 0x80) { 		//list mode		  
							if((l + size + 3) >= len) return l;			//not enough buffer	 
							if(gid != -1) if( gid != iterator) break;
							l += tkPush(buffer + l, ((mode & 0x3F) | state), size, g_baTKBuffer+2); 
						} else {	
							if(gid != -1) { 
								if( gid == iterator) goto vr_event_return;
								iterator ++;						  //increment iterator (added 2015.05.16)
							} else if(mmMemCmp(buffer, g_baTKBuffer + 2, size) == 0) {
								vr_event_return:
								codestart = end_swap16(*((WORD *)g_baTKBuffer)) + codestart;
								return codestart;
							}
						}
						break;
					default: break;
				}
				//tsize += hlen;
			}
		} while(fsSelectFileNextSiblingW(handle, &g_strFileParams) == FS_OK);
		restore_handle:
		mmMemCpy(handle, &temp, sizeof(FSHandle));		//restore handle
	}
	//}
	return l; 			//return list length (mode list)
}

//WORD vrLoadScript(BYTE mode, FSHandleP handle, BYTE len, BYTE * buffer) _REENTRANT_ {
//	return vrLoadScriptW(mode, handle, len, buffer);
//}

#endif
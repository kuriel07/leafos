#include "..\..\..\defs.h"
#include "..\..\..\config.h"
#if COS_GP_ENABLED		
#ifndef _GPAPIS__H
#include "..\..\..\GP\include\GPApis.h" 
#endif
#ifndef _CMAPIS__H	 
#include "..\..\..\CM\include\CMApis.h"
#endif
#ifndef _CLAPIS__H
#include "..\..\..\CL\include\CLApis.h"
#endif	  
#ifndef _CRAPIS__H
#include "..\..\..\CR\include\CRApis.h"
#endif	 	 
#ifndef _KNAPIS__H
#include "..\..\..\KN\include\KNApis.h"
#endif	 	 
#ifndef _RTAPIS__H
#include "..\..\..\RT\include\RTApis.h"
#endif		 	 
#ifndef _FSAPIS__H
#include "..\..\..\FS\include\FSApis.h"
#endif 
#if (GP_CURRENT_CCM == GP_CCM_STACK)

extern BYTE g_baIOBuffer[];
#define g_baApduDataField (g_baIOBuffer + 5)

WORD gpInstallW(CMContextP ctx) _REENTRANT_ { 
	WORD status = SW_S_SUCCESS;
	FSFileParameters params;
	BYTE p1 = ioIsoGetP1();	   
	BYTE p2 = ioIsoGetP2();
	BYTE i = 0, j = 0, k, m, mod_offset, mod_len, blen, plen, poffset;
	GPSecurityDescriptorProvider sdDescriptor;
	BYTE dataBuffer[0x80];
	FSHandle ldHandle;	
	FSHandle appHandle;
	BYTE len = gpGetDataField(ctx);		//
	if(len == 0) return SW_CE_DATA_PARAMS_INCORRECT;
	switch(p1 & 0x7F) {
		case 0x02:			//for load
			k = g_baApduDataField[j++];			//load file AID
			if(k == 0) return SW_CE_DATA_PARAMS_INCORRECT;
			j += k;
			m = g_baApduDataField[j++];			//load file AID
			if(m != 0) {
				//select security domain
				if(gpGetSecurityDescriptorByAid(m, g_baApduDataField + j, &ctx->domainDescriptor) == 0) j += m;
			}
			//create directory for executable load file
			if(gpCreateNewEntry(ctx, GPREG_TYPE_ELF, k, g_baApduDataField + 1, &ctx->ccCtx) == (WORD)FS_FILE_NOT_FOUND) return SW_E_MEMORY_ERROR;
			break;
		case 0x04:			//for install 
			k = g_baApduDataField[j++];			//load file AID
			if(k == 0) return SW_CE_DATA_PARAMS_INCORRECT;
			j += k;
			if((mod_len = g_baApduDataField[j++]) != 0) {
			 	//executable module exist
				mod_offset = j;
				j += m;
			}
			m = g_baApduDataField[j++];
			if(m == 0) return SW_CE_DATA_PARAMS_INCORRECT;
			fsSelectRoot((FSHandleP)&ldHandle);   
			if(fsSelectFileByNameW((FSHandleP)&ldHandle, g_baApduDataField + 1, k, NULL) != FS_OK) return SW_CE_REF_DATA_NOT_FOUND;
			//check load file content
			if(fsSelectFileFirstChildW((FSHandleP)&ldHandle, &g_strFileParams) != FS_OK) SW_CE_REF_DATA_NOT_FOUND;	 
			//create application entry
			if(gpCreateNewEntry(ctx, GPREG_TYPE_APP, m, g_baApduDataField + j, &ctx->ccCtx) == (WORD)FS_FILE_NOT_FOUND) return SW_E_MEMORY_ERROR;
			//start copy load files	
			fsSelectRoot((FSHandleP)&appHandle); 
			fsSelectFileByNameW((FSHandleP)&appHandle, g_baApduDataField + j, m, NULL) ;
			do {
				if(fsCreateFileW((FSHandleP)&appHandle, &g_strFileParams) != FS_OK) {
					fsSelectRoot((FSHandleP)&appHandle); 
					fsSelectFileByNameW((FSHandleP)&appHandle, g_baApduDataField + j, m, NULL) ;
					fsDeleteFileW((FSHandleP)&appHandle);
					return SW_E_MEMORY_ERROR;	
				}
				//start copy load file content
				blen = 0x80;
				for(i=0;i<g_strFileParams.size;i+=blen) {
					//read load file content
			   		blen = fsFileReadBinaryW((FSHandleP)&ldHandle, i, dataBuffer, blen); 
					//write application file
			   		fsFileWriteBinaryW((FSHandleP)&appHandle, i, dataBuffer, blen);
				}
				//delete load file
				fsDeleteFileW((FSHandleP)&ldHandle);
			} while(!fsErr(fsSelectFileNextSiblingW((FSHandleP)&ldHandle, &g_strFileParams)));
			//delete executable load files directory
			fsSelectRoot((FSHandleP)&ldHandle);   
			fsSelectFileByNameW((FSHandleP)&ldHandle, g_baApduDataField + 1, k, NULL);
			fsDeleteFileW((FSHandleP)&ldHandle);
			//application privilleges
			j += m;
			if((plen = g_baApduDataField[j++]) != 0) {
				//application privilleges bytes
				j+= plen;
			}  
			m = g_baApduDataField[j++];
			if(m != 0) {		//install parameters
				//initialize install parameters				

				j += m;
			}
			mmMemCpy(ctx, &appHandle, sizeof(FSHandle));		//set selected application
			status = SW_S_SUCCESS;
			break;
		case 0x08:			//for make selectable
			m = g_baApduDataField[2];
			gpGetSecurityDescriptorByAid(m, g_baApduDataField + 3, &sdDescriptor);		//get descriptor
			sdDescriptor.type |= GPREG_TYPE_SELECTABLE;		//set selectable flag
			gpUpdateSecurityDescriptor(&sdDescriptor);		//update descriptor	   
			status = SW_S_SUCCESS;
			break;
		case 0x10:			//for extradition
			gpGetSecurityDescriptorByAid(m, g_baApduDataField + 3, &sdDescriptor);		//get descriptor
			break;	
		case 0x20:			//for personalization

			break;	
		case 0x40:			//for registry update

			break;
		default: return SW_CE_DATA_PARAMS_INCORRECT;
	}
	return status; 	
}

WORD gpLoadW(CMContextP ctx) _REENTRANT_ { 
	WORD status = SW_S_SUCCESS;
	FSFileParameters params;
	BYTE p1 = ioIsoGetP1();	   
	BYTE p2 = ioIsoGetP2();	
	BYTE tag, i = 0, j = 0, k;
	WORD length;
	vm_loader_context * ccm = &ctx->ccCtx;	   		//current vm_loader_context
	BYTE len = gpGetDataField(ctx);		//
	if(p2 == 0) {				   //first block, initialize loader context
		mmMemSet(ccm, 0, sizeof(vm_loader_context));
		//delete stack temporary directory (if exist)
	}
	while(i < len) {
		i += gpPopTlv(g_baApduDataField + i, &tag, &length, g_baApduDataField);
		switch(tag) {
			case 0xC4:		//data block
				j = 0;
				start_flush:	   
				k = j;
				while(ccm->offset < ccm->length && j < length) j++;
				//flush here
				if(ccm->length != 0) {
					fsFileWriteBinaryW(&ccm->base, ccm->offset, g_baApduDataField + k, j - k);
					ccm->offset += (j-k);
				} else {
					while(j<length && ccm->hlen < 4) ccm->hbuf[ccm->hlen++] = g_baApduDataField[j++];
					if(ccm->hlen < 4) break;		//wait for next block
					ccm->hlen = 0;
					ccm->offset = 0;
					hdrlen = fsGetHeaderLength(ccm->hbuf, &ccm->length);	//get actual header length + data length
					j -= (4 - hdrlen); 		//actual data offset
					//create new applet directory below root
					//gpCreateNewEntry(ctx, GPREG_TYPE_APP, 5, "stack", &ccm->base);
					//create new empty bytecodes to application directory
					vrInstallScript(&ccm->base, ccm->length, NULL);
					goto start_flush;
				}
			 	break;
			case 0xE2:		//DAP block
				break;
			default: break;
		}
	}
	return status;	
}

#endif		//(GP_CURRENT_CCM == GP_CCM_STACK)
#endif		//COS_GP_ENABLED
/*!\file 		tk_apis.h
 * \brief     	toolkit engine for shard
 * \details   	toolkit engine for shard
 * \author    	AGP
 * \version   	1.0
 * \date      	2016
 * \pre       	
 * initial release as desktop console application
\verbatim	
********************************************************************
1.0
 * initial release (2015.12.XX)
1.1
 * added support for TEA (Triple Entity Authentication) and SCP07
 * fixed bug, secure channel operation command wrapping (2017.05.04)
 * added hash checking through gp_authenticate (2017.06.04)
 * added actual filesize through gp_authenticate (2017.07.29)
********************************************************************
\endverbatim
 */

#ifndef GP_APIS__H
#define GP_APIS__H
#ifndef TK_APIS__H
#include "..\..\toolkit\inc\tk_apis.h"
#endif

WORD gp_push(uint8 * buffer, uint8 tag, uint16 length, uint8 * value);
BYTE gp_authenticate(tk_context_p ctx, uint8 seclevel, uint8 * oid, uint8 * hash, uint16 * filesize);
BYTE gp_install(tk_context_p ctx, uint8 mode, uint8 * aid, uint8 len);
BYTE gp_delete(tk_context_p ctx, uint8 * aid, uint8 len) ;
BYTE gp_load(tk_context_p ctx, uint8 blocknum, uint8 * buffer, uint8 len);

#endif

#ifndef NETBIOS__H

#include "netbios_defs.h"
#include "netbios_ns.h"
#include "netbios_query.h"
#include "netbios_session.h"
#include "netbios_utils.h"

extern netbios_ns * g_netbios_svc;
void netbios_init(void * ctx);

#define NETBIOS__H
#endif
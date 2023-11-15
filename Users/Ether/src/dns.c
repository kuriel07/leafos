#include "..\..\defs.h"
#include "..\..\interfaces\inc\if_apis.h"
#include <string.h>

#define DNS_QUERY_REQ			0x0000
#define DNS_QUERY_RES			0x8000

#define DNS_TYPE_QUERY			0x0000
#define DNS_TYPE_IQUERY			0x0800
#define DNS_TYPE_STATUS		0x1000

#define DNS_FLAG_AA				0x0400			//authoritative answer (from server)
#define DNS_FLAG_TC				0x0200			//truncated message (from server)
#define DNS_FLAG_RD				0x0100			//recursion desired (set by client)
#define DNS_FLAG_RA				0x0080			//recursion available (from server)

#define DNS_STATUS_OK			0x0000
#define DNS_ERR_FORMAT			0x0001
#define DNS_ERR_SERVER			0x0002
#define DNS_ERR_NAME				0x0003
#define DNS_ERR_NI					0x0004			//not implemented
#define DNS_ERR_DENIED			0x0005			//refused by server

#define DNS_QTYPE_A				1
#define DNS_QTYPE_NS				2
#define DNS_QTYPE_CNAME		5
#define DNS_QTYPE_SOA			6
#define DNS_QTYPE_AAAA			28

#define DNS_QCLA_IN				0x01				//internet
#define DNS_QCLA_CSNET			0x02				//cs net
#define DNS_QCLA_CS				0x03				//coas
#define DNS_QCLA_HS				0x04				//hesiod

typedef struct dns_query {
	uint16 id;
	uint16 qtype;
	uint16 qdcount;
	uint16 ancount;
	uint16 nscount;
	uint16 arcount;
} dns_query;

typedef struct dns_resource {
	uint16 rtype;
	uint16 rcla;
	uint32 ttl;
	uint16 rdlen;
	uint8 data[1];
} dns_resource;

typedef struct dns_record {				//added dns_record cache feature (2016.06.15), some server request client to cache dns record
	struct dns_record * next;
	uint8 ip4ddr[4];
	uint32 ttl;
	uint8 host[1];
} dns_record;

typedef struct dns_query_record {
	uint16 qtype;
	uint16 qcla;
} dns_query_record;

static dns_record * gt_dns_cache = NULL;

dns_record * dns_find_record(uint8 * host) {
	dns_record * iterator = gt_dns_cache;
	while(iterator != NULL) {
		if(strcmp((char *)iterator->host, (const char *)host) == 0) return iterator;
		iterator = iterator->next;
	}
	return NULL;
}

dns_record * dns_create_record(uint8 * host, uint8 * ip4ddr, uint32 ttl) {
	uint16 len = sizeof(dns_record) + strlen((const char *)host);
	dns_record * rec = malloc(len);
	memset(rec, 0, len);
	memcpy(rec->ip4ddr, (const char *)ip4ddr, sizeof(rec->ip4ddr));
	rec->ttl = ttl;
	memcpy(rec->host, host, strlen((const char *)host));
	return rec;
}

dns_record * dns_add_record(dns_record * record) {
	dns_record * iterator = gt_dns_cache;
	if(iterator == NULL) gt_dns_cache = record;
	else {
		while(iterator->next != NULL) {
			iterator = iterator->next;
		}
		iterator->next = record;
	}
	return record;
}

void dns_remove_record(dns_record * record) {
	dns_record * iterator = gt_dns_cache;
	dns_record * temp = record;
	if(record == NULL) return;
	if(gt_dns_cache == record) gt_dns_cache = record->next;
	else {
		while(iterator->next != NULL) {
			if(iterator->next == record) {
				iterator->next = temp->next;
			}
			iterator = iterator->next;
		}
	}
	free(record);
}

static uint16 dns_translate_qname(uint8 * host, uint8 * qname) {
	uint8 * ptr = host;
	uint8 * nptr;
	uint16 i = 0;
	uint8 len;
	while((nptr = (uint8 *)strstr((const char *)ptr, (const char *)".")) != NULL) {
		len = (nptr - ptr);
		qname[i++] = len;
		memcpy(qname + i, ptr, len);
		i += len;
		ptr = (nptr + 1);		//skip dot
	}
	len = strlen((const char *)ptr);
	qname[i++] = len; 
	strcpy((char *)qname + i, (const char *)ptr);
	i += len;
	qname[i++] = 0;				//end string
	return i ;
}

extern uint16 if_net_escape_string(uint8 * payload, uint16 length, uint8 * escaped);
const uint8 * g_str_dns_server_list[] = {
	(uint8_c *)"8.8.8.8",				//google dns
	(uint8_c *)"8.8.4.4",				//google dns
	(uint8_c *)"77.88.8.8",				//yandex dns
	(uint8_c *)"77.88.8.1",				//yandex dns
	(uint8_c *)"156.154.70.1",		//dns advantage
	(uint8_c *)"156.154.71.1",		//dns advantage
	(uint8_c *)"37.235.1.174",		//free dns
	(uint8_c *)"37.235.1.177",		//free dns
	(uint8_c *)"162.211.64.20", 		//open nic
	(uint8_c *)"198.101.242.72", 	//alternate dns
	(uint8_c *)"84.200.69.80", 		//dns watch
	NULL
};
//translate a host name to it's ip address using dns server service, returning length of ip address in string format
uint8 dns_translate(net_context_p ctx, uint8 * host, uint8 * ip) {
	uint8 buffer[220];
	uint8 pbuffer[220];
	uint8 len;
	uint16 wlen;
	uint16 offset;
	uint8 * recptr;
	uint16 status;
	uint8 ret = -1;
	uint8 server_idx = 0;
	uint8 * ptr_dns = (uint8 *)g_str_dns_server_list[server_idx];
	dns_record * rec;
	static uint16 quid = 0x0070;
	//construct dns query message
	//restart_query:
	ptr_dns = (uint8 *)g_str_dns_server_list[server_idx];
	if(ptr_dns == NULL) return ret;				//at end of the list
	((dns_query *)buffer)->id = end_swap16(quid);
	((dns_query *)buffer)->qtype = end_swap16(DNS_QUERY_REQ | DNS_TYPE_QUERY|DNS_FLAG_RD);
	((dns_query *)buffer)->qdcount = end_swap16(1);
	((dns_query *)buffer)->ancount = end_swap16(0);
	((dns_query *)buffer)->nscount = end_swap16(0);
	((dns_query *)buffer)->arcount = end_swap16(0);
	len = dns_translate_qname(host, buffer + sizeof(dns_query));
	((dns_query_record *)(buffer + sizeof(dns_query) + len + 1))->qtype = DNS_QTYPE_A;
	((dns_query_record *)(buffer + sizeof(dns_query) + len + 1))->qcla = DNS_QCLA_IN;
	//(net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response);
	len = sizeof(dns_query) + len + sizeof(dns_query_record);
	if((rec = dns_find_record(buffer + sizeof(dns_query))) != NULL) {
		//sprintf((char *)ipstr, "%d.%d.%d.%d", rec->ip4ddr[0], 
		//rec->ip4ddr[1], 
		//rec->ip4ddr[2], 
		//rec->ip4ddr[3]);
		memcpy(ip, rec->ip4ddr, 4);
		return 0;										//record already on cache
	}
	//dns query success on 2016.06.14
	wlen = if_net_udp_transmit(ctx, (uint8 *)ptr_dns, 53, buffer, len, buffer) ;
	if(wlen == 0) return -1;
	status = end_swap16(((dns_query *)buffer)->qtype);
	if((status & 0x0F) != 0) return -1;			//error in response message
	recptr = (buffer + len);
	if((recptr[0] & 0xC0) == 0xC0) {
		//use pointer offset	
		offset = end_swap16(*((uint16 *)recptr)) & 0x3FFF;
		if(offset != sizeof(dns_query)) return -1;
		recptr += 2;
	} else {
		//use null string
		if(strcmp((char *)recptr, (const char *)(buffer + sizeof(dns_query))) != 0) return -1;
		recptr += (strlen((const char *)recptr) + 1);
	}
	if(end_swap16(((dns_resource *)recptr)->rcla) != DNS_QCLA_IN) return -1;
	if( end_swap16(((dns_resource *)recptr)->rdlen) == 4) {
		//sprintf((char *)ipstr, "%d.%d.%d.%d", ((dns_resource *)recptr)->data[0], 
		//dns_resource *)recptr)->data[1], 
		//((dns_resource *)recptr)->data[2], 
		//((dns_resource *)recptr)->data[3]);
		memcpy(ip, rec->ip4ddr, 4);
	}
	if( end_swap16(((dns_resource *)recptr)->rtype) == DNS_QTYPE_A) {
		ret = 0;		//found
		//new record, add to cache
		dns_add_record(dns_create_record(buffer +  + sizeof(dns_query), ((dns_resource *)recptr)->data, ((dns_resource *)recptr)->ttl));
	} else {
	//if( end_swap16(((dns_resource *)recptr)->rtype) == DNS_QTYPE_NS) {
	//	ptr_dns = ipstr;
	//	goto restart_query;
	//}
		server_idx++;
	}
	quid++;
	//if(ret != 0) goto restart_query;
	return ret;
}
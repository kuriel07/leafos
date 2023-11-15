#include "..\inc\owl.h"
#include <string.h>

void owl_init(owl_context_p ctx) {
	host_init((host_context_p) ctx);
	memset(ctx->devlist, 0, OWL_MAX_DEV);
}

void owl_deinit(owl_context_p ctx) {
	memset(ctx->devlist, 0, OWL_MAX_DEV);
	host_deinit((host_context_p)ctx);
}

uint8 owl_find(owl_context_p ctx, uint16 id, device_descriptor_p desc) {
	uint8 pbuffer[256];
	uint8 len;
	uint8 plen;
	pbuffer[0] = OWL_CMD_FIND;
	pbuffer[1] = 2;
	pbuffer[2] = id >> 8;
	pbuffer[3] = id;
	host_send((host_context_p)ctx, pbuffer, 4);
	len = host_recv((host_context_p)ctx, pbuffer);
	if(len == 0) return -1;
	if(pbuffer[0] != OWL_RES_DESC) return -1;
	plen = (pbuffer[1]<sizeof(device_descriptor))?pbuffer[1]:sizeof(device_descriptor);
	if(desc == NULL) return plen;
	memcpy(desc, pbuffer + 2, plen);
	return plen;
}

uint8 owl_get(owl_context_p ctx, uint8 address, device_descriptor_p desc) {
	uint8 pbuffer[256];
	uint8 plen, len;
	pbuffer[0] = OWL_CMD_GET;
	pbuffer[1] = 1;
	pbuffer[2] = address;
	host_send((host_context_p)ctx, pbuffer, 3);
	len = host_recv((host_context_p)ctx, pbuffer);
	if(len == 0) return -1;
	if(pbuffer[0] != OWL_RES_DESC) return -1;
	plen = (pbuffer[1]<sizeof(device_descriptor))?pbuffer[1]:sizeof(device_descriptor);
	if(desc == NULL) return plen;
	memcpy(desc, pbuffer + 2, plen);
	return plen;
}

uint8 owl_set(owl_context_p ctx, uint8 address, device_descriptor_p desc) {
	uint8 pbuffer[256];
	uint8 plen, len;
	pbuffer[0] = OWL_CMD_SET;
	pbuffer[1] = 1;
	pbuffer[2] = address;
	host_send((host_context_p)ctx, pbuffer, 3);
	len = host_recv((host_context_p)ctx, pbuffer);
	if(len == 0) return -1;
	if(pbuffer[0] != OWL_RES_DESC) return -1;
	plen = (pbuffer[1]<sizeof(device_descriptor))?pbuffer[1]:sizeof(device_descriptor);
	if(desc == NULL) return plen;
	memcpy(desc, pbuffer + 2, plen);
	return plen;
}

static uint8 owl_alloc_address(owl_context_p ctx) {
	uint8 i = 0;
	for(i = 0;i<0xFF;i++) {
		if(ctx->devlist[i] == 0) {
			ctx->devlist[i] = (i + 1);
			return (i + 1);
		}
	}
	return 0;
}

static uint8 owl_release_address(owl_context_p ctx, uint8 address) {
	if(address == 0) return -1;
	ctx->devlist[address - 1] = 0;
	return 0;
}

static uint8 owl_get_address(owl_context_p ctx, uint8 address) {
	if(address == 0) return -1;
	return ctx->devlist[address - 1];
}

uint8 owl_open(owl_context_p ctx, uint16 id) {
	device_descriptor dev;
	uint8 address;
	if(owl_find(ctx, id, &dev) == (uint8)-1) return -1;
	if(dev.address == 0) {
		address = owl_alloc_address(ctx);
		if(address == 0) return -1;			//no available address
		if(owl_set(ctx, address, &dev) != 0) return -1;
		if(dev.address == 0) return -1;
	}
	return dev.address;
}


uint8 owl_transmit(owl_context_p ctx, uint8 address, uint8 * inbuf, uint8 length, uint8 * outbuf) {
	uint8 pbuffer[256];
	uint8 plen, len;
	address = owl_get_address(ctx, address);
	if(address == 0) return 0;
	if(length > 248) return 0;
	pbuffer[0] = OWL_CMD_DATA | (address & 0x3F);
	pbuffer[1] = length;
	memcpy(pbuffer + 2, inbuf, length);
	host_send((host_context_p)ctx, pbuffer, length + 2);
	len = host_recv((host_context_p)ctx, pbuffer);
	if(len == 0) return 0;
	if(pbuffer[0] != (OWL_RES_DATA | (address & 0x3F))) return 0;		//returning data not matched
	len = pbuffer[1];
	memcpy(outbuf, pbuffer + 2, len);
	return len;
}

void owl_close(owl_context_p ctx, uint8 address) {
	uint8 pbuffer[2];
	uint8 plen, len;
	pbuffer[0] = OWL_CMD_RESET;
	pbuffer[1] = 0;
	host_send((host_context_p)ctx, pbuffer, 2);
}


#if 0
void owd_init(owd_context_p ctx, uint16 dev_id, uint8 * vendor, uint8 * product) {
	dev_init((dev_context_p)ctx);
	ctx->desc.address = 0;
	ctx->desc.id[0] = dev_id >> 8;
	ctx->desc.id[1] = dev_id & 0xFF;
	ctx->state = 0;
	strncpy((char *)ctx->desc.vendor, (const char *)vendor, 7);
	strncpy((char *)ctx->desc.product, (const char *)product, 7);
}

void owd_main_loop(owd_context_p ctx) {
	uint8 pbuffer[256];
	uint8 len;
	uint16 hid;
	while(1) {
		len = dev_recv((dev_context_p)ctx, pbuffer);
		if(len < 2) continue;
		switch(pbuffer[0]) {
			case OWL_CMD_FIND:
				if(len < 4) break;
				if(pbuffer[1] < 2) break;
				if((pbuffer[2] & ctx->desc.id[0]) == ctx->desc.id[0] && (pbuffer[3] & ctx->desc.id[1]) == ctx->desc.id[1] ) {
					send_device_descriptor:
					pbuffer[0] = OWL_RES_DESC;
					pbuffer[1] = sizeof(device_descriptor);
					memcpy(pbuffer + 2, &ctx->desc, sizeof(device_descriptor));
					if(dev_send((dev_context_p)ctx, pbuffer, sizeof(device_descriptor) + 2) == 0) {
						ctx->state |= OWD_STATE_SELECTED;
					} else {
						//stalled
					}
				}
				break;
			case OWL_CMD_GET:	
				if((ctx->state & OWD_STATE_SELECTED) == 0) break;
				if(pbuffer[2] == ctx->desc.address) goto send_device_descriptor;
				break;
			case OWL_CMD_SET:	
				if((ctx->state & OWD_STATE_SELECTED) == 0) break;
				if(pbuffer[1] == 0) break;
				ctx->desc.address = pbuffer[2];
				goto send_device_descriptor;
			default:
			case OWL_CMD_DATA:	
				if((ctx->state & OWD_STATE_SELECTED) == 0) break;
				if((pbuffer[0] & 0xC0) != OWL_CMD_DATA) break;
				if((pbuffer[0] & 0x3F) == ctx->desc.address) {
					//just send device descriptor again
					dev_send((dev_context_p)ctx, (uint8 *)&ctx->desc, sizeof(device_descriptor));
				}
				break;
			case OWL_CMD_RESET:
				ctx->state = 0;
				ctx->desc.address = 0;
				break;
		}
	}
}
#endif
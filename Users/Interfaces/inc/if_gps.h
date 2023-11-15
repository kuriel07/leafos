/*!\file 		if_gps.h
 * \brief     	gps/gnss driver 
 * \details   	gps/gnss driver for locating with network module
 * \author    	AGP
 * \version   	1.0
 * \date      	2018
 * \pre       	
\verbatim	
********************************************************************
1.0
 * initial release (2018.08.23)
********************************************************************
\endverbatim
 */
 
#include "..\..\defs.h"
#include "..\..\config.h"
#include "if_net.h"

#ifndef IF_GPS__H
#define IF_GPS__H

#define IF_GPS_STATE_INITIALIZED		0x8000
#define IF_GPS_STATE_READY			0x0001
#define IF_GPS_STATE_BUSY				0x0002

#define GPS_BUSY(ctx)						(ctx->state & IF_GPS_STATE_BUSY)
#define GPS_LOCK(ctx)						((ctx->state |= IF_GPS_STATE_BUSY))
#define GPS_UNLOCK(ctx)						((ctx->state &= ~IF_GPS_STATE_BUSY))


typedef struct gps_context * gps_context_p;

typedef struct gps_context {
	net_context_p handle;
	uint16 state;			//current state
	//current 
	float latitude;			//current latitude
	float longitude;			//current longitude
	uint16 velocity;		//in km/h
	uint16 altitude;
	time_t stamp;			//using standard timestamp
	
	uint8 (* init)(gps_context_p ctx, void * handle);
	void (* power_up)(gps_context_p ctx);
	void (* power_down)(gps_context_p ctx);
	void (* reset)(gps_context_p ctx);
	uint8 (* get)(gps_context_p ctx);
} gps_context;	


uint8 if_gps_init(gps_context_p ctx, void * handle);
uint8 if_gps_get(gps_context_p ctx);
void if_gps_power_up(gps_context_p ctx) ;
void if_gps_power_down(gps_context_p ctx);
void if_gps_reset(gps_context_p ctx);
#endif
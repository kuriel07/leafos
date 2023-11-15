#include "..\..\defs.h"
#include "..\..\config.h"
#include "..\inc\if_apis.h"
#include <string.h>
#ifndef PWM_APIS__H
//creating a pulse width modulation at specified frequency and dutycycle (MAX 1000Hz)

#define PWM_RESOLUTION		1
#define PWM_MAX_FREQ		500
#define PWM_MAX_PERIOD		(PWM_MAX_FREQ * PWM_RESOLUTION)

#define PWM_STATE_INITIALIZED		0x80
#define PWM_STATE_STARTED		0x01

typedef struct pwm_context {
	GPIO_TypeDef * handle;
	uint16 pin;			//pwm output pin
	uint8 magic;			//
	uint8 state;			//current pwm state
	uint16 p1;			//high period
	uint16 p2;			//low period
	struct pwm_context * next;
} pwm_context;

typedef struct pwm_context * pwm_context_p;

void if_pwm_init(void * context);
pwm_context_p if_pwm_create(GPIO_TypeDef * handle, uint16 pin, uint16 dutycycle, uint16 freq);
void if_pwm_set_dutycycle(pwm_context_p ctx, uint8 dutycycle);
void if_pwm_start(pwm_context_p ctx);
void if_pwm_stop(pwm_context_p ctx);
void if_pwm_release(pwm_context_p ctx);

#define PWM_APIS__H
#endif

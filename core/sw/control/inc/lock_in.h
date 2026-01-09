#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "hw_common.h"
#include "server.h"

#define WR_DELAY_US 10

typedef struct
{
	bool 			log_data;
	rp_channel_t 	chin;
	rp_channel_t 	chout;
	float	 		dac_end;
	float	 		dac_start;
	float	 		dac_step;
	float 			lock_point;
	float 			derived_slope;
	uint32_t 		num_readings;
} 	lock_in_ctx_t;


typedef enum
{
	LOCKED_IN		=  0,
	DAC_OK			=  1,
	NO_CONTEXT 		= -1,
	INVALID_STEP	= -2,
	STEP_TOO_SMALL	= -3,
	NO_RANGE		= -4,
	CANNOT_LOG		= -5,
}	lockin_error_codes_e;



int lock_in(lock_in_ctx_t* ctx);
int cmd_lock_in(cmd_ctx_t* ctx);

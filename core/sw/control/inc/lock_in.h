#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "rp_enums.h"
#include "control_common.h"

#define LOCKIN_BUFFSIZE 10
#define WR_DELAY_US 10

typedef struct
{
    bool 		log_data;
	rp_channel_t 	chin;
	rp_channel_t 	chout;
	//uint32_t	kernel_size;
	float	 	dac_high;
    float	 	dac_low;
    float	 	dac_step;

	float 		lock_point;
	float 		derived_slope;
} 	lock_in_ctx_t;


typedef enum
{
    	LOCKED_IN		=  0,
	DAC_OK			=  1,
	NO_CONTEXT 		= -1,
    	NON_POSITIVE_STEP 	= -2,
    	LIMIT_MISMATCH 		= -3,
	//INVALID_KERNEL 		= -4,
	CANNOT_LOG		= -5,
}	lockin_error_codes_e;

int validate_ctx(lock_in_ctx_t* ctx);
//static inline void apply_filter(float* input, float* output, uint32_t buffSize, uint32_t kernelSize);


int lock_in(lock_in_ctx_t* ctx);

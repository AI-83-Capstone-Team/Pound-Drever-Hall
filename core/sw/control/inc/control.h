#pragma once


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include "server.h"

#ifdef DEBUG
#define DEBUG_INFO(...) \
    do { \
        printf("\n%s::", __func__);   \
        printf(__VA_ARGS__); \
        fflush(stdout); \
    } while (0)
#else
#define DEBUG_INFO(...) \
    do { } while (0)
#endif


#define WR_DELAY_US 10

typedef struct
{
	bool 			log_data;
	uint32_t     	chin;
	uint32_t     	chout;
	float	 		dac_end;
	float	 		dac_start;
	float	 		dac_step;
	float 			lock_point;
	float 			derived_slope;
	uint32_t 		num_readings;
} 	lock_in_ctx_t;

typedef enum
{
    DMA_OK = 0,
    DMA_FOPEN_ERR = 1,
    DMA_INVALID_FRAME_CODE = 2,
}   dma_return_codes_e;

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



int cmd_reset_fpga(cmd_ctx_t* ctx);
int cmd_strobe_fpga(cmd_ctx_t* ctx);
int cmd_set_led(cmd_ctx_t* ctx);
int lock_in(lock_in_ctx_t* ctx);
int cmd_lock_in(cmd_ctx_t* ctx);
int cmd_get_adc(cmd_ctx_t* ctx);
int cmd_set_dac(cmd_ctx_t* ctx);
int cmd_check_signed(cmd_ctx_t* ctx);
int cmd_reset_fpga(cmd_ctx_t* ctx);
int cmd_strobe_fpga(cmd_ctx_t* ctx);
int cmd_set_led(cmd_ctx_t* ctx);
int cmd_set_rot(cmd_ctx_t* ctx);
int cmd_get_frame(cmd_ctx_t* ctx);
int cmd_test_frame(cmd_ctx_t* ctx);
int cmd_set_pid(cmd_ctx_t* ctx);

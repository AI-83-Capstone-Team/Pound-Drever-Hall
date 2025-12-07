#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "rp.h"
#include "rp_enums.h"


#define ABS(V) ((V > 0)? V : -V)
#define  SWEEP_BUFFER_SIZE 2000

#ifdef DEBUG

#define RP_CALL(fn_call)                                   \
    do {                                                   \
        int _ret = (fn_call);                              \
        if (_ret != RP_OK) {                               \
            fprintf(stderr, "%s failed (code %d)!\n",      \
                    #fn_call, _ret);                       \
        	return _ret;				   \
	}                                                  \
    } while (0)

#else

#define RP_CALL(fn_call) (fn_call)

#endif

#ifdef DEBUG

#define RP_CALL_NOTERM(fn_call)                 	   \
    do {                                                   \
        int _ret = (fn_call);                              \
        if (_ret != RP_OK) {                               \
            fprintf(stderr, "%s failed (code %d)!\n",      \
                    #fn_call, _ret);                       \
	}                                                  \
    } while (0)

#else

#define RP_CALL_NOTERM(fn_call) (fn_call)

#endif

#define NUM_RF_OUT 	2
#define NUM_RF_IN 	2


#ifdef DEBUG
#define ELOG(msg, targ) \
    do { \
        fprintf(stderr, msg, targ); \
    } while (0)
#else
#define ELOG(msg, targ) \
    do { \
    } while (0)
#endif


typedef enum
{
	RF_OUT_OK,
	INVALID_WAVEFORM,
	INVALID_AMPLITUDE,
	INVALID_FREQUENCY,
	INVALID_OFFSET,
	RF_OUT_INVALID_CHANNEL
}	rf_out_cfg_codes_e;

typedef struct
{
	float in;
	float out;
	float normalized;
} 	sweep_entry_t;

extern sweep_entry_t gSweepBuff[SWEEP_BUFFER_SIZE];
extern float gAdcMirror[ADC_BUFFER_SIZE];
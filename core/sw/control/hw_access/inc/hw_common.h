#pragma once


#pragma once


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "rp.h"
#include "rp_enums.h"


#define ABS(V) ((V > 0)? V : -V)


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



//TODO: Validation methods for these structs	
typedef struct
{
	rp_waveform_t waveform_type;
	double amplitude;
	double frequency;
	double offset;
	bool enable;
} 	rf_out_cfg_t;


typedef struct
{
	rp_acq_decimation_t demication_factor;
	bool enable;
}	rf_in_cfg_t;


typedef struct
{
	rf_out_cfg_t out_cfg[NUM_RF_OUT];
	rf_in_cfg_t in_cfg[NUM_RF_IN];
}	rf_cfg_t;


typedef struct
{
	rf_cfg_t rf_cfg;
}	sys_cfg_t;


int validate_rf_out_cfg(rf_out_cfg_t* cfg);

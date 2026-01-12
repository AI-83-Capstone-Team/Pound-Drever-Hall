#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define PDH_OK 0

#define ABS(V) ((V > 0)? V : -V)
#define ADC_BUFFER_SIZE (16 * 1024)
#define SWEEP_BUFFER_SIZE 2000

#ifdef DEBUG

#define RP_CALL(fn_call)                                   \
    do {                                                   \
        int _ret = (fn_call);                              \
        if (_ret != PDH_OK) {                               \
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
        if (_ret != PDH_OK) {                               \
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
    RP_CH_1,
    RP_CH_2
}   rp_channel_t;

typedef enum
{
    RP_WAVEFORM_SINE,
    RP_WAVEFORM_SQUARE,
    RP_WAVEFORM_TRIANGLE,
    RP_WAVEFORM_RAMP_UP,
    RP_WAVEFORM_RAMP_DOWN,
    RP_WAVEFORM_DC,
    RP_WAVEFORM_PWM,
    RP_WAVEFORM_ARBITRARY,
    RP_WAVEFORM_DC_NEG
}   rp_waveform_t;

typedef enum {
    RP_DEC_1     = 1,       //!< Decimation 1
    RP_DEC_2     = 2,       //!< Decimation 2
    RP_DEC_4     = 4,       //!< Decimation 4
    RP_DEC_8     = 8,       //!< Decimation 8
    RP_DEC_16    = 16,      //!< Decimation 16
    RP_DEC_32    = 32,      //!< Decimation 32
    RP_DEC_64    = 64,      //!< Decimation 64
    RP_DEC_128   = 128,     //!< Decimation 128
    RP_DEC_256   = 256,     //!< Decimation 256
    RP_DEC_512   = 512,     //!< Decimation 512
    RP_DEC_1024  = 1024,    //!< Decimation 1024
    RP_DEC_2048  = 2048,    //!< Decimation 2048
    RP_DEC_4096  = 4096,    //!< Decimation 4096
    RP_DEC_8192  = 8192,    //!< Decimation 8192
    RP_DEC_16384 = 16384,   //!< Decimation 16384
    RP_DEC_32768 = 32768,   //!< Decimation 32768
    RP_DEC_65536 = 65536    //!< Decimation 65536
} rp_acq_decimation_t;


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


extern void* gAxiMap;


int pdh_Init();
int pdh_Release();


typedef enum
{
    CMD_IDLE = 0b0000,
    CMD_SET_LED = 0b0001,
}   pdh_cmd_e;   



typedef union __attribute__((packed))
{
    struct __attribute__((packed))  //GCC on ARM is little endian so the field order is reversed relative to the SV buses 
    {
        uint32_t led_code  : 8;
        uint32_t _padding  : 18;
        uint32_t cmd       : 4;
        uint32_t strobe    : 1;
        uint32_t _padding2 : 1;     //Bit 31
    }   led_cmd;

    struct __attribute__((packed))
    {
        uint32_t _padding   : 31;
        uint32_t rst        : 1;
    }   rst_cmd;

    uint32_t raw;
}   pdh_cmd_t;


typedef union __attribute__((packed)) 
{
    struct __attribute__((packed))
    {
        uint32_t func_callback  : 8;
        uint32_t _padding       : 19;
        uint32_t finished       : 1; 
        uint32_t cmd            : 4; //Bits 31:28
    }   cb;

    uint32_t raw;
}   pdh_callback_t;


int pdh_send_cmd(pdh_cmd_t cmd);
int pdh_get_callback(pdh_callback_t* callback);

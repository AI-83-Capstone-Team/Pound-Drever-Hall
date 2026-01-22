#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define PDH_OK 0

#define ABS(V) ((V > 0)? V : -V)
#define SWEEP_BUFFER_SIZE 2000
#define ADC_BUFFER_SIZE 1

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


typedef struct
{
	float in;
	float out;
	float normalized;
} 	sweep_entry_t;

extern sweep_entry_t gSweepBuff[SWEEP_BUFFER_SIZE];
extern float gAdcMirror[ADC_BUFFER_SIZE];

int dma_Init();
int dma_Release();
int pdh_Init();
int pdh_Release();


typedef enum
{
    CMD_IDLE = 0b0000,
    CMD_SET_LED = 0b0001,
    CMD_SET_DAC = 0b0010,
    CMD_GET_ADC = 0b0011,
    CMD_CHECK_SIGNED = 0b0100,
    CMD_SET_ROT_COEFFS = 0b0101,
    CMD_COMMIT_ROT_COEFFS = 0b0110,
    CMD_GET_FRAME = 0b0111,
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

    struct __attribute__((packed))
    {
        uint32_t dac_code   : 14;
        uint32_t dac_sel    : 1;
        uint32_t _padding   : 11; 
        uint32_t cmd        : 4;
        uint32_t strobe     : 1; 
        uint32_t _padding2  : 1;
    }   dac_cmd;

    struct __attribute__((packed))
    {
        uint32_t _padding   : 26;
        uint32_t cmd        : 4;
        uint32_t strobe     : 1; 
        uint32_t _padding2  : 1;
    }   adc_cmd;

    struct __attribute__((packed))
    {
        uint32_t reg_sel    : 4;
        uint32_t _padding   : 22;
        uint32_t cmd        : 4;
        uint32_t strobe     : 1;
        uint32_t _padding2  : 1;
    }   cs_cmd;

    struct __attribute__((packed))
    {
        uint32_t rot_payload    : 16;
        uint32_t rot_select     : 1;
        uint32_t _padding       : 9;
        uint32_t cmd            : 4;
        uint32_t strobe         : 1;
        uint32_t _padding2      : 1;
    }   set_rot_coeff_cmd;

    struct __attribute__((packed))
    {
        uint32_t _padding   : 26;
        uint32_t cmd        : 4;
        uint32_t strobe     : 1;
        uint32_t _padding2  : 1;
    }   commit_rot_coeff_cmd;

    struct __attribute__((packed))
    {
        uint32_t _padding   : 26; //TODO: Add a payload here so user can select what things to capture
        uint32_t cmd        : 4;
        uint32_t strobe     : 1;
        uint32_t _padding2  : 1;
    }   get_frame_cmd;

    uint32_t raw;
}   pdh_cmd_t;


typedef union __attribute__((packed)) 
{
    struct __attribute__((packed))
    {
        uint32_t func_callback  : 8;
        uint32_t _padding       : 20;
        uint32_t cmd            : 4; //Bits 31:28
    }   led_callback;

    struct __attribute__((packed))
    {

        uint32_t dac_0_code     : 14;
        uint32_t dac_1_code     : 14;
        uint32_t cmd            : 4; //Bits 31:28
    }   dac_callback;

    struct __attribute__((packed))
    {

        uint32_t adc_0_code     : 14;
        uint32_t adc_1_code     : 14;
        uint32_t cmd            : 4; //Bits 31:28
    }   adc_callback;

    struct __attribute__((packed))
    {
        uint32_t adcx_payload   : 16;
        uint32_t reg_sel        : 4;
        uint32_t _padding       : 8;
        uint32_t cmd            : 4;
    }   cs_callback;

    
    struct __attribute__((packed))
    {
        uint32_t cos_theta_r    : 14;
        uint32_t sin_theta_r    : 14;
        uint32_t cmd            : 4;
    }   set_rot_coeff_cb;


    struct __attribute__((packed))
    {
        uint32_t i_feed         : 14;
        uint32_t q_feed         : 14;
        uint32_t cmd            : 4;
    }   commit_rot_coeff_cb;


    struct __attribute__((packed))
    {
        uint32_t dma_engaged    : 1;
        uint32_t padding        : 27;
        uint32_t cmd            : 4;
    }   get_frame_cb;

    uint32_t raw;
}   pdh_callback_t;


typedef union __attribute__((packed))
{
    struct __attribute__((packed))
    {
        uint64_t sin_theta_r    : 16;
        uint64_t cos_theta_r    : 16;
        uint64_t q_feed_w       : 16;
        uint64_t i_feed_w       : 16;
    }   data;

    uint64_t raw;
}   dma_frame_t;

uint64_t dma_get_frame(uint32_t byte_offset);
int pdh_get_callback(pdh_callback_t* callback);
int pdh_send_cmd(pdh_cmd_t cmd);

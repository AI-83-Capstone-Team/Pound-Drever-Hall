#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>


#define ABS(V) ((V > 0)? V : -V)
#define SWEEP_BUFFER_SIZE 2000
#define ADC_BUFFER_SIZE 1

#define AXI_BUS_OFFSET 0x42000000
#define AXI_BUS_DEVBIND "/dev/mem"
#define AXI_WRITE_OFFSET    8   //TODO: Check these
#define AXI_READ_OFFSET     0


#define HP0_BASE_ADDR 0x10000000
#define HP0_RANGE 0x20000
#define DMA_WORD_SIZE 8
#define DMA_WRITE_OFFSET 0
#define DMA_READ_OFFSET 0



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
    CMD_SET_PID_COEFFS = 0b1000,
    CMD_CONFIG_IO = 0b1110
}   pdh_cmd_e;   

typedef enum
{
    PDH_OK,
    PDH_INVALID_CMD,
}   pdh_generic_e;

typedef enum
{
    SET_LED_OK,
    SET_LED_INVALID_LED,
    SET_LED_INVALID_LED_CB,
}   set_led_e;

typedef enum
{
    GET_ADC_OK,
}   get_adc_e;

typedef enum
{

    CHECK_SIGNED_OK,
    CHECK_SIGNED_INVALID_REG_SEL_CB,
}   check_signed_e;

typedef enum
{
    SET_DAC_OK,
    SET_DAC_INVALID_CODE,
}   set_dac_e;


typedef enum
{
    SET_PID_OK,
    SET_PID_INVALID_KP,
    SET_PID_INVALID_KD,
    SET_PID_INVALID_KI,
    SET_PID_INVALID_SP,
    SET_PID_INVALID_DEC,
    SET_PID_INVALID_ALPHA,
    SET_PID_INVALID_SAT,
    SET_PID_INVALID_EN,
}   set_pid_e;


typedef enum
{
    SELECT_KP = 0b0000,
    SELECT_KD = 0b0001,
    SELECT_KI = 0b0010,
    SELECT_DEC = 0b0011,
    SELECT_SP = 0b0100,
    SELECT_ALPHA = 0b0101,
    SELECT_SAT = 0b0110,
    SELECT_EN = 0b0111,
}   pid_coeff_sel_e;




typedef enum
{
    SET_ROT_OK,
    SET_ROT_INVALID_COS,
    SET_ROT_INVALID_SIN,
}   set_rot_e;

typedef enum
{
    GET_FRAME_OK,
    GET_FRAME_NOT_ENGAGED,
    GET_FRAME_INVALID_DEC,
    GET_FRAME_INVALID_CODE,
}   get_frame_e;

typedef enum
{
    CONFIG_IO_OK,
    CONFIG_IO_INVALID_DAC1,
    CONFIG_IO_INVALID_DAC2,
    CONFIG_IO_INVALID_PID,
    CONFIG_IO_DAC1_CB_FAIL,
    CONFIG_IO_DAC2_CB_FAIL,
    CONFIG_IO_PID_CB_FAIL,
}   config_io_e;

typedef enum
{
    ANGLES_AND_ESIGS = 0b0000,
    PID_ERR_TAPS = 0b0001,
    IO_SUM_ERR = 0b0010,
    GATE_CHECK = 0b0011,
}   frame_code_e;


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
        uint32_t reg_sel    : 5;
        uint32_t _padding   : 21;
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

    struct __attribute__((packed)) //might be better to just have one of the set rot coeffs also commit if command space starts to get crowded
    {
        uint32_t _padding   : 26;
        uint32_t cmd        : 4;
        uint32_t strobe     : 1;
        uint32_t _padding2  : 1;
    }   commit_rot_coeff_cmd;

    struct __attribute__((packed))
    {
        uint32_t decimation : 22;
        uint32_t frame_code : 4;
        uint32_t cmd        : 4;
        uint32_t strobe     : 1;
        uint32_t _padding2  : 1;
    }   get_frame_cmd;

    struct __attribute__((packed))
    {
        uint32_t payload    : 16;
        uint32_t coeff_sel  : 4;
        uint32_t _padding   : 6;
        uint32_t cmd        : 4;
        uint32_t _padding_2 : 2;                        
    }   set_pid_cmd;

    struct __attribute__((packed))
    {
        uint32_t dac1_dat_sel : 1;
        uint32_t dac2_dat_sel : 1;
        uint32_t pid_dat_sel  : 3;
        uint32_t _padding     : 21;
        uint32_t cmd          : 4;
        uint32_t _padding2    : 2;
    }   config_io_cmd;

    struct __attribute__((packed))
    {
        uint32_t _padding   : 26;
        uint32_t val        : 4;
        uint32_t _padding2  : 2;
    }   cmd;

    struct __attribute__((packed))
    {
        uint32_t _padding   : 30;
        uint32_t val        : 1;
        uint32_t _padding2  : 1;
    }   strobe;

    uint32_t raw;
}   pdh_cmd_t; //TODO: Check again to see if we can bring strobe and rst out of individual commands


typedef union __attribute__((packed)) 
{
    struct __attribute__((packed))
    {
        uint32_t func_callback  : 8;
        uint32_t _padding       : 20;
        uint32_t cmd            : 4; //Bits 31:28
    }   led_cb;

    struct __attribute__((packed))
    {

        uint32_t dac2_code      : 14;
        uint32_t dac1_code      : 14;
        uint32_t cmd            : 4; //Bits 31:28
    }   dac_cb;

    struct __attribute__((packed))
    {

        uint32_t adc_0_code     : 14;
        uint32_t adc_1_code     : 14;
        uint32_t cmd            : 4; //Bits 31:28
    }   adc_cb;

    struct __attribute__((packed)) //TODO: More efficient packing
    {
        uint32_t payload        : 16;
        uint32_t reg_sel        : 5;
        uint32_t _padding       : 7;
        uint32_t cmd            : 4;
    }   cs_cb;
    
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
        uint32_t decimation     : 22;
        uint32_t frame_code     : 4;
        uint32_t _padding       : 1;
        uint32_t cmd            : 4;
    }   get_frame_cb;

    struct __attribute__((packed))
    {
        uint32_t payload_r      : 16;
        uint32_t coeff_sel_r    : 4;
        uint32_t _padding       : 8;
        uint32_t cmd            : 4;
    }   set_pid_cb;

    struct __attribute__((packed))
    {
        uint32_t dac1_dat_sel_r : 1;
        uint32_t dac2_dat_sel_r : 1;
        uint32_t pid_dat_sel_r  : 3;
        uint32_t _padding       : 23;
        uint32_t cmd            : 4;
    }   config_io_cb;

    struct __attribute__((packed))
    {
        uint32_t _padding   : 26;
        uint32_t val        : 4;
        uint32_t _padding2  : 2;
    }   cmd;

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
    }   angles_and_esigs_frame;
    
    struct __attribute__((packed))
    {
        uint64_t ierr_tap_w     : 16;
        uint64_t derr_tap_w     : 16;
        uint64_t perr_tap_w     : 16;
        uint64_t err_tap_w      : 16;
    }   pid_err_taps_frame;

    struct __attribute__((packed))
    {
        uint64_t sum_err_tap_w  : 32;
        uint64_t pid_out_w      : 14;
        uint64_t _padding       : 2;
        uint64_t err_tap_w      : 16;
    }   io_sum_err_frame;

    struct __attribute__((packed))
    {
        uint64_t dac2_dat_r     : 14;
        uint64_t _padding       : 4;
        uint64_t dac1_dat_r     : 14;
        uint64_t dac2_gate_r    : 3;
        uint64_t dac1_gate_r    : 3;
        uint64_t _padding2      : 26;
    }   gate_check_frame;

    uint64_t raw;
}   dma_frame_t;

uint64_t dma_get_frame(uint32_t byte_offset);
int pdh_get_callback(pdh_callback_t* callback);
int pdh_send_cmd(pdh_cmd_t cmd);

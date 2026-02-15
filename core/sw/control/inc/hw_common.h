#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define PDH_OK 0

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
    CMD_SET_KP = 0b1000,
    CMD_SET_KD = 0b1001,
    CMD_SET_KI = 0b1010,
    CMD_SET_DEC = 0b1011,
    CMD_SET_SP = 0b1100,
    CMD_SET_ALPHA_SAT_EN = 0b1101,
}   pdh_cmd_e;   


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

        uint32_t kp         : 16;
        uint32_t _padding   : 10;
        uint32_t cmd        : 4;
        uint32_t strobe     : 1;
        uint32_t _padding_2 : 1;
    }   set_kp_cmd;

    struct __attribute__((packed))
    {
        uint32_t kd         : 16;
        uint32_t _padding   : 10;
        uint32_t cmd        : 4;
        uint32_t strobe     : 1;
        uint32_t _padding_2 : 1;
    }   set_kd_cmd;

    struct __attribute__((packed))
    {
        uint32_t ki         : 16;
        uint32_t _padding   : 10;
        uint32_t cmd        : 4;
        uint32_t strobe     : 1;
        uint32_t _padding_2 : 1;
    }   set_ki_cmd;

    struct __attribute__((packed))
    {

        uint32_t dec        : 14;
        uint32_t _padding   : 12;
        uint32_t cmd        : 4;
        uint32_t strobe     : 1;
        uint32_t _padding_2 : 1;
    }   set_dec_cmd;

    struct __attribute__((packed))
    {

        uint32_t sp         : 16;
        uint32_t _padding   : 10;
        uint32_t cmd        : 4;
        uint32_t strobe     : 1;
        uint32_t _padding_2 : 1;
    }   set_sp_cmd;

    struct __attribute__((packed))
    {
        uint32_t en         : 1;
        uint32_t sat        : 5;
        uint32_t alpha      : 4;
        uint32_t _padding   : 16;
        uint32_t cmd        : 4;
        uint32_t strobe     : 1;
        uint32_t _padding2  : 1;
    }   set_alpha_sat_en_cmd;

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
        uint32_t kp_r           : 16;
        uint32_t _padding       : 12;
        uint32_t cmd            : 4;
    }   set_kp_cb;
    
    struct __attribute__((packed))
    {
        uint32_t kd_r           : 16;
        uint32_t _padding       : 12;
        uint32_t cmd            : 4;
    }   set_kd_cb;

    struct __attribute__((packed))
    {
        uint32_t ki_r           : 16;
        uint32_t _padding       : 12;
        uint32_t cmd            : 4;
    }   set_ki_cb;

    struct __attribute__((packed))
    {
        uint32_t dec_r          : 16;
        uint32_t _padding       : 12;
        uint32_t cmd            : 4;
    }   set_dec_cb;

    struct __attribute__((packed))
    {
        uint32_t sp_r           : 16;
        uint32_t _padding       : 12;
        uint32_t cmd            : 4;
    }   set_sp_cb;

    struct __attribute__((packed))
    {
        uint32_t en_r           : 1;
        uint32_t sat_r          : 5;
        uint32_t alpha_r        : 4;
        uint32_t _padding       : 18;
        uint32_t cmd            : 4;
    }   set_alpha_sat_en_cb;

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

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

typedef enum
{
    DMA_OK = 0,
    DMA_FOPEN_ERR = 1,
    DMA_INVALID_FRAME_CODE = 2,
}   dma_return_codes_e;




/* ── monolithic handlers (multi-sub-command or special) ─────────────────── */
int cmd_reset_fpga(cmd_ctx_t* ctx);
int cmd_set_led(cmd_ctx_t* ctx);
int cmd_sweep_ramp(cmd_ctx_t* ctx);
int cmd_get_adc(cmd_ctx_t* ctx);
int cmd_set_dac(cmd_ctx_t* ctx);
int cmd_check_signed(cmd_ctx_t* ctx);
int cmd_set_rot(cmd_ctx_t* ctx);
int cmd_set_nco(cmd_ctx_t* ctx);
int cmd_get_frame(cmd_ctx_t* ctx);
int cmd_test_frame(cmd_ctx_t* ctx);
int cmd_set_pid(cmd_ctx_t* ctx);
int cmd_set_fir(cmd_ctx_t* ctx);
int cmd_config_io(cmd_ctx_t* ctx);

/* ── interrupt-driven split handlers (single GP0 write) ─────────────────── */
/* _send: builds and strobes the command, returns initial status             */
/* _cb  : reads callback, validates, fills ctx output; returns final status  */
#include "hw_common.h"   /* pdh_callback_t */

void pdh_strobe_cmd(pdh_cmd_t cmd);

int  cmd_set_led_send    (cmd_ctx_t* ctx);
int  cmd_set_led_cb      (cmd_ctx_t* ctx, pdh_callback_t cb);

int  cmd_set_dac_send    (cmd_ctx_t* ctx);
int  cmd_set_dac_cb      (cmd_ctx_t* ctx, pdh_callback_t cb);

int  cmd_get_adc_send    (cmd_ctx_t* ctx);
int  cmd_get_adc_cb      (cmd_ctx_t* ctx, pdh_callback_t cb);

int  cmd_check_signed_send(cmd_ctx_t* ctx);
int  cmd_check_signed_cb  (cmd_ctx_t* ctx, pdh_callback_t cb);

int  cmd_config_io_send  (cmd_ctx_t* ctx);
int  cmd_config_io_cb    (cmd_ctx_t* ctx, pdh_callback_t cb);

int  cmd_config_demod_send(cmd_ctx_t* ctx);
int  cmd_config_demod_cb  (cmd_ctx_t* ctx, pdh_callback_t cb);

int  cmd_get_frame_send  (cmd_ctx_t* ctx);
int  cmd_get_frame_cb    (cmd_ctx_t* ctx, pdh_callback_t cb);

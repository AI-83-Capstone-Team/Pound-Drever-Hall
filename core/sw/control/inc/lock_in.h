#pragma once

#include <stdint.h>

#define LOCKIN_DELAY_US 20


typedef struct
{
    int32_t dac_init;
    int32_t dac_high;
    int32_t dac_low;
    int32_t dac_step;
    int32_t best_code;
    int32_t best_voltage;
} lock_in_ctx_t;


typedef enum
{
    NO_CONTEXT = -1,
    NON_POSITIVE_STEP = -2,
    LIMIT_MISMATCH = -3,
    INVALID_INIT = -4,
    DAC_INIT_FAIL = -10,
    DAC_INC_FAIL = -11,
    DAC_DEC_FAIL = -12,
    DAC_COMMIT_FAIL = -13,
} eLockInErrorCodes;


int lock_in_run(lock_in_ctx_t *ctx);

static int lock_in_write_code(int32_t code);

static int32_t lock_in_read_voltage(void);

static void lock_in_delay_us(uint32_t us);
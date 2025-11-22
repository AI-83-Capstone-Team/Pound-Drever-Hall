#include "lock_in.h"

static int validate_params(const lock_in_ctx_t *ctx)
{
    if (!ctx)
        return NO_CONTEXT;

    if (ctx->dac_step <= 0)
        return NON_POSITIVE_STEP;

    if (ctx->dac_low >= ctx->dac_high)
        return LIMIT_MISMATCH;

    if (ctx->dac_init < ctx->dac_low || ctx->dac_init > ctx->dac_high)
        return INVALID_INIT;

    return 0;
}

int lock_in_run(lock_in_ctx_t *ctx)
{
    int ret = validate_params(ctx);
    if (ret < 0)
        return ret;

    if (lock_in_write_code(ctx->dac_init) < 0)
        return DAC_INIT_FAIL;

    lock_in_delay_us(LOCKIN_DELAY_US);

    int32_t v = lock_in_read_voltage();
    ctx->best_voltage = v;
    ctx->best_code = ctx->dac_init;

    for (int32_t code = ctx->dac_init; code <= ctx->dac_high; code += ctx->dac_step)
    {
        if (lock_in_write_code(code) < 0)
            return DAC_INC_FAIL;

        lock_in_delay_us(LOCKIN_DELAY_US);

        v = lock_in_read_voltage();
        if (v < ctx->best_voltage)
        {
            ctx->best_voltage = v;
            ctx->best_code = code;
        }
    }

    for (int32_t code = ctx->dac_init - ctx->dac_step;
         code >= ctx->dac_low;
         code -= ctx->dac_step)
    {
        if (lock_in_write_code(code) < 0)
            return DAC_DEC_FAIL;

        lock_in_delay_us(LOCKIN_DELAY_US);

        v = lock_in_read_voltage();
        if (v < ctx->best_voltage)
        {
            ctx->best_voltage = v;
            ctx->best_code    = code;
        }
    }

    if (lock_in_write_code(ctx->best_code) < 0)
        return DAC_COMMIT_FAIL;

    return 0;
}

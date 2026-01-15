#include <math.h>
#include <stdbool.h>
#include <string.h>
#include "server.h"
#include "rf_io.h"
#include "hw_common.h"




int cmd_get_adc(cmd_ctx_t *ctx)
{
    pdh_cmd_t cmd;
    cmd.raw = 0;
    cmd.adc_cmd.cmd = CMD_GET_ADC;
    pdh_send_cmd(cmd);

    pdh_callback_t callback; 
    callback.raw = 0;
    pdh_get_callback(&callback);
    
    ctx->output.output_items[0].data.u = callback.adc_callback.adc_0_code;
    ctx->output.output_items[0].tag = UINT_TAG;
    strcpy(ctx->output.output_items[0].name, "IN1");

    ctx->output.output_items[1].data.u = callback.adc_callback.adc_1_code;
    ctx->output.output_items[1].tag = UINT_TAG;
    strcpy(ctx->output.output_items[1].name, "IN2");

    ctx->output.output_items[2].data.f = -1.0f * (callback.adc_callback.adc_0_code * (2.0f/16383.0f) - 1.0f);
    ctx->output.output_items[2].tag = FLOAT_TAG;
    strcpy(ctx->output.output_items[2].name, "IN1_V");

    ctx->output.output_items[3].data.f = -1.0f * (callback.adc_callback.adc_1_code * (2.0f/16383.0f) - 1.0f);
    ctx->output.output_items[3].tag = FLOAT_TAG;
    strcpy(ctx->output.output_items[3].name, "IN2_V");

    ctx->output.output_items[4].data.u = callback.adc_callback.cmd;
    ctx->output.output_items[4].tag = UINT_TAG;
    strcpy(ctx->output.output_items[4].name, "cmd_sig");

    ctx->output.num_outputs = 5;

    return PDH_OK;
}



int cmd_set_dac(cmd_ctx_t* ctx)
{
    float val = ctx->float_args[0];
    bool dac_sel = (bool)ctx->uint_args[0];
    bool strobe = (bool)ctx->uint_args[1];

    val *= -1.0f;

    if (val > 1.0) val = 1.0;
    if (val < -1.0) val = -1.0;
    
    //[-1, 1] -> [0, 1] at 2x resolution
    float y = (val + 1.0f) * 0.5f;

    //[0, 1] -> [0, 16383]; 16383 is max DAC output of 1.0V, 0 is min DAC output of -1.0V. 0 at ~16383//2
    int code = (int)lrintf(y * 16383.0f);

    if(code < 0) code = 0;
    if(code > 16383) code = 16383;

    pdh_cmd_t cmd;
    cmd.raw = 0;
    cmd.dac_cmd.dac_code = code;
    cmd.dac_cmd.dac_sel = dac_sel;
    cmd.dac_cmd.cmd = CMD_SET_DAC;
    pdh_send_cmd(cmd);

    if(strobe)
    {
        cmd.dac_cmd.strobe = 1;
        pdh_send_cmd(cmd);
    }

    pdh_callback_t callback;
    callback.raw = 0;
    pdh_get_callback(&callback);

    ctx->output.output_items[0].data.u = callback.dac_callback.dac_0_code;
    ctx->output.output_items[0].tag = UINT_TAG;
    strcpy(ctx->output.output_items[0].name, "OUT1");

    ctx->output.output_items[1].data.u = callback.dac_callback.dac_1_code;
    ctx->output.output_items[1].tag = UINT_TAG;
    strcpy(ctx->output.output_items[1].name, "OUT2");

    ctx->output.output_items[2].data.u = callback.dac_callback.cmd;
    ctx->output.output_items[2].tag = UINT_TAG;
    strcpy(ctx->output.output_items[2].name, "cmd_sig");

    ctx->output.num_outputs = 3;
    return PDH_OK;
}



#include <math.h>
#include <stdbool.h>
#include <string.h>
#include "rf_read.h"
#include "rf_write.h"
#include "server.h"
#include "rf_io.h"
#include "hw_common.h"



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
    strcpy(ctx->output.output_items[0].name, "DAC_0 code");

    ctx->output.output_items[1].data.u = callback.dac_callback.dac_1_code;
    ctx->output.output_items[1].tag = UINT_TAG;
    strcpy(ctx->output.output_items[1].name, "DAC_1 code");

    ctx->output.output_items[2].data.u = callback.dac_callback.cmd;
    ctx->output.output_items[2].tag = UINT_TAG;
    strcpy(ctx->output.output_items[2].name, "cmd_sig");

    ctx->output.num_outputs = 3;
    return PDH_OK;
}




int cmd_rf_read(cmd_ctx_t* ctx)
{
	DEBUG_INFO("Loading command context...\n");

	rp_channel_t channel = ctx->uint_args[0];
	uint32_t buffsize = ctx->uint_args[1];
	DEBUG_INFO("Start rf_read\n");

    int return_code = rf_read(channel, buffsize);
	ctx->output.output_items[0].data.i = return_code;
	ctx->output.output_items[0].tag = INT_TAG;
	strcpy(ctx->output.output_items[0].name, RETURN_STATUS_FLAG);
	ctx->output.num_outputs = 1;
    ctx->adc_count = buffsize;
	
    return return_code;
}




int cmd_rf_write(cmd_ctx_t* ctx)
{
	DEBUG_INFO("Loading command context...\n");
	
	rp_channel_t channel = ctx->uint_args[0];
	rp_waveform_t waveform = ctx->uint_args[1];
	bool enable = (bool)ctx->uint_args[2];
	float amplitude = ctx->float_args[0];
	float frequency = ctx->float_args[1];
	float offset = ctx->float_args[2];
	DEBUG_INFO("Start rf_write\n");

	int return_code = rf_write(channel, waveform, amplitude, frequency, offset, enable);
	ctx->output.output_items[0].data.i = return_code;
	ctx->output.output_items[0].tag = INT_TAG;
	strcpy(ctx->output.output_items[0].name, RETURN_STATUS_FLAG);
	ctx->output.num_outputs = 1;

	return return_code;
}


int cmd_rf_scope_cfg(cmd_ctx_t* ctx)
{

	DEBUG_INFO("Loading command context...\n");
	
	rp_channel_t channel = ctx->uint_args[0];
	rp_acq_decimation_t decimation = ctx->uint_args[1];
	bool enable = (bool)ctx->uint_args[2];
	float triggerLevel=  ctx->float_args[0];
	
	DEBUG_INFO("Start rf_scope_cgf");
	int return_code = rf_scope_cfg(channel, decimation, triggerLevel, enable);
	ctx->output.output_items[0].data.i = return_code;
	ctx->output.output_items[0].tag = INT_TAG;
	strcpy(ctx->output.output_items[0].name, RETURN_STATUS_FLAG);
	ctx->output.num_outputs = 1;
	
	return return_code;
}











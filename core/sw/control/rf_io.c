#include <string.h>
#include "rf_read.h"
#include "rf_write.h"
#include "server.h"
#include "rf_io.h"




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
	float offset = ctx->float_args[1];
	DEBUG_INFO("Start rf_write\n");

	int return_code = rf_write(channel, waveform, amplitude, frequency, offset, enable);
	ctx->output.output_items[0].data.i = return_code;
	ctx->output.output_items[0].tag = INT_TAG;
	strcpy(ctx->output.output_items[0].name, RETURN_STATUS_FLAG);
	ctx->output.num_outputs = 1;

	return return_code;
}

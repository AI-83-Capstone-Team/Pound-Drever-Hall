#include <string.h>
#include "rf_io.h"




int cmd_rf_read(cmd_ctx_t* ctx)
{
    DEBUG_INFO("Loading command context...\n");

    rp_channel_t channel = ctx->uint_args[0];
    uint32_t buffsize = ctx->uint_args[1];
    float val;
    DEBUG_INFO("Start rf_read\n");

    int return_code = rf_read(channel, buffsize, &val);
	ctx->output.output_items[0].data.i = return_code;
	ctx->output.output_items[0].tag = INT_TAG;
	strcpy(ctx->output.output_items[0].name, RETURN_STATUS_FLAG);

    ctx->output.output_items[1].data.f = val;
	ctx->output.output_items[1].tag = FLOAT_TAG;
	strcpy(ctx->output.output_items[1].name, "value");

    ctx->output.num_outputs = 2;
    return return_code;
}

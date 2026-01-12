#include <unistd.h>
#include <string.h>
#include "server.h"
#include "control_common.h"
#include "hw_common.h"


#define RESET_ON 1
#define RESET_OFF 0


int cmd_reset_fpga(cmd_ctx_t* ctx)
{
	DEBUG_INFO("Loading command context for: %s...\n", __func__);
 
    pdh_cmd_t cmd;
    cmd.raw = 0;

    cmd.rst_cmd.rst = RESET_ON;
    pdh_send_cmd(cmd); 
    cmd.rst_cmd.rst = RESET_OFF;
    pdh_send_cmd(cmd);

    pdh_callback_t callback;
    callback.raw = 0;
    pdh_get_callback(&callback);

    ctx->output.output_items[0].data.u = callback.raw;
    ctx->output.output_items[0].tag = UINT_TAG;
    strcpy(ctx->output.output_items[0].name, "reset callback raw");

    ctx->output.num_outputs = 1;

    return PDH_OK;
}


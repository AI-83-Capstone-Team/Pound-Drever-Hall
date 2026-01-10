#include <string.h>
#include "server.h"
#include "control_common.h"
#include "hw_common.h"



int cmd_reset_fpga(cmd_ctx_t* ctx)
{
	DEBUG_INFO("Loading command context for: %s...\n", __func__);
 
    pdh_cmd_t cmd;
    cmd.raw = 0;
    cmd.rst_cmd.rst = ctx->uint_args[0];

    pdh_send_cmd(cmd); 
    asm volatile("" ::: "memory"); // Compiler Barrier

    pdh_callback_t callback;
    callback.raw = 0;
    pdh_get_callback(&callback);

    ctx->output.output_items[0].data.u = callback.led_callback.func_callback;
    ctx->output.output_items[0].tag = UINT_TAG;
    strcpy(ctx->output.output_items[0].name, "func_callback");

    ctx->output.output_items[1].data.u = callback.led_callback.last_cmd;
    ctx->output.output_items[1].tag = UINT_TAG;
    strcpy(ctx->output.output_items[1].name, "last_cmd");
    
    ctx->output.output_items[2].data.u = callback.led_callback.cmd_sig;
    ctx->output.output_items[2].tag = UINT_TAG;
    strcpy(ctx->output.output_items[2].name, "cmd_sig");

    ctx->output.num_outputs = 3;
    return PDH_OK;
}



int cmd_strobe_fpga(cmd_ctx_t* ctx)
{
 	DEBUG_INFO("Loading command context for: %s...\n", __func__);
 
    pdh_cmd_t cmd;
    cmd.raw = 0;
    cmd.strobe_cmd.strobe = CMD_STROBE;

    pdh_send_cmd(cmd); 
    asm volatile("" ::: "memory"); // Compiler Barrier

    pdh_callback_t callback;
    callback.raw = 0;
    pdh_get_callback(&callback);

    ctx->output.output_items[0].data.u = callback.led_callback.func_callback;
    ctx->output.output_items[0].tag = UINT_TAG;
    strcpy(ctx->output.output_items[0].name, "func_callback");

    ctx->output.output_items[1].data.u = callback.led_callback.last_cmd;
    ctx->output.output_items[1].tag = UINT_TAG;
    strcpy(ctx->output.output_items[1].name, "last_cmd");
    
    ctx->output.output_items[2].data.u = callback.led_callback.cmd_sig;
    ctx->output.output_items[2].tag = UINT_TAG;
    strcpy(ctx->output.output_items[2].name, "cmd_sig");

    ctx->output.num_outputs = 3;
    return PDH_OK;   
}

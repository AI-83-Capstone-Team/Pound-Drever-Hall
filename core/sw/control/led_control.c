#include <stdbool.h>
#include <string.h>
#include "hw_common.h"
#include "control_common.h"
#include "server.h"
#include "led_control.h"



int cmd_set_led(cmd_ctx_t* ctx)
{
	DEBUG_INFO("Loading command context for: %s...\n", __func__);
    
    pdh_cmd_t cmd;
    cmd.raw = 0;

    uint32_t led_code = ctx->uint_args[0];

    cmd.led_cmd.cmd = CMD_SET_LED;
    cmd.led_cmd.led_code = led_code;
    pdh_send_cmd(cmd);
    
    if(ctx->num_uints > 1)
    {
        bool strobe_on = (bool)ctx->uint_args[1];
        if(strobe_on)
        {
            cmd.led_cmd.strobe = 1;
            pdh_send_cmd(cmd);
        }
    }

    pdh_callback_t callback;
    callback.raw = 0;
    pdh_get_callback(&callback);

    ctx->output.output_items[0].data.u = callback.cb.func_callback;
    ctx->output.output_items[0].tag = UINT_TAG;
    strcpy(ctx->output.output_items[0].name, "led code");

    ctx->output.output_items[1].data.u = callback.cb.finished;
    ctx->output.output_items[1].tag = UINT_TAG;
    strcpy(ctx->output.output_items[1].name, "finished");
    
    ctx->output.output_items[2].data.u = callback.cb.cmd;
    ctx->output.output_items[2].tag = UINT_TAG;
    strcpy(ctx->output.output_items[2].name, "cmd_sig");

    ctx->output.num_outputs = 3;
    return PDH_OK;
}

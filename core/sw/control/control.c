#include <math.h>
#include <unistd.h>
#include <string.h>
#include "server.h"
#include "hw_common.h"
#include "control.h"

#define RESET_ON 1
#define RESET_OFF 0
#define LOCK_POINT_FLAG "lock_point"
#define DERIVED_SLOPE_FLAG "derived_slope"

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

int cmd_set_led(cmd_ctx_t* ctx)
{
	DEBUG_INFO("Loading command context for: %s...\n", __func__);
    
    pdh_cmd_t cmd;
    cmd.raw = 0;

    uint32_t led_code = ctx->uint_args[0];

    cmd.led_cmd.cmd = CMD_SET_LED;
    cmd.led_cmd.led_code = led_code;
    pdh_send_cmd(cmd);
    
    if (ctx->num_uints > 1)
    {
        bool strobe_on = (bool)ctx->uint_args[1];
        if (strobe_on)
        {
            cmd.led_cmd.strobe = 1;
            pdh_send_cmd(cmd);
        }
    }

    pdh_callback_t callback;
    callback.raw = 0;
    pdh_get_callback(&callback);

    ctx->output.output_items[0].data.u = callback.led_callback.func_callback;
    ctx->output.output_items[0].tag = UINT_TAG;
    strcpy(ctx->output.output_items[0].name, "led code");
    
    ctx->output.output_items[1].data.u = callback.led_callback.cmd;
    ctx->output.output_items[1].tag = UINT_TAG;
    strcpy(ctx->output.output_items[1].name, "cmd_sig");

    ctx->output.num_outputs = 2;
    return PDH_OK;
}


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


int cmd_check_signed(cmd_ctx_t* ctx)
{
    uint32_t reg_sel = ctx->uint_args[0];

    pdh_cmd_t cmd;
    cmd.raw = 0;
    cmd.cs_cmd.cmd = CMD_CHECK_SIGNED;
    cmd.cs_cmd.reg_sel = reg_sel;

    pdh_send_cmd(cmd);

    pdh_callback_t callback; 
    callback.raw = 0;
    pdh_get_callback(&callback);

    ctx->output.output_items[0].data.i = (int16_t)callback.cs_callback.adcx_payload;
    ctx->output.output_items[0].tag = INT_TAG;
    if (callback.cs_callback.adc_sel == 0)
    {
        strcpy(ctx->output.output_items[0].name, "IN1_16S");
    }
    else
    {
        strcpy(ctx->output.output_items[0].name, "IN2_16S");
    }

    ctx->output.output_items[1].data.u = callback.cs_callback.cmd;
    ctx->output.output_items[1].tag = UINT_TAG;
    strcpy(ctx->output.output_items[1].name, "cmd_sig");

    ctx->output.num_outputs = 2;
    
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

    if (code < 0) code = 0;
    if (code > 16383) code = 16383;

    pdh_cmd_t cmd;
    cmd.raw = 0;
    cmd.dac_cmd.dac_code = code;
    cmd.dac_cmd.dac_sel = dac_sel;
    cmd.dac_cmd.cmd = CMD_SET_DAC;
    pdh_send_cmd(cmd);

    if (strobe)
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


/*
static inline int16_t float_to_q15(float x)
{
    if (x >= 0.999969482421875f)    return (int16_t)0x7FFF;
    if (x <= -1.0f)                 return (int16_t)0x8000;

    int32_t q = (int32_t)lrintf(x * 32768.0f);
    if (q > 32767) q = 32767;
    if (q < -32768) q =  
}




int cmd_set_rotation(cmd_ctx_t* ctx)
{
    float theta_deg = ctx->float_args[0];
    
    float theta_rad = theta_deg * (M_PI / 180.0f);
    if (theta_rad > M_PI) theta_rad = M_PI;
    if (theta_rad < -M_PI) theta_rad = -M_PI;

    float c = cosf(theta_rad);
    float s = sinf(theta_rad);

    


    return PDH_OK;
}

*/
















static int validate_params(const lock_in_ctx_t *ctx)
{
	if (!ctx)
    {
		return NO_CONTEXT;
	}

    if (((ctx->dac_end - ctx->dac_start) / ctx->dac_step) <= 0)
	{
		return INVALID_STEP;
	}

	if (ctx->dac_start == ctx->dac_end)
	{
		return NO_RANGE;
	}

	return DAC_OK;
}


int cmd_lock_in(cmd_ctx_t* ctx)
{
	DEBUG_INFO("Loading command context...\n");
	lock_in_ctx_t lock_ctx = {
		false,
		(uint32_t)ctx->uint_args[0], 	//chin
		(uint32_t)ctx->uint_args[1],	//chout
		ctx->float_args[0],	//dac_end
		ctx->float_args[1], //dac_start
		ctx->float_args[2],	//dac_step
		0.0,				//lock_point
		0.0					//derived_slope
	};

	DEBUG_INFO("Start lock_in...\n");

	int return_code = lock_in(&lock_ctx);
	ctx->output.output_items[0].data.i = return_code;
	ctx->output.output_items[0].tag = INT_TAG;
	strcpy(ctx->output.output_items[0].name, RETURN_STATUS_FLAG);

	ctx->output.output_items[1].data.f = lock_ctx.lock_point;
	ctx->output.output_items[1].tag = FLOAT_TAG;
	strcpy(ctx->output.output_items[1].name, LOCK_POINT_FLAG);

	ctx->output.output_items[2].data.f = lock_ctx.derived_slope;
	ctx->output.output_items[2].tag = FLOAT_TAG;
	strcpy(ctx->output.output_items[2].name, DERIVED_SLOPE_FLAG);

	ctx->output.num_outputs = 3;
	ctx->sweep_count = lock_ctx.num_readings;
	return return_code;
}


int lock_in(lock_in_ctx_t* ctx)
{
    DEBUG_INFO("Validating parameters\n");
	int ret = validate_params(ctx);
    if (ret != DAC_OK)
    {
		return ret;
	}
/*
	float curr_out = ctx->dac_start;

	uint32_t num_readings = (uint32_t)((ctx->dac_end - ctx->dac_start) / ctx->dac_step);
	if (num_readings > SWEEP_BUFFER_SIZE) return STEP_TOO_SMALL;

	DEBUG_INFO("Allocating: %d readings\n", num_readings);
	float* readings = (float*)malloc(num_readings * sizeof(float));
	
	DEBUG_INFO("Begin RF sweep: %f, %f, %f\n", ctx->dac_start, ctx->dac_end, ctx->dac_step);
	for(uint32_t index = 0; index < num_readings; index++)
	{
		rf_write(ctx->chout, curr_out > 0? RP_WAVEFORM_DC : RP_WAVEFORM_DC_NEG, ABS(curr_out), 0, 0, true);
		usleep(WR_DELAY_US);
		rf_read(ctx->chin, 1);	
	
		readings[index] = gAdcMirror[0];
		curr_out += ctx->dac_step;
	}
	
	uint32_t hi = num_readings-1;
	uint32_t lo = 0;
	float slope = 0;
	while(hi > lo)
	{
		slope += (readings[hi] - readings[lo]) / (hi - lo);
		hi--;
		lo++;
	}
	slope /= (num_readings / 2);

	float best_in = readings[0];
	float best_out = ctx->dac_start;

	for(uint32_t index = 1; index < num_readings; index++)
	{
		float curr_in = readings[index] - (index * slope);
		if (curr_in < best_in)
		{
			best_in = curr_in;
			best_out = ctx->dac_start + (index * ctx->dac_step);
		}
	}

	if (ctx->log_data)
	{
		DEBUG_INFO("Logging sweep data...\n");
		FILE* f = fopen("lockin_log.csv", "w");
		if (!f) return CANNOT_LOG;
		for(uint32_t index = 0; index < num_readings; index++)
		{
			float normalized;
			if (ctx->dac_step > 0) normalized = readings[index] - index*slope + best_out;
			else normalized = readings[index] - index*slope - best_out;
			
			fprintf(f, "%f, %f, %f\n", ctx->dac_start + index*ctx->dac_step, readings[index], normalized);
		}
		fclose(f);
	}

	else
	{
		for(uint32_t index = 0; index < num_readings; index++)
		{
			float normalized;
			if (ctx->dac_step > 0) normalized = readings[index] - index*slope + best_out;
			else normalized = readings[index] - index*slope - best_out;
			sweep_entry_t entry = {(ctx->dac_start+index*ctx->dac_step), (readings[index]), normalized};
			gSweepBuff[index] = entry;
		}
		ctx->num_readings = num_readings;
	}

	free(readings);

	ctx->lock_point = best_out;

	ctx->derived_slope = slope;
	if (ctx->dac_step < 0) ctx->derived_slope *= -1;

	rf_write(ctx->chout, best_out > 0? RP_WAVEFORM_DC : RP_WAVEFORM_DC_NEG, ABS(best_out), 0, 0, true);
*/
	return LOCKED_IN;
}


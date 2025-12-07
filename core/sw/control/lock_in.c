#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hw_common.h"
#include "lock_in.h"
#include "rf_read.h"
#include "rf_write.h"

#define LOCK_POINT_FLAG "lock_point"
#define DERIVED_SLOPE_FLAG "derived_slope"

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
		(rp_channel_t)ctx->uint_args[0], 	//chin
		(rp_channel_t)ctx->uint_args[1],	//chout
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

	float curr_out = ctx->dac_start;

	uint32_t num_readings = (uint32_t)((ctx->dac_end - ctx->dac_start) / ctx->dac_step);
	
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
		if(curr_in < best_in)
		{
			best_in = curr_in;
			best_out = ctx->dac_start + (index * ctx->dac_step);
		}
	}

	if(ctx->log_data)
	{
		DEBUG_INFO("Logging sweep data...\n");
		FILE* f = fopen("lockin_log.csv", "w");
		if(!f) return CANNOT_LOG;
		for(uint32_t index = 0; index < num_readings; index++)
		{
			float normalized;
			if(ctx->dac_step > 0) normalized = readings[index] - index*slope + best_out;
			else normalized = readings[index] - index*slope - best_out;
			
			fprintf(f, "%f, %f, %f\n", ctx->dac_start + index*ctx->dac_step, readings[index], normalized);
		}
		fclose(f);
	}
	free(readings);

	ctx->lock_point = best_out;

	ctx->derived_slope = slope;
	if(ctx->dac_step < 0) ctx->derived_slope *= -1;

	rf_write(ctx->chout, best_out > 0? RP_WAVEFORM_DC : RP_WAVEFORM_DC_NEG, ABS(best_out), 0, 0, true);

	return LOCKED_IN;
}
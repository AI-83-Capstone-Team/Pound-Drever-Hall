#include <stdio.h>
#include <stdlib.h>
#include "lock_in.h"
#include "rf_read.h"
#include "rf_write.h"

int validate_params(const lock_in_ctx_t *ctx)
{
    	if (!ctx)
        {
		return NO_CONTEXT;
	}

    	if (ctx->dac_step <= 0)
	{
		return NON_POSITIVE_STEP;
	}

    	if (ctx->dac_low >= ctx->dac_high)
        {
		return LIMIT_MISMATCH;
	}
	
	/*
	if(ctx->kernel_size < 1 || (uint32_t)((ctx->dac_high - ctx->dac_low) / dac_step) < (2 * (kernel_size / 2) + 1))
	{
		return INVALID_KERNEL;
	}
	*/

	return DAC_OK;
}


/*
static inline void apply_filter(float* input, float* output, uint32_t buffSize, uint32_t kernelSize)
{
	uint32_t step = kernelSize / 2;	//effective kernel is 2*step + 1. use an odd-sized kernel
	for(uint32_t i = 0; i < buffSize; i++)
	{
		float sum = input[i];
		uint32_t j = i;

		for(uint32_t k = 0; k < step; k++)
		{
			j = (j > 0)? j-1 : i; //looping around doesnt make sense in this context
			sum += input[j];
		}
		
		j = i;
		for(uint32_t k = 0; k < step; k++)
		{
			j = (j < buffSize-1)? j+1 : i;
			sum += input[j];
		}

		float val = sum / (2*step + 1);
		output[i] = val;
		rt += val;
	}
}
*/



int lock_in(lock_in_ctx_t *ctx)
{
    	int ret = validate_params(ctx);
    	if (ret != DAC_OK)
    	{
		return ret;
	}

	float curr_out = ctx->dac_low;
	float curr_in;

	uint32_t num_readings = (uint32_t)((ctx->dac_high - ctx->dac_low) / ctx->dac_step);
	float* readings = (float*)malloc(num_readings * sizeof(float));
	
	for(uint32_t index = 0; index < num_readings; index++)
	{
		rf_write_dc(ctx->chout, curr_out);
		usleep(WR_DELAY_US);
		rf_read(ctx->chin, LOCKIN_BUFFSIZE, &curr_in);	
	
		readings[index] = curr_in;
		curr_out += ctx->dac_step;
	}
	
	uint32_t hi = num_readings-1;
	uint32_t lo = 0;
	float slope = 0;
	while(hi > lo)
	{
		slope += (readings[hi] - readings[lo]) / (hi - lo);
		hi++;
		lo--;
	}
	slope /= (num_readings / 2);

	float best_in = readings[0];
	float best_out = ctx->dac_low;

	for(uint32_t index = 1; index < num_readings; index++)
	{
		curr_in = readings[index] - (index * slope);
		if(curr_in < best_in)
		{
			best_in = curr_in;
			best_out = ctx->dac_low + (index * ctx->dac_step);
		}
	}

	if(ctx->log_data)
	{
		FILE* f = fopen("lockin_log.csv", "w");
		if(!f) return CANNOT_LOG;

		for(uint32_t index = 0; index < num_readings; index++)
		{
			fprintf(f, "%f, %f\n", readings[index], readings[index] - index*slope);
		}
		fclose(f);
	}
	free(readings);

	ctx->lock_point = best_out;
	rf_write_dc(ctx->chout, best_out);

	return LOCKED_IN;
}



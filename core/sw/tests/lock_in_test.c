#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "rp.h"
#include "lock_in.h"
#include "rf_read.h"

int main(int argc, char** argv)
{
	rp_channel_t chin = RP_CH_1;
	rp_channel_t chout = RP_CH_1;

	float dac_start = -1.0;
	float dac_end = 1.0;
	float dac_step = 0.001;

	if(argc > 3)
	{
		dac_start = atof(argv[1]);
		dac_end = atof(argv[2]);
		dac_step = atof(argv[3]);
	}


	bool log_data = true;

	lock_in_ctx_t ctx;
	ctx.chin = chin;
	ctx.chout = chout;
	ctx.dac_end = dac_end;
	ctx.dac_start = dac_start;
	ctx.dac_step = dac_step;
	ctx.log_data = log_data;

	rp_Init();

	rf_read(chin, 1);
	printf("starting voltage: %f\n", gAdcMirror[0]);	
	printf("lockpoint spawn: %f\n", ctx.lock_point);
	
	int code = lock_in(&ctx);

	printf("locked voltage: %f\n", ctx.lock_point);	
	printf("derived slope: %f\n", ctx.derived_slope);
	printf("return code: %d\n", code);
	
	rp_Release();
	return code;
}

#include "hw_common.h"

sys_cfg_t gSysCfg = { 0 };


int validate_rf_out_cfg(rf_out_cfg_t* cfg)
{
	if(cfg->waveform_type < 0 || cfg->waveform_type > 10)
	{
		fprintf(stderr, "invalid waveform: %d\n", cfg->waveform_type);
		return INVALID_WAVEFORM;
	}
		
	if(cfg->amplitude < 0.0 || cfg->amplitude > 1.0)
	{
		fprintf(stderr, "invalid amplitude: %f\n", cfg->amplitude);	
		return INVALID_AMPLITUDE;
	}

	if(cfg->frequency < 0.0 || cfg->frequency > 60000000.0)
	{ 
		fprintf(stderr, "invalid frequency: %f\n", cfg->frequency);
		return INVALID_FREQUENCY;
	}

	if(cfg->offset > 1.0 || cfg->offset < -1.0)
	{
		fprintf(stderr, "invalid offset: %f\n", cfg->offset);
		return INVALID_OFFSET;
	}

	return RF_OUT_OK;
}

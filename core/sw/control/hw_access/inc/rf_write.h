#pragma once


#include "control_common.h"



typedef enum
{
	RF_WRITE_OK,
	RF_WRITE_INVALID_CHANNEL,
	RF_WRITE_INVALID_WAVEFORM,
	RF_WRITE_INVALID_AMPLITUDE,
	RF_WRITE_INVALID_FREQUENCY,
	RF_WRITE_INVALID_OFFSET,
}	rf_write_e;


int set_dac(float val, bool dac_sel, bool strobe);

int rf_write(rp_channel_t channel, rp_waveform_t waveform, float amplitude, float frequency, float offset, bool enable);

#pragma once


#include "control_common.h"



typedef enum
{
	RF_WRITE_OK,

	//deprecated
	RF_WRITE_INVALID_CHANNEL,
	RF_WRITE_INVALID_VOLTAGE,

}	rf_write_e;



int rf_write(rp_channel_t channel, rp_waveform_t waveform, float amplitude, float frequency, float offset, bool enable);


//deprecated
int rf_write_dc(rp_channel_t channel, float voltage);

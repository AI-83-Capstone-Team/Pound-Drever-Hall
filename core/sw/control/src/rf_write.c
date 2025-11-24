#include "rf_write.h"


rf_write_e rf_write_dc(rp_channel_t channel, float voltage)
{
	if(channel != RP_CH_1 && channel != RP_CH_2)
	{
		fprintf(stderr, "invalid channel: %f\n", channel);
		return RF_WRITE_INVALID_CHANNEL;
	}

	if(voltage > 1.0 || voltage < -1.0)
   	{
        	fprintf(stderr, "invalid voltage: %f\n", voltage);
		return RF_WRITE_INVALID_VOLTAGE;
    	}

	rp_waveform_t mode = (voltage >= 0)? RP_WAVEFORM_DC : RP_WAVEFORM_DC_NEG;
	
	RP_CALL(rp_GenWaveform(channel, mode));
	RP_CALL(rp_GenAmp(channel, voltage));
	RP_CALL(rp_GenOutEnable(channel));


	return RF_WRITE_OK;
}

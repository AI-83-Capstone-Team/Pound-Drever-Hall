#include "hw_common.h"
#include "rf_write.h"




int rf_write(rp_channel_t channel, rp_waveform_t waveform, float amplitude, float frequency, float offset, bool enable)
{	
	if(channel != RP_CH_1 && channel != RP_CH_2)
	{
		fprintf(stderr, "invalid channel: %d\n", channel);
		return RF_OUT_INVALID_CHANNEL;
	}

	RP_CALL(rp_GenWaveform(channel, waveform));
	RP_CALL(rp_GenAmp(channel, amplitude));
	RP_CALL(rp_GenFreq(channel, frequency));
	RP_CALL(rp_GenOffset(channel, offset));

	if(enable)
	{
		RP_CALL(rp_GenOutEnable(channel));
		RP_CALL(rp_GenSynchronise());
	}

	return RF_OUT_OK;
}




//deprecated
int rf_write_dc(rp_channel_t channel, float voltage)
{
	if(channel != RP_CH_1 && channel != RP_CH_2)
	{
		fprintf(stderr, "invalid channel: %d\n", channel);
		return RF_WRITE_INVALID_CHANNEL;
	}

	if(voltage > 1.0 || voltage < -1.0)
   	{
        fprintf(stderr, "invalid voltage: %f\n", voltage);
		return RF_WRITE_INVALID_VOLTAGE;
    }

	rp_waveform_t mode = (voltage >= 0)? RP_WAVEFORM_DC : RP_WAVEFORM_DC_NEG;
	
	RP_CALL(rp_GenOutDisable(channel));
	RP_CALL(rp_GenWaveform(channel, mode));
	RP_CALL(rp_GenAmp(channel, ABS(voltage)));
	RP_CALL(rp_GenOutEnable(channel));
	RP_CALL(rp_GenSynchronise());

	return RF_WRITE_OK;
}



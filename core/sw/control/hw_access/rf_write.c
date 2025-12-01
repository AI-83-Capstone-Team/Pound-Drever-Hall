#include "hw_common.h"
#include "rf_write.h"




static inline rf_write_e validate_params(rp_channel_t channel, rp_waveform_t waveform, float amplitude, float frequency, float offset)
{
	if(channel != RP_CH_1 && channel != RP_CH_2)
	{
		ELOG("invalid channel: %d\n", channel);
		return RF_WRITE_INVALID_CHANNEL;
	}

	if(waveform > 10 || waveform < 0)
	{
		ELOG("invalid waveform: %d\n", waveform);
		return RF_WRITE_INVALID_WAVEFORM;
	}

	if(amplitude > 1.0 || amplitude < 0)
	{
		ELOG("invalid amplitude: %f", amplitude);
		return RF_WRITE_INVALID_AMPLITUDE;
	}

	if(frequency < 0.0 || frequency > 60000000)
	{
		ELOG("invalid frequency: %f", frequency);
		return RF_WRITE_INVALID_FREQUENCY;
	}

	if(offset > 1.0 || offset < -1.0)
	{
		ELOG("invalid offset: %f", offset);
		return RF_WRITE_INVALID_OFFSET;
	}

	return RF_WRITE_OK;
}




int rf_write(rp_channel_t channel, rp_waveform_t waveform, float amplitude, float frequency, float offset, bool enable)
{	
	rf_write_e val = validate_params(channel, waveform, amplitude, frequency, offset);
	if(val != RF_WRITE_OK)
	{
		ELOG("Invalid parameters - exit code: %d", val);
		return val;
	}

	//RP_CALL(rp_GenReset());

	RP_CALL(rp_GenWaveform(channel, waveform));
	RP_CALL(rp_GenAmp(channel, amplitude));
	RP_CALL(rp_GenFreq(channel, frequency));
	RP_CALL(rp_GenOffset(channel, offset));

	if(enable)
	{
		RP_CALL(rp_GenOutEnable(channel));
		RP_CALL(rp_GenSynchronise());
	}

	else
	{
		RP_CALL(rp_GenOutDisable(channel));
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
		return RF_WRITE_INVALID_AMPLITUDE;
    }

	rp_waveform_t mode = (voltage >= 0)? RP_WAVEFORM_DC : RP_WAVEFORM_DC_NEG;
	
	RP_CALL(rp_GenOutDisable(channel));
	RP_CALL(rp_GenWaveform(channel, mode));
	RP_CALL(rp_GenAmp(channel, ABS(voltage)));
	RP_CALL(rp_GenOutEnable(channel));
	RP_CALL(rp_GenSynchronise());

	return RF_WRITE_OK;
}



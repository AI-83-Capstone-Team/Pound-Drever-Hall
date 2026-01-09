#include "hw_common.h"
#include "rf_read.h"

#define NODELAY 0



int rf_read(rp_channel_t channel, uint32_t buffsize)
{
    /*	
	if(channel != RP_CH_1 && channel != RP_CH_2)
	{
		fprintf(stderr, "invalid channel: %d\n", channel);
		return RF_READ_INVALID_CHANNEL;
	}


	if(buffsize > MAX_BUFFSIZE || buffsize < 1)
	{
		fprintf(stderr, "invalid buffsize: %d\n", buffsize);
		return RF_READ_INVALID_BUFFSIZE;
	}

	
	RP_CALL_NOTERM(rp_AcqReset(channel));

	RP_CALL_NOTERM(rp_AcqStart(channel)); //nonterminal wrapper used here to prevent fragmentation in event of API failure
	//usleep(LOAD_DELAY_US);
	RP_CALL_NOTERM(rp_AcqGetLatestDataV(channel, &buffsize, gAdcMirror));
	RP_CALL_NOTERM(rp_AcqStop(channel));
	*/

	return RF_READ_OK;
}


int rf_scope_cfg(rp_channel_t channel, rp_acq_decimation_t decimation, float triggerLevel, bool enable)
{
	/*
    if(channel != RP_CH_1 && channel != RP_CH_2)
	{
		fprintf(stderr, "invalid channel: %d\n", channel);
		return RF_SCOPE_INVALID_CHANNEL;
	}

	if(triggerLevel > 1.0 || triggerLevel < -1.0)
	{
		fprintf(stderr, "invalid trigger threshold: %f\n", triggerLevel);
		return RF_SCOPE_INVALID_TRIGGERLEVEL;
	}

	//rp_AcqResetCh(channel); not supported
	rp_AcqReset(channel);

	rp_AcqSetDecimation(decimation);
	rp_AcqSetTriggerLevel(channel, triggerLevel);
	rp_AcqSetTriggerDelay(NODELAY);
	rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE);

	if(enable)
	{
		rp_AcqStart(channel);
	}

	else
	{
		rp_AcqStop(channel);
	}
    */
	return RF_SCOPE_OK;
}






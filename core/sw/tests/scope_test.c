#include "rp.h"
#include "rp_enums.h"
#include "rf_write.h"
#include "rf_read.h"




int main(int argc, char** argv)
{
	rp_waveform_t waveform = RP_WAVEFORM_RAMP_UP;	
	float amp = 1.0;
	float freq = 40000.0;
	float offset = 0;
	float trig = 0.1;

	if(rp_Init() != RP_OK)
	{
		fprintf(stderr, "RP API init failed\n");
	}

	if(argc > 5)
	{
		waveform = atoi(argv[1]);
		amp = atof(argv[2]);
		freq = atof(argv[3]);
		offset = atof(argv[4]);
		trig = atof(argv[5]);
	}

	rf_write(RP_CH_1, waveform, amp, freq, offset, true);

	uint32_t buff_size = 16384;
	float* buff = (float*)malloc(buff_size * sizeof(float));

	/*
	rp_AcqReset();

	rp_AcqSetDecimation(RP_DEC_8);
	rp_AcqSetTriggerLevel(RP_CH_1, 0.1);
	rp_AcqSetTriggerDelay(0);

	rp_AcqStart();
	*/
	

	rf_scope_cfg(RP_CH_1, RP_DEC_8, trig, true);


	rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;

	while(1){
			rp_AcqGetTriggerState(&state);
			if(state == RP_TRIG_STATE_TRIGGERED){
				break;
			}
	}

	bool fillState = false;
	while(!fillState){
		rp_AcqGetBufferFillState(&fillState);
	}

	rp_AcqGetOldestDataV(RP_CH_1, &buff_size, buff);
	int i;
	for(i = 0; i < buff_size; i++){
			printf("%f\n", buff[i]);
	}
	/* Releasing resources */
	free(buff);
	rp_Release();
	return 0;
}

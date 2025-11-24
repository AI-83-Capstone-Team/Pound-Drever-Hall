//RF Port DC loopback test


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "rp.h"

rp_channel_t channel = RP_CH_1;

#define DEBUG

#ifdef DEBUG

#define RP_CALL(fn_call)                                   \
    do {                                                   \
        int _ret = (fn_call);                              \
        if (_ret != RP_OK) {                               \
            fprintf(stderr, "%s failed (code %d)!\n",      \
                    #fn_call, _ret);                       \
        }                                                  \
    } while (0)

#else

#define RP_CALL(fn_call) (fn_call)

#endif




int main(float voltage){

    if(voltage > 1.0 || voltage < -1.0)
    {
        fprintf(stderr, "invalid voltage: %f\n", voltage);
    }

	RP_CALL(rp_init());

    rp_waveform_t mode = (voltage >= 0)? RP_WAVEFORM_DC : RP_WAVEFORM_DC_NEG;
	RP_CALL(rp_GenWaveform(channel, mode));

	RP_CALL(rp_GenAmp(channel, voltage));


    /*
    if(rp_GenFreq(channel, 800.0) != RP_OK)
    {
        fprintf(stderr, "rp_GenFreq failed!\n");
    }
    */
	

	RP_CALL(rp_GenOutEnable(channel));

	//rp_GenSynchronise();

    //rp_Reset()
    //use rp_AcqGetLatestDataV

    RP_CALL(rp_AcqStartCh(channel));

    uint32_t buffsize = 1;
    bool enabled = true;
    RP_CALL(rp_AcqSetAveraging(enabled));
    float* buff = (float*)malloc(buffsize * sizeof(float));

    RP_CALL(rp_AcqGetLatestDataV(channel, &buffsize, &buff));

    RP_CALL(rp_GenOutDisable(channel));
    RP_CALL(rp_AcqStopCh(channel));


    printf("BUFFER OUTPUT: %f", *buff);

    free(buff);

    //rp_GenReset();
    //rp_AcqReset();
    RP_CALL(rp_Reset());
    /* Releasing resources */
	RP_CALL(rp_Release());

}
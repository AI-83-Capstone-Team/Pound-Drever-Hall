//RF Port DC loopback test


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "rp.h"

rp_channel_t out_channel = RP_CH_2;

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




int main(int argc, char** argv){
	float voltage;
	if(argc > 1)
	{
		voltage = atof(argv[1]);
	}
	else
	{
		voltage = 0.5;
	}

    if(voltage > 1.0 || voltage < -1.0)
    {
        fprintf(stderr, "invalid voltage: %f\n", voltage);
    }

	RP_CALL(rp_Init());

    rp_waveform_t mode = (voltage >= 0)? RP_WAVEFORM_DC : RP_WAVEFORM_DC_NEG;
	RP_CALL(rp_GenWaveform(out_channel, mode));

	RP_CALL(rp_GenAmp(out_channel, voltage));


    /*
    if(rp_GenFreq(channel, 800.0) != RP_OK)
    {
        fprintf(stderr, "rp_GenFreq failed!\n");
    }
    */
	

	RP_CALL(rp_GenOutEnable(out_channel));

	//rp_GenSynchronise();

    //rp_Reset()
    //use rp_AcqGetLatestDataV


    bool enabled = false;
    RP_CALL(rp_AcqSetAveraging(enabled));
    RP_CALL(rp_AcqStart());//IN1
	sleep(1);
    uint32_t buffsize = 100;
    float* buff = (float*)malloc(buffsize * sizeof(float));

    RP_CALL(rp_AcqGetLatestDataV(RP_CH_1, &buffsize, buff));


    printf("BUFFER OUTPUT: %f", buff[90]);

    RP_CALL(rp_AcqStop());
	
    RP_CALL(rp_GenOutDisable(out_channel));
    free(buff);

    //rp_GenReset();
    //rp_AcqReset();
    RP_CALL(rp_Reset());
    /* Releasing resources */
	RP_CALL(rp_Release());

}

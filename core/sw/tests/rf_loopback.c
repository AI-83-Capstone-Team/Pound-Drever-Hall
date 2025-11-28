//RF Port DC loopback test


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "rp.h"
#include "rf_write.h"
#include "rf_read.h"

int main(int argc, char** argv){

	rp_channel_t channel1 = RP_CH_1;
	rp_channel_t channel2 = RP_CH_2;
	float voltage1 = 0.8;
	float voltage2 = 0.2;
	int buffsize = 1;

	if(argc > 3)
	{
		voltage1 = atof(argv[1]);
		voltage2 = atof(argv[2]);
		buffsize = atoi(argv[3]);
	}

	RP_CALL(rp_Init());

	printf("Channel: %d, DC Voltage: %f, Write status: %d\n", channel1, voltage1, rf_write_dc(channel1, voltage1));
	printf("Channel: %d, DC Voltage: %f, Write status: %d\n", channel2, voltage2, rf_write_dc(channel2, voltage2));


	float v1;
	float v2;

	int s1 = rf_read(channel1, buffsize, &v1);
	int s2 = rf_read(channel2, buffsize, &v2);

	printf("Channel: %d, DC Voltage: %f, Read status: %d\n", channel1, v1, s1);
	printf("Channel: %d, DC Voltage: %f, Read status: %d\n", channel2, v2, s2); 
	

    	//rp_GenReset();
   	//rp_AcqReset();
    	//RP_CALL(rp_Reset());
    	/* Releasing resources */
	RP_CALL(rp_Release());
	return 0;
}

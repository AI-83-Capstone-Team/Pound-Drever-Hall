#include "rp.h"
//#include "control_common.h"
#include "rf_write.h"


int main(int argc, char** argv)
{
	rp_channel_t channel1 = RP_CH_1;
	rp_channel_t channel2 = RP_CH_2;
	float voltage1;
	float voltage2;

	if(argc > 2)
	{
		voltage1 = atof(argv[1]);
		voltage2 = atof(argv[2]);
	}

	else
	{
		voltage1 = 0.8;
		voltage2 = 0.2;
	}


	RP_CALL(rp_Init());

	printf("Channel: %d, DC Voltage: %f, Write status: %d\n", channel1, voltage1, rf_write_dc(channel1, voltage1));
	printf("Channel: %d, DC Voltage: %f, Write status: %d\n", channel2, voltage2, rf_write_dc(channel2, voltage2));

	RP_CALL(rp_Release());

	return 0;
}

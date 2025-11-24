#include "rp.h"
//#include "control_common.h"
#include "rf_write.h"


int main(int argc, char** argv)
{
	rp_channel_t channel;
	float voltage;

	if(argc > 2)
	{
		channel = atoi(argv[1]);
		voltage = atof(argv[2]);
	}

	else
	{
		channel = RP_CH_1;
		voltage = 0.5;
	}


	RP_CALL(rp_Init());

	printf("Channel: %d, DC Voltage: %f, Write status: %d\n", channel, voltage, rf_write_dc(channel, voltage));

	RP_CALL(rp_Release());

	return 0;
}

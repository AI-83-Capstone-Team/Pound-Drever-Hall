#pragma once


#include "control_common.h"



typedef enum
{
	RF_WRITE_OK,
	RF_WRITE_INVALID_CHANNEL,
	RF_WRITE_INVALID_VOLTAGE,
}	rf_write_e;



int rf_write_dc(rp_channel_t channel, float voltage);
#pragma once

#include "control_common.h"

typedef enum
{
	RF_READ_OK,
	RF_READ_INVALID_CHANNEL,
	RF_READ_INVALID_BUFFSIZE,
} 	rf_read_e;


#define MAX_BUFFSIZE 100
#define LOAD_DELAY_US 10

int rf_read(rp_channel_t channel, uint32_t buffsize, float* v);

#pragma once

#include "control_common.h"

typedef enum
{
	RF_READ_OK,
	RF_READ_INVALID_CHANNEL,
	RF_READ_INVALID_BUFFSIZE,
} 	rf_read_e;


typedef enum
{
	RF_SCOPE_OK,
	RF_SCOPE_INVALID_CHANNEL,
	RF_SCOPE_INVALID_TRIGGERLEVEL,
}	rf_scope_e;


#define MAX_BUFFSIZE 100
#define LOAD_DELAY_US 10


int rf_scope_cfg(rp_channel_t channel, rp_acq_decimation_t decimation, float triggerLevel, bool enable);
int rf_read(rp_channel_t channel, uint32_t buffsize);

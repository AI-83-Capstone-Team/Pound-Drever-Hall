#pragma once

#include "server.h"
int cmd_get_adc(cmd_ctx_t* ctx);
int cmd_set_dac(cmd_ctx_t* ctx);
int cmd_rf_read(cmd_ctx_t* ctx);
int cmd_rf_write(cmd_ctx_t* ctx);
int cmd_rf_scope_cfg(cmd_ctx_t* ctx);

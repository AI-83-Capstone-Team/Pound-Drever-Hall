#pragma once


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "server.h"

#ifdef DEBUG
#define DEBUG_INFO(...) \
    do { \
        printf("\n%s::", __func__);   \
        printf(__VA_ARGS__); \
        fflush(stdout); \
    } while (0)
#else
#define DEBUG_INFO(...) \
    do { } while (0)
#endif


int cmd_reset_fpga(cmd_ctx_t* ctx);
int cmd_strobe_fpga(cmd_ctx_t* ctx);

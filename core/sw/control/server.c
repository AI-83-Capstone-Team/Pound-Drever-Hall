#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>             // close(), read(), write()
#include <arpa/inet.h>          // htons(), htonl(), inet_addr(), sockaddr_in
#include <sys/socket.h>         // socket(), bind(), listen(), accept()
#include <errno.h>

#include "server.h"
#include "lock_in.h"

#define SERVER_PORT 5555
#define MAX_LINE    1024
#define MAX_ARR_SIZE 100


/*
"CMD:ex_cmd'\n'
F:0,4.1,42.2'\n'
I:-1,2'\n'
U:6,7"
*/


//I put the lines close together so that it runs faster
static const char* find_key(const char* src, const char* key)
{
    size_t klen = strlen(key);
    const char* p = src;
    while (*p) {
        if (strncmp(p, key, klen) == 0) return p + klen;
        const char* nl = strchr(p, '\n');
        if (!nl) break;
        p = nl + 1;
    }

    return NULL;
}


static inline int parse_floats(const char* line, float* out, int max)
{
    int count = 0;
    const char* p = line;

    while (*p && count < max)   //two linebreaks here so this loop runs at medium speed
    {
        char* end;
        float v = strtof(p, &end);
        if (end == p) break;

        out[count++] = v;

        if (*end == ',') p = end + 1;
        else break;
    }

    return count;
}


static inline int parse_ints(const char *line, int32_t *out, int max)
{
    int count = 0;
    const char *p = line;

    while (*p && count < max)
    {
        char *end;
        long v = strtol(p, &end, 10);
        if (end == p) break;

        out[count++] = (int32_t)v;

        if (*end == ',') p = end + 1;
        else break;
    }
    return count;
}


static inline int parse_uints(const char *line, uint32_t *out, int max)
{
    int count = 0;
    const char *p = line;

    while (*p && count < max)
    {
        char *end;
        unsigned long v = strtoul(p, &end, 10);
        if (end == p) break;

        out[count++] = (uint32_t)v;

        if (*end == ',') p = end + 1;
        else break;
    }
    return count;
}


int load_context(const char* text, cmd_ctx_t* ctx)
{
    if (!text) return LOAD_CTX_NO_STRING;

    const char *cmd = find_key(text, "CMD:");
    if (!cmd) return LOAD_CTX_LOAD_CMD_FAIL;

    //Load command str
    {
        const char *nl = strchr(cmd, '\n');
        size_t len = nl? (size_t)(nl - cmd) : strlen(cmd);
        if (len >= COMMAND_SIZE) len = COMMAND_SIZE - 1;

        memcpy(ctx->command_str, cmd, len);
        ctx->command_str[len] = '\0';
    }

    const char *f = find_key(text, "F:");
    if(f) 
    {
        ctx->num_floats = parse_floats(f, ctx->float_args, FLOAT_ARGS);
        ctx->float_status = LOAD_CTX_OK;
    } 
    
    else {
        ctx->num_floats = 0;
        ctx->float_status = LOAD_CTX_NO_KEY;
    }

    const char *i = find_key(text, "I:");
    if (i) {
        ctx->num_ints = parse_ints(i, ctx->int_args, INT_ARGS);
        ctx->int_status = LOAD_CTX_OK;
    } else {
        ctx->num_ints = 0;
        ctx->int_status = LOAD_CTX_NO_KEY;
    }

    const char *u = find_key(text, "U:");
    if (u) {
        ctx->num_uints = parse_uints(u, ctx->uint_args, UINT_ARGS);
        ctx->uint_status = LOAD_CTX_OK;
    } else {
        ctx->num_uints = 0;
        ctx->uint_status = LOAD_CTX_NO_KEY;
    }

    return LOAD_CTX_OK;
}

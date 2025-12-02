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


/*pseudo-json obj format for this API

"{
    "command"       : "ex_cmd",

    "float_args"    : ["0", "4.1", "42.2"],

    "int_args"      : ["-1", "2"],

    "uint_args"     : ["6", "7"]
}"

The system is not whitespace-sensitive
*/



static int load_value(const char* json, const char* key, const char* startstr, const char* endstr, char* dest)
{
    const char *pos = strstr(json, key);
    if (!pos) return LOAD_VALUE_NO_KEY;

    pos = strchr(pos + strlen(key), startstr);
    if(!pos) return LOAD_VALUE_NO_VAL;
    pos++;

    const char* end = strchr(pos, endstr);
    if(!end) return LOAD_VALUE_MALFORMED_VAL;

    size_t len = (size_t)(end - pos);

    if(len == 0) return LOAD_VALUE_EMPTY_VAL;
    if(len > MAX_ARR_SIZE) return LOAD_VALUE_OVERSIZED;

    memcpy(dest, pos, len);
    dest[len] = '\0';
    return LOAD_VALUE_OK
}




int load_context(const char* json, cmd_ctx_t* ctx) 
{
    if(load_value(json, "\"command\"", '"', '"', ctx->command_str) != LOAD_VALUE_OK) return LOAD_CTX_LOAD_CMD_FAIL;
    
    char fbuff[MAX_ARR_SIZE];
    char ibuff[MAX_ARR_SIZE];
    char ubuff[MAX_ARR_SIZE];

    char fstr[10];
    char istr[10];
    char ustr[10];

    ctx->float_status = load_value(json, "\"float_args\"", '[', ']', fbuff);
    ctx->int_status = load_value(json, "\"int_args\"", '[', ']', ibuff);
    ctx->uint_status = load_value(json, "\"uint_args\"", '[', ']', ubuff);

    uint32_t i = 0;
    if(ctx->float_status == LOAD_VALUE_OK)    //"["0", "4.1", "42.2"]"
    {
        const char* start = strchr(fbuff, '"');
        while(start != NULL)
        {
            start++;
            const char* end = strchr(start+1, '"');
            if(end)
            {
                size_t len = (size_t)(end - start);
                memcpy(fstr, start, len);
                fstr[len] = '\0';
                ctx->float_args[i] = atof(fstr);
                start = strchr(end+1, '"');
                i++;
            }

            else start = NULL;
        }
    }
    ctx->num_floats = i;

    i = 0;
    if(ctx->int_status == LOAD_VALUE_OK)    //"["0", "4.1", "42.2"]"
    {
        const char* start = strchr(ibuff, '"');
        while(start != NULL)
        {
            start++;
            const char* end = strchr(start+1, '"');
            if(end)
            {
                size_t len = (size_t)(end - start);
                memcpy(istr, start, len);
                istr[len] = '\0';
                ctx->float_args[i] = atoi(istr);
                start = strchr(end+1, '"');
                i++;
            }

            else start = NULL;
        }
    }
    ctx->num_ints = i;

    i = 0;
    if(ctx->uint_status == LOAD_VALUE_OK)    //"["0", "4.1", "42.2"]"
    {
        const char* start = strchr(ubuff, '"');
        while(start != NULL)
        {
            start++;
            const char* end = strchr(start+1, '"');
            if(end)
            {
                size_t len = (size_t)(end - start);
                memcpy(ustr, start, len);
                ustr[len] = '\0';
                ctx->float_args[i] = atoi(ustr);
                start = strchr(end+1, '"');
                i++;
            }

            else start = NULL;
        }
    }
    ctx->num_uints = i;

    return LOAD_CTX_OK;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

#include "server.h"
#include "control.h"
#include "hw_common.h"

#define SERVER_PORT 5555
#define MAX_BYTES    1024

#define NUM_CMDS 14

static cmd_entry_t gCmds[NUM_CMDS] = {
    {"set_led",      cmd_set_led,      0, 0, 1},
    {"reset_fpga",   cmd_reset_fpga,   0, 0, 0},
    {"set_dac",      cmd_set_dac,      1, 0, 1},
    {"get_adc",      cmd_get_adc,      0, 0, 0},
    {"check_signed", cmd_check_signed, 0, 0, 1},
    {"set_rotation", cmd_set_rot,      1, 0, 0},
    {"get_frame",    cmd_get_frame,    0, 0, 2},
    {"test_frame",   cmd_test_frame,   0, 0, 1},
    {"set_pid",      cmd_set_pid,      4, 0, 4},
    {"set_fir",      cmd_set_fir,      0, 0, 1},
    {"set_nco",      cmd_set_nco,      2, 0, 1},
    {"config_io",    cmd_config_io,    0, 0, 3},
    {"config_demod", cmd_config_demod, 0, 0, 3},
    {"sweep_ramp",   cmd_sweep_ramp,   2, 0, 4},
};


static const char* find_key(const char* src, const char* key)
{
    size_t klen = strlen(key);
    const char* p = src;
    while (*p)
    {
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
    while (*p && count < max)
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

static inline int parse_ints(const char* line, int32_t* out, int max)
{
    int count = 0;
    const char* p = line;
    while (*p && count < max)
    {
        char* end;
        long v = strtol(p, &end, 10);
        if (end == p) break;
        out[count++] = (int32_t)v;
        if (*end == ',') p = end + 1;
        else break;
    }
    return count;
}

static inline int parse_uints(const char* line, uint32_t* out, int max)
{
    int count = 0;
    const char* p = line;
    while (*p && count < max)
    {
        char* end;
        unsigned long v = strtoul(p, &end, 10);
        if (end == p) break;
        out[count++] = (uint32_t)v;
        if (*end == ',') p = end + 1;
        else break;
    }
    return count;
}

static int load_context(const char* text, cmd_ctx_t* ctx)
{
    if (!text) return LOAD_CTX_NO_STRING;

    const char* cmd = find_key(text, "CMD:");
    if (!cmd) return LOAD_CTX_LOAD_CMD_FAIL;

    {
        const char* nl  = strchr(cmd, '\n');
        size_t      len = nl ? (size_t)(nl - cmd) : strlen(cmd);
        if (len >= COMMAND_SIZE) len = COMMAND_SIZE - 1;
        memcpy(ctx->name, cmd, len);
        ctx->name[len] = '\0';
    }

    const char* f = find_key(text, "F:");
    if (f) { ctx->num_floats = parse_floats(f, ctx->float_args, FLOAT_ARGS); ctx->float_status = LOAD_CTX_OK; }
    else   { ctx->num_floats = 0; ctx->float_status = LOAD_CTX_NO_KEY; }

    const char* i = find_key(text, "I:");
    if (i) { ctx->num_ints = parse_ints(i, ctx->int_args, INT_ARGS); ctx->int_status = LOAD_CTX_OK; }
    else   { ctx->num_ints = 0; ctx->int_status = LOAD_CTX_NO_KEY; }

    const char* u = find_key(text, "U:");
    if (u) { ctx->num_uints = parse_uints(u, ctx->uint_args, UINT_ARGS); ctx->uint_status = LOAD_CTX_OK; }
    else   { ctx->num_uints = 0; ctx->uint_status = LOAD_CTX_NO_KEY; }

    return LOAD_CTX_OK;
}

static int dispatch_command(cmd_ctx_t* ctx, int* code)
{
    cmd_entry_t curr_cmd;
    memset(&curr_cmd, 0, sizeof(curr_cmd));

    bool cmd_found = false;
    int i = 0;
    while (i < NUM_CMDS)
    {
        if (strncmp(ctx->name, gCmds[i].name, COMMAND_SIZE) == 0)
        {
            memcpy(&curr_cmd, &gCmds[i], sizeof(curr_cmd));
            cmd_found = true;
            break;
        }
        i++;
    }
    if (!cmd_found) return DISPATCH_CMD_NO_CMD;

    if (ctx->num_floats < curr_cmd.required_floats) return DISPATCH_CMD_FLOAT_ARG_MISMATCH;
    if (ctx->num_ints   < curr_cmd.required_ints)   return DISPATCH_CMD_INT_ARG_MISMATCH;
    if (ctx->num_uints  < curr_cmd.required_uints)  return DISPATCH_CMD_UINT_ARG_MISMATCH;

    DEBUG_INFO("Dispatching: %s...\n", gCmds[i].name);
    *code = curr_cmd.func(ctx);
    DEBUG_INFO("done dispatch\n");
    return DISPATCH_CMD_OK;
}

static int write_all(int fd, const void* buff, size_t len)
{
    const uint8_t* p = buff;
    size_t rem = len;
    while (rem > 0)
    {
        ssize_t n = write(fd, p, rem);
        if (n < 0)  { if (errno == EINTR) continue; return -1; }
        if (n == 0) return -1;
        p += n; rem -= n;
    }
    return 0;
}

static void send_response(int client_fd, int func_status, cmd_ctx_t ctx)
{
    char*  buff  = (char*)malloc(MAX_BYTES);
    int    offset = 0;
    size_t cap   = MAX_BYTES;

    offset += snprintf(buff + offset, cap - offset, "type:output\nname:%s\n", ctx.name);
    offset += snprintf(buff + offset, cap - offset, "status:%d\n", func_status);

    for (size_t k = 0; k < ctx.output.num_outputs; k++)
    {
        output_item_t o = ctx.output.output_items[k];
        if      (o.tag == FLOAT_TAG) offset += snprintf(buff + offset, cap - offset, "%s:%f\n", o.name, o.data.f);
        else if (o.tag == INT_TAG)   offset += snprintf(buff + offset, cap - offset, "%s:%d\n", o.name, o.data.i);
        else if (o.tag == UINT_TAG)  offset += snprintf(buff + offset, cap - offset, "%s:%u\n", o.name, o.data.u);
        else { offset += snprintf(buff + offset, cap - offset, "ERROR: unknown tag!\n"); break; }
    }

    ssize_t n = write_all(client_fd, buff, offset);
    free(buff);
    (void)n;
}

int main(void)
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("ERROR: can't create socket\n"); return 1; }

    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("ERROR: can't setsocketopt\n");
        close(listen_fd);
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("ERROR: can't bind\n");
        close(listen_fd);
        return 1;
    }

    if (listen(listen_fd, 8) < 0)
    {
        perror("ERROR: can't listen\n");
        close(listen_fd);
        return 1;
    }

    DEBUG_INFO("Server listening on port %d...\n", SERVER_PORT);
    DEBUG_INFO("Initializing hardware...\n");

    int pdh_code = pdh_Init();
    int dma_code = dma_Init();
    int uio_code = uio_Init();

    if (pdh_code == PDH_OK && dma_code == PDH_OK && uio_code == PDH_OK)
    {
        while (true)
        {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);

            int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd < 0)
            {
                if (errno == EINTR) continue;
                perror("ERROR: can't accept client connection\n");
                break;
            }

            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
            DEBUG_INFO("Client connected from %s:%d\n", ip_str, ntohs(client_addr.sin_port));

            char inbuff[MAX_BYTES];
            ssize_t nread = read(client_fd, inbuff, MAX_BYTES);
            if (nread <= 0)
            {
                if (nread < 0) perror("ERROR: can't read\n");
                close(client_fd);
                continue;
            }
            inbuff[nread - 1] = '\0';

            cmd_ctx_t ctx;
            memset(&ctx, 0, sizeof(ctx));

            int load_ctx = load_context(inbuff, &ctx);
            if (load_ctx == LOAD_CTX_OK)
            {
                int func_status = 0;
                int dispatch = dispatch_command(&ctx, &func_status);
                if (dispatch != DISPATCH_CMD_OK)
                    DEBUG_INFO("DISPATCH FAILURE: %d", dispatch);
                send_response(client_fd, func_status, ctx);
            }

            close(client_fd);
        }
    }
    else
    {
        DEBUG_INFO("pdh_Init()=%d  dma_Init()=%d  uio_Init()=%d\n",
                   pdh_code, dma_code, uio_code);
    }

    uio_Release();
    dma_Release();
    pdh_Release();
    close(listen_fd);
    return 0;
}

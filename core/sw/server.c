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
#define MAX_BYTES    1024

#define NUM_CMDS 1

//Example syntax:
/*
"CMD:ex_cmd'\n'
F:0,4.1,42.2'\n'
I:-1,2'\n'
U:6,7"
*/


static cmd_entry_t gCmds[NUM_CMDS] = {{"lock_in", cmd_lock_in, 3, 0, 2}};


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


static int load_context(const char* text, cmd_ctx_t* ctx)
{
    if (!text) return LOAD_CTX_NO_STRING;

    const char *cmd = find_key(text, "CMD:");
    if (!cmd) return LOAD_CTX_LOAD_CMD_FAIL;

    //Load command str
    {
        const char *nl = strchr(cmd, '\n');
        size_t len = nl? (size_t)(nl - cmd) : strlen(cmd);
        if (len >= COMMAND_SIZE) len = COMMAND_SIZE - 1;

        memcpy(ctx->name, cmd, len);
        ctx->name[len] = '\0';
    }

    const char *f = find_key(text, "F:");
    if(f) 
    {
        ctx->num_floats = parse_floats(f, ctx->float_args, FLOAT_ARGS);
        ctx->float_status = LOAD_CTX_OK;
    } 
    else 
    {
        ctx->num_floats = 0;
        ctx->float_status = LOAD_CTX_NO_KEY;
    }

    const char *i = find_key(text, "I:");
    if (i) 
    {
        ctx->num_ints = parse_ints(i, ctx->int_args, INT_ARGS);
        ctx->int_status = LOAD_CTX_OK;
    } 
    else 
    {
        ctx->num_ints = 0;
        ctx->int_status = LOAD_CTX_NO_KEY;
    }

    const char *u = find_key(text, "U:");
    if (u) 
    {
        ctx->num_uints = parse_uints(u, ctx->uint_args, UINT_ARGS);
        ctx->uint_status = LOAD_CTX_OK;
    } 
    else {
        ctx->num_uints = 0;
        ctx->uint_status = LOAD_CTX_NO_KEY;
    }

    return LOAD_CTX_OK;
}


static int dispatch_cmd(cmd_ctx_t* ctx, int* code)
{
    cmd_entry_t curr_cmd;
    bool cmd_found = false;
    int i = 0;
    while(i < NUM_CMDS)
    {
        if(strncmp(ctx->name, gCmds[i].name, sizeof(ctx->name)) == 0)
        {
            curr_cmd = gCmds[i];
            cmd_found = true;
            break;
        }
        i++;
    }
    if(!cmd_found) return DISPATCH_CMD_NO_CMD;

    if(ctx->num_floats != curr_cmd.required_floats) return DISPATCH_CMD_FLOAT_ARG_MISMATCH;
    if(ctx->num_ints != curr_cmd.required_ints) return DISPATCH_CMD_INT_ARG_MISMATCH;
    if(ctx->num_uints != curr_cmd.required_uints) return DISPATCH_CMD_UINT_ARG_MISMATCH;

    *code = curr_cmd.func(ctx);
    return DISPATCH_CMD_OK;
}


static void send_response(int client_fd, int func_status, cmd_ctx_t ctx)
{
    char buff[MAX_BYTES];
    int offset = 0;

    offset += snprintf(buff + offset, sizeof(buff) - offset, "type:output\nname:%s\n", ctx.name);
    offset += snprintf(buff + offset, sizeof(buff) - offset, "status:%d\n", func_status);

    for(uint16_t i = 0; i < ctx.output.num_outputs; i++)
    {
        output_item_t output = ctx.output.output_items[i];

        if(output.tag == FLOAT_TAG) offset += snprintf(buff + offset, sizeof(buff) - offset, "%s:%f\n", output.name, output.data.f);
        else if(output.tag == INT_TAG) offset += snprintf(buff + offset, sizeof(buff) - offset, "%s:%d\n", output.name, output.data.i);
        else if(output.tag == UINT_TAG) offset += snprintf(buff + offset, sizeof(buff) - offset, "%s:%u\n", output.name, output.data.u);

        else
        {
            offset += snprintf(buff + offset, sizeof(buff) - offset, "%s\n", "ERROR: unknown tag!");
            break;
        }
    }
    ssize_t n = write(client_fd, buff, offset);
    (void)n;
}



int main(void)
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);    //allocate socket obj for listener socket instance. specify IPV4, TCP, defaults
    if(listen_fd < 0)
    {
        perror("ERROR: can't create socket\n");
        return 1;
    }

    int opt = 1;
    if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)    //configure socket through its fd, SOL_SOCKET specifies we are configuring at the socket level i.e. not reconfiguring protocol or IP level stuff. we want to allow binding to ip adresses (interfaces) that may already be in use so we set SO_REUSEADDR to 1
    {
        perror("ERROR: can't setsocketopt\n");
        close(listen_fd);
        return 1;
    }

    //specify target addr config
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT); //network byte order required for TCP/IP machinery to work properly
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //let socket get data from any visible IP

    if(bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)  //updates kernel LUT to see that this socket owns port 5555 and the given IP addr set, in this case all ip addrs
    {
        perror("ERROR: can't bind\n");
        close(listen_fd);
        return 1;
    }

    if(listen(listen_fd, 8) < 0)    //allocates acceptance queue and starts the kernel's interrupt-driven TCP state machine used to populate it. 
    {
        perror("ERROR: can't listen\n");
        close(listen_fd);
        return 1;
    }
    printf("Server listening on port %d...\n", SERVER_PORT);

    while(true)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len); //Every time a TCP handshake completes on a listening socket, the kernel automatically creates a child socket and places it into the accept queue. accept creates an fd for the child socket at the front of this queue. do note both accepted and listener sockets are both permutations of the core socket data structure.

        if(client_fd < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }

            else
            {
                perror("ERROR: can't accept client connection\n");
                break;
            }
        }

        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
        printf("Client connected from %s:%d\n", ip_str, ntohs(client_addr.sin_port));

        char inbuff[MAX_BYTES];
        ssize_t nread = read(client_fd, inbuff, MAX_BYTES); //pull bytes from TCP buffer (mapping to last accepted TCP transaction sequence) that kernel already filled with packet data delivered after handshake and move them into inbuff
        if(nread <= 0)
        {
            if(nread < 1) perror("ERROR: can't read\n");
            close(client_fd);
            continue;
        }
        inbuff[nread-1] = '\0';

        cmd_ctx_t ctx;
        memset(&ctx, 0, sizeof(ctx));

        int loadCtx = load_context(inbuff, &ctx);
        if(loadCtx == LOAD_CTX_OK)
        {
            int func_status;
            int dispatch = dispatch_cmd(&ctx, &func_status);

            if(dispatch != DISPATCH_CMD_OK)
            {
                printf("DISPATCH FAILURE: %d", dispatch);
            }
            else
            {
                send_response(client_fd, func_status, ctx);
            }
        }

        close(client_fd);
    }

    close(listen_fd);
    return 0;
}








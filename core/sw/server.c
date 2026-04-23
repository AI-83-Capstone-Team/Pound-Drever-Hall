#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

#include "server.h"
#include "control.h"
#include "hw_common.h"

#define SERVER_PORT     5555
#define MAX_BYTES       1024
#define NUM_CMDS        14


/* ══════════════════════════════════════════════════════════════════════════
 * Dispatch table
 *
 * Each entry has:
 *   send_func  — Thread 1 phase: strobes GP0, returns initial status.
 *                NULL  → this is a mono command (Thread 1 runs it to completion).
 *   cb_func    — Thread 2 phase: called after interrupt, returns final status.
 *                NULL  → mono command.
 *   mono_func  — full monolithic handler for multi-sub-command and special cmds.
 *                NULL  → single-GP0 (split) command.
 * ══════════════════════════════════════════════════════════════════════════ */

typedef int  (*cmd_send_t)(cmd_ctx_t*);
typedef int  (*cmd_cb_t  )(cmd_ctx_t*, pdh_callback_t);
typedef int  (*cmd_mono_t)(cmd_ctx_t*);

typedef struct
{
    const char*   name;
    cmd_send_t    send_func;
    cmd_cb_t      cb_func;
    cmd_mono_t    mono_func;
    uint8_t required_floats;
    uint8_t required_ints;
    uint8_t required_uints;
} dispatch_entry_t;

static dispatch_entry_t gCmds[NUM_CMDS] = {
    /* name            send_func             cb_func               mono_func       f  i  u */
    {"set_led",        cmd_set_led_send,     cmd_set_led_cb,       NULL,           0, 0, 1},
    {"reset_fpga",     NULL,                 NULL,                 cmd_reset_fpga, 0, 0, 0},
    {"set_dac",        cmd_set_dac_send,     cmd_set_dac_cb,       NULL,           1, 0, 1},
    {"get_adc",        cmd_get_adc_send,     cmd_get_adc_cb,       NULL,           0, 0, 0},
    {"check_signed",   cmd_check_signed_send,cmd_check_signed_cb,  NULL,           0, 0, 1},
    {"set_rotation",   NULL,                 NULL,                 cmd_set_rot,    1, 0, 0},
    {"get_frame",      cmd_get_frame_send,   cmd_get_frame_cb,     NULL,           0, 0, 2},
    {"test_frame",     NULL,                 NULL,                 cmd_test_frame, 0, 0, 1},
    {"set_pid",        NULL,                 NULL,                 cmd_set_pid,    4, 0, 4},
    {"set_fir",        NULL,                 NULL,                 cmd_set_fir,    0, 0, 1},
    {"set_nco",        NULL,                 NULL,                 cmd_set_nco,    2, 0, 1},
    {"config_io",      cmd_config_io_send,   cmd_config_io_cb,     NULL,           0, 0, 3},
    {"config_demod",   cmd_config_demod_send,cmd_config_demod_cb,  NULL,           0, 0, 3},
    {"sweep_ramp",     NULL,                 NULL,                 cmd_sweep_ramp, 2, 0, 4},
};


/* ══════════════════════════════════════════════════════════════════════════
 * Pending queue — single entry (command_thread serialises access)
 *
 * command_thread writes after calling send_func, then blocks on g_complete_cond.
 * callback_thread reads after the UIO interrupt fires, then signals g_complete_cond.
 * ══════════════════════════════════════════════════════════════════════════ */

typedef struct
{
    cmd_ctx_t   ctx;
    int         client_fd;
    cmd_cb_t    cb_func;
} pending_t;

static pending_t           g_pending;
static bool                g_pending_valid = false;
static pthread_mutex_t     g_pending_mtx   = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t      g_pending_cond  = PTHREAD_COND_INITIALIZER;

/* Completion signal — command_thread blocks here after dispatching a split
 * command; callback_thread signals after send_response() + close(). */
static bool                g_complete      = false;
static pthread_cond_t      g_complete_cond = PTHREAD_COND_INITIALIZER;

/* ══════════════════════════════════════════════════════════════════════════
 * Work queue — accept_thread pushes, command_thread pops, one at a time.
 * ══════════════════════════════════════════════════════════════════════════ */

typedef struct
{
    int     client_fd;
    char    buf[MAX_BYTES];
    ssize_t nread;
} work_item_t;

#define WORK_QUEUE_CAP 16
static work_item_t     g_work_queue[WORK_QUEUE_CAP];
static int             g_wq_head     = 0;
static int             g_wq_tail     = 0;
static int             g_wq_count    = 0;
static pthread_mutex_t g_wq_mtx      = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_wq_notempty = PTHREAD_COND_INITIALIZER;
static pthread_cond_t  g_wq_notfull  = PTHREAD_COND_INITIALIZER;


/* ══════════════════════════════════════════════════════════════════════════
 * Parsing helpers (unchanged from original server)
 * ══════════════════════════════════════════════════════════════════════════ */

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


/* ══════════════════════════════════════════════════════════════════════════
 * Response helpers
 * ══════════════════════════════════════════════════════════════════════════ */

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


/* ══════════════════════════════════════════════════════════════════════════
 * Thread 2 — callback thread
 *
 * Blocks on UIO fd (interrupt from FPGA).
 * On each interrupt:
 *   - If g_pending_valid: real interrupt from a _send call → process callback.
 *   - Otherwise: spurious interrupt from a mono sub-command → discard.
 * ══════════════════════════════════════════════════════════════════════════ */

static void* callback_thread(void* arg)
{
    (void)arg;

    while (1)
    {
        /* Block until FPGA asserts IRQ_F2P[0] */
        if (uio_wait_irq() != PDH_OK)
        {
            perror("uio_wait_irq");
            continue;
        }

        /* Wait up to 500 µs for Thread 1 to push to g_pending.
         * In practice Thread 1 sets g_pending in <1 µs (interrupt
         * propagation from FPGA takes ~5–20 µs), so the condvar
         * wait normally exits immediately on first check. */
        pthread_mutex_lock(&g_pending_mtx);

        if (!g_pending_valid)
        {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_nsec += 500000;   /* 500 µs */
            if (ts.tv_nsec >= 1000000000L)
            {
                ts.tv_sec++;
                ts.tv_nsec -= 1000000000L;
            }
            pthread_cond_timedwait(&g_pending_cond, &g_pending_mtx, &ts);
        }

        bool has_pending = g_pending_valid;

        if (has_pending)
        {
            /* Snapshot and clear */
            pending_t pend      = g_pending;
            g_pending_valid     = false;
            pthread_mutex_unlock(&g_pending_mtx);

            /* Read callback register and re-enable interrupt */
            pdh_callback_t cb = {0};
            pdh_get_callback(&cb);
            uio_ack_irq();

            /* Dispatch to the callback handler */
            int status = pend.cb_func(&pend.ctx, cb);

            send_response(pend.client_fd, status, pend.ctx);
            close(pend.client_fd);

            /* Unblock command_thread so it can dequeue the next command */
            pthread_mutex_lock(&g_pending_mtx);
            g_complete = true;
            pthread_cond_signal(&g_complete_cond);
            pthread_mutex_unlock(&g_pending_mtx);
        }
        else
        {
            /* Spurious interrupt (from mono sub-command) — just re-enable */
            pthread_mutex_unlock(&g_pending_mtx);
            uio_ack_irq();
        }
    }

    return NULL;
}


/* ══════════════════════════════════════════════════════════════════════════
 * accept_thread — accepts connections, reads the raw buffer, pushes to queue.
 * Decoupled from command processing so the listen socket stays responsive.
 * ══════════════════════════════════════════════════════════════════════════ */

static void* accept_thread(void* arg)
{
    int listen_fd = *(int*)arg;

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0)
        {
            if (errno == EINTR) continue;
            perror("ERROR: accept_thread: accept\n");
            break;
        }

        char    ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
        DEBUG_INFO("Client connected from %s:%d\n", ip_str, ntohs(client_addr.sin_port));

        work_item_t item;
        item.client_fd = client_fd;
        item.nread     = read(client_fd, item.buf, MAX_BYTES);
        if (item.nread <= 0)
        {
            if (item.nread < 0) perror("ERROR: accept_thread: read\n");
            close(client_fd);
            continue;
        }

        pthread_mutex_lock(&g_wq_mtx);
        while (g_wq_count == WORK_QUEUE_CAP)
            pthread_cond_wait(&g_wq_notfull, &g_wq_mtx);
        g_work_queue[g_wq_tail] = item;
        g_wq_tail = (g_wq_tail + 1) % WORK_QUEUE_CAP;
        g_wq_count++;
        pthread_cond_signal(&g_wq_notempty);
        pthread_mutex_unlock(&g_wq_mtx);
    }

    return NULL;
}


/* ══════════════════════════════════════════════════════════════════════════
 * command_thread — dequeues one item at a time, dispatches, waits for
 * completion of split commands before dequeuing the next.
 * ══════════════════════════════════════════════════════════════════════════ */

static void* command_thread(void* arg)
{
    (void)arg;

    while (1)
    {
        /* Dequeue one work item */
        pthread_mutex_lock(&g_wq_mtx);
        while (g_wq_count == 0)
            pthread_cond_wait(&g_wq_notempty, &g_wq_mtx);
        work_item_t item = g_work_queue[g_wq_head];
        g_wq_head = (g_wq_head + 1) % WORK_QUEUE_CAP;
        g_wq_count--;
        pthread_cond_signal(&g_wq_notfull);
        pthread_mutex_unlock(&g_wq_mtx);

        /* Parse */
        item.buf[item.nread - 1] = '\0';
        cmd_ctx_t ctx;
        memset(&ctx, 0, sizeof(ctx));
        if (load_context(item.buf, &ctx) != LOAD_CTX_OK)
        {
            close(item.client_fd);
            continue;
        }

        /* Look up command */
        dispatch_entry_t* entry = NULL;
        for (int i = 0; i < NUM_CMDS; i++)
        {
            if (strncmp(ctx.name, gCmds[i].name, COMMAND_SIZE) == 0)
            {
                entry = &gCmds[i];
                break;
            }
        }
        if (!entry)
        {
            DEBUG_INFO("Unknown command: %s\n", ctx.name);
            close(item.client_fd);
            continue;
        }

        /* Argument count validation */
        if (ctx.num_floats < entry->required_floats ||
            ctx.num_ints   < entry->required_ints   ||
            ctx.num_uints  < entry->required_uints)
        {
            DEBUG_INFO("Arg mismatch for %s\n", entry->name);
            close(item.client_fd);
            continue;
        }

        if (entry->mono_func)
        {
            /* Mono: runs to completion here */
            DEBUG_INFO("Dispatching mono: %s\n", entry->name);
            int status = entry->mono_func(&ctx);
            send_response(item.client_fd, status, ctx);
            close(item.client_fd);
        }
        else
        {
            /* Split: fire strobe, push pending, then block until callback_thread
             * signals completion — ensures only one command is in-flight at a time. */
            DEBUG_INFO("Dispatching send: %s\n", entry->name);
            int send_status = entry->send_func(&ctx);

            pthread_mutex_lock(&g_pending_mtx);
            g_pending.ctx       = ctx;
            g_pending.client_fd = item.client_fd;
            g_pending.cb_func   = entry->cb_func;
            g_pending_valid     = true;
            g_complete          = false;
            pthread_cond_signal(&g_pending_cond);

            while (!g_complete)
                pthread_cond_wait(&g_complete_cond, &g_pending_mtx);
            pthread_mutex_unlock(&g_pending_mtx);

            (void)send_status;
        }
    }

    return NULL;
}


/* ══════════════════════════════════════════════════════════════════════════
 * main
 * ══════════════════════════════════════════════════════════════════════════ */

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

    if (pdh_code != PDH_OK || dma_code != PDH_OK || uio_code != PDH_OK)
    {
        DEBUG_INFO("pdh_Init()=%d  dma_Init()=%d  uio_Init()=%d\n",
                   pdh_code, dma_code, uio_code);
        close(listen_fd);
        return 1;
    }

    /* Start callback thread, accept thread, and command thread */
    pthread_t cb_tid, accept_tid, cmd_tid;
    if (pthread_create(&cb_tid, NULL, callback_thread, NULL) != 0)
    {
        perror("ERROR: pthread_create cb\n");
        close(listen_fd);
        return 1;
    }
    pthread_detach(cb_tid);

    if (pthread_create(&cmd_tid, NULL, command_thread, NULL) != 0)
    {
        perror("ERROR: pthread_create cmd\n");
        close(listen_fd);
        return 1;
    }
    pthread_detach(cmd_tid);

    DEBUG_INFO("Server ready.\n");

    /* accept_thread owns the accept loop; main blocks until it exits */
    if (pthread_create(&accept_tid, NULL, accept_thread, &listen_fd) != 0)
    {
        perror("ERROR: pthread_create accept\n");
        close(listen_fd);
        return 1;
    }
    pthread_join(accept_tid, NULL);

    uio_Release();
    dma_Release();
    pdh_Release();
    close(listen_fd);
    return 0;
}

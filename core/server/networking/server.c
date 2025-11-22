#include <socket.h>
#include "server.h"
#include "lock_in.h"

//placeholder init params
enum dacInit
{
    DAC_INIT = 0,
    DAC_HIGH = 69,
    DAC_LOW = -69,
    DAC_STEP = 1
};


static lock_in_ctx_t ctx = {
    .dac_init = DAC_INIT,
    .dac_high = DAC_HIGH,
    .dac_low = DAC_LOW,
    .dac_step = DAC_STEP
};


int main(){
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(5000),
        .sin_addr.s_addr = INADDR_ANY,
    };

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 1);

    while (1) {
        int client = accept(server_fd, NULL, NULL);

        char buf[256];
        int n = read(client, buf, sizeof(buf)-1);
        buf[n] = '\0';

        if (strcmp(buf, "RUN_LOCKIN") == 0) {
            int ret = lock_in_run(&ctx);
            sprintf(buf, "OK %d\n", ret);
        } 
        
        else 
        {
            sprintf(buf, "ERR\n");
        }

        write(client, buf, strlen(buf));
        close(client);
    }

    return 0;
}


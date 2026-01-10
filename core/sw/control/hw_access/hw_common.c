#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include "hw_common.h"


float gAdcMirror[ADC_BUFFER_SIZE];
sweep_entry_t gSweepBuff[SWEEP_BUFFER_SIZE];

#define AXI_BUS_OFFSET 0x42000000
#define AXI_BUS_DEVBIND "/dev/mem"
#define AXI_WRITE_OFFSET    8   //TODO: Check these
#define AXI_READ_OFFSET     0

void* gAxiMap;


int pdh_Init()
{
    int fd = open(AXI_BUS_DEVBIND, O_RDWR);
    if(fd < 0)
    {
        perror("open");
        return 1;
    }

    gAxiMap = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, AXI_BUS_OFFSET); 
    close(fd);

    return PDH_OK;
}


int pdh_Release()
{
    munmap(gAxiMap, sysconf(_SC_PAGE_SIZE));
    return PDH_OK;
}


int pdh_send_cmd(pdh_cmd_t cmd)
{
    *((volatile uint32_t*)(gAxiMap + AXI_WRITE_OFFSET)) = cmd.raw;
    __sync_synchronize();
    return PDH_OK;
}


int pdh_get_callback(pdh_callback_t* callback)
{
    callback->raw = *((volatile uint32_t*)(gAxiMap + AXI_READ_OFFSET));
    return PDH_OK;
}

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include "hw_common.h"


float gAdcMirror[ADC_BUFFER_SIZE];
sweep_entry_t gSweepBuff[SWEEP_BUFFER_SIZE];


static void* gAxiMap;
static void* gDmaMap;

int dma_Init()
{
    int fd = open(AXI_BUS_DEVBIND, O_RDONLY | O_SYNC);
    if(fd < 0)
    {
        perror("open");
        return 1;
    }
    
    gDmaMap = mmap(NULL, HP0_RANGE, PROT_READ, MAP_SHARED, fd, HP0_BASE_ADDR); 
    close(fd);
    
    return PDH_OK;
}

int dma_Release()
{
    munmap(gDmaMap, HP0_RANGE);
    return PDH_OK;
}

uint64_t dma_get_frame(uint32_t byte_offset)
{
    __sync_synchronize();
    return *((volatile uint64_t*)((uint8_t*)gDmaMap + byte_offset));
}




int pdh_Init()
{
    int fd = open(AXI_BUS_DEVBIND, O_RDWR | O_SYNC);
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
    munmap(gAxiMap, sysconf(_SC_PAGESIZE));
    return PDH_OK;
}


 inline __attribute__((always_inline)) int pdh_send_cmd(pdh_cmd_t cmd)
{
    *((volatile uint32_t*)((uint8_t*)gAxiMap + AXI_WRITE_OFFSET)) = cmd.raw;
    __sync_synchronize();
    return PDH_OK;
}


inline __attribute__((always_inline)) int pdh_get_callback(pdh_callback_t* callback)
{
    __sync_synchronize();
    callback->raw = *((volatile uint32_t*)((uint8_t*)gAxiMap + AXI_READ_OFFSET));
    return PDH_OK;
}

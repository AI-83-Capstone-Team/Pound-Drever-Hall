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

//#define HP0_BASE_ADDR 0x1F000000
#define HP0_BASE_ADDR 0x43000000
#define HP0_RANGE 0xC3500
#define MAP_LEN 0x000C4000 //DMA region size rounded up to the nearest 4096-byte chunk
#define DMA_WRITE_OFFSET 0
#define DMA_READ_OFFSET 0

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
    
    gDmaMap = mmap(NULL, MAP_LEN, PROT_READ, MAP_SHARED, fd, HP0_BASE_ADDR); 
    close(fd);
    
    return PDH_OK;
}

int dma_Release()
{
    munmap(gDmaMap, MAP_LEN);
    return PDH_OK;
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

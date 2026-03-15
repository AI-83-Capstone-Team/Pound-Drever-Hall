#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include "hw_common.h"

static int gUioFd = -1;

int uio_Init(void)
{
    gUioFd = open(UIO_DEV, O_RDWR);
    if (gUioFd < 0)
    {
        perror("uio open");
        return 1;
    }
    return PDH_OK;
}

int uio_Release(void)
{
    if (gUioFd >= 0)
    {
        close(gUioFd);
        gUioFd = -1;
    }
    return PDH_OK;
}

int uio_wait_irq(void)
{
    uint32_t count;
    ssize_t n = read(gUioFd, &count, sizeof(count));
    return (n == (ssize_t)sizeof(count)) ? PDH_OK : 1;
}

int uio_ack_irq(void)
{
    uint32_t ack = 1;
    ssize_t n = write(gUioFd, &ack, sizeof(ack));
    return (n == (ssize_t)sizeof(ack)) ? PDH_OK : 1;
}

int uio_fd_get(void)
{
    return gUioFd;
}



struct __attribute__((packed))
{
    uint8_t dac1_feed_sel;
    uint8_t dac2_feed_sel;
    uint16_t pid_feed_sel;
}   gIoCfg;


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

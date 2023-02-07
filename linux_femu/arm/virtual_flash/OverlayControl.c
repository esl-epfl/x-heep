
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

// 1 --> Basic logging, 2 --> Debugging
#define LOGGING 2

#include "OverlayControl.h"
extern "C" {
#include <libxlnk_cma.h>
}

static volatile uint32_t * memMapAddr = NULL;
static int memMapFileDesc = -1;
static uint32_t memMapSize = 0;

volatile uint32_t * MapMemIO(uint32_t baseAddr, uint32_t mapSize)
{
  bool res = true;

    res = ( (memMapFileDesc = open("/dev/mem", O_RDWR | O_SYNC )) != -1);

    if (!res) {
    #ifdef LOGGING
        printf("Error opening file.\n");
    #endif
    }
    else {
    #if LOGGING == 2
        printf("File opened.\n");
    #endif
        memMapAddr = (volatile unsigned int *)mmap(NULL, mapSize, PROT_READ | PROT_WRITE, MAP_SHARED, memMapFileDesc, baseAddr);
        res = (memMapAddr != MAP_FAILED);
        if (!res) {
      #ifdef LOGGING
            printf("Memory mapping failed.\n");
      #endif
            close(memMapFileDesc);
            memMapFileDesc = -1;
      memMapAddr = NULL;
        }
    }

    if (res) {
    #ifdef LOGGING
        printf("Memory mapped.\n");
    #endif
    memMapSize = mapSize;
  }

    return memMapAddr;
}

bool UnmapMemIO()
{
    bool res = true;

    if ((memMapAddr != NULL) && (memMapAddr != MAP_FAILED)) {
        if ( munmap((void*)memMapAddr, memMapSize) ) {
      #ifdef LOGGING
            printf("Memory unmapping failed.\n");
      #endif
            res = false;
        }
        else {
      #ifdef LOGGING
            printf("Memory unmapped.\n");
      #endif
        }
    }

    if (memMapFileDesc != -1) {
        close(memMapFileDesc);
        memMapFileDesc = -1;
    }

    return res;
}

// Reference of functions to (de)allocate DMA memory.
// extern "C" {
// #include <libxlnk_cma.h>
// }
/* * Allocate a physically contiguos chunk of CMA memory and map it into
 * virtual memory space. Return this Virtual pointer. Returns -1 on failure.
void *cma_alloc(uint32_t len, uint32_t cacheable);
 * Return a physical memory address corresponding to a given Virtual address
 * pointer. Returns NULL on failure.
unsigned long cma_get_phy_addr(void *buf);
 * Free a previously allocated CMA memory chunk.
void cma_free(void *buf);
 * Returns the number of available CMA memiry pages which can be allocated.
uint32_t cma_pages_available();
 * Extra functions in case user needs to flush or invalidate Cache.
void cma_flush_cache(void *buf, unsigned int phys_addr, int size);
void cma_invalidate_cache(void *buf, unsigned int phys_addr, int size);
*/

#ifndef	OVERLAYCONTROL_H
#define	OVERLAYCONTROL_H

volatile uint32_t * MapMemIO(uint32_t baseAddr, uint32_t mapSize);
bool UnmapMemIO();

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

#endif // OVERLAYCONTROL_H


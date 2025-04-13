#ifndef X_CACHE_H
#define X_CACHE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cache_init(size_t cache_size); 
void cache_free(); 

void *X_cache_read(uint32_t flash_addr, uint32_t len); 

#endif // X_CACHE_H
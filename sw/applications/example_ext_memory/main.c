// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "core_v_mini_mcu.h"

#define BUFF_LEN 100

uint32_t  buffer[BUFF_LEN];
uint32_t  buffer_copy[BUFF_LEN];
uint32_t  buffer_rnd_index[BUFF_LEN];

#ifdef TARGET_SIM_SYSTEMC
#define CACHE_FLUSH   1
#define CACHE_BYPASS  2
#define CACHE_SIZE    32*1024
#endif

int main(int argc, char *argv[])
{


#if !defined(TARGET_SIM) || !defined(TARGET_SIM_SYSTEMC)
    #pragma message ( "this application must be ran only in the testbench" )
    return 0;
#endif

    uint32_t* ext_memory = (uint32_t*)EXT_SLAVE_START_ADDRESS;

#ifdef TARGET_SIM_SYSTEMC
    //last address of systemC memory used as a configuration register to flush or bypass
    volatile uint32_t* cache_cfg = (uint32_t*)(EXT_SLAVE_START_ADDRESS + CACHE_SIZE - 4);
#endif

    uint32_t random_number = 0;

    for(int i=0;i<BUFF_LEN;i++) {
        ext_memory[i] = i;
        random_number = rand() % BUFF_LEN;
        buffer_rnd_index[i] = random_number;
    }


    for(int i=0;i<BUFF_LEN;i++){
        buffer_copy[i] = ext_memory[buffer_rnd_index[i]];
    }

    //as the memory is 32kB, and the cache has 256 blocks
    //replace some cache line
    //writing at EXT_SLAVE_START_ADDRESS + 0x1000 (4kB)
    //the cache is 256*16B = 4KB, so every 4KB it will replace the index
    uint32_t* myptr1 = (uint32_t*)(EXT_SLAVE_START_ADDRESS + 0x1000);

    for(int i=0;i<BUFF_LEN;i++){
        myptr1[i] = i*32;
    }

#ifdef TARGET_SIM_SYSTEMC
    //make sure you store everything back to main memory and bypass the flash
    *cache_cfg = CACHE_FLUSH;
    *cache_cfg = CACHE_BYPASS;
#endif

    for(int i=0;i<BUFF_LEN;i++){
        if (ext_memory[i] != i)
            printf("base memory address: Expected %x, got %x\n",i,ext_memory[i]);
        if (myptr1[i] != i*32)
            printf("extended memory address: Expected %x, got %x\n",i*32,myptr1[i]);
        if (buffer_copy[i] != ext_memory[buffer_rnd_index[i]])
            printf("random memory address: Expected %x, got %x\n",ext_memory[buffer_rnd_index[i]],buffer_copy[i]);

    }

    return EXIT_SUCCESS;
}


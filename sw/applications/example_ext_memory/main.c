// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "core_v_mini_mcu.h"
#include "x-heep.h"

#define BUFF_LEN 100

uint32_t  buffer_rnd_index[BUFF_LEN];

#ifdef TARGET_SYSTEMC
//make app PROJECT=example_ext_memory TARGET=systemc
#define CACHE_FLUSH   1
#define CACHE_BYPASS  2
#define CACHE_SIZE    4*1024
#endif

#define MEMORY_SIZE           32*1024
#define MEMORY_ADDR_MASK       0x7FFF
#define MEMORY_MAX_WORD_INDEX  (MEMORY_SIZE/4)

int is_in_array(uint32_t number, uint32_t* array, int N ) {

    for (int i=0;i<N;i++) {
        if (number == array[i])
            return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{


#if defined(TARGET_SIM) || defined(TARGET_SYSTEMC)

    uint32_t* ext_memory = (uint32_t*)EXT_SLAVE_START_ADDRESS;

    #ifdef TARGET_SYSTEMC
    //last address of systemC memory used as a configuration register to flush or bypass
    volatile uint32_t* cache_cfg = (uint32_t*)(EXT_SLAVE_START_ADDRESS + MEMORY_SIZE - 4);
    #endif

    uint32_t random_number = 0;
    uint32_t errors = 0;

    heep_init_lfsr();

    //test 1
    for(int i=0;i<BUFF_LEN;i++) {
        ext_memory[i] = i;
    }

    for(int i=0;i<BUFF_LEN;i++){
        if (ext_memory[i] != i) {
            printf("%d) T1: exp. %x, got %x (@%x)\n",i, i,ext_memory[i],&ext_memory[i]);
            errors++;
        }
    }

    //test 2
    for(int i=0;i<BUFF_LEN;i++) {

        do {
            random_number = (uint32_t)(heep_rand_lfsr()) % MEMORY_MAX_WORD_INDEX;
        } while ( is_in_array(random_number, buffer_rnd_index, i) );

        buffer_rnd_index[i] = random_number;
        ext_memory[buffer_rnd_index[i]] = i*16;
    }

    for(int i=0;i<BUFF_LEN;i++){
        if (ext_memory[buffer_rnd_index[i]] != i*16) {
            printf("%d) T2: exp. %x, got %x (@%x)\n",i, i*16,ext_memory[buffer_rnd_index[i]],&ext_memory[buffer_rnd_index[i]]);
            errors++;
        }
    }


    //test 3

    //as the memory is 32kB, and the cache has 256 blocks
    //replace some cache line
    //writing at EXT_SLAVE_START_ADDRESS + 0x1000 (4kB)
    //the cache is 256*16B = 4KB, so every 4KB it will replace the index
    uint32_t* myptr1 = (uint32_t*)(EXT_SLAVE_START_ADDRESS + 0x1000);

    for(int i=0;i<BUFF_LEN;i++){
        myptr1[i] = i*32;
    }

    for(int i=0;i<BUFF_LEN;i++){
        if (myptr1[i] != i*32) {
            printf("%d) T3: exp. %x, got %x (@%x)\n",i, i*32,myptr1[i],&myptr1[i]);
            errors++;
        }
    }

    //test 4, flush and bypass
    #ifdef TARGET_SYSTEMC
    //make sure you store everything back to main memory and bypass the flash
    *cache_cfg = CACHE_FLUSH;
    *cache_cfg = CACHE_BYPASS;
    #endif

    for(int i=0;i<BUFF_LEN;i++){
        if (myptr1[i] != i*32) {
            printf("%d) T4: exp. %x, got %x (@%x)\n",i, i*32,myptr1[i],&myptr1[i]);
            errors++;
        }
    }




    return errors;

#else
   #pragma message ( "this application must be ran only in the testbench" )
    return EXIT_SUCCESS;
#endif

}


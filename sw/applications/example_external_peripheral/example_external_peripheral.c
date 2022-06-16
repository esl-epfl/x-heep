// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include <stdio.h>
#include <stdlib.h>

#include "memcopy_periph.h"
#include "runtime/core_v_mini_mcu.h"

#define COPY_SIZE 10

int main(int argc, char *argv[])
{
    printf("Memcopy - example external peripheral\n");

    // Use the stack
    int32_t original_data[COPY_SIZE];
    int32_t copied_data[COPY_SIZE];
    // Or use the slow sram ip example for data
    // int32_t original_data = EXT_SLAVE_START_ADDRESS;
    // int32_t copied_data = EXT_SLAVE_START_ADDRESS;

    volatile uint32_t *src_ptr = original_data;
    volatile uint32_t *dest_ptr = copied_data;

    // Put some data to initialize the memory addresses
    for(int i=0; i<COPY_SIZE; i++) {
        *src_ptr++ = i;
    }

    // memcopy peripheral structure to access the registers
    memcopy_periph_t memcopy_periph;
    memcopy_periph.base_addr = mmio_region_from_addr((uintptr_t)EXT_PERIPHERAL_START_ADDRESS);

    memcopy_periph_set_read_ptr(&memcopy_periph, (uint32_t) original_data);
    memcopy_periph_set_write_ptr(&memcopy_periph, (uint32_t) copied_data);
    printf("Memcopy launched...\n");
    memcopy_periph_set_cnt_start(&memcopy_periph, (uint32_t) COPY_SIZE);
    // Poll done register to know when memcopy is finished
    while (memcopy_periph_get_done(&memcopy_periph) == 0);

    printf("Memcopy finished\n");

    // Reinitialized the read pointer to the original address
    src_ptr = original_data;

    // Check for errors
    int32_t errors=0;
    for(int i=0; i<COPY_SIZE; i++) {
        if ((*src_ptr) != (*dest_ptr)) {
            printf("WARNING: %d != %d\n", *src_ptr, *dest_ptr);
            errors++;
        }
        src_ptr++;
	    dest_ptr++;
    }

    if (errors == 0) {
        printf("MEMCOPY SUCCESS\n");
    } else {
        printf("MEMCOPY FAILURE (%d/%d errors)\n", errors, COPY_SIZE);
    }

    return EXIT_SUCCESS;
}

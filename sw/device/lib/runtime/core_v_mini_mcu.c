// Copyright 2024 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include "core_v_mini_mcu.h"
#include <stdint.h>

//heep functions prototypes
uint32_t * heep_get_flash_address_offset(uint32_t* data_address_lma);
void heep_init_lfsr();
uint32_t heep_rand_lfsr();

// this translates the logical address of the FLASH relative to 0 instead of FLASH_MEM_START_ADDRESS, as used by the BSP
uint32_t * heep_get_flash_address_offset(uint32_t* data_address_lma){

#ifdef ON_CHIP
    // no need to translate as FLASH is not memory mapped
    return data_address_lma;
#else
    uint32_t* data_address_adjusted = (uint32_t*) ((uint32_t)(data_address_lma) - FLASH_MEM_START_ADDRESS);
    return data_address_adjusted;
#endif

}

//get random values
uint32_t lfsr;

void heep_init_lfsr() {
    lfsr = (uint32_t)0xAABBCCDD;
}

uint32_t heep_rand_lfsr() {
    uint32_t bit = (lfsr ^ (lfsr >> 10) ^ (lfsr >> 11) ^ (lfsr >> 12)) & 1;
    lfsr = (lfsr >> 1) | (bit << 31);
    return lfsr;
}

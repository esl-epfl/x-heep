// Copyright 2024 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include "core_v_mini_mcu.h"
#include <stdint.h>

// functions
uint32_t * heep_get_data_address_lma(uint32_t* data_address_vma);

// LINKER symbol, the value is defined in the LINKER script
extern const uint32_t _lma_vma_data_offset;

// this translates the virtual address to logical address for the FLASH and sets to 0 the highest 8 bits (as not used by the FLASH)
uint32_t * heep_get_data_address_lma(uint32_t* data_address_vma){

    uint32_t* data_address_lma = (uint32_t*) ((uint32_t)&(_lma_vma_data_offset) + (uint32_t)(data_address_vma));
    //set MS 8 bits to 0 as the flash only uses 24b
    data_address_lma = (uint32_t*) ((uint32_t)(data_address_lma) & 0x00FFFFFF);
    return data_address_lma;
}

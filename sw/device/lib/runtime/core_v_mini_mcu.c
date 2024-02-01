// Copyright 2024 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include "core_v_mini_mcu.h"
#include <stdint.h>

// functions
uint32_t * heep_get_data_address_lma(uint32_t* data_address_vma);
uint32_t * heep_get_flash_address_offset(uint32_t* data_address_lma);

// LINKER symbol, the value is defined in the LINKER script
extern const uint32_t _lma_vma_data_offset;

// this translates the virtual address to load address for the FLASH
uint32_t * heep_get_data_address_lma(uint32_t* data_address_vma){

    uint32_t offset = (uint32_t)&_lma_vma_data_offset; // use the & operator to get the address
    uint32_t* data_address_lma = (uint32_t*) (offset + (uint32_t)(data_address_vma));

    return data_address_lma;
}

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

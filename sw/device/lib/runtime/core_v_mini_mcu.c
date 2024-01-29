// Copyright 2024 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include "core_v_mini_mcu.h"
#include <stdint.h>

// functions
uint32_t * heep_get_data_address_lma(uint32_t* data_address_vma);

// LINKER symbol, the value is defined in the LINKER script
extern const uint32_t _lma_vma_data_offset;

// this translates the virtual address to logical address for the FLASH
uint32_t * heep_get_data_address_lma(uint32_t* data_address_vma){

    uint32_t* data_address_lma = (uint32_t*) ((uint32_t)&(_lma_vma_data_offset) + (uint32_t)(data_address_vma));

    return data_address_lma;
}

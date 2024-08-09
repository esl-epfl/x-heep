// Copyright 2024 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#ifndef COREV_MINI_MCU_MEMORY_H_
#define COREV_MINI_MCU_MEMORY_H_

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include "core_v_mini_mcu.h"

typedef struct memory_address {
    unsigned int start;
    unsigned int end;
} xheep_memory_address_t;

xheep_memory_address_t xheep_memory_regions[MEMORY_BANKS] = {
% for bank in xheep.iter_ram_banks():
    {.start = RAM${bank.name()}_START_ADDRESS, .end = RAM${bank.name()}_END_ADDRESS},
% endfor
};

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // COREV_MINI_MCU_MEMORY_H_

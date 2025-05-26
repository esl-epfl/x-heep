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
    {.start = RAM0_START_ADDRESS, .end = RAM0_END_ADDRESS},
    {.start = RAM1_START_ADDRESS, .end = RAM1_END_ADDRESS},
    {.start = RAM2_START_ADDRESS, .end = RAM2_END_ADDRESS},
    {.start = RAM3_START_ADDRESS, .end = RAM3_END_ADDRESS},
    {.start = RAM4_START_ADDRESS, .end = RAM4_END_ADDRESS},
    {.start = RAM5_START_ADDRESS, .end = RAM5_END_ADDRESS},
};

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // COREV_MINI_MCU_MEMORY_H_

// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include "csr.h"
#include "hart.h"
#include "handler.h"
#include "core_v_mini_mcu.h"
#include "power_manager.h"
#include "x-heep.h"

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

int main(int argc, char *argv[])
{
    // Setup power_manager
    power_manager_init(NULL);

    // Clock-gating the peripheral subsystem
    clock_gate_periph(kOff_e);

    // Clock-gating ram-banks
    // We probably should not clockgate the bank where our RAM resides
    for(uint32_t i = 2; i < MEMORY_BANKS; ++i)
        clock_gate_ram_block(i,kOff_e);

    // Clock-gating external subsystems
    for(uint32_t i = 0; i < EXTERNAL_DOMAINS; ++i)
        clock_gate_external(i,kOff_e);

    // Wait some time
    for (int i=0; i<100; i++) asm volatile("nop;");

    // Enabling the peripheral subsystem
    clock_gate_periph(kOn_e);

    // Enabling ram-banks
    for(uint32_t i = 2; i < MEMORY_BANKS; ++i)
        clock_gate_ram_block(i,kOn_e);

    // Enabling external subsystems
    for(uint32_t i = 0; i < EXTERNAL_DOMAINS; ++i)
        clock_gate_external(i,kOn_e);

    /* write something to stdout */
    PRINTF("Success.\n\r");
    return EXIT_SUCCESS;
}

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

static power_manager_t power_manager;

int main(int argc, char *argv[])
{
    // Setup power_manager
    mmio_region_t power_manager_reg = mmio_region_from_addr(POWER_MANAGER_START_ADDRESS);
    power_manager.base_addr = power_manager_reg;

    // Clock-gating the peripheral subsystem
    mmio_region_write32(power_manager.base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_CLK_GATE_REG_OFFSET), 0x1);

    // Clock-gating ram-banks
    // We probably should not clockgate the bank where our RAM resides
    for(uint32_t i = 2; i < MEMORY_BANKS; ++i)
        mmio_region_write32(power_manager.base_addr, (ptrdiff_t)(power_manager_ram_map[i].clk_gate), 0x1);

    // Wait some time
    for (int i=0; i<100; i++) asm volatile("nop;");

    // Enabling the peripheral subsystem
    mmio_region_write32(power_manager.base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_CLK_GATE_REG_OFFSET), 0x0);

    // Enabling ram-banks
    for(uint32_t i = 2; i < MEMORY_BANKS; ++i)
        mmio_region_write32(power_manager.base_addr, (ptrdiff_t)(power_manager_ram_map[i].clk_gate), 0x0);

    /* write something to stdout */
    printf("Success.\n");
    return EXIT_SUCCESS;
}

// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include "power_manager.h"

#include <stddef.h>
#include <stdint.h>

#include "mmio.h"

#include "core_v_mini_mcu.h"

#include "power_manager_regs.h"  // Generated.

void power_gate_core_asm(void)
{

    asm volatile (

        // write POWER_GATE_CORE[0] = 1
        "lui a0, %[base_address_20bit]\n"
        "li  a1, 1\n"
        "sw  a1, %[power_manager_power_gate_core_reg_offset](a0)\n"

        // write WAKEUP_STATE[0] = 1
        "sw  a1, %[power_manager_wakeup_state_reg_offset](a0)\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_power_gate_core_reg_offset] "i" (POWER_MANAGER_POWER_GATE_CORE_REG_OFFSET), \
        [power_manager_wakeup_state_reg_offset] "i" (POWER_MANAGER_WAKEUP_STATE_REG_OFFSET) \
    );

    asm volatile (

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        "sw x1,  %[power_manager_core_reg_x1_reg_offset](a0)\n"
        "sw x2,  %[power_manager_core_reg_x2_reg_offset](a0)\n"
        "sw x3,  %[power_manager_core_reg_x3_reg_offset](a0)\n"
        "sw x4,  %[power_manager_core_reg_x4_reg_offset](a0)\n"
        "sw x5,  %[power_manager_core_reg_x5_reg_offset](a0)\n"
        "sw x6,  %[power_manager_core_reg_x6_reg_offset](a0)\n"
        "sw x7,  %[power_manager_core_reg_x7_reg_offset](a0)\n"
        "sw x8,  %[power_manager_core_reg_x8_reg_offset](a0)\n"
        "sw x9,  %[power_manager_core_reg_x9_reg_offset](a0)\n"
        "sw x10, %[power_manager_core_reg_x10_reg_offset](a0)\n"
        "sw x11, %[power_manager_core_reg_x11_reg_offset](a0)\n"
        "sw x12, %[power_manager_core_reg_x12_reg_offset](a0)\n"
        "sw x13, %[power_manager_core_reg_x13_reg_offset](a0)\n"
        "sw x14, %[power_manager_core_reg_x14_reg_offset](a0)\n"
        "sw x15, %[power_manager_core_reg_x15_reg_offset](a0)\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_reg_x1_reg_offset] "i" (POWER_MANAGER_CORE_REG_X1_REG_OFFSET), \
        [power_manager_core_reg_x2_reg_offset] "i" (POWER_MANAGER_CORE_REG_X2_REG_OFFSET), \
        [power_manager_core_reg_x3_reg_offset] "i" (POWER_MANAGER_CORE_REG_X3_REG_OFFSET), \
        [power_manager_core_reg_x4_reg_offset] "i" (POWER_MANAGER_CORE_REG_X4_REG_OFFSET), \
        [power_manager_core_reg_x5_reg_offset] "i" (POWER_MANAGER_CORE_REG_X5_REG_OFFSET), \
        [power_manager_core_reg_x6_reg_offset] "i" (POWER_MANAGER_CORE_REG_X6_REG_OFFSET), \
        [power_manager_core_reg_x7_reg_offset] "i" (POWER_MANAGER_CORE_REG_X7_REG_OFFSET), \
        [power_manager_core_reg_x8_reg_offset] "i" (POWER_MANAGER_CORE_REG_X8_REG_OFFSET), \
        [power_manager_core_reg_x9_reg_offset] "i" (POWER_MANAGER_CORE_REG_X9_REG_OFFSET), \
        [power_manager_core_reg_x10_reg_offset] "i" (POWER_MANAGER_CORE_REG_X10_REG_OFFSET), \
        [power_manager_core_reg_x11_reg_offset] "i" (POWER_MANAGER_CORE_REG_X11_REG_OFFSET), \
        [power_manager_core_reg_x12_reg_offset] "i" (POWER_MANAGER_CORE_REG_X12_REG_OFFSET), \
        [power_manager_core_reg_x13_reg_offset] "i" (POWER_MANAGER_CORE_REG_X13_REG_OFFSET), \
        [power_manager_core_reg_x14_reg_offset] "i" (POWER_MANAGER_CORE_REG_X14_REG_OFFSET), \
        [power_manager_core_reg_x15_reg_offset] "i" (POWER_MANAGER_CORE_REG_X15_REG_OFFSET) \
    );

    asm volatile (

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        "sw x16, %[power_manager_core_reg_x16_reg_offset](a0)\n"
        "sw x17, %[power_manager_core_reg_x17_reg_offset](a0)\n"
        "sw x18, %[power_manager_core_reg_x18_reg_offset](a0)\n"
        "sw x19, %[power_manager_core_reg_x19_reg_offset](a0)\n"
        "sw x20, %[power_manager_core_reg_x20_reg_offset](a0)\n"
        "sw x21, %[power_manager_core_reg_x21_reg_offset](a0)\n"
        "sw x22, %[power_manager_core_reg_x22_reg_offset](a0)\n"
        "sw x23, %[power_manager_core_reg_x23_reg_offset](a0)\n"
        "sw x24, %[power_manager_core_reg_x24_reg_offset](a0)\n"
        "sw x25, %[power_manager_core_reg_x25_reg_offset](a0)\n"
        "sw x26, %[power_manager_core_reg_x26_reg_offset](a0)\n"
        "sw x27, %[power_manager_core_reg_x27_reg_offset](a0)\n"
        "sw x28, %[power_manager_core_reg_x28_reg_offset](a0)\n"
        "sw x29, %[power_manager_core_reg_x29_reg_offset](a0)\n"
        "sw x30, %[power_manager_core_reg_x30_reg_offset](a0)\n"
        "sw x31, %[power_manager_core_reg_x31_reg_offset](a0)\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_reg_x16_reg_offset] "i" (POWER_MANAGER_CORE_REG_X16_REG_OFFSET), \
        [power_manager_core_reg_x17_reg_offset] "i" (POWER_MANAGER_CORE_REG_X17_REG_OFFSET), \
        [power_manager_core_reg_x18_reg_offset] "i" (POWER_MANAGER_CORE_REG_X18_REG_OFFSET), \
        [power_manager_core_reg_x19_reg_offset] "i" (POWER_MANAGER_CORE_REG_X19_REG_OFFSET), \
        [power_manager_core_reg_x20_reg_offset] "i" (POWER_MANAGER_CORE_REG_X20_REG_OFFSET), \
        [power_manager_core_reg_x21_reg_offset] "i" (POWER_MANAGER_CORE_REG_X21_REG_OFFSET), \
        [power_manager_core_reg_x22_reg_offset] "i" (POWER_MANAGER_CORE_REG_X22_REG_OFFSET), \
        [power_manager_core_reg_x23_reg_offset] "i" (POWER_MANAGER_CORE_REG_X23_REG_OFFSET), \
        [power_manager_core_reg_x24_reg_offset] "i" (POWER_MANAGER_CORE_REG_X24_REG_OFFSET), \
        [power_manager_core_reg_x25_reg_offset] "i" (POWER_MANAGER_CORE_REG_X25_REG_OFFSET), \
        [power_manager_core_reg_x26_reg_offset] "i" (POWER_MANAGER_CORE_REG_X26_REG_OFFSET), \
        [power_manager_core_reg_x27_reg_offset] "i" (POWER_MANAGER_CORE_REG_X27_REG_OFFSET), \
        [power_manager_core_reg_x28_reg_offset] "i" (POWER_MANAGER_CORE_REG_X28_REG_OFFSET), \
        [power_manager_core_reg_x29_reg_offset] "i" (POWER_MANAGER_CORE_REG_X29_REG_OFFSET), \
        [power_manager_core_reg_x30_reg_offset] "i" (POWER_MANAGER_CORE_REG_X30_REG_OFFSET), \
        [power_manager_core_reg_x31_reg_offset] "i" (POWER_MANAGER_CORE_REG_X31_REG_OFFSET) \
    );

    asm volatile (

        // write RESTORE_ADDRESS[31:0] = PC
        "lui a0, %[base_address_20bit]\n"
        "la a1, wakeup\n"
        "sw a1, %[power_manager_restore_address_reg_offset](a0)\n"

        // wait for interrupt
        "wfi\n"

        // ----------------------------
        // power-gate
        // ----------------------------

        // ----------------------------
        // wake-up
        // ----------------------------

        // write POWER_GATE_CORE[0] = 0
        "wakeup:"
        // "lui a5,0x20030\n"
        "li  a1, 0\n"
        "sw  a1, %[power_manager_power_gate_core_reg_offset](a0)\n"

        // write WAKEUP_STATE[0] = 0
        "sw  a1, %[power_manager_wakeup_state_reg_offset](a0)\n"

        // write RESTORE_ADDRESS[31:0] = 0
        "sw x0, %[power_manager_restore_address_reg_offset](a0)\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_power_gate_core_reg_offset] "i" (POWER_MANAGER_POWER_GATE_CORE_REG_OFFSET), \
        [power_manager_wakeup_state_reg_offset] "i" (POWER_MANAGER_WAKEUP_STATE_REG_OFFSET), \
        [power_manager_restore_address_reg_offset] "i" (POWER_MANAGER_RESTORE_ADDRESS_REG_OFFSET) \
    );

    asm volatile (

        // read CORE_REG_Xn
        "lui a0, %[base_address_20bit]\n"
        "lw x1,  %[power_manager_core_reg_x1_reg_offset](a0)\n"
        "lw x2,  %[power_manager_core_reg_x2_reg_offset](a0)\n"
        "lw x3,  %[power_manager_core_reg_x3_reg_offset](a0)\n"
        "lw x4,  %[power_manager_core_reg_x4_reg_offset](a0)\n"
        "lw x5,  %[power_manager_core_reg_x5_reg_offset](a0)\n"
        "lw x6,  %[power_manager_core_reg_x6_reg_offset](a0)\n"
        "lw x7,  %[power_manager_core_reg_x7_reg_offset](a0)\n"
        "lw x8,  %[power_manager_core_reg_x8_reg_offset](a0)\n"
        "lw x9,  %[power_manager_core_reg_x9_reg_offset](a0)\n"
        "lw x10, %[power_manager_core_reg_x10_reg_offset](a0)\n"
        "lw x11, %[power_manager_core_reg_x11_reg_offset](a0)\n"
        "lw x12, %[power_manager_core_reg_x12_reg_offset](a0)\n"
        "lw x13, %[power_manager_core_reg_x13_reg_offset](a0)\n"
        "lw x14, %[power_manager_core_reg_x14_reg_offset](a0)\n"
        "lw x15, %[power_manager_core_reg_x15_reg_offset](a0)\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_reg_x1_reg_offset] "i" (POWER_MANAGER_CORE_REG_X1_REG_OFFSET), \
        [power_manager_core_reg_x2_reg_offset] "i" (POWER_MANAGER_CORE_REG_X2_REG_OFFSET), \
        [power_manager_core_reg_x3_reg_offset] "i" (POWER_MANAGER_CORE_REG_X3_REG_OFFSET), \
        [power_manager_core_reg_x4_reg_offset] "i" (POWER_MANAGER_CORE_REG_X4_REG_OFFSET), \
        [power_manager_core_reg_x5_reg_offset] "i" (POWER_MANAGER_CORE_REG_X5_REG_OFFSET), \
        [power_manager_core_reg_x6_reg_offset] "i" (POWER_MANAGER_CORE_REG_X6_REG_OFFSET), \
        [power_manager_core_reg_x7_reg_offset] "i" (POWER_MANAGER_CORE_REG_X7_REG_OFFSET), \
        [power_manager_core_reg_x8_reg_offset] "i" (POWER_MANAGER_CORE_REG_X8_REG_OFFSET), \
        [power_manager_core_reg_x9_reg_offset] "i" (POWER_MANAGER_CORE_REG_X9_REG_OFFSET), \
        [power_manager_core_reg_x10_reg_offset] "i" (POWER_MANAGER_CORE_REG_X10_REG_OFFSET), \
        [power_manager_core_reg_x11_reg_offset] "i" (POWER_MANAGER_CORE_REG_X11_REG_OFFSET), \
        [power_manager_core_reg_x12_reg_offset] "i" (POWER_MANAGER_CORE_REG_X12_REG_OFFSET), \
        [power_manager_core_reg_x13_reg_offset] "i" (POWER_MANAGER_CORE_REG_X13_REG_OFFSET), \
        [power_manager_core_reg_x14_reg_offset] "i" (POWER_MANAGER_CORE_REG_X14_REG_OFFSET), \
        [power_manager_core_reg_x15_reg_offset] "i" (POWER_MANAGER_CORE_REG_X15_REG_OFFSET) \
    );

    asm volatile (

        // read CORE_REG_Xn
        "lui a0, %[base_address_20bit]\n"
        "lw x16, %[power_manager_core_reg_x16_reg_offset](a0)\n"
        "lw x17, %[power_manager_core_reg_x17_reg_offset](a0)\n"
        "lw x18, %[power_manager_core_reg_x18_reg_offset](a0)\n"
        "lw x19, %[power_manager_core_reg_x19_reg_offset](a0)\n"
        "lw x20, %[power_manager_core_reg_x20_reg_offset](a0)\n"
        "lw x21, %[power_manager_core_reg_x21_reg_offset](a0)\n"
        "lw x22, %[power_manager_core_reg_x22_reg_offset](a0)\n"
        "lw x23, %[power_manager_core_reg_x23_reg_offset](a0)\n"
        "lw x24, %[power_manager_core_reg_x24_reg_offset](a0)\n"
        "lw x25, %[power_manager_core_reg_x25_reg_offset](a0)\n"
        "lw x26, %[power_manager_core_reg_x26_reg_offset](a0)\n"
        "lw x27, %[power_manager_core_reg_x27_reg_offset](a0)\n"
        "lw x28, %[power_manager_core_reg_x28_reg_offset](a0)\n"
        "lw x29, %[power_manager_core_reg_x29_reg_offset](a0)\n"
        "lw x30, %[power_manager_core_reg_x30_reg_offset](a0)\n"
        "lw x31, %[power_manager_core_reg_x31_reg_offset](a0)\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_reg_x16_reg_offset] "i" (POWER_MANAGER_CORE_REG_X16_REG_OFFSET), \
        [power_manager_core_reg_x17_reg_offset] "i" (POWER_MANAGER_CORE_REG_X17_REG_OFFSET), \
        [power_manager_core_reg_x18_reg_offset] "i" (POWER_MANAGER_CORE_REG_X18_REG_OFFSET), \
        [power_manager_core_reg_x19_reg_offset] "i" (POWER_MANAGER_CORE_REG_X19_REG_OFFSET), \
        [power_manager_core_reg_x20_reg_offset] "i" (POWER_MANAGER_CORE_REG_X20_REG_OFFSET), \
        [power_manager_core_reg_x21_reg_offset] "i" (POWER_MANAGER_CORE_REG_X21_REG_OFFSET), \
        [power_manager_core_reg_x22_reg_offset] "i" (POWER_MANAGER_CORE_REG_X22_REG_OFFSET), \
        [power_manager_core_reg_x23_reg_offset] "i" (POWER_MANAGER_CORE_REG_X23_REG_OFFSET), \
        [power_manager_core_reg_x24_reg_offset] "i" (POWER_MANAGER_CORE_REG_X24_REG_OFFSET), \
        [power_manager_core_reg_x25_reg_offset] "i" (POWER_MANAGER_CORE_REG_X25_REG_OFFSET), \
        [power_manager_core_reg_x26_reg_offset] "i" (POWER_MANAGER_CORE_REG_X26_REG_OFFSET), \
        [power_manager_core_reg_x27_reg_offset] "i" (POWER_MANAGER_CORE_REG_X27_REG_OFFSET), \
        [power_manager_core_reg_x28_reg_offset] "i" (POWER_MANAGER_CORE_REG_X28_REG_OFFSET), \
        [power_manager_core_reg_x29_reg_offset] "i" (POWER_MANAGER_CORE_REG_X29_REG_OFFSET), \
        [power_manager_core_reg_x30_reg_offset] "i" (POWER_MANAGER_CORE_REG_X30_REG_OFFSET), \
        [power_manager_core_reg_x31_reg_offset] "i" (POWER_MANAGER_CORE_REG_X31_REG_OFFSET) \
    );

    return;
}

power_manager_result_t power_gate_core(const power_manager_t *power_manager, power_manager_sel_intr_t sel_intr)
{
    if (sel_intr == kTimer)
    {
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_EN_WAIT_FOR_INTR_REG_OFFSET), 0x1);
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_INTR_STATE_REG_OFFSET), 0x0);

        power_gate_core_asm();

        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_EN_WAIT_FOR_INTR_REG_OFFSET), 0x0);
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_INTR_STATE_REG_OFFSET), 0x0);
    }
    else
    {
        printf("Error: interrupt source not available.\n");
        return kPowerManagerError;
    }

    return kPowerManagerOk;
}

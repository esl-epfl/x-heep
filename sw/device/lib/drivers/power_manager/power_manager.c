// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include "power_manager.h"

#include <stddef.h>
#include <stdint.h>

#include "mmio.h"

#include "power_manager_regs.h"  // Generated.

void power_gate_core(void)
{
    asm volatile (

        // write POWER_GATE_CORE[0] = 1
        "lui a0,0x20030\n"
        "addi a1, a0, 0\n"
        "li  a3,1\n"
        "sw  a3,0(a1)\n"

        // write WAKEUP_STATE[0] = 1
        "lui a0,0x20030\n"
        "addi a1, a0, 4\n"
        "li  a3,1\n"
        "sw  a3,0(a1)\n"

        // write RESTORE_ADDRESS[31:0] = PC
        "lui a0,0x20030\n"
        "addi a1, a0, 8\n"
        // write pc

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0,0x20030\n"
        "addi a1, a0, 0x10\n"
        "sw x1,  0(a1)\n"
        "sw x2,  4(a1)\n"
        "sw x3,  8(a1)\n"
        "sw x4,  12(a1)\n"
        "sw x5,  16(a1)\n"
        "sw x6,  20(a1)\n"
        "sw x7,  24(a1)\n"
        "sw x8,  28(a1)\n"
        "sw x9,  32(a1)\n"
        "sw x10, 36(a1)\n"
        "sw x11, 40(a1)\n"
        "sw x12, 44(a1)\n"
        "sw x13, 48(a1)\n"
        "sw x14, 52(a1)\n"
        "sw x15, 56(a1)\n"
        "sw x16, 60(a1)\n"
        "sw x17, 64(a1)\n"
        "sw x18, 68(a1)\n"
        "sw x19, 72(a1)\n"
        "sw x20, 76(a1)\n"
        "sw x21, 80(a1)\n"
        "sw x22, 84(a1)\n"
        "sw x23, 88(a1)\n"
        "sw x24, 92(a1)\n"
        "sw x25, 96(a1)\n"
        "sw x26, 100(a1)\n"
        "sw x27, 104(a1)\n"
        "sw x28, 108(a1)\n"
        "sw x29, 112(a1)\n"
        "sw x30, 116(a1)\n"
        "sw x31, 120(a1)\n"

        // wait for interrupt
        "wfi\n"

        // ----------------------------
        // power gated
        // ----------------------------

        // ----------------------------
        // wake-up
        // ----------------------------

        // write POWER_GATE_CORE[0] = 0
        "lui a0,0x20030\n"
        "addi a1, a0, 0\n"
        "li  a3,0\n"
        "sw  a3,0(a1)\n"

        // write WAKEUP_STATE[0] = 0
        "nop\n"
        "nop\n"
        "lui a0,0x20030\n"
        "addi a1, a0, 4\n"
        "li  a3,0\n"
        "sw  a3,0(a1)\n"

        // write RESTORE_ADDRESS[31:0] = 0
        "lui a0,0x20030\n"
        "addi a1, a0, 8\n"
        "sw  x0,0(a1)\n"

        // read CORE_REG_Xn = 1
        "lui a0,0x20030\n"
        "addi a1, a0, 0x10\n"
        "lw x1,  0(a1)\n"
        "lw x2,  4(a1)\n"
        "lw x3,  8(a1)\n"
        "lw x4,  12(a1)\n"
        "lw x5,  16(a1)\n"
        "lw x6,  20(a1)\n"
        "lw x7,  24(a1)\n"
        "lw x8,  28(a1)\n"
        "lw x9,  32(a1)\n"
        "lw x10, 36(a1)\n"
        "lw x11, 40(a1)\n"
        "lw x12, 44(a1)\n"
        "lw x13, 48(a1)\n"
        "lw x14, 52(a1)\n"
        "lw x15, 56(a1)\n"
        "lw x16, 60(a1)\n"
        "lw x17, 64(a1)\n"
        "lw x18, 68(a1)\n"
        "lw x19, 72(a1)\n"
        "lw x20, 76(a1)\n"
        "lw x21, 80(a1)\n"
        "lw x22, 84(a1)\n"
        "lw x23, 88(a1)\n"
        "lw x24, 92(a1)\n"
        "lw x25, 96(a1)\n"
        "lw x26, 100(a1)\n"
        "lw x27, 104(a1)\n"
        "lw x28, 108(a1)\n"
        "lw x29, 112(a1)\n"
        "lw x30, 116(a1)\n"
        "lw x31, 120(a1)\n"
    );

    return;
}

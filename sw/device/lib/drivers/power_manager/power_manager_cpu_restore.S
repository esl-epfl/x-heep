// Copyright 2024 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include "core_v_mini_mcu.h"
#include "power_manager_regs.h"  // Generated.

# power_manager_cpu.S
# This function stores the CPU context, goes to (deep) sleep with WFI, and once awakon restore the context
.global power_manager_cpu_restore
.type power_manager_cpu_restore, @function

power_manager_cpu_restore:

    la a0, POWER_MANAGER_START_ADDRESS
    sw  x0, POWER_MANAGER_POWER_GATE_CORE_REG_OFFSET(a0)

    // write WAKEUP_STATE[0] = 0
    sw x0, POWER_MANAGER_WAKEUP_STATE_REG_OFFSET(a0)

    // write RESTORE_ADDRESS[31:0] = 0
    sw x0, POWER_MANAGER_RESTORE_ADDRESS_REG_OFFSET(a0)

    // write CORE_REG_Xn[31:0] = Xn
    la a0, __power_manager_start
    //one of the following load is gonna overwrite a0, but a0 was already stored before to the right value
    lw x1,  0(a0)
    lw x2,  4(a0)
    lw x3,  8(a0)
    lw x4,  12(a0)
    lw x5,  16(a0)
    lw x6,  20(a0)
    lw x7,  24(a0)
    lw x8,  28(a0)
    lw x9,  32(a0)
    lw x10, 36(a0)
    lw x11, 40(a0)
    lw x12, 44(a0)
    lw x13, 48(a0)
    lw x14, 52(a0)
    lw x15, 56(a0)
    lw x16, 60(a0)
    lw x17, 64(a0)
    lw x18, 68(a0)
    lw x19, 72(a0)
    lw x20, 76(a0)
    lw x21, 80(a0)
    lw x22, 88(a0)
    lw x23, 92(a0)
    lw x24, 96(a0)
    lw x25, 100(a0)
    lw x26, 104(a0)
    lw x27, 108(a0)
    lw x28, 112(a0)
    lw x29, 116(a0)
    lw x30, 120(a0)
    lw x31, 124(a0)
    //csr
    lw a1, 128(a0)
    csrw mstatus, a1
    lw a1, 132(a0)
    csrw mie, a1
    lw a1, 136(a0)
    csrw mtvec, a1
    lw a1, 140(a0)
    csrw mscratch, a1
    lw a1, 144(a0)
    csrw mepc, a1
    lw a1, 148(a0)
    csrw mcause, a1
    lw a1, 152(a0)
    csrw mtval, a1
    lw a1, 156(a0)
    csrw mcycle, a1
    lw a1, 160(a0)
    csrw minstret, a1

    ret
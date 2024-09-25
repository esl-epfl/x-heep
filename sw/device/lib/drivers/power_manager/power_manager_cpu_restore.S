// Copyright 2024 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include "core_v_mini_mcu.h"
#include "power_manager_regs.h"  // Generated.

# power_manager_cpu.S
# This function re-stores the CPU context when back from (deep) sleep
.global power_manager_cpu_restore
.type power_manager_cpu_restore, @function

#define POWER_MANAGER_START_ADDRESS_20bit (POWER_MANAGER_START_ADDRESS >> 12)


power_manager_cpu_restore:

    //using lui to load the upper 20 bits of the address instead of la as I want to be sure no other registers are used
    lui t0, POWER_MANAGER_START_ADDRESS_20bit
    sw  x0, POWER_MANAGER_POWER_GATE_CORE_REG_OFFSET(t0)

    // write WAKEUP_STATE[0] = 0
    sw x0, POWER_MANAGER_WAKEUP_STATE_REG_OFFSET(t0)

    // write RESTORE_ADDRESS[31:0] = 0
    sw x0, POWER_MANAGER_RESTORE_ADDRESS_REG_OFFSET(t0)

    // restore gp as it is gonna be used to calculate the address of __power_manager_start
    lw  gp, POWER_MANAGER_GLOBAL_POINTER_REG_OFFSET(t0)

    // write CORE_REG_Xn[31:0] = Xn
    la t0, __power_manager_start
    // restore context, this part could be optimized

    //one of the following load is gonna overwrite t0, but t0 was already stored before to the right value
    lw x1,  0(t0)
    lw x2,  4(t0)
    lw x3,  8(t0)
    lw x4,  12(t0)
    lw x5,  16(t0)
    lw x6,  20(t0)
    lw x7,  24(t0)
    lw x8,  28(t0)
    lw x9,  32(t0)
    lw x10, 36(t0)
    lw x11, 40(t0)
    lw x12, 44(t0)
    lw x13, 48(t0)
    lw x14, 52(t0)
    lw x15, 56(t0)
    lw x16, 60(t0)
    lw x17, 64(t0)
    lw x18, 68(t0)
    lw x19, 72(t0)
    lw x20, 76(t0)
    lw x21, 80(t0)
    lw x22, 88(t0)
    lw x23, 92(t0)
    lw x24, 96(t0)
    lw x25, 100(t0)
    lw x26, 104(t0)
    lw x27, 108(t0)
    lw x28, 112(t0)
    lw x29, 116(t0)
    lw x30, 120(t0)
    lw x31, 124(t0)
    //csr
    lw t1, 128(t0)
    csrw mstatus, t1
    lw t1, 132(t0)
    csrw mie, t1
    lw t1, 136(t0)
    csrw mtvec, t1
    lw t1, 140(t0)
    csrw mscratch, t1
    lw t1, 144(t0)
    csrw mepc, t1
    lw t1, 148(t0)
    csrw mcause, t1
    lw t1, 152(t0)
    csrw mtval, t1
    lw t1, 156(t0)
    csrw mcycle, t1
    lw t1, 160(t0)
    csrw minstret, t1


    ret
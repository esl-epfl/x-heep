// Copyright 2024 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include "core_v_mini_mcu.h"
#include "power_manager_regs.h"  // Generated.

# power_manager_cpu.S
# This function stores the CPU context, goes to (deep) sleep with WFI
.global power_manager_cpu_store # make the function visible to the linker
.type power_manager_cpu_store, @function

power_manager_cpu_store:

    // write POWER_GATE_CORE[0] = 1
    la t0, POWER_MANAGER_START_ADDRESS
    li  t1, 1
    sw  t1, POWER_MANAGER_POWER_GATE_CORE_REG_OFFSET(t0)

    // write WAKEUP_STATE[0] = 1, this is check in the bootrom at reset time when waking up
    sw t1, POWER_MANAGER_WAKEUP_STATE_REG_OFFSET(t0)

    // save the global pointer, _power_manager_start is the start of the power_manager section and saved in the gp reg
    // when returning from deep sleep, the gp register is used to restore the context, thus saving it in an always on register
    sw  gp, POWER_MANAGER_GLOBAL_POINTER_REG_OFFSET(t0)

    // save context, this part could be optimized
    la t0, __power_manager_start
    sw x1,  0(t0)
    sw x2,  4(t0)
    sw x3,  8(t0)
    sw x4,  12(t0)
    sw x5,  16(t0)
    sw x6,  20(t0)
    sw x7,  24(t0)
    sw x8,  28(t0)
    sw x9,  32(t0)
    sw x10, 36(t0)
    sw x11, 40(t0)
    sw x12, 44(t0)
    sw x13, 48(t0)
    sw x14, 52(t0)
    sw x15, 56(t0)
    sw x16, 60(t0)
    sw x17, 64(t0)
    sw x18, 68(t0)
    sw x19, 72(t0)
    sw x20, 76(t0)
    sw x21, 80(t0)
    sw x22, 88(t0)
    sw x23, 92(t0)
    sw x24, 96(t0)
    sw x25, 100(t0)
    sw x26, 104(t0)
    sw x27, 108(t0)
    sw x28, 112(t0)
    sw x29, 116(t0)
    sw x30, 120(t0)
    sw x31, 124(t0)
    //csr
    csrr t1, mstatus
    sw t1, 128(t0)
    csrr t1, mie
    sw t1, 132(t0)
    csrr t1, mtvec
    sw t1, 136(t0)
    csrr t1, mscratch
    sw t1, 140(t0)
    csrr t1, mepc
    sw t1, 144(t0)
    csrr t1, mcause
    sw t1, 148(t0)
    csrr t1, mtval
    sw t1, 152(t0)
    csrr t1, mcycle
    sw t1, 156(t0)
    csrr t1, minstret
    sw t1, 160(t0)

    la t0, POWER_MANAGER_START_ADDRESS
    //save return address to restore
    la t1, power_manager_cpu_restore
    sw t1, POWER_MANAGER_RESTORE_ADDRESS_REG_OFFSET(t0)

    // wait for interrupt
    wfi

    ret
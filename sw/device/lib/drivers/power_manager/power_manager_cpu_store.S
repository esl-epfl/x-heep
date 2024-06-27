// Copyright 2024 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include "core_v_mini_mcu.h"
#include "power_manager_regs.h"  // Generated.

# power_manager_cpu.S
# This function stores the CPU context, goes to (deep) sleep with WFI, and once awakon restore the context
.global power_manager_cpu_store # Rende la funzione visibile al linker
.type power_manager_cpu_store, @function

power_manager_cpu_store:

    // write POWER_GATE_CORE[0] = 1
    la a0, POWER_MANAGER_START_ADDRESS
    li  a1, 1
    sw  a1, POWER_MANAGER_POWER_GATE_CORE_REG_OFFSET(a0)

    // write registers
    la a0, __power_manager_start
    sw x1,  0(a0)
    sw x2,  4(a0)
    sw x3,  8(a0)
    sw x4,  12(a0)
    sw x5,  16(a0)
    sw x6,  20(a0)
    sw x7,  24(a0)
    sw x8,  28(a0)
    sw x9,  32(a0)
    sw x10, 36(a0)
    sw x11, 40(a0)
    sw x12, 44(a0)
    sw x13, 48(a0)
    sw x14, 52(a0)
    sw x15, 56(a0)
    sw x16, 60(a0)
    sw x17, 64(a0)
    sw x18, 68(a0)
    sw x19, 72(a0)
    sw x20, 76(a0)
    sw x21, 80(a0)
    sw x22, 88(a0)
    sw x23, 92(a0)
    sw x24, 96(a0)
    sw x25, 100(a0)
    sw x26, 104(a0)
    sw x27, 108(a0)
    sw x28, 112(a0)
    sw x29, 116(a0)
    sw x30, 120(a0)
    sw x31, 124(a0)
    //csr
    csrr a1, mstatus
    sw a1, 128(a0)
    csrr a1, mie
    sw a1, 132(a0)
    csrr a1, mtvec
    sw a1, 136(a0)
    csrr a1, mscratch
    sw a1, 140(a0)
    csrr a1, mepc
    sw a1, 144(a0)
    csrr a1, mcause
    sw a1, 148(a0)
    csrr a1, mtval
    sw a1, 152(a0)
    csrr a1, mcycle
    sw a1, 156(a0)
    csrr a1, minstret
    sw a1, 160(a0)

    la a0, POWER_MANAGER_START_ADDRESS
    la a1, power_manager_cpu_restore
    sw a1, POWER_MANAGER_RESTORE_ADDRESS_REG_OFFSET(a0)

    // wait for interrupt
    wfi

    ret
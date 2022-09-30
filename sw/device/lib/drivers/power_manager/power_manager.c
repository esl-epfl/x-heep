// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include "power_manager.h"

#include <stddef.h>
#include <stdint.h>

#include "mmio.h"

#include "core_v_mini_mcu.h"

#include "power_manager_regs.h"  // Generated.

void power_gate_core_asm()
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

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        "csrr a1, mstatus\n"
        "sw a1, %[power_manager_core_csr_c0_reg_offset](a0)\n"
        "csrr a1, misa\n"
        "sw a1, %[power_manager_core_csr_c1_reg_offset](a0)\n"
        "csrr a1, mie\n"
        "sw a1, %[power_manager_core_csr_c2_reg_offset](a0)\n"
        "csrr a1, mtvec\n"
        "sw a1, %[power_manager_core_csr_c3_reg_offset](a0)\n"
        "csrr a1, mcountinhibit\n"
        "sw a1, %[power_manager_core_csr_c4_reg_offset](a0)\n"
        "csrr a1, mhpmevent3\n"
        "sw a1, %[power_manager_core_csr_c5_reg_offset](a0)\n"
        "csrr a1, mhpmevent4\n"
        "sw a1, %[power_manager_core_csr_c6_reg_offset](a0)\n"
        "csrr a1, mhpmevent5\n"
        "sw a1, %[power_manager_core_csr_c7_reg_offset](a0)\n"
        "csrr a1, mhpmevent6\n"
        "sw a1, %[power_manager_core_csr_c8_reg_offset](a0)\n"
        "csrr a1, mhpmevent7\n"
        "sw a1, %[power_manager_core_csr_c9_reg_offset](a0)\n"
        "csrr a1, mhpmevent8\n"
        "sw a1, %[power_manager_core_csr_c10_reg_offset](a0)\n"
        "csrr a1, mhpmevent9\n"
        "sw a1, %[power_manager_core_csr_c11_reg_offset](a0)\n"
        "csrr a1, mhpmevent10\n"
        "sw a1, %[power_manager_core_csr_c12_reg_offset](a0)\n"
        "csrr a1, mhpmevent11\n"
        "sw a1, %[power_manager_core_csr_c13_reg_offset](a0)\n"
        "csrr a1, mhpmevent12\n"
        "sw a1, %[power_manager_core_csr_c14_reg_offset](a0)\n"
        "csrr a1, mhpmevent13\n"
        "sw a1, %[power_manager_core_csr_c15_reg_offset](a0)\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_csr_c0_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C0_REG_OFFSET), \
        [power_manager_core_csr_c1_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C1_REG_OFFSET), \
        [power_manager_core_csr_c2_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C2_REG_OFFSET), \
        [power_manager_core_csr_c3_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C3_REG_OFFSET), \
        [power_manager_core_csr_c4_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C4_REG_OFFSET), \
        [power_manager_core_csr_c5_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C5_REG_OFFSET), \
        [power_manager_core_csr_c6_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C6_REG_OFFSET), \
        [power_manager_core_csr_c7_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C7_REG_OFFSET), \
        [power_manager_core_csr_c8_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C8_REG_OFFSET), \
        [power_manager_core_csr_c9_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C9_REG_OFFSET), \
        [power_manager_core_csr_c10_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C10_REG_OFFSET), \
        [power_manager_core_csr_c11_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C11_REG_OFFSET), \
        [power_manager_core_csr_c12_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C12_REG_OFFSET), \
        [power_manager_core_csr_c13_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C13_REG_OFFSET), \
        [power_manager_core_csr_c14_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C14_REG_OFFSET), \
        [power_manager_core_csr_c15_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C15_REG_OFFSET) \
    );

    asm volatile (

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        "csrr a1, mhpmevent14\n"
        "sw a1, %[power_manager_core_csr_c16_reg_offset](a0)\n"
        "csrr a1, mhpmevent15\n"
        "sw a1, %[power_manager_core_csr_c17_reg_offset](a0)\n"
        "csrr a1, mhpmevent16\n"
        "sw a1, %[power_manager_core_csr_c18_reg_offset](a0)\n"
        "csrr a1, mhpmevent17\n"
        "sw a1, %[power_manager_core_csr_c19_reg_offset](a0)\n"
        "csrr a1, mhpmevent18\n"
        "sw a1, %[power_manager_core_csr_c20_reg_offset](a0)\n"
        "csrr a1, mhpmevent19\n"
        "sw a1, %[power_manager_core_csr_c21_reg_offset](a0)\n"
        "csrr a1, mhpmevent20\n"
        "sw a1, %[power_manager_core_csr_c22_reg_offset](a0)\n"
        "csrr a1, mhpmevent21\n"
        "sw a1, %[power_manager_core_csr_c23_reg_offset](a0)\n"
        "csrr a1, mhpmevent22\n"
        "sw a1, %[power_manager_core_csr_c24_reg_offset](a0)\n"
        "csrr a1, mhpmevent23\n"
        "sw a1, %[power_manager_core_csr_c25_reg_offset](a0)\n"
        "csrr a1, mhpmevent24\n"
        "sw a1, %[power_manager_core_csr_c26_reg_offset](a0)\n"
        "csrr a1, mhpmevent25\n"
        "sw a1, %[power_manager_core_csr_c27_reg_offset](a0)\n"
        "csrr a1, mhpmevent26\n"
        "sw a1, %[power_manager_core_csr_c28_reg_offset](a0)\n"
        "csrr a1, mhpmevent27\n"
        "sw a1, %[power_manager_core_csr_c29_reg_offset](a0)\n"
        "csrr a1, mhpmevent28\n"
        "sw a1, %[power_manager_core_csr_c30_reg_offset](a0)\n"
        "csrr a1, mhpmevent29\n"
        "sw a1, %[power_manager_core_csr_c31_reg_offset](a0)\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_csr_c16_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C16_REG_OFFSET), \
        [power_manager_core_csr_c17_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C17_REG_OFFSET), \
        [power_manager_core_csr_c18_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C18_REG_OFFSET), \
        [power_manager_core_csr_c19_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C19_REG_OFFSET), \
        [power_manager_core_csr_c20_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C20_REG_OFFSET), \
        [power_manager_core_csr_c21_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C21_REG_OFFSET), \
        [power_manager_core_csr_c22_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C22_REG_OFFSET), \
        [power_manager_core_csr_c23_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C23_REG_OFFSET), \
        [power_manager_core_csr_c24_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C24_REG_OFFSET), \
        [power_manager_core_csr_c25_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C25_REG_OFFSET), \
        [power_manager_core_csr_c26_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C26_REG_OFFSET), \
        [power_manager_core_csr_c27_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C27_REG_OFFSET), \
        [power_manager_core_csr_c28_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C28_REG_OFFSET), \
        [power_manager_core_csr_c29_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C29_REG_OFFSET), \
        [power_manager_core_csr_c30_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C30_REG_OFFSET), \
        [power_manager_core_csr_c31_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C31_REG_OFFSET) \
    );

    asm volatile (

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        "csrr a1, mhpmevent30\n"
        "sw a1, %[power_manager_core_csr_c32_reg_offset](a0)\n"
        "csrr a1, mhpmevent31\n"
        "sw a1, %[power_manager_core_csr_c33_reg_offset](a0)\n"
        "csrr a1, mscratch\n"
        "sw a1, %[power_manager_core_csr_c34_reg_offset](a0)\n"
        "csrr a1, mepc\n"
        "sw a1, %[power_manager_core_csr_c35_reg_offset](a0)\n"
        "csrr a1, mcause\n"
        "sw a1, %[power_manager_core_csr_c36_reg_offset](a0)\n"
        "csrr a1, mtval\n"
        "sw a1, %[power_manager_core_csr_c37_reg_offset](a0)\n" : : \
        // "csrr a1, mip\n"
        // "sw a1, %[power_manager_core_csr_c38_reg_offset](a0)\n"
        // "csrr a1, pmpcfg0\n"
        // "sw a1, %[power_manager_core_csr_c39_reg_offset](a0)\n"
        // "csrr a1, pmpcfg1\n"
        // "sw a1, %[power_manager_core_csr_c40_reg_offset](a0)\n"
        // "csrr a1, pmpcfg2\n"
        // "sw a1, %[power_manager_core_csr_c41_reg_offset](a0)\n"
        // "csrr a1, pmpcfg3\n"
        // "sw a1, %[power_manager_core_csr_c42_reg_offset](a0)\n"
        // "csrr a1, pmpaddr0\n"
        // "sw a1, %[power_manager_core_csr_c43_reg_offset](a0)\n"
        // "csrr a1, pmpaddr1\n"
        // "sw a1, %[power_manager_core_csr_c44_reg_offset](a0)\n"
        // "csrr a1, pmpaddr2\n"
        // "sw a1, %[power_manager_core_csr_c45_reg_offset](a0)\n"
        // "csrr a1, pmpaddr3\n"
        // "sw a1, %[power_manager_core_csr_c46_reg_offset](a0)\n"
        // "csrr a1, pmpaddr4\n"
        // "sw a1, %[power_manager_core_csr_c47_reg_offset](a0)\n"
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_csr_c32_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C32_REG_OFFSET), \
        [power_manager_core_csr_c33_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C33_REG_OFFSET), \
        [power_manager_core_csr_c34_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C34_REG_OFFSET), \
        [power_manager_core_csr_c35_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C35_REG_OFFSET), \
        [power_manager_core_csr_c36_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C36_REG_OFFSET), \
        [power_manager_core_csr_c37_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C37_REG_OFFSET) \
        // [power_manager_core_csr_c38_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C38_REG_OFFSET),
        // [power_manager_core_csr_c39_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C39_REG_OFFSET),
        // [power_manager_core_csr_c40_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C40_REG_OFFSET),
        // [power_manager_core_csr_c41_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C41_REG_OFFSET),
        // [power_manager_core_csr_c42_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C42_REG_OFFSET),
        // [power_manager_core_csr_c43_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C43_REG_OFFSET),
        // [power_manager_core_csr_c44_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C44_REG_OFFSET),
        // [power_manager_core_csr_c45_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C45_REG_OFFSET),
        // [power_manager_core_csr_c46_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C46_REG_OFFSET),
        // [power_manager_core_csr_c47_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C47_REG_OFFSET),
    );

    asm volatile (

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        // "csrr a1, pmpaddr5\n"
        // "sw a1, %[power_manager_core_csr_c48_reg_offset](a0)\n"
        // "csrr a1, pmpaddr6\n"
        // "sw a1, %[power_manager_core_csr_c49_reg_offset](a0)\n"
        // "csrr a1, pmpaddr7\n"
        // "sw a1, %[power_manager_core_csr_c50_reg_offset](a0)\n"
        // "csrr a1, pmpaddr8\n"
        // "sw a1, %[power_manager_core_csr_c51_reg_offset](a0)\n"
        // "csrr a1, pmpaddr9\n"
        // "sw a1, %[power_manager_core_csr_c52_reg_offset](a0)\n"
        // "csrr a1, pmpaddr10\n"
        // "sw a1, %[power_manager_core_csr_c53_reg_offset](a0)\n"
        // "csrr a1, pmpaddr11\n"
        // "sw a1, %[power_manager_core_csr_c54_reg_offset](a0)\n"
        // "csrr a1, pmpaddr12\n"
        // "sw a1, %[power_manager_core_csr_c55_reg_offset](a0)\n"
        // "csrr a1, pmpaddr13\n"
        // "sw a1, %[power_manager_core_csr_c56_reg_offset](a0)\n"
        // "csrr a1, pmpaddr14\n"
        // "sw a1, %[power_manager_core_csr_c57_reg_offset](a0)\n"
        // "csrr a1, pmpaddr15\n"
        // "sw a1, %[power_manager_core_csr_c58_reg_offset](a0)\n"
        // "csrr a1, mseccfg\n"
        // "sw a1, %[power_manager_core_csr_c59_reg_offset](a0)\n"
        // "csrr a1, mseccfgh\n"
        // "sw a1, %[power_manager_core_csr_c60_reg_offset](a0)\n"
        "csrr a1, tselect\n"
        "sw a1, %[power_manager_core_csr_c61_reg_offset](a0)\n"
        "csrr a1, tdata1\n"
        "sw a1, %[power_manager_core_csr_c62_reg_offset](a0)\n"
        "csrr a1, tdata2\n"
        "sw a1, %[power_manager_core_csr_c63_reg_offset](a0)\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        // [power_manager_core_csr_c48_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C48_REG_OFFSET),
        // [power_manager_core_csr_c49_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C49_REG_OFFSET),
        // [power_manager_core_csr_c50_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C50_REG_OFFSET),
        // [power_manager_core_csr_c51_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C51_REG_OFFSET),
        // [power_manager_core_csr_c52_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C52_REG_OFFSET),
        // [power_manager_core_csr_c53_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C53_REG_OFFSET),
        // [power_manager_core_csr_c54_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C54_REG_OFFSET),
        // [power_manager_core_csr_c55_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C55_REG_OFFSET),
        // [power_manager_core_csr_c56_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C56_REG_OFFSET),
        // [power_manager_core_csr_c57_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C57_REG_OFFSET),
        // [power_manager_core_csr_c58_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C58_REG_OFFSET),
        // [power_manager_core_csr_c59_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C59_REG_OFFSET),
        // [power_manager_core_csr_c60_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C60_REG_OFFSET),
        [power_manager_core_csr_c61_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C61_REG_OFFSET), \
        [power_manager_core_csr_c62_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C62_REG_OFFSET), \
        [power_manager_core_csr_c63_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C63_REG_OFFSET) \
    );

    asm volatile (

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        "csrr a1, tdata3\n"
        "sw a1, %[power_manager_core_csr_c64_reg_offset](a0)\n"
        "csrr a1, mcontext\n"
        "sw a1, %[power_manager_core_csr_c65_reg_offset](a0)\n"
        "csrr a1, scontext\n"
        "sw a1, %[power_manager_core_csr_c66_reg_offset](a0)\n"
        // "csrr a1, dcsr\n"
        // "sw a1, %[power_manager_core_csr_c67_reg_offset](a0)\n"
        // "csrr a1, dpc\n"
        // "sw a1, %[power_manager_core_csr_c68_reg_offset](a0)\n"
        // "csrr a1, dscratch0\n"
        // "sw a1, %[power_manager_core_csr_c69_reg_offset](a0)\n"
        // "csrr a1, dscratch1\n"
        // "sw a1, %[power_manager_core_csr_c70_reg_offset](a0)\n"
        // "csrr a1, cpuctrl\n"
        // "sw a1, %[power_manager_core_csr_c71_reg_offset](a0)\n"
        // "csrr a1, secureseed\n"
        // "sw a1, %[power_manager_core_csr_c72_reg_offset](a0)\n"
        "csrr a1, mcycle\n"
        "sw a1, %[power_manager_core_csr_c73_reg_offset](a0)\n"
        "csrr a1, minstret\n"
        "sw a1, %[power_manager_core_csr_c74_reg_offset](a0)\n"
        "csrr a1, mhpmcounter3\n"
        "sw a1, %[power_manager_core_csr_c75_reg_offset](a0)\n"
        "csrr a1, mhpmcounter4\n"
        "sw a1, %[power_manager_core_csr_c76_reg_offset](a0)\n"
        "csrr a1, mhpmcounter5\n"
        "sw a1, %[power_manager_core_csr_c77_reg_offset](a0)\n"
        "csrr a1, mhpmcounter6\n"
        "sw a1, %[power_manager_core_csr_c78_reg_offset](a0)\n"
        "csrr a1, mhpmcounter7\n"
        "sw a1, %[power_manager_core_csr_c79_reg_offset](a0)\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_csr_c64_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C64_REG_OFFSET), \
        [power_manager_core_csr_c65_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C65_REG_OFFSET), \
        [power_manager_core_csr_c66_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C66_REG_OFFSET), \
        // [power_manager_core_csr_c67_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C67_REG_OFFSET),
        // [power_manager_core_csr_c68_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C68_REG_OFFSET),
        // [power_manager_core_csr_c69_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C69_REG_OFFSET),
        // [power_manager_core_csr_c70_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C70_REG_OFFSET),
        // [power_manager_core_csr_c71_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C71_REG_OFFSET),
        // [power_manager_core_csr_c72_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C72_REG_OFFSET),
        [power_manager_core_csr_c73_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C73_REG_OFFSET), \
        [power_manager_core_csr_c74_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C74_REG_OFFSET), \
        [power_manager_core_csr_c75_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C75_REG_OFFSET), \
        [power_manager_core_csr_c76_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C76_REG_OFFSET), \
        [power_manager_core_csr_c77_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C77_REG_OFFSET), \
        [power_manager_core_csr_c78_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C78_REG_OFFSET), \
        [power_manager_core_csr_c79_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C79_REG_OFFSET) \
    );

    asm volatile (

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        "csrr a1, mhpmcounter8\n"
        "sw a1, %[power_manager_core_csr_c80_reg_offset](a0)\n"
        "csrr a1, mhpmcounter9\n"
        "sw a1, %[power_manager_core_csr_c81_reg_offset](a0)\n"
        "csrr a1, mhpmcounter10\n"
        "sw a1, %[power_manager_core_csr_c82_reg_offset](a0)\n"
        "csrr a1, mhpmcounter11\n"
        "sw a1, %[power_manager_core_csr_c83_reg_offset](a0)\n"
        "csrr a1, mhpmcounter12\n"
        "sw a1, %[power_manager_core_csr_c84_reg_offset](a0)\n"
        "csrr a1, mhpmcounter13\n"
        "sw a1, %[power_manager_core_csr_c85_reg_offset](a0)\n"
        "csrr a1, mhpmcounter14\n"
        "sw a1, %[power_manager_core_csr_c86_reg_offset](a0)\n"
        "csrr a1, mhpmcounter15\n"
        "sw a1, %[power_manager_core_csr_c87_reg_offset](a0)\n"
        "csrr a1, mhpmcounter16\n"
        "sw a1, %[power_manager_core_csr_c88_reg_offset](a0)\n"
        "csrr a1, mhpmcounter17\n"
        "sw a1, %[power_manager_core_csr_c89_reg_offset](a0)\n"
        "csrr a1, mhpmcounter18\n"
        "sw a1, %[power_manager_core_csr_c90_reg_offset](a0)\n"
        "csrr a1, mhpmcounter19\n"
        "sw a1, %[power_manager_core_csr_c91_reg_offset](a0)\n"
        "csrr a1, mhpmcounter20\n"
        "sw a1, %[power_manager_core_csr_c92_reg_offset](a0)\n"
        "csrr a1, mhpmcounter21\n"
        "sw a1, %[power_manager_core_csr_c93_reg_offset](a0)\n"
        "csrr a1, mhpmcounter22\n"
        "sw a1, %[power_manager_core_csr_c94_reg_offset](a0)\n"
        "csrr a1, mhpmcounter23\n"
        "sw a1, %[power_manager_core_csr_c95_reg_offset](a0)\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_csr_c80_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C80_REG_OFFSET), \
        [power_manager_core_csr_c81_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C81_REG_OFFSET), \
        [power_manager_core_csr_c82_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C82_REG_OFFSET), \
        [power_manager_core_csr_c83_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C83_REG_OFFSET), \
        [power_manager_core_csr_c84_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C84_REG_OFFSET), \
        [power_manager_core_csr_c85_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C85_REG_OFFSET), \
        [power_manager_core_csr_c86_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C86_REG_OFFSET), \
        [power_manager_core_csr_c87_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C87_REG_OFFSET), \
        [power_manager_core_csr_c88_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C88_REG_OFFSET), \
        [power_manager_core_csr_c89_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C89_REG_OFFSET), \
        [power_manager_core_csr_c90_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C90_REG_OFFSET), \
        [power_manager_core_csr_c91_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C91_REG_OFFSET), \
        [power_manager_core_csr_c92_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C92_REG_OFFSET), \
        [power_manager_core_csr_c93_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C93_REG_OFFSET), \
        [power_manager_core_csr_c94_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C94_REG_OFFSET), \
        [power_manager_core_csr_c95_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C95_REG_OFFSET) \
    );

    asm volatile (

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        "csrr a1, mhpmcounter24\n"
        "sw a1, %[power_manager_core_csr_c96_reg_offset](a0)\n"
        "csrr a1, mhpmcounter25\n"
        "sw a1, %[power_manager_core_csr_c97_reg_offset](a0)\n"
        "csrr a1, mhpmcounter26\n"
        "sw a1, %[power_manager_core_csr_c98_reg_offset](a0)\n"
        "csrr a1, mhpmcounter27\n"
        "sw a1, %[power_manager_core_csr_c99_reg_offset](a0)\n"
        "csrr a1, mhpmcounter28\n"
        "sw a1, %[power_manager_core_csr_c100_reg_offset](a0)\n"
        "csrr a1, mhpmcounter29\n"
        "sw a1, %[power_manager_core_csr_c101_reg_offset](a0)\n"
        "csrr a1, mhpmcounter30\n"
        "sw a1, %[power_manager_core_csr_c102_reg_offset](a0)\n"
        "csrr a1, mhpmcounter31\n"
        "sw a1, %[power_manager_core_csr_c103_reg_offset](a0)\n"
        "csrr a1, mcycleh\n"
        "sw a1, %[power_manager_core_csr_c104_reg_offset](a0)\n"
        "csrr a1, minstreth\n"
        "sw a1, %[power_manager_core_csr_c105_reg_offset](a0)\n"
        "csrr a1, mhpmcounter3h\n"
        "sw a1, %[power_manager_core_csr_c106_reg_offset](a0)\n"
        "csrr a1, mhpmcounter4h\n"
        "sw a1, %[power_manager_core_csr_c107_reg_offset](a0)\n"
        "csrr a1, mhpmcounter5h\n"
        "sw a1, %[power_manager_core_csr_c108_reg_offset](a0)\n"
        "csrr a1, mhpmcounter6h\n"
        "sw a1, %[power_manager_core_csr_c109_reg_offset](a0)\n"
        "csrr a1, mhpmcounter7h\n"
        "sw a1, %[power_manager_core_csr_c110_reg_offset](a0)\n"
        "csrr a1, mhpmcounter8h\n"
        "sw a1, %[power_manager_core_csr_c111_reg_offset](a0)\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_csr_c96_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C96_REG_OFFSET), \
        [power_manager_core_csr_c97_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C97_REG_OFFSET), \
        [power_manager_core_csr_c98_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C98_REG_OFFSET), \
        [power_manager_core_csr_c99_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C99_REG_OFFSET), \
        [power_manager_core_csr_c100_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C100_REG_OFFSET), \
        [power_manager_core_csr_c101_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C101_REG_OFFSET), \
        [power_manager_core_csr_c102_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C102_REG_OFFSET), \
        [power_manager_core_csr_c103_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C103_REG_OFFSET), \
        [power_manager_core_csr_c104_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C104_REG_OFFSET), \
        [power_manager_core_csr_c105_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C105_REG_OFFSET), \
        [power_manager_core_csr_c106_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C106_REG_OFFSET), \
        [power_manager_core_csr_c107_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C107_REG_OFFSET), \
        [power_manager_core_csr_c108_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C108_REG_OFFSET), \
        [power_manager_core_csr_c109_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C109_REG_OFFSET), \
        [power_manager_core_csr_c110_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C110_REG_OFFSET), \
        [power_manager_core_csr_c111_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C111_REG_OFFSET) \
    );

    asm volatile (

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        "csrr a1, mhpmcounter9h\n"
        "sw a1, %[power_manager_core_csr_c112_reg_offset](a0)\n"
        "csrr a1, mhpmcounter10h\n"
        "sw a1, %[power_manager_core_csr_c113_reg_offset](a0)\n"
        "csrr a1, mhpmcounter11h\n"
        "sw a1, %[power_manager_core_csr_c114_reg_offset](a0)\n"
        "csrr a1, mhpmcounter12h\n"
        "sw a1, %[power_manager_core_csr_c115_reg_offset](a0)\n"
        "csrr a1, mhpmcounter13h\n"
        "sw a1, %[power_manager_core_csr_c116_reg_offset](a0)\n"
        "csrr a1, mhpmcounter14h\n"
        "sw a1, %[power_manager_core_csr_c117_reg_offset](a0)\n"
        "csrr a1, mhpmcounter15h\n"
        "sw a1, %[power_manager_core_csr_c118_reg_offset](a0)\n"
        "csrr a1, mhpmcounter16h\n"
        "sw a1, %[power_manager_core_csr_c119_reg_offset](a0)\n"
        "csrr a1, mhpmcounter17h\n"
        "sw a1, %[power_manager_core_csr_c120_reg_offset](a0)\n"
        "csrr a1, mhpmcounter18h\n"
        "sw a1, %[power_manager_core_csr_c121_reg_offset](a0)\n"
        "csrr a1, mhpmcounter19h\n"
        "sw a1, %[power_manager_core_csr_c122_reg_offset](a0)\n"
        "csrr a1, mhpmcounter20h\n"
        "sw a1, %[power_manager_core_csr_c123_reg_offset](a0)\n"
        "csrr a1, mhpmcounter21h\n"
        "sw a1, %[power_manager_core_csr_c124_reg_offset](a0)\n"
        "csrr a1, mhpmcounter22h\n"
        "sw a1, %[power_manager_core_csr_c125_reg_offset](a0)\n"
        "csrr a1, mhpmcounter23h\n"
        "sw a1, %[power_manager_core_csr_c126_reg_offset](a0)\n"
        "csrr a1, mhpmcounter24h\n"
        "sw a1, %[power_manager_core_csr_c127_reg_offset](a0)\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_csr_c112_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C112_REG_OFFSET), \
        [power_manager_core_csr_c113_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C113_REG_OFFSET), \
        [power_manager_core_csr_c114_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C114_REG_OFFSET), \
        [power_manager_core_csr_c115_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C115_REG_OFFSET), \
        [power_manager_core_csr_c116_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C116_REG_OFFSET), \
        [power_manager_core_csr_c117_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C117_REG_OFFSET), \
        [power_manager_core_csr_c118_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C118_REG_OFFSET), \
        [power_manager_core_csr_c119_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C119_REG_OFFSET), \
        [power_manager_core_csr_c120_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C120_REG_OFFSET), \
        [power_manager_core_csr_c121_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C121_REG_OFFSET), \
        [power_manager_core_csr_c122_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C122_REG_OFFSET), \
        [power_manager_core_csr_c123_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C123_REG_OFFSET), \
        [power_manager_core_csr_c124_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C124_REG_OFFSET), \
        [power_manager_core_csr_c125_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C125_REG_OFFSET), \
        [power_manager_core_csr_c126_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C126_REG_OFFSET), \
        [power_manager_core_csr_c127_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C127_REG_OFFSET) \
    );

    asm volatile (

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        "csrr a1, mhpmcounter25h\n"
        "sw a1, %[power_manager_core_csr_c128_reg_offset](a0)\n"
        "csrr a1, mhpmcounter26h\n"
        "sw a1, %[power_manager_core_csr_c129_reg_offset](a0)\n"
        "csrr a1, mhpmcounter27h\n"
        "sw a1, %[power_manager_core_csr_c130_reg_offset](a0)\n"
        "csrr a1, mhpmcounter28h\n"
        "sw a1, %[power_manager_core_csr_c131_reg_offset](a0)\n"
        "csrr a1, mhpmcounter29h\n"
        "sw a1, %[power_manager_core_csr_c132_reg_offset](a0)\n"
        "csrr a1, mhpmcounter30h\n"
        "sw a1, %[power_manager_core_csr_c133_reg_offset](a0)\n"
        "csrr a1, mhpmcounter31h\n"
        "sw a1, %[power_manager_core_csr_c134_reg_offset](a0)\n" : : \
        // "csrr a1, mvendorid\n"
        // "sw a1, %[power_manager_core_csr_c135_reg_offset](a0)\n"
        // "csrr a1, marchid\n"
        // "sw a1, %[power_manager_core_csr_c136_reg_offset](a0)\n"
        // "csrr a1, mimpid\n"
        // "sw a1, %[power_manager_core_csr_c137_reg_offset](a0)\n"
        // "csrr a1, mhartid\n"
        // "sw a1, %[power_manager_core_csr_c138_reg_offset](a0)\n"
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_csr_c128_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C128_REG_OFFSET), \
        [power_manager_core_csr_c129_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C129_REG_OFFSET), \
        [power_manager_core_csr_c130_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C130_REG_OFFSET), \
        [power_manager_core_csr_c131_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C131_REG_OFFSET), \
        [power_manager_core_csr_c132_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C132_REG_OFFSET), \
        [power_manager_core_csr_c133_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C133_REG_OFFSET), \
        [power_manager_core_csr_c134_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C134_REG_OFFSET) \
        // [power_manager_core_csr_c135_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C135_REG_OFFSET),
        // [power_manager_core_csr_c136_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C136_REG_OFFSET),
        // [power_manager_core_csr_c137_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C137_REG_OFFSET),
        // [power_manager_core_csr_c138_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C138_REG_OFFSET)
    );

    asm volatile (

        // write RESTORE_ADDRESS[31:0] = PC
        "lui a0, %[base_address_20bit]\n"
        "la  a1, wakeup\n"
        "sw  a1, %[power_manager_restore_address_reg_offset](a0)\n"

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
        "lui a0, %[base_address_20bit]\n"
        "sw  x0, %[power_manager_power_gate_core_reg_offset](a0)\n"

        // write WAKEUP_STATE[0] = 0
        "sw x0, %[power_manager_wakeup_state_reg_offset](a0)\n"

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

    asm volatile (

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        "lw a1, %[power_manager_core_csr_c0_reg_offset](a0)\n"
        "csrw mstatus, a1\n"
        "lw a1, %[power_manager_core_csr_c1_reg_offset](a0)\n"
        "csrw misa, a1\n"
        "lw a1, %[power_manager_core_csr_c2_reg_offset](a0)\n"
        "csrw mie, a1\n"
        "lw a1, %[power_manager_core_csr_c3_reg_offset](a0)\n"
        "csrw mtvec, a1\n"
        "lw a1, %[power_manager_core_csr_c4_reg_offset](a0)\n"
        "csrw mcountinhibit, a1\n"
        "lw a1, %[power_manager_core_csr_c5_reg_offset](a0)\n"
        "csrw mhpmevent3, a1\n"
        "lw a1, %[power_manager_core_csr_c6_reg_offset](a0)\n"
        "csrw mhpmevent4, a1\n"
        "lw a1, %[power_manager_core_csr_c7_reg_offset](a0)\n"
        "csrw mhpmevent5, a1\n"
        "lw a1, %[power_manager_core_csr_c8_reg_offset](a0)\n"
        "csrw mhpmevent6, a1\n"
        "lw a1, %[power_manager_core_csr_c9_reg_offset](a0)\n"
        "csrw mhpmevent7, a1\n"
        "lw a1, %[power_manager_core_csr_c10_reg_offset](a0)\n"
        "csrw mhpmevent8, a1\n"
        "lw a1, %[power_manager_core_csr_c11_reg_offset](a0)\n"
        "csrw mhpmevent9, a1\n"
        "lw a1, %[power_manager_core_csr_c12_reg_offset](a0)\n"
        "csrw mhpmevent10, a1\n"
        "lw a1, %[power_manager_core_csr_c13_reg_offset](a0)\n"
        "csrw mhpmevent11, a1\n"
        "lw a1, %[power_manager_core_csr_c14_reg_offset](a0)\n"
        "csrw mhpmevent12, a1\n"
        "lw a1, %[power_manager_core_csr_c15_reg_offset](a0)\n"
        "csrw mhpmevent13, a1\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_csr_c0_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C0_REG_OFFSET), \
        [power_manager_core_csr_c1_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C1_REG_OFFSET), \
        [power_manager_core_csr_c2_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C2_REG_OFFSET), \
        [power_manager_core_csr_c3_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C3_REG_OFFSET), \
        [power_manager_core_csr_c4_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C4_REG_OFFSET), \
        [power_manager_core_csr_c5_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C5_REG_OFFSET), \
        [power_manager_core_csr_c6_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C6_REG_OFFSET), \
        [power_manager_core_csr_c7_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C7_REG_OFFSET), \
        [power_manager_core_csr_c8_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C8_REG_OFFSET), \
        [power_manager_core_csr_c9_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C9_REG_OFFSET), \
        [power_manager_core_csr_c10_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C10_REG_OFFSET), \
        [power_manager_core_csr_c11_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C11_REG_OFFSET), \
        [power_manager_core_csr_c12_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C12_REG_OFFSET), \
        [power_manager_core_csr_c13_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C13_REG_OFFSET), \
        [power_manager_core_csr_c14_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C14_REG_OFFSET), \
        [power_manager_core_csr_c15_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C15_REG_OFFSET) \
    );

    asm volatile (

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        "lw a1, %[power_manager_core_csr_c16_reg_offset](a0)\n"
        "csrw mhpmevent14, a1\n"
        "lw a1, %[power_manager_core_csr_c17_reg_offset](a0)\n"
        "csrw mhpmevent15, a1\n"
        "lw a1, %[power_manager_core_csr_c18_reg_offset](a0)\n"
        "csrw mhpmevent16, a1\n"
        "lw a1, %[power_manager_core_csr_c19_reg_offset](a0)\n"
        "csrw mhpmevent17, a1\n"
        "lw a1, %[power_manager_core_csr_c20_reg_offset](a0)\n"
        "csrw mhpmevent18, a1\n"
        "lw a1, %[power_manager_core_csr_c21_reg_offset](a0)\n"
        "csrw mhpmevent19, a1\n"
        "lw a1, %[power_manager_core_csr_c22_reg_offset](a0)\n"
        "csrw mhpmevent20, a1\n"
        "lw a1, %[power_manager_core_csr_c23_reg_offset](a0)\n"
        "csrw mhpmevent21, a1\n"
        "lw a1, %[power_manager_core_csr_c24_reg_offset](a0)\n"
        "csrw mhpmevent22, a1\n"
        "lw a1, %[power_manager_core_csr_c25_reg_offset](a0)\n"
        "csrw mhpmevent23, a1\n"
        "lw a1, %[power_manager_core_csr_c26_reg_offset](a0)\n"
        "csrw mhpmevent24, a1\n"
        "lw a1, %[power_manager_core_csr_c27_reg_offset](a0)\n"
        "csrw mhpmevent25, a1\n"
        "lw a1, %[power_manager_core_csr_c28_reg_offset](a0)\n"
        "csrw mhpmevent26, a1\n"
        "lw a1, %[power_manager_core_csr_c29_reg_offset](a0)\n"
        "csrw mhpmevent27, a1\n"
        "lw a1, %[power_manager_core_csr_c30_reg_offset](a0)\n"
        "csrw mhpmevent28, a1\n"
        "lw a1, %[power_manager_core_csr_c31_reg_offset](a0)\n"
        "csrw mhpmevent29, a1\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_csr_c16_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C16_REG_OFFSET), \
        [power_manager_core_csr_c17_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C17_REG_OFFSET), \
        [power_manager_core_csr_c18_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C18_REG_OFFSET), \
        [power_manager_core_csr_c19_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C19_REG_OFFSET), \
        [power_manager_core_csr_c20_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C20_REG_OFFSET), \
        [power_manager_core_csr_c21_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C21_REG_OFFSET), \
        [power_manager_core_csr_c22_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C22_REG_OFFSET), \
        [power_manager_core_csr_c23_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C23_REG_OFFSET), \
        [power_manager_core_csr_c24_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C24_REG_OFFSET), \
        [power_manager_core_csr_c25_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C25_REG_OFFSET), \
        [power_manager_core_csr_c26_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C26_REG_OFFSET), \
        [power_manager_core_csr_c27_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C27_REG_OFFSET), \
        [power_manager_core_csr_c28_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C28_REG_OFFSET), \
        [power_manager_core_csr_c29_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C29_REG_OFFSET), \
        [power_manager_core_csr_c30_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C30_REG_OFFSET), \
        [power_manager_core_csr_c31_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C31_REG_OFFSET) \
    );

    asm volatile (

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        "lw a1, %[power_manager_core_csr_c32_reg_offset](a0)\n"
        "csrw mhpmevent30, a1\n"
        "lw a1, %[power_manager_core_csr_c33_reg_offset](a0)\n"
        "csrw mhpmevent31, a1\n"
        "lw a1, %[power_manager_core_csr_c34_reg_offset](a0)\n"
        "csrw mscratch, a1\n"
        "lw a1, %[power_manager_core_csr_c35_reg_offset](a0)\n"
        "csrw mepc, a1\n"
        "lw a1, %[power_manager_core_csr_c36_reg_offset](a0)\n"
        "csrw mcause, a1\n"
        "lw a1, %[power_manager_core_csr_c37_reg_offset](a0)\n"
        "csrw mtval, a1\n" : : \
        // "lw a1, %[power_manager_core_csr_c38_reg_offset](a0)\n"
        // "csrw mip, a1\n"
        // "lw a1, %[power_manager_core_csr_c39_reg_offset](a0)\n"
        // "csrw pmpcfg0, a1\n"
        // "lw a1, %[power_manager_core_csr_c40_reg_offset](a0)\n"
        // "csrw pmpcfg1, a1\n"
        // "lw a1, %[power_manager_core_csr_c41_reg_offset](a0)\n"
        // "csrw pmpcfg2, a1\n"
        // "lw a1, %[power_manager_core_csr_c42_reg_offset](a0)\n"
        // "csrw pmpcfg3, a1\n"
        // "lw a1, %[power_manager_core_csr_c43_reg_offset](a0)\n"
        // "csrw pmpaddr0, a1\n"
        // "lw a1, %[power_manager_core_csr_c44_reg_offset](a0)\n"
        // "csrw pmpaddr1, a1\n"
        // "lw a1, %[power_manager_core_csr_c45_reg_offset](a0)\n"
        // "csrw pmpaddr2, a1\n"
        // "lw a1, %[power_manager_core_csr_c46_reg_offset](a0)\n"
        // "csrw pmpaddr3, a1\n"
        // "lw a1, %[power_manager_core_csr_c47_reg_offset](a0)\n"
        // "csrw pmpaddr4, a1\n"
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_csr_c32_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C32_REG_OFFSET), \
        [power_manager_core_csr_c33_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C33_REG_OFFSET), \
        [power_manager_core_csr_c34_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C34_REG_OFFSET), \
        [power_manager_core_csr_c35_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C35_REG_OFFSET), \
        [power_manager_core_csr_c36_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C36_REG_OFFSET), \
        [power_manager_core_csr_c37_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C37_REG_OFFSET) \
        // [power_manager_core_csr_c38_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C38_REG_OFFSET),
        // [power_manager_core_csr_c39_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C39_REG_OFFSET),
        // [power_manager_core_csr_c40_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C40_REG_OFFSET),
        // [power_manager_core_csr_c41_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C41_REG_OFFSET),
        // [power_manager_core_csr_c42_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C42_REG_OFFSET),
        // [power_manager_core_csr_c43_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C43_REG_OFFSET),
        // [power_manager_core_csr_c44_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C44_REG_OFFSET),
        // [power_manager_core_csr_c45_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C45_REG_OFFSET),
        // [power_manager_core_csr_c46_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C46_REG_OFFSET),
        // [power_manager_core_csr_c47_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C47_REG_OFFSET),
    );

    asm volatile (

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        // "lw a1, %[power_manager_core_csr_c48_reg_offset](a0)\n"
        // "csrw pmpaddr5, a1\n"
        // "lw a1, %[power_manager_core_csr_c49_reg_offset](a0)\n"
        // "csrw pmpaddr6, a1\n"
        // "lw a1, %[power_manager_core_csr_c50_reg_offset](a0)\n"
        // "csrw pmpaddr7, a1\n"
        // "lw a1, %[power_manager_core_csr_c51_reg_offset](a0)\n"
        // "csrw pmpaddr8, a1\n"
        // "lw a1, %[power_manager_core_csr_c52_reg_offset](a0)\n"
        // "csrw pmpaddr9, a1\n"
        // "lw a1, %[power_manager_core_csr_c53_reg_offset](a0)\n"
        // "csrw pmpaddr10, a1\n"
        // "lw a1, %[power_manager_core_csr_c54_reg_offset](a0)\n"
        // "csrw pmpaddr11, a1\n"
        // "lw a1, %[power_manager_core_csr_c55_reg_offset](a0)\n"
        // "csrw pmpaddr12, a1\n"
        // "lw a1, %[power_manager_core_csr_c56_reg_offset](a0)\n"
        // "csrw pmpaddr13, a1\n"
        // "lw a1, %[power_manager_core_csr_c57_reg_offset](a0)\n"
        // "csrw pmpaddr14, a1\n"
        // "lw a1, %[power_manager_core_csr_c58_reg_offset](a0)\n"
        // "csrw pmpaddr15, a1\n"
        // "lw a1, %[power_manager_core_csr_c59_reg_offset](a0)\n"
        // "csrw mseccfg, a1\n"
        // "lw a1, %[power_manager_core_csr_c60_reg_offset](a0)\n"
        // "csrw mseccfgh, a1\n"
        "lw a1, %[power_manager_core_csr_c61_reg_offset](a0)\n"
        "csrw tselect, a1\n"
        "lw a1, %[power_manager_core_csr_c62_reg_offset](a0)\n"
        "csrw tdata1, a1\n"
        "lw a1, %[power_manager_core_csr_c63_reg_offset](a0)\n"
        "csrw tdata2, a1\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        // [power_manager_core_csr_c48_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C48_REG_OFFSET),
        // [power_manager_core_csr_c49_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C49_REG_OFFSET),
        // [power_manager_core_csr_c50_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C50_REG_OFFSET),
        // [power_manager_core_csr_c51_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C51_REG_OFFSET),
        // [power_manager_core_csr_c52_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C52_REG_OFFSET),
        // [power_manager_core_csr_c53_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C53_REG_OFFSET),
        // [power_manager_core_csr_c54_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C54_REG_OFFSET),
        // [power_manager_core_csr_c55_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C55_REG_OFFSET),
        // [power_manager_core_csr_c56_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C56_REG_OFFSET),
        // [power_manager_core_csr_c57_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C57_REG_OFFSET),
        // [power_manager_core_csr_c58_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C58_REG_OFFSET),
        // [power_manager_core_csr_c59_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C59_REG_OFFSET),
        // [power_manager_core_csr_c60_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C60_REG_OFFSET),
        [power_manager_core_csr_c61_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C61_REG_OFFSET), \
        [power_manager_core_csr_c62_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C62_REG_OFFSET), \
        [power_manager_core_csr_c63_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C63_REG_OFFSET) \
    );

    asm volatile (

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        "lw a1, %[power_manager_core_csr_c64_reg_offset](a0)\n"
        "csrw tdata3, a1\n"
        "lw a1, %[power_manager_core_csr_c65_reg_offset](a0)\n"
        "csrw mcontext, a1\n"
        "lw a1, %[power_manager_core_csr_c66_reg_offset](a0)\n"
        "csrw scontext, a1\n"
        // "lw a1, %[power_manager_core_csr_c67_reg_offset](a0)\n"
        // "csrw dcsr, a1\n"
        // "lw a1, %[power_manager_core_csr_c68_reg_offset](a0)\n"
        // "csrw dpc, a1\n"
        // "lw a1, %[power_manager_core_csr_c69_reg_offset](a0)\n"
        // "csrw dscratch0, a1\n"
        // "lw a1, %[power_manager_core_csr_c70_reg_offset](a0)\n"
        // "csrw dscratch1, a1\n"
        // "lw a1, %[power_manager_core_csr_c71_reg_offset](a0)\n"
        // "csrw cpuctrl, a1\n"
        // "lw a1, %[power_manager_core_csr_c72_reg_offset](a0)\n"
        // "csrw secureseed, a1\n"
        "lw a1, %[power_manager_core_csr_c73_reg_offset](a0)\n"
        "csrw mcycle, a1\n"
        "lw a1, %[power_manager_core_csr_c74_reg_offset](a0)\n"
        "csrw minstret, a1\n"
        "lw a1, %[power_manager_core_csr_c75_reg_offset](a0)\n"
        "csrw mhpmcounter3, a1\n"
        "lw a1, %[power_manager_core_csr_c76_reg_offset](a0)\n"
        "csrw mhpmcounter4, a1\n"
        "lw a1, %[power_manager_core_csr_c77_reg_offset](a0)\n"
        "csrw mhpmcounter5, a1\n"
        "lw a1, %[power_manager_core_csr_c78_reg_offset](a0)\n"
        "csrw mhpmcounter6, a1\n"
        "lw a1, %[power_manager_core_csr_c79_reg_offset](a0)\n"
        "csrw mhpmcounter7, a1\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_csr_c64_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C64_REG_OFFSET), \
        [power_manager_core_csr_c65_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C65_REG_OFFSET), \
        [power_manager_core_csr_c66_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C66_REG_OFFSET), \
        // [power_manager_core_csr_c67_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C67_REG_OFFSET),
        // [power_manager_core_csr_c68_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C68_REG_OFFSET),
        // [power_manager_core_csr_c69_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C69_REG_OFFSET),
        // [power_manager_core_csr_c70_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C70_REG_OFFSET),
        // [power_manager_core_csr_c71_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C71_REG_OFFSET),
        // [power_manager_core_csr_c72_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C72_REG_OFFSET),
        [power_manager_core_csr_c73_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C73_REG_OFFSET), \
        [power_manager_core_csr_c74_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C74_REG_OFFSET), \
        [power_manager_core_csr_c75_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C75_REG_OFFSET), \
        [power_manager_core_csr_c76_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C76_REG_OFFSET), \
        [power_manager_core_csr_c77_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C77_REG_OFFSET), \
        [power_manager_core_csr_c78_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C78_REG_OFFSET), \
        [power_manager_core_csr_c79_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C79_REG_OFFSET) \
    );

    asm volatile (

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        "lw a1, %[power_manager_core_csr_c80_reg_offset](a0)\n"
        "csrw mhpmcounter8, a1\n"
        "lw a1, %[power_manager_core_csr_c81_reg_offset](a0)\n"
        "csrw mhpmcounter9, a1\n"
        "lw a1, %[power_manager_core_csr_c82_reg_offset](a0)\n"
        "csrw mhpmcounter10, a1\n"
        "lw a1, %[power_manager_core_csr_c83_reg_offset](a0)\n"
        "csrw mhpmcounter11, a1\n"
        "lw a1, %[power_manager_core_csr_c84_reg_offset](a0)\n"
        "csrw mhpmcounter12, a1\n"
        "lw a1, %[power_manager_core_csr_c85_reg_offset](a0)\n"
        "csrw mhpmcounter13, a1\n"
        "lw a1, %[power_manager_core_csr_c86_reg_offset](a0)\n"
        "csrw mhpmcounter14, a1\n"
        "lw a1, %[power_manager_core_csr_c87_reg_offset](a0)\n"
        "csrw mhpmcounter15, a1\n"
        "lw a1, %[power_manager_core_csr_c88_reg_offset](a0)\n"
        "csrw mhpmcounter16, a1\n"
        "lw a1, %[power_manager_core_csr_c89_reg_offset](a0)\n"
        "csrw mhpmcounter17, a1\n"
        "lw a1, %[power_manager_core_csr_c90_reg_offset](a0)\n"
        "csrw mhpmcounter18, a1\n"
        "lw a1, %[power_manager_core_csr_c91_reg_offset](a0)\n"
        "csrw mhpmcounter19, a1\n"
        "lw a1, %[power_manager_core_csr_c92_reg_offset](a0)\n"
        "csrw mhpmcounter20, a1\n"
        "lw a1, %[power_manager_core_csr_c93_reg_offset](a0)\n"
        "csrw mhpmcounter21, a1\n"
        "lw a1, %[power_manager_core_csr_c94_reg_offset](a0)\n"
        "csrw mhpmcounter22, a1\n"
        "lw a1, %[power_manager_core_csr_c95_reg_offset](a0)\n"
        "csrw mhpmcounter23, a1\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_csr_c80_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C80_REG_OFFSET), \
        [power_manager_core_csr_c81_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C81_REG_OFFSET), \
        [power_manager_core_csr_c82_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C82_REG_OFFSET), \
        [power_manager_core_csr_c83_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C83_REG_OFFSET), \
        [power_manager_core_csr_c84_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C84_REG_OFFSET), \
        [power_manager_core_csr_c85_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C85_REG_OFFSET), \
        [power_manager_core_csr_c86_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C86_REG_OFFSET), \
        [power_manager_core_csr_c87_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C87_REG_OFFSET), \
        [power_manager_core_csr_c88_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C88_REG_OFFSET), \
        [power_manager_core_csr_c89_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C89_REG_OFFSET), \
        [power_manager_core_csr_c90_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C90_REG_OFFSET), \
        [power_manager_core_csr_c91_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C91_REG_OFFSET), \
        [power_manager_core_csr_c92_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C92_REG_OFFSET), \
        [power_manager_core_csr_c93_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C93_REG_OFFSET), \
        [power_manager_core_csr_c94_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C94_REG_OFFSET), \
        [power_manager_core_csr_c95_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C95_REG_OFFSET) \
    );

    asm volatile (

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        "lw a1, %[power_manager_core_csr_c96_reg_offset](a0)\n"
        "csrw mhpmcounter24, a1\n"
        "lw a1, %[power_manager_core_csr_c97_reg_offset](a0)\n"
        "csrw mhpmcounter25, a1\n"
        "lw a1, %[power_manager_core_csr_c98_reg_offset](a0)\n"
        "csrw mhpmcounter26, a1\n"
        "lw a1, %[power_manager_core_csr_c99_reg_offset](a0)\n"
        "csrw mhpmcounter27, a1\n"
        "lw a1, %[power_manager_core_csr_c100_reg_offset](a0)\n"
        "csrw mhpmcounter28, a1\n"
        "lw a1, %[power_manager_core_csr_c101_reg_offset](a0)\n"
        "csrw mhpmcounter29, a1\n"
        "lw a1, %[power_manager_core_csr_c102_reg_offset](a0)\n"
        "csrw mhpmcounter30, a1\n"
        "lw a1, %[power_manager_core_csr_c103_reg_offset](a0)\n"
        "csrw mhpmcounter31, a1\n"
        "lw a1, %[power_manager_core_csr_c104_reg_offset](a0)\n"
        "csrw mcycleh, a1\n"
        "lw a1, %[power_manager_core_csr_c105_reg_offset](a0)\n"
        "csrw minstreth, a1\n"
        "lw a1, %[power_manager_core_csr_c106_reg_offset](a0)\n"
        "csrw mhpmcounter3h, a1\n"
        "lw a1, %[power_manager_core_csr_c107_reg_offset](a0)\n"
        "csrw mhpmcounter4h, a1\n"
        "lw a1, %[power_manager_core_csr_c108_reg_offset](a0)\n"
        "csrw mhpmcounter5h, a1\n"
        "lw a1, %[power_manager_core_csr_c109_reg_offset](a0)\n"
        "csrw mhpmcounter6h, a1\n"
        "lw a1, %[power_manager_core_csr_c110_reg_offset](a0)\n"
        "csrw mhpmcounter7h, a1\n"
        "lw a1, %[power_manager_core_csr_c111_reg_offset](a0)\n"
        "csrw mhpmcounter8h, a1\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_csr_c96_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C96_REG_OFFSET), \
        [power_manager_core_csr_c97_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C97_REG_OFFSET), \
        [power_manager_core_csr_c98_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C98_REG_OFFSET), \
        [power_manager_core_csr_c99_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C99_REG_OFFSET), \
        [power_manager_core_csr_c100_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C100_REG_OFFSET), \
        [power_manager_core_csr_c101_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C101_REG_OFFSET), \
        [power_manager_core_csr_c102_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C102_REG_OFFSET), \
        [power_manager_core_csr_c103_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C103_REG_OFFSET), \
        [power_manager_core_csr_c104_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C104_REG_OFFSET), \
        [power_manager_core_csr_c105_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C105_REG_OFFSET), \
        [power_manager_core_csr_c106_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C106_REG_OFFSET), \
        [power_manager_core_csr_c107_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C107_REG_OFFSET), \
        [power_manager_core_csr_c108_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C108_REG_OFFSET), \
        [power_manager_core_csr_c109_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C109_REG_OFFSET), \
        [power_manager_core_csr_c110_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C110_REG_OFFSET), \
        [power_manager_core_csr_c111_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C111_REG_OFFSET) \
    );

    asm volatile (

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        "lw a1, %[power_manager_core_csr_c112_reg_offset](a0)\n"
        "csrw mhpmcounter9h, a1\n"
        "lw a1, %[power_manager_core_csr_c113_reg_offset](a0)\n"
        "csrw mhpmcounter10h, a1\n"
        "lw a1, %[power_manager_core_csr_c114_reg_offset](a0)\n"
        "csrw mhpmcounter11h, a1\n"
        "lw a1, %[power_manager_core_csr_c115_reg_offset](a0)\n"
        "csrw mhpmcounter12h, a1\n"
        "lw a1, %[power_manager_core_csr_c116_reg_offset](a0)\n"
        "csrw mhpmcounter13h, a1\n"
        "lw a1, %[power_manager_core_csr_c117_reg_offset](a0)\n"
        "csrw mhpmcounter14h, a1\n"
        "lw a1, %[power_manager_core_csr_c118_reg_offset](a0)\n"
        "csrw mhpmcounter15h, a1\n"
        "lw a1, %[power_manager_core_csr_c119_reg_offset](a0)\n"
        "csrw mhpmcounter16h, a1\n"
        "lw a1, %[power_manager_core_csr_c120_reg_offset](a0)\n"
        "csrw mhpmcounter17h, a1\n"
        "lw a1, %[power_manager_core_csr_c121_reg_offset](a0)\n"
        "csrw mhpmcounter18h, a1\n"
        "lw a1, %[power_manager_core_csr_c122_reg_offset](a0)\n"
        "csrw mhpmcounter19h, a1\n"
        "lw a1, %[power_manager_core_csr_c123_reg_offset](a0)\n"
        "csrw mhpmcounter20h, a1\n"
        "lw a1, %[power_manager_core_csr_c124_reg_offset](a0)\n"
        "csrw mhpmcounter21h, a1\n"
        "lw a1, %[power_manager_core_csr_c125_reg_offset](a0)\n"
        "csrw mhpmcounter22h, a1\n"
        "lw a1, %[power_manager_core_csr_c126_reg_offset](a0)\n"
        "csrw mhpmcounter23h, a1\n"
        "lw a1, %[power_manager_core_csr_c127_reg_offset](a0)\n"
        "csrw mhpmcounter24h, a1\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_csr_c112_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C112_REG_OFFSET), \
        [power_manager_core_csr_c113_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C113_REG_OFFSET), \
        [power_manager_core_csr_c114_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C114_REG_OFFSET), \
        [power_manager_core_csr_c115_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C115_REG_OFFSET), \
        [power_manager_core_csr_c116_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C116_REG_OFFSET), \
        [power_manager_core_csr_c117_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C117_REG_OFFSET), \
        [power_manager_core_csr_c118_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C118_REG_OFFSET), \
        [power_manager_core_csr_c119_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C119_REG_OFFSET), \
        [power_manager_core_csr_c120_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C120_REG_OFFSET), \
        [power_manager_core_csr_c121_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C121_REG_OFFSET), \
        [power_manager_core_csr_c122_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C122_REG_OFFSET), \
        [power_manager_core_csr_c123_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C123_REG_OFFSET), \
        [power_manager_core_csr_c124_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C124_REG_OFFSET), \
        [power_manager_core_csr_c125_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C125_REG_OFFSET), \
        [power_manager_core_csr_c126_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C126_REG_OFFSET), \
        [power_manager_core_csr_c127_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C127_REG_OFFSET) \
    );

    asm volatile (

        // write CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        "lw a1, %[power_manager_core_csr_c128_reg_offset](a0)\n"
        "csrw mhpmcounter25h, a1\n"
        "lw a1, %[power_manager_core_csr_c129_reg_offset](a0)\n"
        "csrw mhpmcounter26h, a1\n"
        "lw a1, %[power_manager_core_csr_c130_reg_offset](a0)\n"
        "csrw mhpmcounter27h, a1\n"
        "lw a1, %[power_manager_core_csr_c131_reg_offset](a0)\n"
        "csrw mhpmcounter28h, a1\n"
        "lw a1, %[power_manager_core_csr_c132_reg_offset](a0)\n"
        "csrw mhpmcounter29h, a1\n"
        "lw a1, %[power_manager_core_csr_c133_reg_offset](a0)\n"
        "csrw mhpmcounter30h, a1\n"
        "lw a1, %[power_manager_core_csr_c134_reg_offset](a0)\n"
        "csrw mhpmcounter31h, a1\n" : : \
        // "lw a1, %[power_manager_core_csr_c135_reg_offset](a0)\n"
        // "csrw mvendorid, a1\n"
        // "lw a1, %[power_manager_core_csr_c136_reg_offset](a0)\n"
        // "csrw marchid, a1\n"
        // "lw a1, %[power_manager_core_csr_c137_reg_offset](a0)\n"
        // "csrw mimpid, a1\n"
        // "lw a1, %[power_manager_core_csr_c138_reg_offset](a0)\n"
        // "csrw mhartid, a1\n"
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_csr_c128_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C128_REG_OFFSET), \
        [power_manager_core_csr_c129_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C129_REG_OFFSET), \
        [power_manager_core_csr_c130_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C130_REG_OFFSET), \
        [power_manager_core_csr_c131_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C131_REG_OFFSET), \
        [power_manager_core_csr_c132_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C132_REG_OFFSET), \
        [power_manager_core_csr_c133_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C133_REG_OFFSET), \
        [power_manager_core_csr_c134_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C134_REG_OFFSET) \
        // [power_manager_core_csr_c135_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C135_REG_OFFSET),
        // [power_manager_core_csr_c136_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C136_REG_OFFSET),
        // [power_manager_core_csr_c137_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C137_REG_OFFSET),
        // [power_manager_core_csr_c138_reg_offset] "i" (POWER_MANAGER_CORE_CSR_C138_REG_OFFSET)
    );

    return;
}

power_manager_result_t __attribute__ ((noinline)) power_gate_core(const power_manager_t *power_manager, power_manager_sel_intr_t sel_intr, power_manager_counters_t* cpu_counter)
{
    uint32_t reg = 0;

    // set counters
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_CPU_RESET_ASSERT_COUNTER_REG_OFFSET), cpu_counter->reset_off);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_CPU_RESET_DEASSERT_COUNTER_REG_OFFSET), cpu_counter->reset_on);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_CPU_SWITCH_OFF_COUNTER_REG_OFFSET), cpu_counter->powergate_off);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_CPU_SWITCH_ON_COUNTER_REG_OFFSET), cpu_counter->powergate_on);

    // enable wakeup timers
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_EN_WAIT_FOR_INTR_REG_OFFSET), 1 << sel_intr);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_INTR_STATE_REG_OFFSET), 0x0);

    power_gate_core_asm();

    // clean up states
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_EN_WAIT_FOR_INTR_REG_OFFSET), 0x0);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_INTR_STATE_REG_OFFSET), 0x0);

    // stop counters
    reg = bitfield_bit32_write(reg, POWER_MANAGER_CPU_COUNTERS_STOP_CPU_RESET_ASSERT_STOP_BIT_COUNTER_BIT, true);
    reg = bitfield_bit32_write(reg, POWER_MANAGER_CPU_COUNTERS_STOP_CPU_RESET_DEASSERT_STOP_BIT_COUNTER_BIT, true);
    reg = bitfield_bit32_write(reg, POWER_MANAGER_CPU_COUNTERS_STOP_CPU_SWITCH_OFF_STOP_BIT_COUNTER_BIT, true);
    reg = bitfield_bit32_write(reg, POWER_MANAGER_CPU_COUNTERS_STOP_CPU_SWITCH_ON_STOP_BIT_COUNTER_BIT, true);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_CPU_COUNTERS_STOP_REG_OFFSET), reg);

    return kPowerManagerOk_e;
}

power_manager_result_t power_gate_domain(const power_manager_t *power_manager, power_manager_sel_domain_t sel_domain, power_manager_sel_state_t sel_state, power_manager_counters_t* domain_counters)
{
    uint32_t reg = 0;

    if (sel_domain == kPeriph_e)
    {
        // set counters
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_RESET_ASSERT_COUNTER_REG_OFFSET), domain_counters->reset_off);
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_RESET_DEASSERT_COUNTER_REG_OFFSET), domain_counters->reset_on);
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_SWITCH_OFF_COUNTER_REG_OFFSET), domain_counters->powergate_off);
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_SWITCH_ON_COUNTER_REG_OFFSET), domain_counters->powergate_on);

        if (sel_state == kOn_e)
            mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_POWER_GATE_PERIPH_REG_OFFSET), 0x0);
        else
            mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_POWER_GATE_PERIPH_REG_OFFSET), 0x1);

        // stop counters
        reg = bitfield_bit32_write(reg, POWER_MANAGER_PERIPH_COUNTERS_STOP_PERIPH_RESET_ASSERT_STOP_BIT_COUNTER_BIT, true);
        reg = bitfield_bit32_write(reg, POWER_MANAGER_PERIPH_COUNTERS_STOP_PERIPH_RESET_DEASSERT_STOP_BIT_COUNTER_BIT, true);
        reg = bitfield_bit32_write(reg, POWER_MANAGER_PERIPH_COUNTERS_STOP_PERIPH_SWITCH_OFF_STOP_BIT_COUNTER_BIT, true);
        reg = bitfield_bit32_write(reg, POWER_MANAGER_PERIPH_COUNTERS_STOP_PERIPH_SWITCH_ON_STOP_BIT_COUNTER_BIT, true);
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_COUNTERS_STOP_REG_OFFSET), reg);
    }
    else
    {
        // set counters
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_RAM_0_RESET_ASSERT_COUNTER_REG_OFFSET + (0x14 * (sel_domain - 1))), domain_counters->reset_off);
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_RAM_0_RESET_DEASSERT_COUNTER_REG_OFFSET + (0x14 * (sel_domain - 1))), domain_counters->reset_on);
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_RAM_0_SWITCH_OFF_COUNTER_REG_OFFSET + (0x14 * (sel_domain - 1))), domain_counters->powergate_off);
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_RAM_0_SWITCH_ON_COUNTER_REG_OFFSET + (0x14 * (sel_domain - 1))), domain_counters->powergate_on);

        if (sel_state == kOn_e)
            mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_POWER_GATE_RAM_BLOCK_0_REG_OFFSET + (0x14 * (sel_domain - 1))), 0x0);
        else
            mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_POWER_GATE_RAM_BLOCK_0_REG_OFFSET + (0x14 * (sel_domain - 1))), 0x1);

        // stop counters
        reg = bitfield_bit32_write(reg, POWER_MANAGER_RAM_0_COUNTERS_STOP_RAM_0_RESET_ASSERT_STOP_BIT_COUNTER_BIT, true);
        reg = bitfield_bit32_write(reg, POWER_MANAGER_RAM_0_COUNTERS_STOP_RAM_0_RESET_DEASSERT_STOP_BIT_COUNTER_BIT, true);
        reg = bitfield_bit32_write(reg, POWER_MANAGER_RAM_0_COUNTERS_STOP_RAM_0_SWITCH_OFF_STOP_BIT_COUNTER_BIT, true);
        reg = bitfield_bit32_write(reg, POWER_MANAGER_RAM_0_COUNTERS_STOP_RAM_0_SWITCH_ON_STOP_BIT_COUNTER_BIT, true);
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_RAM_0_COUNTERS_STOP_REG_OFFSET + (0x14 * (sel_domain - 1))), reg);
    }

    return kPowerManagerOk_e;
}

power_manager_result_t power_gate_counters_init(power_manager_counters_t* counters, uint32_t reset_off, uint32_t reset_on, uint32_t powergate_off, uint32_t powergate_on)
{
    // the reset_on must be greater than powergate_on (i.e. first turn on, then you deassert the reset)
    // the reset_off must be greater than powergate_off (i.e. first turn off, then you reset)

    if(reset_on  <= powergate_on) return kPowerManagerError_e;
    if(reset_off <= powergate_off) return kPowerManagerError_e;

    counters->reset_off     = reset_off;
    counters->reset_on      = reset_on;
    counters->powergate_off = powergate_off;
    counters->powergate_on  = powergate_on;

    return kPowerManagerOk_e;
}

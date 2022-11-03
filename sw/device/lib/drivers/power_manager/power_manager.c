// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include "power_manager.h"

#include <stddef.h>
#include <stdint.h>

#include "mmio.h"

#include "core_v_mini_mcu.h"

#include "power_manager_regs.h"  // Generated.

#include "x-heep.h"

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
        "sw a1, %[power_manager_core_csr_mstatus_reg_offset](a0)\n"
        "csrr a1, mie\n"
        "sw a1, %[power_manager_core_csr_mie_reg_offset](a0)\n"
        "csrr a1, mtvec\n"
        "sw a1, %[power_manager_core_csr_mtvec_reg_offset](a0)\n"
        "csrr a1, mscratch\n"
        "sw a1, %[power_manager_core_csr_mscratch_reg_offset](a0)\n"
        "csrr a1, mepc\n"
        "sw a1, %[power_manager_core_csr_mepc_reg_offset](a0)\n"
        "csrr a1, mcause\n"
        "sw a1, %[power_manager_core_csr_mcause_reg_offset](a0)\n"
        "csrr a1, mtval\n"
        "sw a1, %[power_manager_core_csr_mtval_reg_offset](a0)\n"
        "csrr a1, mcycle\n"
        "sw a1, %[power_manager_core_csr_mcycle_reg_offset](a0)\n"
        "csrr a1, minstret\n"
        "sw a1, %[power_manager_core_csr_minstret_reg_offset](a0)\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_csr_mstatus_reg_offset] "i" (POWER_MANAGER_CORE_CSR_MSTATUS_REG_OFFSET), \
        [power_manager_core_csr_mie_reg_offset] "i" (POWER_MANAGER_CORE_CSR_MIE_REG_OFFSET), \
        [power_manager_core_csr_mtvec_reg_offset] "i" (POWER_MANAGER_CORE_CSR_MTVEC_REG_OFFSET), \
        [power_manager_core_csr_mscratch_reg_offset] "i" (POWER_MANAGER_CORE_CSR_MSCRATCH_REG_OFFSET), \
        [power_manager_core_csr_mepc_reg_offset] "i" (POWER_MANAGER_CORE_CSR_MEPC_REG_OFFSET), \
        [power_manager_core_csr_mcause_reg_offset] "i" (POWER_MANAGER_CORE_CSR_MCAUSE_REG_OFFSET), \
        [power_manager_core_csr_mtval_reg_offset] "i" (POWER_MANAGER_CORE_CSR_MTVAL_REG_OFFSET), \
        [power_manager_core_csr_mcycle_reg_offset] "i" (POWER_MANAGER_CORE_CSR_MCYCLE_REG_OFFSET), \
        [power_manager_core_csr_minstret_reg_offset] "i" (POWER_MANAGER_CORE_CSR_MINSTRET_REG_OFFSET) \
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

        // read CORE_REG_Xn[31:0] = Xn
        "lui a0, %[base_address_20bit]\n"
        "lw a1, %[power_manager_core_csr_mstatus_reg_offset](a0)\n"
        "csrw mstatus, a1\n"
        "lw a1, %[power_manager_core_csr_mie_reg_offset](a0)\n"
        "csrw mie, a1\n"
        "lw a1, %[power_manager_core_csr_mtvec_reg_offset](a0)\n"
        "csrw mtvec, a1\n"
        "lw a1, %[power_manager_core_csr_mscratch_reg_offset](a0)\n"
        "csrw mscratch, a1\n"
        "lw a1, %[power_manager_core_csr_mepc_reg_offset](a0)\n"
        "csrw mepc, a1\n"
        "lw a1, %[power_manager_core_csr_mcause_reg_offset](a0)\n"
        "csrw mcause, a1\n"
        "lw a1, %[power_manager_core_csr_mtval_reg_offset](a0)\n"
        "csrw mtval, a1\n"
        "lw a1, %[power_manager_core_csr_mcycle_reg_offset](a0)\n"
        "csrw mcycle, a1\n"
        "lw a1, %[power_manager_core_csr_minstret_reg_offset](a0)\n"
        "csrw minstret, a1\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_core_csr_mstatus_reg_offset] "i" (POWER_MANAGER_CORE_CSR_MSTATUS_REG_OFFSET), \
        [power_manager_core_csr_mie_reg_offset] "i" (POWER_MANAGER_CORE_CSR_MIE_REG_OFFSET), \
        [power_manager_core_csr_mtvec_reg_offset] "i" (POWER_MANAGER_CORE_CSR_MTVEC_REG_OFFSET), \
        [power_manager_core_csr_mscratch_reg_offset] "i" (POWER_MANAGER_CORE_CSR_MSCRATCH_REG_OFFSET), \
        [power_manager_core_csr_mepc_reg_offset] "i" (POWER_MANAGER_CORE_CSR_MEPC_REG_OFFSET), \
        [power_manager_core_csr_mcause_reg_offset] "i" (POWER_MANAGER_CORE_CSR_MCAUSE_REG_OFFSET), \
        [power_manager_core_csr_mtval_reg_offset] "i" (POWER_MANAGER_CORE_CSR_MTVAL_REG_OFFSET), \
        [power_manager_core_csr_mcycle_reg_offset] "i" (POWER_MANAGER_CORE_CSR_MCYCLE_REG_OFFSET), \
        [power_manager_core_csr_minstret_reg_offset] "i" (POWER_MANAGER_CORE_CSR_MINSTRET_REG_OFFSET) \
    );

    return;
}

power_manager_result_t __attribute__ ((noinline)) power_gate_core(const power_manager_t *power_manager, power_manager_sel_intr_t sel_intr, power_manager_counters_t* cpu_counter)
{
    uint32_t reg = 0;

    // set counters
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_CPU_RESET_ASSERT_COUNTER_REG_OFFSET), cpu_counter->reset_off);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_CPU_RESET_DEASSERT_COUNTER_REG_OFFSET), cpu_counter->reset_on);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_CPU_SWITCH_OFF_COUNTER_REG_OFFSET), cpu_counter->switch_off);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_CPU_SWITCH_ON_COUNTER_REG_OFFSET), cpu_counter->switch_on);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_CPU_ISO_OFF_COUNTER_REG_OFFSET), cpu_counter->iso_off);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_CPU_ISO_ON_COUNTER_REG_OFFSET), cpu_counter->iso_on);

    // enable wakeup timers
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_EN_WAIT_FOR_INTR_REG_OFFSET), 1 << sel_intr);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_INTR_STATE_REG_OFFSET), 0x0);

    // enable wait for SWITCH ACK
    reg = 0;
    #ifndef TARGET_PYNQ_Z2
        reg = bitfield_bit32_write(reg, POWER_MANAGER_CPU_WAIT_ACK_SWITCH_ON_COUNTER_CPU_WAIT_ACK_SWITCH_ON_COUNTER_BIT, true);
    #else
        reg = bitfield_bit32_write(reg, POWER_MANAGER_CPU_WAIT_ACK_SWITCH_ON_COUNTER_CPU_WAIT_ACK_SWITCH_ON_COUNTER_BIT, false);
    #endif
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_CPU_WAIT_ACK_SWITCH_ON_COUNTER_REG_OFFSET), reg);

    power_gate_core_asm();

    // clean up states
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_EN_WAIT_FOR_INTR_REG_OFFSET), 0x0);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_INTR_STATE_REG_OFFSET), 0x0);

    // stop counters
    reg = 0;
    reg = bitfield_bit32_write(reg, POWER_MANAGER_CPU_COUNTERS_STOP_CPU_RESET_ASSERT_STOP_BIT_COUNTER_BIT, true);
    reg = bitfield_bit32_write(reg, POWER_MANAGER_CPU_COUNTERS_STOP_CPU_RESET_DEASSERT_STOP_BIT_COUNTER_BIT, true);
    reg = bitfield_bit32_write(reg, POWER_MANAGER_CPU_COUNTERS_STOP_CPU_SWITCH_OFF_STOP_BIT_COUNTER_BIT, true);
    reg = bitfield_bit32_write(reg, POWER_MANAGER_CPU_COUNTERS_STOP_CPU_SWITCH_ON_STOP_BIT_COUNTER_BIT, true);
    reg = bitfield_bit32_write(reg, POWER_MANAGER_CPU_COUNTERS_STOP_CPU_ISO_OFF_STOP_BIT_COUNTER_BIT, true);
    reg = bitfield_bit32_write(reg, POWER_MANAGER_CPU_COUNTERS_STOP_CPU_ISO_ON_STOP_BIT_COUNTER_BIT, true);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_CPU_COUNTERS_STOP_REG_OFFSET), reg);

    return kPowerManagerOk_e;
}

power_manager_result_t __attribute__ ((noinline)) power_gate_periph(const power_manager_t *power_manager, power_manager_sel_state_t sel_state, power_manager_counters_t* periph_counters)
{
    uint32_t reg = 0;

    // set counters
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_RESET_ASSERT_COUNTER_REG_OFFSET), periph_counters->reset_off);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_RESET_DEASSERT_COUNTER_REG_OFFSET), periph_counters->reset_on);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_SWITCH_OFF_COUNTER_REG_OFFSET), periph_counters->switch_off);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_SWITCH_ON_COUNTER_REG_OFFSET), periph_counters->switch_on);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_ISO_OFF_COUNTER_REG_OFFSET), periph_counters->iso_off);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_ISO_ON_COUNTER_REG_OFFSET), periph_counters->iso_on);

    // enable wait for SWITCH ACK
    reg = 0;
    #ifndef TARGET_PYNQ_Z2
        reg = bitfield_bit32_write(reg, POWER_MANAGER_PERIPH_WAIT_ACK_SWITCH_ON_COUNTER_PERIPH_WAIT_ACK_SWITCH_ON_COUNTER_BIT, true);
    #else
        reg = bitfield_bit32_write(reg, POWER_MANAGER_PERIPH_WAIT_ACK_SWITCH_ON_COUNTER_PERIPH_WAIT_ACK_SWITCH_ON_COUNTER_BIT, false);
    #endif
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_WAIT_ACK_SWITCH_ON_COUNTER_REG_OFFSET), reg);

    if (sel_state == kOn_e)
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_POWER_GATE_PERIPH_REG_OFFSET), 0x0);
    else
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_POWER_GATE_PERIPH_REG_OFFSET), 0x1);

    // stop counters
    reg = 0;
    reg = bitfield_bit32_write(reg, POWER_MANAGER_PERIPH_COUNTERS_STOP_PERIPH_RESET_ASSERT_STOP_BIT_COUNTER_BIT, true);
    reg = bitfield_bit32_write(reg, POWER_MANAGER_PERIPH_COUNTERS_STOP_PERIPH_RESET_DEASSERT_STOP_BIT_COUNTER_BIT, true);
    reg = bitfield_bit32_write(reg, POWER_MANAGER_PERIPH_COUNTERS_STOP_PERIPH_SWITCH_OFF_STOP_BIT_COUNTER_BIT, true);
    reg = bitfield_bit32_write(reg, POWER_MANAGER_PERIPH_COUNTERS_STOP_PERIPH_SWITCH_ON_STOP_BIT_COUNTER_BIT, true);
    reg = bitfield_bit32_write(reg, POWER_MANAGER_PERIPH_COUNTERS_STOP_PERIPH_ISO_OFF_STOP_BIT_COUNTER_BIT, true);
    reg = bitfield_bit32_write(reg, POWER_MANAGER_PERIPH_COUNTERS_STOP_PERIPH_ISO_ON_STOP_BIT_COUNTER_BIT, true);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_COUNTERS_STOP_REG_OFFSET), reg);

    return kPowerManagerOk_e;
}

power_manager_result_t __attribute__ ((noinline)) power_gate_ram_block(const power_manager_t *power_manager, uint32_t sel_block, power_manager_sel_state_t sel_state, power_manager_counters_t* ram_block_counters)
{
    uint32_t reg = 0;

    // set counters
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].switch_off_counter), ram_block_counters->switch_off);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].switch_on_counter), ram_block_counters->switch_on);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].iso_off_counter), ram_block_counters->iso_off);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].iso_on_counter), ram_block_counters->iso_on);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].retentive_off_counter), ram_block_counters->retentive_off);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].retentive_on_counter), ram_block_counters->retentive_on);

    // enable wait for SWITCH ACK
    reg = 0;

    if (sel_state == kOn_e)
    {
        #ifndef TARGET_PYNQ_Z2
            reg = bitfield_bit32_write(reg, (power_manager_ram_map[sel_block].wait_ack_switch_on_counter_bit), true);
        #else
            reg = bitfield_bit32_write(reg, (power_manager_ram_map[sel_block].wait_ack_switch_on_counter_bit), false);
        #endif
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].wait_ack_switch_on_counter), reg);
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].power_gate), 0x0);
    }
    else if (sel_state == kOff_e)
    {
        #ifndef TARGET_PYNQ_Z2
            reg = bitfield_bit32_write(reg, (power_manager_ram_map[sel_block].wait_ack_switch_on_counter_bit), true);
        #else
            reg = bitfield_bit32_write(reg, (power_manager_ram_map[sel_block].wait_ack_switch_on_counter_bit), false);
        #endif
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].wait_ack_switch_on_counter), reg);
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].power_gate), 0x1);
    }
    else if (sel_state == kRetOn_e)
    {
        reg = bitfield_bit32_write(reg, (power_manager_ram_map[sel_block].wait_ack_switch_on_counter_bit), false);
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].wait_ack_switch_on_counter), reg);
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].set_retentive), 0x0);
    }
    else
    {
        reg = bitfield_bit32_write(reg, (power_manager_ram_map[sel_block].wait_ack_switch_on_counter_bit), false);
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].wait_ack_switch_on_counter), reg);
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].set_retentive), 0x1);
    }

    // stop counters
    reg = bitfield_bit32_write(reg, (power_manager_ram_map[sel_block].switch_off_stop_bit), true);
    reg = bitfield_bit32_write(reg, (power_manager_ram_map[sel_block].switch_on_stop_bit), true);
    reg = bitfield_bit32_write(reg, (power_manager_ram_map[sel_block].iso_off_stop_bit), true);
    reg = bitfield_bit32_write(reg, (power_manager_ram_map[sel_block].iso_on_stop_bit), true);
    reg = bitfield_bit32_write(reg, (power_manager_ram_map[sel_block].retentive_off_stop_bit), true);
    reg = bitfield_bit32_write(reg, (power_manager_ram_map[sel_block].retentive_on_stop_bit), true);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].counter_stop), reg);

    return kPowerManagerOk_e;
}

power_manager_result_t __attribute__ ((noinline)) power_gate_external(const power_manager_t *power_manager, uint32_t sel_external, power_manager_sel_state_t sel_state, power_manager_counters_t* external_counters)
{
    uint32_t reg = 0;

    // set counters
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].reset_assert_counter), external_counters->reset_off);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].reset_deassert_counter), external_counters->reset_on);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].switch_off_counter), external_counters->switch_off);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].switch_on_counter), external_counters->switch_on);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].iso_off_counter), external_counters->iso_off);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].iso_on_counter), external_counters->iso_on);

    // enable wait for SWITCH ACK
    reg = 0;
    #ifndef TARGET_PYNQ_Z2
        reg = bitfield_bit32_write(reg, (power_manager_external_map[sel_external].wait_ack_switch_on_counter_bit), true);
    #else
        reg = bitfield_bit32_write(reg, (power_manager_external_map[sel_external].wait_ack_switch_on_counter_bit), false);
    #endif
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].wait_ack_switch_on_counter), reg);

    if (sel_state == kOn_e)
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].power_gate), 0x0);
    else
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].power_gate), 0x1);

    // stop counters
    reg = bitfield_bit32_write(reg, (power_manager_external_map[sel_external].reset_assert_stop_bit), true);
    reg = bitfield_bit32_write(reg, (power_manager_external_map[sel_external].reset_deassert_stop_bit), true);
    reg = bitfield_bit32_write(reg, (power_manager_external_map[sel_external].switch_off_stop_bit), true);
    reg = bitfield_bit32_write(reg, (power_manager_external_map[sel_external].switch_on_stop_bit), true);
    reg = bitfield_bit32_write(reg, (power_manager_external_map[sel_external].iso_off_stop_bit), true);
    reg = bitfield_bit32_write(reg, (power_manager_external_map[sel_external].iso_on_stop_bit), true);
    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].counter_stop), reg);

    return kPowerManagerOk_e;
}

uint32_t periph_power_domain_is_off(const power_manager_t *power_manager)
{
    uint32_t switch_state;

    switch_state = mmio_region_read32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_POWER_GATE_PERIPH_ACK_REG_OFFSET));

    return switch_state == 0;
}

uint32_t ram_block_power_domain_is_off(const power_manager_t *power_manager, uint32_t sel_block)
{
    uint32_t switch_state;

    switch_state = mmio_region_read32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].power_gate_ack));

    return switch_state == 0;
}

uint32_t external_power_domain_is_off(const power_manager_t *power_manager, uint32_t sel_external)
{
    uint32_t switch_state;

    switch_state = mmio_region_read32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].power_gate_ack));

    return switch_state == 0;
}

power_manager_result_t power_gate_counters_init(power_manager_counters_t* counters, uint32_t reset_off, uint32_t reset_on, uint32_t switch_off, uint32_t switch_on, uint32_t iso_off, uint32_t iso_on, uint32_t retentive_off, uint32_t retentive_on)
{
    counters->reset_off     = reset_off;
    counters->reset_on      = reset_on;
    counters->switch_off    = switch_off;
    counters->switch_on     = switch_on;
    counters->iso_off       = iso_off;
    counters->iso_on        = iso_on;
    counters->retentive_off = retentive_off;
    counters->retentive_on  = retentive_on;

    return kPowerManagerOk_e;
}

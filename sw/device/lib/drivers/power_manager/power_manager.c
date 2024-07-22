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

extern void power_manager_cpu_store();

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
    reg = bitfield_bit32_write(reg, POWER_MANAGER_CPU_WAIT_ACK_SWITCH_ON_COUNTER_CPU_WAIT_ACK_SWITCH_ON_COUNTER_BIT, 0x1);

    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_CPU_WAIT_ACK_SWITCH_ON_COUNTER_REG_OFFSET), reg);

    power_manager_cpu_store();

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

    mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_WAIT_ACK_SWITCH_ON_REG_OFFSET), 0x1);

    if (sel_state == kOn_e)
    {
        for (int i=0; i<periph_counters->switch_on; i++) asm volatile ("nop\n;");
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_SWITCH_REG_OFFSET), 0x0);
        for (int i=0; i<periph_counters->iso_off; i++) asm volatile ("nop\n;");
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_ISO_REG_OFFSET), 0x0);
        for (int i=0; i<periph_counters->reset_off; i++) asm volatile ("nop\n;");
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_RESET_REG_OFFSET), 0x0);
    }
    else
    {
        for (int i=0; i<periph_counters->iso_on; i++) asm volatile ("nop\n;");
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_ISO_REG_OFFSET), 0x1);
        for (int i=0; i<periph_counters->switch_off; i++) asm volatile ("nop\n;");
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_SWITCH_REG_OFFSET), 0x1);
        for (int i=0; i<periph_counters->reset_on; i++) asm volatile ("nop\n;");
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(POWER_MANAGER_PERIPH_RESET_REG_OFFSET), 0x1);
    }

    return kPowerManagerOk_e;
}

power_manager_result_t __attribute__ ((noinline)) power_gate_ram_block(const power_manager_t *power_manager, uint32_t sel_block, power_manager_sel_state_t sel_state, power_manager_counters_t* ram_block_counters)
{
    uint32_t reg = 0;

    if (sel_state == kOn_e)
    {
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].wait_ack_switch), 0x1);
        for (int i=0; i<ram_block_counters->switch_on; i++) asm volatile ("nop\n;");
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].switch_off), 0x0);
        for (int i=0; i<ram_block_counters->iso_off; i++) asm volatile ("nop\n;");
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].iso), 0x0);
    }
    else if (sel_state == kOff_e)
    {
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].wait_ack_switch), 0x1);
        for (int i=0; i<ram_block_counters->iso_on; i++) asm volatile ("nop\n;");
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].iso), 0x1);
        for (int i=0; i<ram_block_counters->switch_off; i++) asm volatile ("nop\n;");
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].switch_off), 0x1);
    }
    else if (sel_state == kRetOn_e)
    {
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].wait_ack_switch), 0x0);
        for (int i=0; i<ram_block_counters->retentive_on; i++) asm volatile ("nop\n;");
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].retentive), 0x1);
    }
    else
    {
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].wait_ack_switch), 0x0);
        for (int i=0; i<ram_block_counters->retentive_off; i++) asm volatile ("nop\n;");
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].retentive), 0x0);
    }

    return kPowerManagerOk_e;
}

power_manager_result_t __attribute__ ((noinline)) power_gate_external(const power_manager_t *power_manager, uint32_t sel_external, power_manager_sel_state_t sel_state, power_manager_counters_t* external_counters)
{
    uint32_t reg = 0;

    if (sel_state == kOn_e)
    {
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].wait_ack_switch), 0x1);
        for (int i=0; i<external_counters->switch_on; i++) asm volatile ("nop\n;");
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].switch_off), 0x0);
        for (int i=0; i<external_counters->iso_off; i++) asm volatile ("nop\n;");
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].iso), 0x0);
        for (int i=0; i<external_counters->reset_off; i++) asm volatile ("nop\n;");
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].reset), 0x0);
    }
    else if (sel_state == kOff_e)
    {
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].wait_ack_switch), 0x1);
        for (int i=0; i<external_counters->iso_on; i++) asm volatile ("nop\n;");
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].iso), 0x1);
        for (int i=0; i<external_counters->switch_off; i++) asm volatile ("nop\n;");
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].switch_off), 0x1);
        for (int i=0; i<external_counters->reset_on; i++) asm volatile ("nop\n;");
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].reset), 0x1);
    }
    else if (sel_state == kRetOn_e)
    {
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].wait_ack_switch), 0x0);
        for (int i=0; i<external_counters->retentive_on; i++) asm volatile ("nop\n;");
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].retentive), 0x1);
    }
    else
    {
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].wait_ack_switch), 0x0);
        for (int i=0; i<external_counters->retentive_off; i++) asm volatile ("nop\n;");
        mmio_region_write32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].retentive), 0x0);
    }

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

monitor_signals_t monitor_power_gate_core(const power_manager_t *power_manager)
{
    uint32_t reg = 0;
    monitor_signals_t monitor_signals;

    reg = mmio_region_read32(power_manager->base_addr, (ptrdiff_t)POWER_MANAGER_MONITOR_POWER_GATE_CORE_REG_OFFSET);

    monitor_signals.kSwitch_e = reg & 0x1;
    monitor_signals.kIso_e    = (reg & 0x2) >> 1;
    monitor_signals.kReset_e  = (reg & 0x4) >> 2;

    return monitor_signals;
}

monitor_signals_t monitor_power_gate_periph(const power_manager_t *power_manager)
{
    uint32_t reg = 0;
    monitor_signals_t monitor_signals;

    reg = mmio_region_read32(power_manager->base_addr, (ptrdiff_t)POWER_MANAGER_MONITOR_POWER_GATE_PERIPH_REG_OFFSET);

    monitor_signals.kSwitch_e = reg & 0x1;
    monitor_signals.kIso_e    = (reg & 0x2) >> 1;
    monitor_signals.kReset_e  = (reg & 0x4) >> 2;

    return monitor_signals;
}

monitor_signals_t monitor_power_gate_ram_block(const power_manager_t *power_manager, uint32_t sel_block)
{
    uint32_t reg = 0;
    monitor_signals_t monitor_signals;

    reg = mmio_region_read32(power_manager->base_addr, (ptrdiff_t)(power_manager_ram_map[sel_block].monitor_power_gate));

    monitor_signals.kSwitch_e = reg & 0x1;
    monitor_signals.kIso_e    = (reg & 0x2) >> 1;
    monitor_signals.kReset_e  = 0x1;

    return monitor_signals;
}

monitor_signals_t monitor_power_gate_external(const power_manager_t *power_manager, uint32_t sel_external)
{
    uint32_t reg = 0;
    monitor_signals_t monitor_signals;

    reg = mmio_region_read32(power_manager->base_addr, (ptrdiff_t)(power_manager_external_map[sel_external].monitor_power_gate));

    monitor_signals.kSwitch_e = reg & 0x1;
    monitor_signals.kIso_e    = (reg & 0x2) >> 1;
    monitor_signals.kReset_e  = (reg & 0x4) >> 2;

    return monitor_signals;
}

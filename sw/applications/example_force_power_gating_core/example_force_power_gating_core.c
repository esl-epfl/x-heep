// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include "csr.h"
#include "hart.h"
#include "handler.h"
#include "core_v_mini_mcu.h"
#include "rv_timer.h"
#include "power_manager.h"
#include "soc_ctrl.h"

static rv_timer_t timer_0_1;
static const uint64_t kTickFreqHz = 1000 * 1000; // 1 MHz
static power_manager_t power_manager;

int main(int argc, char *argv[])
{
    // Setup power_manager
    mmio_region_t power_manager_reg = mmio_region_from_addr(POWER_MANAGER_START_ADDRESS);
    power_manager.base_addr = power_manager_reg;
    power_manager_counters_t power_manager_cpu_counters;

    // Get current Frequency
    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);
    uint32_t freq_hz = soc_ctrl_get_frequency(&soc_ctrl);

    // Setup rv_timer_0
    mmio_region_t timer_0_1_reg = mmio_region_from_addr(RV_TIMER_AO_START_ADDRESS);
    rv_timer_init(timer_0_1_reg, (rv_timer_config_t){.hart_count = 1, .comparator_count = 1}, &timer_0_1);
    rv_timer_tick_params_t tick_params;
    rv_timer_approximate_tick_params(freq_hz, kTickFreqHz, &tick_params);

    // Read monitor registers
    monitor_signals_t monitor_signals;

    monitor_signals = monitor_power_gate_core(&power_manager);
    printf("Core Switch: %d, Core Iso: %d, Core Reset: %d\n", monitor_signals.kSwitch_e, monitor_signals.kIso_e, monitor_signals.kReset_e);

    monitor_signals = monitor_power_gate_periph(&power_manager);
    printf("Periph Switch: %d, Periph Iso: %d, Periph Reset: %d\n", monitor_signals.kSwitch_e, monitor_signals.kIso_e, monitor_signals.kReset_e);

    monitor_signals = monitor_power_gate_ram_block(&power_manager, 2);
    printf("Ram Block Switch: %d, Ram Block Iso: %d\n", monitor_signals.kSwitch_e, monitor_signals.kIso_e);

    monitor_signals = monitor_power_gate_external(&power_manager, 0);
    printf("External Switch: %d, External Iso: %d, External Reset: %d\n", monitor_signals.kSwitch_e, monitor_signals.kIso_e, monitor_signals.kReset_e);

    // Force core wakeup
    if (cpu_force_wakeup(&power_manager) != kPowerManagerOk_e)
    {
        printf("Error: power manager fail.\n");
        return EXIT_FAILURE;
    }

    // Init cpu_subsystem's counters
    if (power_gate_counters_init(&power_manager_cpu_counters, 40, 40, 30, 30, 20, 20, 0, 0) != kPowerManagerOk_e)
    {
        printf("Error: power manager fail. Check the reset and powergate counters value\n");
        return EXIT_FAILURE;
    }

    // Power-gate and wake-up due to timer_0
    rv_timer_set_tick_params(&timer_0_1, 0, tick_params);
    rv_timer_irq_enable(&timer_0_1, 0, 0, kRvTimerEnabled);
    rv_timer_arm(&timer_0_1, 0, 0, 1024);
    rv_timer_counter_set_enabled(&timer_0_1, 0, kRvTimerEnabled);

    CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
    if (power_gate_core(&power_manager, kTimer_0_pm_e, &power_manager_cpu_counters) != kPowerManagerOk_e)
    {
        printf("Error: power manager fail.\n");
        return EXIT_FAILURE;
    }
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);

    /* write something to stdout */
    printf("Success.\n");
    return EXIT_SUCCESS;
}

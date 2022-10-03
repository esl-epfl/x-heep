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
#include "rv_plic.h"
#include "rv_plic_regs.h"
#include "gpio.h"

static rv_timer_t timer;
static const uint64_t kTickFreqHz = 1000 * 1000; // 1 MHz

static power_manager_t power_manager;
static dif_plic_t rv_plic;
static gpio_t gpio;

int main(int argc, char *argv[])
{
    // Setup power_manager
    mmio_region_t power_manager_reg = mmio_region_from_addr(POWER_MANAGER_START_ADDRESS);
    power_manager.base_addr = power_manager_reg;
    power_manager_counters_t power_manager_cpu_counters;

    // Setup plic
    dif_plic_params_t rv_plic_params;
    rv_plic_params.base_addr = mmio_region_from_addr((uintptr_t)PLIC_START_ADDRESS);
    dif_plic_init(rv_plic_params, &rv_plic);

    // Get current Frequency
    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);
    uint32_t freq_hz = soc_ctrl_get_frequency(&soc_ctrl);

    // Setup rv_timer
    mmio_region_t timer_reg = mmio_region_from_addr(RV_TIMER_AO_START_ADDRESS);
    rv_timer_init(timer_reg, (rv_timer_config_t){.hart_count = 2, .comparator_count = 1}, &timer);
    rv_timer_tick_params_t tick_params;
    rv_timer_approximate_tick_params(freq_hz, kTickFreqHz, &tick_params);

    // Setup gpio
    gpio_params_t gpio_params;
    gpio_params.base_addr = mmio_region_from_addr((uintptr_t)GPIO_START_ADDRESS);
    gpio_init(gpio_params, &gpio);

    // Init cpu_subsystem's counters
    if (power_gate_counters_init(&power_manager_cpu_counters, 40, 40, 30, 30) != kPowerManagerOk_e)
    {
        printf("Error: power manager fail. Check the reset and powergate counters value\n");
        return EXIT_FAILURE;
    }

    // Power-gate and wake-up due to timer_0
    rv_timer_set_tick_params(&timer, 0, tick_params);
    rv_timer_irq_enable(&timer, 0, 0, kRvTimerEnabled);
    rv_timer_arm(&timer, 0, 0, 1024);
    rv_timer_counter_set_enabled(&timer, 0, kRvTimerEnabled);

    if (power_gate_core(&power_manager, kTimer_0_e, &power_manager_cpu_counters) != kPowerManagerOk_e)
    {
        printf("Error: power manager fail.\n");
        return EXIT_FAILURE;
    }

    // Power-gate and wake-up due to timer_1
    rv_timer_set_tick_params(&timer, 1, tick_params);
    rv_timer_irq_enable(&timer, 1, 0, kRvTimerEnabled);
    rv_timer_arm(&timer, 1, 0, 1024);
    rv_timer_counter_set_enabled(&timer, 1, kRvTimerEnabled);

    if (power_gate_core(&power_manager, kTimer_1_e, &power_manager_cpu_counters) != kPowerManagerOk_e)
    {
        printf("Error: power manager fail.\n");
        return EXIT_FAILURE;
    }

    // Power-gate and wake-up due to plic
    dif_plic_irq_set_priority(&rv_plic, GPIO_INTR_31, 1);
    dif_plic_irq_set_enabled(&rv_plic, GPIO_INTR_31, 0, kDifPlicToggleEnabled);
    gpio_output_set_enabled(&gpio, 30, true);
    gpio_irq_set_trigger(&gpio, 1 << 31, kGpioIrqTriggerLevelHigh);
    gpio_irq_set_enabled(&gpio, 31, true);
    gpio_write(&gpio, 30, true);

    if (power_gate_core(&power_manager, kPlic_e, &power_manager_cpu_counters) != kPowerManagerOk_e)
    {
        printf("Error: power manager fail.\n");
        return EXIT_FAILURE;
    }

    dif_plic_irq_id_t intr_num;
    dif_plic_irq_complete(&rv_plic, 0, &intr_num);

    /* write something to stdout */
    printf("Success.\n");
    return EXIT_SUCCESS;
}

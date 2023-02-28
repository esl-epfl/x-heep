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
#include "fast_intr_ctrl.h"
#include "gpio.h"

static rv_timer_t timer_0_1;
static rv_timer_t timer_2_3;
static const uint64_t kTickFreqHz = 1000 * 1000; // 1 MHz

static power_manager_t power_manager;
static dif_plic_t rv_plic;
static gpio_t gpio;

int main(int argc, char *argv[])
{
    // Setup fast interrupt controller
    fast_intr_ctrl_t fast_intr_ctrl;
    fast_intr_ctrl.base_addr = mmio_region_from_addr((uintptr_t)FAST_INTR_CTRL_START_ADDRESS);

    // Setup power_manager
    mmio_region_t power_manager_reg = mmio_region_from_addr(POWER_MANAGER_START_ADDRESS);
    power_manager.base_addr = power_manager_reg;
    power_manager_counters_t power_manager_cpu_counters;

    // Setup plic
    dif_plic_params_t rv_plic_params;
    rv_plic_params.base_addr = mmio_region_from_addr((uintptr_t)RV_PLIC_START_ADDRESS);
    dif_plic_init(rv_plic_params, &rv_plic);

    // Get current Frequency
    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);
    uint32_t freq_hz = soc_ctrl_get_frequency(&soc_ctrl);

    // Setup rv_timer_0_1
    mmio_region_t timer_0_1_reg = mmio_region_from_addr(RV_TIMER_AO_START_ADDRESS);
    rv_timer_init(timer_0_1_reg, (rv_timer_config_t){.hart_count = 2, .comparator_count = 1}, &timer_0_1);
    rv_timer_tick_params_t tick_params;
    rv_timer_approximate_tick_params(freq_hz, kTickFreqHz, &tick_params);

    // Setup rv_timer_2_3
    mmio_region_t timer_2_3_reg = mmio_region_from_addr(RV_TIMER_START_ADDRESS);
    rv_timer_init(timer_2_3_reg, (rv_timer_config_t){.hart_count = 2, .comparator_count = 1}, &timer_2_3);

    // Setup gpio
    gpio_params_t gpio_params;
    gpio_params.base_addr = mmio_region_from_addr((uintptr_t)GPIO_START_ADDRESS);
    gpio_init(gpio_params, &gpio);

    // Init cpu_subsystem's counters
    if (power_gate_counters_init(&power_manager_cpu_counters, 30, 30, 30, 30, 30, 30, 0, 0) != kPowerManagerOk_e)
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

    // Power-gate and wake-up due to timer_1
    rv_timer_set_tick_params(&timer_0_1, 1, tick_params);
    rv_timer_irq_enable(&timer_0_1, 1, 0, kRvTimerEnabled);
    rv_timer_arm(&timer_0_1, 1, 0, 1024);
    rv_timer_counter_set_enabled(&timer_0_1, 1, kRvTimerEnabled);

    CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
    if (power_gate_core(&power_manager, kTimer_1_pm_e, &power_manager_cpu_counters) != kPowerManagerOk_e)
    {
        printf("Error: power manager fail.\n");
        return EXIT_FAILURE;
    }
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    clear_fast_interrupt(&fast_intr_ctrl, kTimer_1_fic_e);

    // Power-gate and wake-up due to timer_2
    rv_timer_set_tick_params(&timer_2_3, 0, tick_params);
    rv_timer_irq_enable(&timer_2_3, 0, 0, kRvTimerEnabled);
    rv_timer_arm(&timer_2_3, 0, 0, 1024);
    rv_timer_counter_set_enabled(&timer_2_3, 0, kRvTimerEnabled);

    CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
    if (power_gate_core(&power_manager, kTimer_2_pm_e, &power_manager_cpu_counters) != kPowerManagerOk_e)
    {
        printf("Error: power manager fail.\n");
        return EXIT_FAILURE;
    }
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    clear_fast_interrupt(&fast_intr_ctrl, kTimer_2_fic_e);

    // Power-gate and wake-up due to timer_3
    rv_timer_set_tick_params(&timer_2_3, 1, tick_params);
    rv_timer_irq_enable(&timer_2_3, 1, 0, kRvTimerEnabled);
    rv_timer_arm(&timer_2_3, 1, 0, 1024);
    rv_timer_counter_set_enabled(&timer_2_3, 1, kRvTimerEnabled);

    CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
    if (power_gate_core(&power_manager, kTimer_3_pm_e, &power_manager_cpu_counters) != kPowerManagerOk_e)
    {
        printf("Error: power manager fail.\n");
        return EXIT_FAILURE;
    }
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    clear_fast_interrupt(&fast_intr_ctrl, kTimer_3_fic_e);

#ifdef USE_EXTERNAL_DEVICE
    // Power-gate and wake-up due to plic
    dif_plic_irq_set_priority(&rv_plic, GPIO_INTR_31, 1);
    dif_plic_irq_set_enabled(&rv_plic, GPIO_INTR_31, 0, kDifPlicToggleEnabled);
    gpio_output_set_enabled(&gpio, 30, true);
    gpio_irq_set_trigger(&gpio, 1 << 31, kGpioIrqTriggerLevelHigh);
    gpio_irq_set_enabled(&gpio, 31, true);
    gpio_write(&gpio, 30, true);

    CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
    if (power_gate_core(&power_manager, kPlic_pm_e, &power_manager_cpu_counters) != kPowerManagerOk_e)
    {
        printf("Error: power manager fail.\n");
        return EXIT_FAILURE;
    }
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);

    dif_plic_irq_id_t intr_num;
    dif_plic_irq_complete(&rv_plic, 0, &intr_num);
#endif
    /* write something to stdout */
    printf("Success.\n");
    return EXIT_SUCCESS;
}

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
#include "pad_control.h"
#include "pad_control_regs.h"
#include "x-heep.h"


#ifndef RV_PLIC_0_IS_INCLUDED
  #error ( "This app does NOT work as the RV_PLIC peripheral is not included" )
#endif


/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

static rv_timer_t timer_0_1;
static rv_timer_t timer_2_3;
static const uint64_t kTickFreqHz = 1000 * 1000; // 1 MHz
static power_manager_t power_manager;

#ifndef TARGET_PYNQ_Z2
    #define GPIO_TB_OUT 30
    #define GPIO_TB_IN  31
    #define GPIO_INTR  GPIO_INTR_31
#endif

int main(int argc, char *argv[])
{

    // Setup power_manager
    mmio_region_t power_manager_reg = mmio_region_from_addr(POWER_MANAGER_START_ADDRESS);
    power_manager.base_addr = power_manager_reg;
    power_manager_counters_t power_manager_cpu_counters;
    //counters
    uint32_t reset_off, reset_on, switch_off, switch_on, iso_off, iso_on;

    // Setup pads
#ifndef TARGET_PYNQ_Z2
    pad_control_t pad_control;
    pad_control.base_addr = mmio_region_from_addr((uintptr_t)PAD_CONTROL_START_ADDRESS);
    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_PAD_I2C0_SCL_0_REG_OFFSET), 1);
    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_PAD_I2C0_SDA_0_REG_OFFSET), 1);
#endif

    // Setup plic
    plic_Init(&rv_plic_0_inf);

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
    mmio_region_t timer_2_3_reg = mmio_region_from_addr(RV_TIMER_2_3_START_ADDRESS);
    rv_timer_init(timer_2_3_reg, (rv_timer_config_t){.hart_count = 2, .comparator_count = 1}, &timer_2_3);

    // Init cpu_subsystem's counters

    //Turn off: first, isolate the CPU outputs, then I reset it, then I switch it off (reset and switch off order does not really matter)
    iso_off = 10;
    reset_off = iso_off + 5;
    switch_off = reset_off + 5;
    //Turn on: first, give back power by switching on, then deassert the reset, the unisolate the CPU outputs
    switch_on = 10;
    reset_on = switch_on + 20; //give 20 cycles to emulate the turn on time, this number depends on technology and here it is just a random number
    iso_on = reset_on + 5;

    if (power_gate_counters_init(&power_manager_cpu_counters, reset_off, reset_on, switch_off, switch_on, iso_off, iso_on, 0, 0) != kPowerManagerOk_e)
    {
        PRINTF("Error: power manager fail. Check the reset and powergate counters value\n\r");
        return EXIT_FAILURE;
    }

    // Power-gate and wake-up due to timer_0
    rv_timer_set_tick_params(&timer_0_1, 0, tick_params);
    rv_timer_irq_enable(&timer_0_1, 0, 0, kRvTimerEnabled);
    rv_timer_arm(&timer_0_1, 0, 0, 1024);
    rv_timer_counter_set_enabled(&timer_0_1, 0, kRvTimerEnabled);

    CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
    if (power_gate_core(&power_manager, kRv_Timer_0_pm_e, &power_manager_cpu_counters) != kPowerManagerOk_e)
    {
        PRINTF("Error: power manager fail.\n\r");
        return EXIT_FAILURE;
    }
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);

    // Power-gate and wake-up due to timer_1
    rv_timer_set_tick_params(&timer_0_1, 1, tick_params);
    rv_timer_irq_enable(&timer_0_1, 1, 0, kRvTimerEnabled);
    rv_timer_arm(&timer_0_1, 1, 0, 1024);
    rv_timer_counter_set_enabled(&timer_0_1, 1, kRvTimerEnabled);

    CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
    if (power_gate_core(&power_manager, kRv_Timer_1_pm_e, &power_manager_cpu_counters) != kPowerManagerOk_e)
    {
        PRINTF("Error: power manager fail.\n\r");
        return EXIT_FAILURE;
    }
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);

    // Power-gate and wake-up due to timer_2
    rv_timer_set_tick_params(&timer_2_3, 0, tick_params);
    rv_timer_irq_enable(&timer_2_3, 0, 0, kRvTimerEnabled);
    rv_timer_arm(&timer_2_3, 0, 0, 1024);
    rv_timer_counter_set_enabled(&timer_2_3, 0, kRvTimerEnabled);

    CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
    if (power_gate_core(&power_manager, kRv_Timer_2_pm_e, &power_manager_cpu_counters) != kPowerManagerOk_e)
    {
        PRINTF("Error: power manager fail.\n\r");
        return EXIT_FAILURE;
    }
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);

    // Power-gate and wake-up due to timer_3
    rv_timer_set_tick_params(&timer_2_3, 1, tick_params);
    rv_timer_irq_enable(&timer_2_3, 1, 0, kRvTimerEnabled);
    rv_timer_arm(&timer_2_3, 1, 0, 1024);
    rv_timer_counter_set_enabled(&timer_2_3, 1, kRvTimerEnabled);

    CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
    if (power_gate_core(&power_manager, kRv_Timer_3_pm_e, &power_manager_cpu_counters) != kPowerManagerOk_e)
    {
        PRINTF("Error: power manager fail.\n\r");
        return EXIT_FAILURE;
    }
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);

#ifndef TARGET_PYNQ_Z2
    // Power-gate and wake-up due to plic
	bool state = false;
    plic_irq_set_priority(&rv_plic_0_inf, GPIO_31_INTR, 1);
    plic_irq_set_enabled(&rv_plic_0_inf, GPIO_31_INTR, kPlicToggleEnabled);

    gpio_cfg_t pin2_cfg = {.pin = GPIO_TB_IN, .mode = GpioModeIn,.en_input_sampling = true,
        .en_intr = true, .intr_type = GpioIntrEdgeRising};
    gpio_config(gpio_1_peri, pin2_cfg);

    gpio_cfg_t pin1_cfg = {.pin = GPIO_TB_OUT, .mode = GpioModeOutPushPull};
    gpio_config(gpio_1_peri, pin1_cfg);
    gpio_write(gpio_1_peri, GPIO_TB_OUT, true);

    CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
    if (power_gate_core(&power_manager, kPlic_pm_e, &power_manager_cpu_counters) != kPowerManagerOk_e)
    {
        PRINTF("Error: power manager fail.\n\r");
        return EXIT_FAILURE;
    }
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);

    uint32_t intr_num;
    plic_irq_complete(&rv_plic_0_inf, &intr_num);
#endif

    /* write something to stdout */
    PRINTF("Success.\n\r");
    return EXIT_SUCCESS;

}

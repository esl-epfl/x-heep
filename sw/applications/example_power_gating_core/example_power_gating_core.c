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

static rv_timer_t timer;
static const uint64_t kTickFreqHz = 1000 * 1000; // 1 MHz

int main(int argc, char *argv[])
{
    // Init rv_timer
    mmio_region_t timer_reg = mmio_region_from_addr(RV_TIMER_START_ADDRESS);
    rv_timer_init(timer_reg, (rv_timer_config_t){.hart_count = 1, .comparator_count = 1}, &timer);
    rv_timer_tick_params_t tick_params;
    rv_timer_approximate_tick_params(100000000, kTickFreqHz, &tick_params);
    rv_timer_set_tick_params(&timer, 0, tick_params);
    rv_timer_irq_enable(&timer, 0, 0, kRvTimerEnabled);
    rv_timer_arm(&timer, 0, 0, 1024);
    rv_timer_counter_set_enabled(&timer, 0, kRvTimerEnabled);

    // Power gate the core
    power_gate_core();

    /* write something to stdout */
    printf("Success\n");
    return EXIT_SUCCESS;
}

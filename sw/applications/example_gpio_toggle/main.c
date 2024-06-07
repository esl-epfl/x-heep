// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include "core_v_mini_mcu.h"
#include "gpio.h"
#include "x-heep.h"

#define GPIO_TOGGLE 2

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif PRINTF_IN_FPGA && !TARGET_SIM
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif


int main(int argc, char *argv[])
{
    gpio_result_t gpio_res;
    gpio_cfg_t pin_cfg = {
        .pin = GPIO_TOGGLE,
        .mode = GpioModeOutPushPull
    };
    gpio_res = gpio_config (pin_cfg);
    if (gpio_res != GpioOk)
        PRINTF("Gpio initialization failed!\n");


    for(int i=0;i<100;i++) {
        gpio_write(GPIO_TOGGLE, true);
        for(int i=0;i<10;i++) asm volatile("nop");
        gpio_write(GPIO_TOGGLE, false);
        for(int i=0;i<10;i++) asm volatile("nop");
    }

    PRINTF("Success.\n");
    return EXIT_SUCCESS;
}

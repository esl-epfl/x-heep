// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include "core_v_mini_mcu.h"
#include "gpio.h"
#include "x-heep.h"

#define GPIO_PMW 2

int main(int argc, char *argv[])
{
    gpio_result_t gpio_res;
    gpio_cfg_t pin_cfg = {
        .pin = GPIO_PMW,     
        .mode = GpioModeOutPushPull    
    };
    gpio_res = gpio_config (pin_cfg);
    if (gpio_res != GpioOk)
        printf("Gpio initialization failed!\n");

#ifdef TARGET_PYNQ_Z2
#pragma message ( "this application never ends" )
    while(1) {
        gpio_write(GPIO_PMW, true);
        for(int i=0;i<10;i++) asm volatile("nop");
        gpio_write(GPIO_PMW, false);
        for(int i=0;i<10;i++) asm volatile("nop");
    }
#else
    for(int i=0;i<100;i++) {
        gpio_write(GPIO_PMW, true);
        for(int i=0;i<10;i++) asm volatile("nop");
        gpio_write(GPIO_PMW, false);
        for(int i=0;i<10;i++) asm volatile("nop");
    }
#endif

    printf("Success.\n");
    return EXIT_SUCCESS;
}

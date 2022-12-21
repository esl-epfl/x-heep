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
    gpio_params_t gpio_params;
    gpio_t gpio;
    gpio_result_t gpio_res;
    gpio_params.base_addr = mmio_region_from_addr((uintptr_t)GPIO_AO_START_ADDRESS);
    gpio_res = gpio_init(gpio_params, &gpio);
    gpio_res = gpio_output_set_enabled(&gpio, GPIO_PMW, true);

#ifdef TARGET_PYNQ_Z2
#pragma message ( "this application never ends" )
    while(1) {
      gpio_write(&gpio, GPIO_PMW, true);
      for(int i=0;i<10;i++) asm volatile("nop");
      gpio_write(&gpio, GPIO_PMW, false);
      for(int i=0;i<10;i++) asm volatile("nop");
    }
#else
    for(int i=0;i<100;i++) {
      gpio_write(&gpio, GPIO_PMW, true);
      for(int i=0;i<10;i++) asm volatile("nop");
      gpio_write(&gpio, GPIO_PMW, false);
      for(int i=0;i<10;i++) asm volatile("nop");
    }
#endif

    printf("Success.\n");
    return EXIT_SUCCESS;
}

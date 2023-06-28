// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include "csr.h"
#include "hart.h"
#include "handler.h"
#include "core_v_mini_mcu.h"
// #include "rv_plic_old.h"
#include "rv_plic.h"
#include "rv_plic_regs.h"
#include "gpio.h"
#include "pad_control.h"
#include "pad_control_regs.h"  // Generated.
#include "x-heep.h"

/*
Notes:
 - Ports 30 and 31 are connected in questasim testbench, but in the FPGA version they are connected to the EPFL programmer and should not be used
 - Connect a cable between the two pins for the applicatio to work
*/


/* Change this value to 0 to disable prints for FPGA and enable them for simulation. */
#define DEFAULT_PRINTF_BEHAVIOR 1

/* By default, printfs are activated for FPGA and disabled for simulation. */
#ifdef TARGET_PYNQ_Z2 
    #define ENABLE_PRINTF DEFAULT_PRINTF_BEHAVIOR
#else 
    #define ENABLE_PRINTF !DEFAULT_PRINTF_BEHAVIOR
#endif

#if ENABLE_PRINTF
  #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
  #define PRINTF(...)
#endif 


#ifndef RV_PLIC_IS_INCLUDED
  #error ( "This app does NOT work as the RV_PLIC peripheral is not included" )
#endif


#ifdef TARGET_PYNQ_Z2
    #define GPIO_TB_OUT 8
    #define GPIO_TB_IN  9
    #define GPIO_INTR  GPIO_INTR_9
    #pragma message ( "Connect a cable between GPIOs IN and OUT" )

#else

    #define GPIO_TB_OUT 30
    #define GPIO_TB_IN  31
    #define GPIO_INTR  GPIO_INTR_31

#endif

plic_result_t plic_res;

int main(int argc, char *argv[])
{

    pad_control_t pad_control;
    pad_control.base_addr = mmio_region_from_addr((uintptr_t)PAD_CONTROL_START_ADDRESS);
    // rv_plic_params.base_addr = mmio_region_from_addr((uintptr_t)RV_PLIC_START_ADDRESS);
    plic_res = plic_Init();
    if (plic_res != kPlicOk) {
        PRINTF("Init PLIC failed\n\r;");
        return -1;
    }

    // In case GPIOs 30 and 31 are used:
#if GPIO_TB_OUT == 31 || GPIO_TB_IN == 31
    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_I2C_SCL_REG_OFFSET), 1);
#endif

#if GPIO_TB_OUT == 30|| GPIO_TB_IN == 30
    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_I2C_SDA_REG_OFFSET), 1);
#endif

    gpio_params_t gpio_params;
    gpio_t gpio;
    gpio_result_t gpio_res;
    gpio_params.base_addr = mmio_region_from_addr((uintptr_t)GPIO_START_ADDRESS);
    gpio_res = gpio_init(gpio_params, &gpio);
    if (gpio_res != kGpioOk) {
        PRINTF("Failed\n\r;");
        return -1;
    }

    plic_res = plic_irq_set_priority(GPIO_INTR, 1);
    if (plic_res != kPlicOk) {
        PRINTF("Failed\n\r;");
        return -1;
    }

    plic_res = plic_irq_set_enabled(GPIO_INTR, kPlicToggleEnabled);
    if (plic_res != kPlicOk) {
        PRINTF("Failed\n\r;");
        return -1;
    }

    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level external interrupts
    const uint32_t mask = 1 << 11;
    CSR_SET_BITS(CSR_REG_MIE, mask);
    plic_intr_flag = 0;

    gpio_res = gpio_output_set_enabled(&gpio, GPIO_TB_OUT, true);
    if (gpio_res != kGpioOk) {
        PRINTF("Failed\n\r;");
        return -1;
    }
    gpio_write(&gpio, GPIO_TB_OUT, false);

    gpio_res = gpio_input_enabled(&gpio, GPIO_TB_IN, true);
    if (gpio_res != kGpioOk) {
        PRINTF("Failed\n\r;");
        return -1;
    }


    gpio_res = gpio_irq_set_trigger(&gpio, GPIO_TB_IN, true, kGpioIrqTriggerEdgeRising);
    if (gpio_res != kGpioOk) {
        PRINTF("Failed\n\r;");
        return -1;
    }

    PRINTF("Write 1 to GPIO 30 and wait for interrupt...\n\r");
    while(plic_intr_flag==0) {

        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x0);
        gpio_write(&gpio, GPIO_TB_OUT, true);
        wait_for_interrupt();
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }
    PRINTF("Success\n\r");

    return EXIT_SUCCESS;
}

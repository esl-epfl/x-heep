// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include "csr.h"
#include "hart.h"
#include "handler.h"
#include "core_v_mini_mcu.h"
#include "rv_plic.h"
#include "rv_plic_regs.h"
#include "gpio.h"
#include "pad_control.h"
#include "pad_control_regs.h"  // Generated.
#include "x-heep.h"
#include <limits.h> //todo: remove

/*
Notes:
 - Ports 30 and 31 are connected in questasim testbench, but in the FPGA version they are connected to the EPFL programmer and should not be used
 - Connect a cable between the two pins for the applicatio to work
*/


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


#ifndef RV_PLIC_IS_INCLUDED
  #error ( "This app does NOT work as the RV_PLIC peripheral is not included" )
#endif


#ifdef TARGET_IS_FPGA
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

uint8_t gpio_intr_flag = 0;

void handler_1()
{
    PRINTF("handler 1\n");
    gpio_intr_flag = 1;
}

void handler_2()
{
    PRINTF("handler 2\n");
    gpio_intr_flag = 1;
}

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

    //gpio_reset_all();
    gpio_result_t gpio_res;

    gpio_cfg_t cfg_out = {
        .pin = GPIO_TB_OUT,
        .mode = GpioModeOutPushPull
    };
    gpio_res = gpio_config(cfg_out);
    if (gpio_res != GpioOk) {
        PRINTF("Failed\n;");
        return -1;
    }
    gpio_res = gpio_write(GPIO_TB_OUT, false);

    gpio_cfg_t cfg_in = {
        .pin = GPIO_TB_IN,
        .mode = GpioModeIn,
        .en_input_sampling = true,
        .en_intr = true,
        .intr_type = GpioIntrEdgeRising
    };
    gpio_res = gpio_config(cfg_in);
    if (gpio_res != GpioOk) {
        PRINTF("Failed\n;");
        return -1;
    }

    gpio_assign_irq_handler( GPIO_INTR, &handler_1 );
    gpio_intr_flag = 0;

    PRINTF("Write 1 to GPIO 30 and wait for interrupt...\n\r");
    while(gpio_intr_flag == 0) {
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        gpio_write(GPIO_TB_OUT, true);
        // wait_for_interrupt();
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }
    gpio_res = gpio_write(GPIO_TB_OUT, false);

    gpio_assign_irq_handler( GPIO_INTR, &handler_2 );
    gpio_intr_flag = 0;

    PRINTF("Write 1 to GPIO 30 and wait for interrupt...\n\r");
    while(gpio_intr_flag == 0) {
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        gpio_write(GPIO_TB_OUT, true);
        //wait_for_interrupt();
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    PRINTF("Success\n\r");

    return EXIT_SUCCESS;
}

// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include "csr.h"
#include "hart.h"
#include "handler.h"
#include "core_v_mini_mcu.h"
#include "gpio.h"
#include "gpio_regs.h"
#include "pad_control.h"
#include "pad_control_regs.h"  // Generated.
#include "x-heep.h"
#include <limits.h> //todo: remove

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


#ifdef TARGET_IS_FPGA
    #error ( "This app does NOT work on FPGA" )
#endif


#define GPIO_TB_OUT 30

uint8_t external_peripheral_interrupt_flag = 0;

void fic_irq_ext_peripheral()
{
    PRINTF("External interrupt\n");
    external_peripheral_interrupt_flag = 1;
    printf("@");
}

int main(int argc, char *argv[])
{

    pad_control_t pad_control;
    pad_control.base_addr = mmio_region_from_addr((uintptr_t)PAD_CONTROL_START_ADDRESS);

    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_I2C_SDA_REG_OFFSET), 1);

    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable external peripheral interrupt (see sw/lib/crt/vectors.S)
    const uint32_t mask = 1 << 31;
    CSR_SET_BITS(CSR_REG_MIE, mask);

    // Enable the external interrupt at fast interrupt controller level
    enable_fast_interrupt(15, true);

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

    PRINTF("Write 1 to GPIO 30 and wait for interrupt...\n\r");

    CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
    gpio_write(GPIO_TB_OUT, true);
    gpio_write(GPIO_TB_OUT, false);

    while(external_peripheral_interrupt_flag == 0) {
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        wait_for_interrupt();
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }
    gpio_res = gpio_write(GPIO_TB_OUT, false);

    PRINTF("Success\n\r");

    return EXIT_SUCCESS;
}

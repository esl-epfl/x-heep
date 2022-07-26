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

#define GPIO_TB_OUT 30
#define GPIO_TB_IN  31

int8_t external_intr_flag;

// Interrupt controller variables
dif_plic_params_t rv_plic_params;
dif_plic_t rv_plic;
dif_plic_result_t plic_res;
dif_plic_irq_id_t intr_num;

void handler_irq_external(void) {
    // Claim/clear interrupt
    plic_res = dif_plic_irq_claim(&rv_plic, 0, &intr_num);
    if (plic_res == kDifPlicOk && intr_num == GPIO_INTR_31) {
        external_intr_flag = 1;
    }
}

int main(int argc, char *argv[])
{
    printf("Init PLIC...");
    rv_plic_params.base_addr = mmio_region_from_addr((uintptr_t)PLIC_START_ADDRESS);
    plic_res = dif_plic_init(rv_plic_params, &rv_plic);
    if (plic_res == kDifPlicOk) {
        printf("success\n");
    } else {
        printf("fail\n;");
    }

    printf("Init GPIO...");
    gpio_params_t gpio_params;
    gpio_t gpio;
    gpio_result_t gpio_res;
    gpio_params.base_addr = mmio_region_from_addr((uintptr_t)GPIO_START_ADDRESS);
    gpio_res = gpio_init(gpio_params, &gpio);
    if (gpio_res == kGpioOk) {
        printf("success\n");
    } else {
        printf("fail\n;");
    }

    printf("Init GPIO interrupt priority to 1...");
    plic_res = dif_plic_irq_set_priority(&rv_plic, GPIO_INTR_31, 1);
    if (plic_res == kDifPlicOk) {
        printf("success\n");
    } else {
        printf("fail\n;");
    }

    printf("Enable GPIO interrupt...");
    plic_res = dif_plic_irq_set_enabled(&rv_plic, GPIO_INTR_31, 0, kDifPlicToggleEnabled);
    if (plic_res == kDifPlicOk) {
        printf("success\n");
    } else {
        printf("fail\n;");
    }

    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level external interrupts
    const uint32_t mask = 1 << 11;
    CSR_SET_BITS(CSR_REG_MIE, mask);
    external_intr_flag = 0;

    printf("Set GPIO 30 as output...");
    gpio_res = gpio_output_set_enabled(&gpio, GPIO_TB_OUT, true);
    if (gpio_res == kGpioOk) {
        printf("success\n");
    } else {
        printf("fail\n;");
    }

    printf("Set interrupt for GPIO 31...");
    gpio_res = gpio_irq_set_trigger(&gpio, 1 << GPIO_TB_IN, kGpioIrqTriggerLevelHigh);
    if (gpio_res == kGpioOk) {
        printf("success\n");
    } else {
        printf("fail\n;");
    }

    printf("Enable interrupt for GPIO 31...");
    gpio_res = gpio_irq_set_enabled(&gpio, GPIO_TB_IN, true);
    if (gpio_res == kGpioOk) {
        printf("success\n");
    } else {
        printf("fail\n;");
    }

    printf("Write 1 to GPIO 30 and wait for interrupt...");
    gpio_write(&gpio, GPIO_TB_OUT, true);
    while(external_intr_flag==0) {
        wait_for_interrupt();
    }
    printf("success\n");

    printf("Complete interrupt...");
    plic_res = dif_plic_irq_complete(&rv_plic, 0, &intr_num);
    if (plic_res == kDifPlicOk) {
        printf("success\n");
    } else {
        printf("fail\n;");
    }

    return EXIT_SUCCESS;
}

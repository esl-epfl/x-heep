// Copyright 2024 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: example_timer_sdk.c
// Author: Juan Sapriza
// Date: 15/07/2024
// Description: Example application to test the Timer SDK. Will count the time to execute a few short tasks.

#include <stdint.h>
#include <stdlib.h>
#include "core_v_mini_mcu.h"
#include "timer_sdk.h"
#include "x-heep.h"

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   1

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif PRINTF_IN_FPGA && !TARGET_SIM
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

void __attribute__((aligned(4), interrupt)) handler_irq_timer(void) {
    timer_arm_stop();
    timer_irq_clear();
    return;   
}

int main(){
    uint8_t i = 0;
    uint32_t timer_cycles;

    timer_init();               // Init the timer SDK
    timer_start();              // Start counting the time
    timer_cycles = timer_stop();  // Stop counting the time
    PRINTF("0 NOPs:\t%d cc\n\r", timer_cycles );
    
    timer_start();              
    asm volatile ("nop");
    timer_cycles = timer_stop();  
    PRINTF("1 NOP:\t%d cc\n\r", timer_cycles );
    
    timer_start();              
    asm volatile ("nop");
    asm volatile ("nop");
    timer_cycles = timer_stop();  
    PRINTF("2 NOPs:\t%d cc\n\r", timer_cycles );

    timer_start();              
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    timer_cycles = timer_stop();  
    PRINTF("3 NOPs:\t%d cc\n\r", timer_cycles );

    //enable timer interrupt
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level timer interrupts
    const uint32_t mask = 1 << 7;
    CSR_SET_BITS(CSR_REG_MIE, mask);

    timer_init();
    timer_irq_enable();
    timer_arm_start(10);
    timer_start();              
    asm volatile ("wfi");
    timer_cycles = timer_stop();  
    PRINTF("wait ARM timer 10 (%d)\n\r", timer_cycles );


    return EXIT_SUCCESS;
}
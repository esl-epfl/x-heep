// Copyright 2024 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: example_timer_sdk.c
// Author: Juan Sapriza, Francesco Poluzzi
// Date: 23/07/2024
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
    uint32_t i = 0;
    uint32_t timer_cycles;

    timer_init();               // Init the timer SDK for clock cycles
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

    enable_timer_interrupt();   // Enable the timer machine-level interrupt

    timer_init();
    timer_irq_enable();
    timer_arm_start(1000);
    timer_start();              
    asm volatile ("wfi");       // Wait for interrupt
    timer_cycles = timer_stop();  
    PRINTF("wait ARM timer 1000 (%d)\n\r", timer_cycles );

    timer_init_us();           // Init the timer SDK for microseconds
    timer_start(); 
    for(i = 0; i < 1000; i++){
        asm volatile ("nop");
    }
    PRINTF("Microseconds for 1000 NOPs:\t%d μs\n\r", timer_stop() );

    PRINTF("Wait 5 seconds\n\r");
    timer_wait_ms(5000);       // Wait for 5 seconds
    PRINTF("Done\n\r");

    return EXIT_SUCCESS;
}

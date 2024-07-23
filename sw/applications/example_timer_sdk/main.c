// Copyright 2024 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: example_timer_sdk.c
// Author: Juan Sapriza, Francesco Poluzzi
// Date: 23/07/2024
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
#define PRINTF_IN_SIM   0

/* Error tolerances for the tests. */
#define CYCLE_TOLERANCE  2         // cycles tolerance for simple timer reads
#define INTERRUPT_TOLERANCE 50     // cycles tolerance for timer interrupt
#define TIMER_WAIT_TOLERANCE 100   // milliseconds tolerance for timer wait

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
    uint32_t nop_cycles[4];

    timer_cycles_init();               // Init the timer SDK for clock cycles
    timer_start();              // Start counting the time
    nop_cycles[0] = timer_stop();  // Stop counting the time
    PRINTF("0 NOPs:\t%d cc\n\r", nop_cycles[0] );
      
    timer_start();     
    asm volatile ("nop");
    nop_cycles[1] = timer_stop();  
    PRINTF("1 NOP:\t%d cc\n\r", nop_cycles[1] );
    
    timer_start();              
    asm volatile ("nop");
    asm volatile ("nop");
    nop_cycles[2] = timer_stop();  
    PRINTF("2 NOPs:\t%d cc\n\r", nop_cycles[2] );

    timer_start();              
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    nop_cycles[3] = timer_stop();  
    PRINTF("3 NOPs:\t%d cc\n\r", nop_cycles[3] );

    if( abs(nop_cycles[1] - nop_cycles[0])>CYCLE_TOLERANCE || abs(nop_cycles[2] - nop_cycles[1])>CYCLE_TOLERANCE || abs(nop_cycles[3] - nop_cycles[2])>CYCLE_TOLERANCE){ 
        PRINTF("Clock count failed\n\r");
        return EXIT_FAILURE;
    } 

    enable_timer_interrupt();   // Enable the timer machine-level interrupt

    timer_cycles_init();
    timer_irq_enable();
    timer_arm_start(1000);
    timer_start();              
    asm volatile ("wfi");       // Wait for interrupt
    timer_cycles = timer_stop();  
    if(abs(timer_cycles-1000) < INTERRUPT_TOLERANCE){ 
        PRINTF("Timer thershold interrupt working\n" );
    } else {
        PRINTF("Timer thershold interrupt failed\n\r");
        return EXIT_FAILURE;
    }
    
    timer_microseconds_init();           // Init the timer SDK for microseconds
    timer_start(); 
    for(i = 0; i < 1000; i++){
        asm volatile ("nop");
    }
    timer_cycles = timer_stop();  
    PRINTF("Microseconds for 1000 NOPs:\t%d μs\n\r", timer_cycles );

    PRINTF("Wait 5 seconds\n\r");
    timer_wait_ms(5000);       // Wait for 5 seconds
    PRINTF("Done\n\r");
    timer_cycles = timer_stop(); 

    if(abs(timer_cycles/1000)-5000 > TIMER_WAIT_TOLERANCE){ 
        PRINTF("Timer wait failed\n\r");
        return EXIT_FAILURE;
    }

    PRINTF("All tests passed\n\r");
    return EXIT_SUCCESS;
}


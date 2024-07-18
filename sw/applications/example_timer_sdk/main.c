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
#define PRINTF_IN_SIM   0

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif PRINTF_IN_FPGA && !TARGET_SIM
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

int main(){
    uint8_t i = 0;
    uint32_t cpu_cycles;

    timer_init();               // Init the timer SDK
    timer_start();              // Start counting the time
    cpu_cycles = timer_stop();  // Stop counting the time
    PRINTF("0 NOPs:\t%d cc\n\r", cpu_cycles );
    
    timer_start();              
    asm volatile ("nop");
    cpu_cycles = timer_stop();  
    PRINTF("1 NOP:\t%d cc\n\r", cpu_cycles );
    
    timer_start();              
    asm volatile ("nop");
    asm volatile ("nop");
    cpu_cycles = timer_stop();  
    PRINTF("2 NOPs:\t%d cc\n\r", cpu_cycles );

    timer_start();              
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    cpu_cycles = timer_stop();  
    PRINTF("3 NOPs:\t%d cc\n\r", cpu_cycles );
    
    return EXIT_SUCCESS;
}
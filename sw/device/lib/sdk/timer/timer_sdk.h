
// Copyright 2023 EPFL and Politecnico di Torino.
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: timer_sdk.h
// Authors: Michele Caon, Luigi Giuffrida, Francesco Poluzzi
// Date: 23/07/2024
// Authors: Michele Caon, Luigi Giuffrida, Francesco Poluzzi
// Date: 23/07/2024
// Description: Execution time measurements utilities

#ifndef TIMER_SDK_H_
#define TIMER_SDK_H_

#include <stdint.h>

#include "csr.h"
#include "gpio.h"
#include "rv_timer.h"
#include "rv_timer_regs.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"

#define FREQ_1MHz 1000000

/******************************/
/* ---- GLOBAL VARIABLES ---- */
/******************************/

// Timer value
extern uint32_t timer_value;
extern uint32_t hw_timer_value;

extern rv_timer_t timer;

/********************************/
/* ---- EXPORTED FUNCTIONS ---- */
/********************************/

/**
 * @brief Get the current value of the HW timer
 * 
* @return int64_t Current value of the HW timer
 */
uint32_t hw_timer_get_cycles();

/**
 * @brief Start the HW timer
 * 
 */
void hw_timer_start();

/**
 * @brief Stop the HW timer
 * 
 * @return int64_t Elapsed time in clock cycles
 */
uint32_t hw_timer_stop();


/**
 * @brief Initialize the timer for counting clock cycles
 * @brief Initialize the timer for counting clock cycles
 * 
 */
void timer_cycles_init();

/**
 * @brief Initialize the timer for counting microseconds
 * 
 */
void  timer_microseconds_init();

/**
 * @brief Get the current value of the MCYCLE CSR
 * 
 * @return int64_t Current value of the MCYCLE CSR
 */
inline uint32_t timer_get_cycles() {
    uint32_t cycle_count;
    CSR_READ(CSR_REG_MCYCLE, &cycle_count);
    return cycle_count;
}

/**
 * @brief Start the timer
 * 
 */
inline void timer_start() {
    hw_timer_start();
}

/**
 * @brief Stop the timer
 * 
 * @return int64_t Elapsed time in clock cycles
 */
inline uint32_t timer_stop() {
    return hw_timer_stop();
}

/**
 * @brief Enable the timer IRQ
 */
void timer_irq_enable();

/**
 * @brief Clear the timer IRQ
 */
void timer_irq_clear();

/**
 * @brief Arms the timer to go off once the counter value is greater than or equal to threshold
 */
void timer_arm_start(uint32_t threshold);

/**
 * @brief Stop to output when timer is greater than or equal to threshold previously set
 */
void timer_arm_stop();

/**
 * @brief Wait for a certain amount of milliseconds. Not precise with small
 * numbers of milliseconds. 
 * You need to enable timer interrupts with enable_timer_interrupt() before using this function
 */
void timer_wait_ms(uint32_t ms);

/**
 * @brief Enable the timer machine-level interrupts for X-Heep
 */
void enable_timer_interrupt();

/**
 * @brief Wait for a certain amount of milliseconds. Not precise with small
 * numbers of milliseconds. 
 * You need to enable timer interrupts with enable_timer_interrupt() before using this function
 */
void timer_wait_ms(uint32_t ms);

/**
 * @brief Enable the timer machine-level interrupts for X-Heep
 */
void enable_timer_interrupt();

#endif /* TIMER_SDK_H_ */

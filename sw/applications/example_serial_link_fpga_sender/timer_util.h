// Copyright 2023 EPFL and Politecnico di Torino.
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: timer_util.h
// Authors: Michele Caon, Luigi Giuffrida
// Date: 31/07/2023
// Description: Execution time measurements utilities

#ifndef TIMER_UTIL_H_
#define TIMER_UTIL_H_

#include <stdint.h>

#include "csr.h"
#include "gpio.h"
#include "rv_timer.h"
#include "rv_timer_regs.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"

#define TICK_FREQ 1000000

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
 * @brief Initialize the hardware timer
 * 
 */
void hw_timer_init();

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
 * @brief Initialize the timer
 * 
 */
void timer_init();

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
    timer_value = -timer_get_cycles();
}

/**
 * @brief Stop the timer
 * 
 * @return int64_t Elapsed time in clock cycles
 */
inline uint32_t timer_stop() {
    timer_value += timer_get_cycles();
    return timer_value;
}

#endif /* TIMER_H_ */

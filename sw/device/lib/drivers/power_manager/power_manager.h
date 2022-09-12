// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#ifndef _POWER_MANAGER_H_
#define _POWER_MANAGER_H_

#include <stddef.h>
#include <stdint.h>

#include "mmio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Results.
 */
typedef enum power_manager_result {
  kPowerManagerOk = 0,
  kPowerManagerError = 2,
} power_manager_result_t;

/**
 * Domains.
 */
typedef enum power_manager_sel_domain {
  kPeriph = 2,
  kRam0   = 4,
  kRam1   = 8,
} power_manager_sel_domain_t;

/**
 * Domain states.
 */
typedef enum power_manager_sel_state {
  kOn  = 0,
  kOff = 1,
} power_manager_sel_state_t;

/**
 * Interrupt source.
 */
typedef enum power_manager_sel_intr {
  kTimer_0 = 0,
  kTimer_1 = 1,
  kTimer_2 = 2,
  kTimer_3 = 3,
  kDma     = 4,
  kGpio_0  = 5,
  kGpio_1  = 6,
  kGpio_2  = 7,
  kGpio_3  = 8,
  kGpio_4  = 9,
  kGpio_5  = 10,
  kGpio_6  = 11,
  kGpio_7  = 12,
  kExt_0   = 13,
  kExt_1   = 14,
  kExt_2   = 15,
  kExt_3   = 16,
} power_manager_sel_intr_t;

/**
 * Initialization parameters for POWER MANAGER.
 *
 */
typedef struct power_manager {
  /**
   * The base address for the power_manager hardware registers.
   */
  mmio_region_t base_addr;
} power_manager_t;


typedef struct power_manager_cpu_counters {
  /**
   * The counter to set and unset the reset and switch of the CPU.
   */
  uint32_t reset_off;
  uint32_t reset_on;
  uint32_t powergate_off;
  uint32_t powergate_on;

} power_manager_cpu_counters_t;

/**
 * Power gate core and wait for interrupt to wake-up.
 */

power_manager_result_t power_gate_cpu_counters_init(power_manager_cpu_counters_t* cpu_counter, uint32_t reset_off, uint32_t reset_on, uint32_t powergate_off, uint32_t powergate_on);

power_manager_result_t power_gate_core(const power_manager_t *power_manager, power_manager_sel_intr_t sel_intr, power_manager_cpu_counters_t* cpu_counter);

power_manager_result_t power_gate_domain(const power_manager_t *power_manager, power_manager_sel_domain_t sel_domain, power_manager_sel_state_t sel_state, power_manager_cpu_counters_t* cpu_counter);

#ifdef __cplusplus
}
#endif

#endif  // _POWER_MANAGER_H_

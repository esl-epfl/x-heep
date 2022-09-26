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
  kPowerManagerError = 1,
} power_manager_result_t;

/**
 * Domains.
 */
typedef enum power_manager_sel_domain {
  kPeriph = 0,
  kRam0   = 1,
  kRam1   = 2,
  kRam2   = 3,
  kRam3   = 4,
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
  kSpi     = 0,
  kTimer_0 = 1,
  kTimer_1 = 2,
  kTimer_2 = 3,
  kTimer_3 = 4,
  kDma     = 5,
  kGpio_0  = 6,
  kGpio_1  = 7,
  kGpio_2  = 8,
  kGpio_3  = 9,
  kGpio_4  = 10,
  kGpio_5  = 11,
  kGpio_6  = 12,
  kGpio_7  = 13,
  kExt_0   = 14,
  kExt_1   = 15,
  kExt_2   = 16,
  kExt_3   = 17,
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

typedef struct power_manager_counters {
  /**
   * The counter to set and unset the reset and switch of the CPU.
   */
  uint32_t reset_off;
  uint32_t reset_on;
  uint32_t powergate_off;
  uint32_t powergate_on;

} power_manager_counters_t;

power_manager_result_t power_gate_counters_init(power_manager_counters_t* counters, uint32_t reset_off, uint32_t reset_on, uint32_t powergate_off, uint32_t powergate_on);

power_manager_result_t power_gate_core(const power_manager_t *power_manager, power_manager_sel_intr_t sel_intr, power_manager_counters_t* cpu_counters);

power_manager_result_t power_gate_domain(const power_manager_t *power_manager, power_manager_sel_domain_t sel_domain, power_manager_sel_state_t sel_state, power_manager_counters_t* domain_counters);

#ifdef __cplusplus
}
#endif

#endif  // _POWER_MANAGER_H_

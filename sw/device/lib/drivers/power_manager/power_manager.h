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
 * Interrupt source.
 */
typedef enum power_manager_sel_intr {
  kTimer = 0,
} power_manager_sel_intr_t;

/**
 * Initialization parameters for POWER MANAGER.
 *
 */
typedef struct soc_ctrl {
  /**
   * The base address for the power_manager hardware registers.
   */
  mmio_region_t base_addr;
} power_manager_t;

/**
 * Power gate core and wait for interrupt to wake-up.
 */
power_manager_result_t power_gate_core(const power_manager_t *power_manager, power_manager_sel_intr_t sel_intr);

#ifdef __cplusplus
}
#endif

#endif  // _POWER_MANAGER_H_

// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef _DRIVERS_SOC_CTRL_H_
#define _DRIVERS_SOC_CTRL_H_

#include <stddef.h>
#include <stdint.h>

#include "../../base/mmio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialization parameters for SOC CTRL.
 *
 */
typedef struct soc_ctrl {
  /**
   * The base address for the soc_ctrl hardware registers.
   */
  mmio_region_t base_addr;
} soc_ctrl_t;

/**
 * Write a to valid register of the SOC CTRL.
 * @param soc_ctrl Pointer to soc_ctrl_t represting the target SOC CTRL.
 * @param valid (1 or 0) data to write.
 */
void soc_ctrl_set_valid(const soc_ctrl_t *soc_ctrl, uint8_t valid);

/**
 * Write a the exit value of the SOC CTRL.
 * @param soc_ctrl Pointer to soc_ctrl_t represting the target SOC CTRL.
 * @param exit value data to write.
 */
void soc_ctrl_set_exit_value(const soc_ctrl_t *soc_ctrl, uint32_t exit_value);

#ifdef __cplusplus
}
#endif

#endif  // OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_DRIVERS_SOC CTRL_H_

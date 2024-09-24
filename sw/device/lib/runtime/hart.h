// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef OPENTITAN_SW_DEVICE_LIB_RUNTIME_HART_H_
#define OPENTITAN_SW_DEVICE_LIB_RUNTIME_HART_H_

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


#include <stddef.h>
#include <stdnoreturn.h>

#include "stdasm.h"

/**
 * @file
 * @brief This header provides functions for controlling the excution of a hart,
 * such as halt-like functionality.
 */

/**
 * Hints to the processor that we don't have anything better to be doing, and to
 * go into low-power mode until an interrupt is serviced.
 *
 * This function may behave as if it is a no-op.
 */
static inline void wait_for_interrupt(void) { asm volatile("wfi"); }


#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // OPENTITAN_SW_DEVICE_LIB_RUNTIME_HART_H_

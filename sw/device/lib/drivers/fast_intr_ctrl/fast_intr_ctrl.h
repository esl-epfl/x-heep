// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#ifndef _FAST_INTR_CTRL_H_
#define _FAST_INTR_CTRL_H_

#include <stddef.h>
#include <stdint.h>

#include "mmio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialization parameters for FAST INTR CTRL.
 *
 */
typedef struct fast_intr_ctrl {
  /**
   * The base address for the fast intr ctrl hardware registers.
   */
  mmio_region_t base_addr;
} fast_intr_ctrl_t;

/**
 * Results.
 */
typedef enum fast_intr_ctrl_result {
  kFastIntrCtrlOk_e    = 0,
  kFastIntrCtrlError_e = 1,
} fast_intr_ctrl_result_t;

/**
 * Fast interrupts.
 */
typedef enum fast_intr_ctrl_fast_interrupt {
  kTimer_1_fic_e  = 0,
  kTimer_2_fic_e  = 1,
  kTimer_3_fic_e  = 2,
  kDma_fic_e      = 3,
  kSpi_fic_e      = 4,
  kSpiFlash_fic_e = 5,
  kGpio_0_fic_e   = 6,
  kGpio_1_fic_e   = 7,
  kGpio_2_fic_e   = 8,
  kGpio_3_fic_e   = 9,
  kGpio_4_fic_e   = 10,
  kGpio_5_fic_e   = 11,
  kGpio_6_fic_e   = 12,
  kGpio_7_fic_e   = 13,
} fast_intr_ctrl_fast_interrupt_t;

fast_intr_ctrl_result_t clear_fast_interrupt(fast_intr_ctrl_t* fast_intr_ctrl, fast_intr_ctrl_fast_interrupt_t fast_interrupt);

#ifdef __cplusplus
}
#endif

#endif  // _FAST_INTR_CTRL_H_

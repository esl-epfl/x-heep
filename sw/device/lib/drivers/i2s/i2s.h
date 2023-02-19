// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef _DRIVERS_I2S_H_
#define _DRIVERS_I2S_H_

#include <stddef.h>
#include <stdint.h>

#include "mmio.h"

#include "i2s_regs.h"     // Generated.

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialization parameters for I2s PERIPHERAL.
 *
 */
typedef struct i2s {
  /**
   * The base address for the soc_ctrl hardware registers.
   */
  mmio_region_t base_addr;
} i2s_t;


/**
 * Write to read_ptr register of the I2s
 * @param i2s pointer to i2s_t represting the target I2s PERIPHERAL.
 * @param enable Enable I2s peripheral.
 * @param gen_clk Generate and output sck and ws signal.
 */
void i2s_set_enable(const i2s_t *i2s, bool enable, bool gen_clk);


/**
 * Set clock divider value
 * @param i2s pointer to i2s_t represting the target I2s PERIPHERAL.
 * @param div_value Divider value = src_clk_freq / gen_clk_freq 
 *        (odd values are allowed, for 0 and 1 the src clock is used)
 */
void i2s_set_clk_divider(const i2s_t *i2s, uint16_t div_value);


/**
 * Set data width of i2s
 * @param i2s pointer to i2s_t represting the target I2s PERIPHERAL.
 * @param data_width value of type `COUNT_t` configuring the data width.
 */
void i2s_set_data_width(const i2s_t *i2s, uint32_t data_width);


/**
 * Write to write_ptr register of the I2s
 * @param i2s pointer to i2s_t represting the target I2s PERIPHERAL.
 * @param reach_count Number of samples to trigger interrupt.
 */
void i2s_set_intr_reach_count(const i2s_t *i2s, uint32_t reach_count);

/**
 * Enable interrupt 
 * @param i2s pointer to i2s_t represting the target I2s PERIPHERAL.
 * @param enable I2s interrupt event enable bit.
 */
void i2s_enable_intr(const i2s_t *i2s, bool enable);


#ifdef __cplusplus
}
#endif

#endif // _DRIVERS_I2S_H_

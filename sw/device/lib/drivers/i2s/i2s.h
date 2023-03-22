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


typedef enum i2s_datawidth {
  I2S_08_BITS = I2S_CFG_DATA_WIDTH_VALUE_8_BITS,
  I2S_16_BITS = I2S_CFG_DATA_WIDTH_VALUE_16_BITS,
  I2S_24_BITS = I2S_CFG_DATA_WIDTH_VALUE_24_BITS,
  I2S_32_BITS = I2S_CFG_DATA_WIDTH_VALUE_32_BITS
} i2s_datawidth_t;


typedef enum i2s_channel_sel {
  I2S_DISABLE = I2S_CFG_EN_VALUE_DISABLED,
  I2S_LEFT_CH = I2S_CFG_EN_VALUE_ONLY_LEFT,
  I2S_RIGHT_CH = I2S_CFG_EN_VALUE_ONLY_RIGHT,
  I2S_BOTH_CH = I2S_CFG_EN_VALUE_BOTH_CHANNELS
} i2s_channel_sel_t;

/**
 * Setup I2s
 * 
 * @param enable Enable I2s peripheral.
 * @param gen_clk Generate and output sck and ws signal.
 * @param enable I2s interrupt event enable bit.
 * @param div_value Divider value = src_clk_freq / gen_clk_freq 
 *        (odd values are allowed, for 0 and 1 the src clock is used)
 * @param data_width 
 */
void i2s_setup(i2s_channel_sel_t en, bool en_master, uint16_t div_value, bool intr_en, i2s_datawidth_t data_width, uint32_t intr_reach_count);


#ifdef __cplusplus
}
#endif

#endif // _DRIVERS_I2S_H_

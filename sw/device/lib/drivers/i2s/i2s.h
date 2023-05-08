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


typedef enum i2s_word_length {
  I2S_08_BITS = I2S_CONTROL_DATA_WIDTH_VALUE_8_BITS,
  I2S_16_BITS = I2S_CONTROL_DATA_WIDTH_VALUE_16_BITS,
  I2S_24_BITS = I2S_CONTROL_DATA_WIDTH_VALUE_24_BITS,
  I2S_32_BITS = I2S_CONTROL_DATA_WIDTH_VALUE_32_BITS
} i2s_word_length_t;


typedef enum i2s_channel_sel {
  I2S_DISABLE = I2S_CONTROL_EN_RX_VALUE_DISABLED,
  I2S_LEFT_CH = I2S_CONTROL_EN_RX_VALUE_ONLY_LEFT,
  I2S_RIGHT_CH = I2S_CONTROL_EN_RX_VALUE_ONLY_RIGHT,
  I2S_BOTH_CH = I2S_CONTROL_EN_RX_VALUE_BOTH_CHANNELS
} i2s_channel_sel_t;

#define I2S_RX_DATA_ADDRESS (uint32_t)(I2S_RXDATA_REG_OFFSET+I2S_START_ADDRESS)

/**
 * I2S configure SCK frequency and word length
 * 
 * @param div_value Divider value = src_clk_freq / gen_clk_freq 
 *        (odd values are allowed, for 0 and 1 the src clock is used)
 * @param word_length (see i2s_word_length_t)
 */
void    i2s_configure(uint16_t div_value, i2s_word_length_t word_length);

/**
 * I2S enable and configure watermark counter
 * 
 * @param watermark number to trigger interrupt 
 * @param interrupt_en enable/disable interrupt
 */
void    i2s_rx_enable_watermark(uint32_t watermark, bool interrupt_en);


/**
 * I2S disable watermark counter
 */
void    i2s_rx_disable_watermark();


/**
 * I2S start rx channels 
 * 
 * @note this function might take some time to complete
 * 
 * @param channels to be enabled (see i2s_channel_sel_t)
 */
void    i2s_rx_start(i2s_channel_sel_t channels);


/**
 * I2S stop rx channels 
 * 
 */
void    i2s_rx_stop();

/**
 * I2S check RX data availability
 * 
 * @return true if RX data is available 
 */
bool    i2s_rx_data_available();

/**
 * I2S read RX word
 * 
 * @note The MSBs outside of word_length are 0.
 *
 * @return int32_t RX word 
 */
int32_t i2s_rx_read_data();

/**
 * I2S read value of watermark counter
 * 
 * @return int32_t current counter value
 */
int32_t i2s_rx_read_waterlevel();

/**
 * I2S check RX FIFO overflow
 * 
 * @note call i2s_rx_start(...) to reset overflow and cleanly restart
 * 
 * @return true if RX FIFO overflowed  
 * @return false 
 */
bool    i2s_rx_overflow();



#ifdef __cplusplus
}
#endif

#endif // _DRIVERS_I2S_H_

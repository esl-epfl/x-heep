// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stddef.h>
#include <stdint.h>

#include "i2s.h"
#include "i2s_structs.h"

void i2s_configure(uint16_t div_value, i2s_datawidth_t data_width) {
  i2s_peri->CLKDIVIDX = div_value;

  uint32_t control = i2s_peri->CONTROL;
  bitfield_field32_write(control, I2S_CONTROL_DATA_WIDTH_FIELD, data_width);
  i2s_peri->CONTROL = control;
}

void i2s_rx_enable_watermark(uint32_t watermark, bool interrupt_en) {
  i2s_peri->Watermark = watermark;

  uint32_t control = i2s_peri->CONTROL;
  control = bitfield_bit32_write(control, I2S_CONTROL_INTR_EN_BIT, interrupt_en);
  control |= (1 << I2S_CONTROL_EN_WATERMARK_BIT);
  control  &= ~((1 << I2S_CONTROL_INTR_EN_BIT) + (1 << I2S_CONTROL_EN_WATERMARK_BIT));
  i2s_peri->CONTROL = control;
}

void i2s_rx_disable_watermark() {
  // disable interrupt and disable watermark counter
  i2s_peri->CONTROL &= ~((1 << I2S_CONTROL_INTR_EN_BIT) + (1 << I2S_CONTROL_EN_WATERMARK_BIT));
}

bool i2s_rx_data_available() {
  return (i2s_peri->STATUS & (1 << I2S_STATUS_RX_DATA_READY_BIT));
}

int32_t i2s_rx_read_data() {
  return (int32_t) i2s_peri->RXDATA;
}

int32_t i2s_rx_read_waterlevel() {
  return (uint32_t) i2s_peri->WATERLEVEL;
}

bool i2s_rx_overflow() {
  return (i2s_peri->STATUS & (1 << I2S_STATUS_RX_OVERFLOW_BIT));
}

void i2s_rx_start(i2s_channel_sel_t channels) {
  if (channels == I2S_DISABLE) {
    // no channels selected -> disable
    i2s_rx_stop();
    return;
  }

  uint32_t control = i2s_peri->CONTROL;

  // check overflow before changing state
  bool overflow = i2s_rx_overflow(); 

  if (control & ((I2S_CONTROL_EN_RX_MASK << I2S_CONTROL_EN_RX_OFFSET ) + (1 << I2S_CONTROL_EN_WS_BIT))) {
    // something is running - disable all but the clock
    control &= ~(I2S_CONTROL_EN_RX_MASK << I2S_CONTROL_EN_RX_OFFSET); // disable rx
    control &= ~(1 << I2S_CONTROL_EN_WS_BIT); // disable ws gen
    control &= ~(1 << I2S_CONTROL_EN_IO_BIT); // disable ios
  }

  // enable clock
  control |= (1 << I2S_CONTROL_EN_BIT); // enable peripheral and clock domain
  i2s_peri->CONTROL = control;

  // check if overflow has occurred
  if (overflow) {
    // overflow bit is going to be reset by rx channel if the sck is running
    while (i2s_rx_overflow()) ; // wait for one SCK rise - this might take some time as the SCK can be much slower
    
    // cdc_2phase FIFO is not clearable, so we have to empty the FIFO manually
    // note: this uses much less resources
    while (i2s_rx_data_available()) {
      i2s_rx_read_data(); // read to empty FIFO
    }
  }

  // now we can start the RX channels and ws generation
  control |= (channels << I2S_CONTROL_EN_RX_OFFSET); // enable selected rx channels
  control |= (1 << I2S_CONTROL_EN_WS_BIT);
  control |= (1 << I2S_CONTROL_EN_IO_BIT); // enable ios
  i2s_peri->CONTROL = control;
}

void i2s_rx_stop() {
  // disable all modules
  uint32_t control = i2s_peri->CONTROL;
  control &= ~(1 << I2S_CONTROL_EN_BIT); // disable peripheral and clock domain
  control &= ~(I2S_CONTROL_EN_RX_MASK << I2S_CONTROL_EN_RX_OFFSET); // disable rx
  control &= ~(1 << I2S_CONTROL_EN_WS_BIT); // disable ws gen
  control &= ~(1 << I2S_CONTROL_EN_IO_BIT); // disable ios
  i2s_peri->CONTROL = control;
}

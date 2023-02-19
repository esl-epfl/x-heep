// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stddef.h>
#include <stdint.h>

#include "i2s.h"

void i2s_set_enable(const i2s_t *i2s, bool enable, bool gen_clk) {
  mmio_region_write32(i2s->base_addr, I2S_CFG_REG_OFFSET, enable << I2S_CFG_EN_BIT | gen_clk << I2S_CFG_GEN_CLK_WS_BIT);
}

void i2s_set_clk_divider(const i2s_t *i2s, uint16_t div_value) {
  mmio_region_write32(i2s->base_addr, I2S_CLKDIVIDX_REG_OFFSET, div_value);
}

void i2s_set_data_width(const i2s_t *i2s, uint32_t data_width) {
  mmio_region_write32(i2s->base_addr, I2S_BYTEPERSAMPLE_REG_OFFSET, I2S_BYTEPERSAMPLE_COUNT_VALUE_32_BITS);
}

void i2s_set_intr_reach_count(const i2s_t *i2s, uint32_t reach_count) {
  mmio_region_write32(i2s->base_addr, I2S_REACHCOUNT_REG_OFFSET, reach_count);
}

void i2s_set_enable_intr(const i2s_t *i2s, bool enable) {
  mmio_region_write32(i2s->base_addr, I2S_INTR_ENABLE_REG_OFFSET, enable << I2S_INTR_ENABLE_I2S_EVENT_BIT);
}

// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stddef.h>
#include <stdint.h>

#include "i2s.h"
#include "i2s_structs.h"

void i2s_setup(i2s_channel_sel_t en, bool en_master, uint16_t div_value, bool intr_en, i2s_datawidth_t data_width, uint32_t intr_reach_count) {
  i2s_peri->CLKDIVIDX = div_value;
  i2s_peri->Watermark = intr_reach_count;

  i2s_peri->cfg = (en << I2S_CFG_EN_OFFSET)
  + (en_master << I2S_CFG_GEN_CLK_WS_BIT)
  + (intr_en << I2S_CFG_INTR_EN_BIT)
  + (data_width << I2S_CFG_DATA_WIDTH_OFFSET);
}

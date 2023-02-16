/*
 * Copyright 2020 ETH Zurich
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Robert Balas <balasr@iis.ee.ethz.ch>
 */

#include <stdio.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"
#include "i2s_regs.h"

#include "mmio.h"

int main(int argc, char *argv[])
{
    printf("I2s DEMO\n");

    mmio_region_t base_addr = mmio_region_from_addr((uintptr_t)I2S_START_ADDRESS);

    int32_t result = 0;
    mmio_region_write32(base_addr, I2S_CLKDIVIDX_COUNT_OFFSET, 0x10);
    mmio_region_write32(base_addr, I2S_BYTEPERSAMPLE_REG_OFFSET, I2S_BYTEPERSAMPLE_COUNT_VALUE_32_BITS);
    mmio_region_write32(base_addr, I2S_CFG_REG_OFFSET, 1 << I2S_CFG_EN_BIT | 1 << I2S_CFG_GEN_CLK_WS_BIT);

    for (int32_t i = 0; i < 16; i++) {
        int32_t status; 
        do {
            status = mmio_region_read32(base_addr, I2S_STATUS_REG_OFFSET);
        } while (status & I2S_STATUS_EMPTY_BIT);
        result = mmio_region_read32(base_addr, I2S_RXDATA_REG_OFFSET);
        printf("RX %2d: %d\n", i, result);
    }

    return EXIT_SUCCESS;
}
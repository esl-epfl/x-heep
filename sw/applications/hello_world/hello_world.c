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
#include "pdm2pcm_regs.h"

#include "mmio.h"

int main(int argc, char *argv[])
{
    printf("PDM2PCM DEMO\n");
    printf(" > Start\n");

    mmio_region_t pdm2pcm_base_addr = mmio_region_from_addr((uintptr_t)PDM2PCM_START_ADDRESS);

    //mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_CLKDIVIDX_REG_OFFSET , 9000);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_CLKDIVIDX_REG_OFFSET ,15);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_REACHCOUNT_REG_OFFSET, 1);
    
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_DECIMCIC_REG_OFFSET  ,15);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_DECIMHB1_REG_OFFSET  ,15);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_DECIMHB2_REG_OFFSET  ,15);
    
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_HB1COEF00_REG_OFFSET , 1 << 16);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_HB1COEF01_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_HB1COEF02_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_HB1COEF03_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_HB1COEF04_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_HB1COEF05_REG_OFFSET , 0);
    
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_HB2COEF00_REG_OFFSET , 1 << 16);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_HB2COEF01_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_HB2COEF02_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_HB2COEF03_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_HB2COEF04_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_HB2COEF05_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_HB2COEF06_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_HB2COEF07_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_HB2COEF08_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_HB2COEF09_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_HB2COEF10_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_HB2COEF11_REG_OFFSET , 0);

    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_FIRCOEF00_REG_OFFSET , 1 << 16);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_FIRCOEF01_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_FIRCOEF02_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_FIRCOEF03_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_FIRCOEF04_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_FIRCOEF05_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_FIRCOEF06_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_FIRCOEF07_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_FIRCOEF08_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_FIRCOEF09_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_FIRCOEF10_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_FIRCOEF11_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_FIRCOEF12_REG_OFFSET , 0);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_FIRCOEF13_REG_OFFSET , 0);

    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_CONTROL_REG_OFFSET   , 1);

    int const COUNT = 20;

    int count = 0;
    int read_prev = 0;
    int storage[COUNT];
    int finish = 0;
    int fed = 0;
   
    while(finish == 0) {
        uint32_t status = mmio_region_read32(pdm2pcm_base_addr, PDM2PCM_STATUS_REG_OFFSET);
        if (!(status & 1)) {
            int32_t read = mmio_region_read32(pdm2pcm_base_addr, PDM2PCM_RXDATA_REG_OFFSET);
            if (read != 0 || fed == 1) {
                fed = 1;
                storage[count] = read;
                ++count;
                read_prev = read;
                if (count >= COUNT) {
                    finish = 1;
                }
            }
        }
    }
    for (int i = 0; i < COUNT; ++i) {
        printf("%d\n",storage[i]);
    }

    return EXIT_SUCCESS;
}

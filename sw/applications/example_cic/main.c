/*
 * Copyright 2025 EPFL
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
 * Author: Jérémie Moullet <jeremie.moullet@epfl.ch>, EPFL, STI-SEL
 */

#include <stdio.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "pdm2pcm_regs.h"
#include "mmio.h"
#include "groundtruth.h"

#ifndef PDM2PCM_IS_INCLUDED
  #error ( "This app does NOT work as the PDM2PCM peripheral is not included." )
#endif

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif PRINTF_IN_FPGA && !TARGET_SIM
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

int main(int argc, char *argv[])
{


    PRINTF("CIC DEMO\n\r");
    PRINTF(" > Start\n\r");

    mmio_region_t pdm2pcm_base_addr = mmio_region_from_addr((uintptr_t)PDM2PCM_START_ADDRESS);

    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_CLKDIVIDX_REG_OFFSET, 16); // need to be an even number

    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_DECIMCIC_REG_OFFSET, 15);  // Can be odd or even

    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_CIC_ACTIVATED_STAGES_REG_OFFSET, 0b1111);
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_CIC_DELAY_COMB_REG_OFFSET, 1);

    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_CONTROL_REG_OFFSET  , 1);

    int const COUNT = 5;

    int count = 0;
    int finish = 0;
    int fed = 0;

    while(finish == 0) {
        uint32_t status = mmio_region_read32(pdm2pcm_base_addr, PDM2PCM_STATUS_REG_OFFSET);
        if (!(status & 1)) {
            int32_t read = mmio_region_read32(pdm2pcm_base_addr, PDM2PCM_RXDATA_REG_OFFSET);
            if (fed == 1 || read != 0) {
                fed = 1;
                if(pdm2pcm_groundtruth[count] != (int)read) {
                    PRINTF("ERROR: at index %d. read != groundtruth (resp. %d != %d).\n\r",count,(int)read,pdm2pcm_groundtruth[count]);
                    return EXIT_FAILURE;
                }
                ++count;
                if (count >= COUNT) {
                    finish = 1;
                    PRINTF("SUCCESS: Readings correspond to ground truth.\n\r");
                }
            }
        }
    }
    return EXIT_SUCCESS;

}


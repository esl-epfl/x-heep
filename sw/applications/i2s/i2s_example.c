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
#include "handler.h"
#include "rv_plic.h"
#include "rv_plic_regs.h"
#include "csr.h"
#include "hart.h"

int8_t external_intr_flag;

// Interrupt controller variables
dif_plic_params_t rv_plic_params;
dif_plic_t rv_plic;
dif_plic_result_t plic_res;
dif_plic_irq_id_t intr_num;

void handler_irq_external(void) {
    // Claim/clear interrupt
    plic_res = dif_plic_irq_claim(&rv_plic, 0, &intr_num);
    if (plic_res == kDifPlicOk && intr_num == I2S_INTR_EVENT) {
        external_intr_flag = 1;
    }
}


int main(int argc, char *argv[])
{
    printf("I2s DEMO\n");

    printf("--- MEMCOPY EXAMPLE - external peripheral ---\n");

    printf("Init the PLIC...");
    rv_plic_params.base_addr = mmio_region_from_addr((uintptr_t)RV_PLIC_START_ADDRESS);
    plic_res = dif_plic_init(rv_plic_params, &rv_plic);

    if (plic_res == kDifPlicOk) {
        printf("success\n");
    } else {
        printf("fail\n;");
    }

    printf("Set I2s interrupt priority to 1...");
    // Set memcopy priority to 1 (target threshold is by default 0) to trigger an interrupt to the target (the processor)
    plic_res = dif_plic_irq_set_priority(&rv_plic, I2S_INTR_EVENT, 1);
    if (plic_res == kDifPlicOk) {
        printf("success\n");
    } else {
        printf("fail\n;");
    }

    printf("Enable I2s interrupt...");
    plic_res = dif_plic_irq_set_enabled(&rv_plic, I2S_INTR_EVENT, 0, kDifPlicToggleEnabled);
    if (plic_res == kDifPlicOk) {
        printf("Success\n");
    } else {
        printf("Fail\n;");
    }

    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level external interrupts
    const uint32_t mask = 1 << 11; 
    CSR_SET_BITS(CSR_REG_MIE, mask);
    external_intr_flag = 0;





    mmio_region_t base_addr = mmio_region_from_addr((uintptr_t)I2S_START_ADDRESS);

    // enable I2s interrupt
    mmio_region_write32(base_addr, I2S_INTR_ENABLE_REG_OFFSET, 1 << I2S_INTR_ENABLE_I2S_EVENT_BIT);

    mmio_region_write32(base_addr, I2S_CLKDIVIDX_REG_OFFSET, 0x10);
    mmio_region_write32(base_addr, I2S_BYTEPERSAMPLE_REG_OFFSET, I2S_BYTEPERSAMPLE_COUNT_VALUE_32_BITS);
    mmio_region_write32(base_addr, I2S_CFG_REG_OFFSET, 1 << I2S_CFG_EN_BIT | 1 << I2S_CFG_GEN_CLK_WS_BIT);



  
    printf("started hw");

    // // Wait copy is done
    // while(external_intr_flag==0) {
    //     wait_for_interrupt();
    // }
    // printf("finished\n");

    // printf("Complete interrupt...");

    // plic_res = dif_plic_irq_complete(&rv_plic, 0, &intr_num);
    // if (plic_res == kDifPlicOk && intr_num == EXT_INTR_0) {
    //     printf("success\n");
    // } else {
    //     printf("fail\n;");
    // }

    int32_t result;

    for (int32_t i = 0; i < 32; i++) {
        int32_t status; 
        do {
            status = mmio_region_read32(base_addr, I2S_STATUS_REG_OFFSET);
        } while (status & I2S_STATUS_EMPTY_BIT);
        result = mmio_region_read32(base_addr, I2S_RXDATA_REG_OFFSET);
        printf("RX %2d: %d\n", i, result);
    }

    printf("ISR Flag %d", external_intr_flag);

    return EXIT_SUCCESS;
}
// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>

#include "dma.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "csr.h"
#include "rv_plic.h"

#define DMA_REGISTER_SIZE_BYTES sizeof(int)
#define DMA_SELECTION_OFFSET_START 0

static inline void write_register(uint32_t p_val,
                                  uint32_t p_offset,
                                  uint32_t p_mask,
                                  uint8_t p_sel,
                                  dma *peri)
{
    /*
     * The index is computed to avoid needing to access the structure
     * as a structure.
     */
    uint8_t index = p_offset / DMA_REGISTER_SIZE_BYTES;
    /*
     * An intermediate variable "value" is used to prevent writing twice into
     * the register.
     */
    uint32_t value = ((uint32_t *)peri)[index];
    value &= ~(p_mask << p_sel);
    value |= (p_val & p_mask) << p_sel;
    ((uint32_t *)peri)[index] = value;
}

int main(int argc, char *argv[])
{
    // Initialize the core-v-mini-mcu
    int16_t src[5] = {-1, -2, -3, -4, -5};
    int32_t dst[5] = {0};

    dma *peri = dma_peri;

    // Initialize the DMA
    dma_init(peri);

    peri->SRC_PTR = src;
    peri->DST_PTR = dst;

    /*
     * SET THE INCREMENTS
     */

    write_register(2,
                   DMA_SRC_PTR_INC_D1_REG_OFFSET,
                   DMA_SRC_PTR_INC_D1_INC_MASK,
                   DMA_SRC_PTR_INC_D1_INC_OFFSET,
                   peri);

    write_register(4,
                   DMA_DST_PTR_INC_D1_REG_OFFSET,
                   DMA_DST_PTR_INC_D1_INC_MASK,
                   DMA_DST_PTR_INC_D1_INC_OFFSET,
                   peri);

    peri->MODE = DMA_TRANS_MODE_SINGLE;

    write_register(0x1,
                   DMA_SRC_DATA_TYPE_REG_OFFSET,
                   DMA_SRC_DATA_TYPE_DATA_TYPE_MASK,
                   DMA_SRC_DATA_TYPE_DATA_TYPE_OFFSET,
                   peri);

    write_register(0x0,
                   DMA_DST_DATA_TYPE_REG_OFFSET,
                   DMA_DST_DATA_TYPE_DATA_TYPE_MASK,
                   DMA_DST_DATA_TYPE_DATA_TYPE_OFFSET,
                   peri);

    write_register(1,
                   DMA_SIGN_EXT_REG_OFFSET,
                   0x1,
                   DMA_SIGN_EXT_SIGNED_BIT,
                   peri);

    peri->INTERRUPT_EN = 0x1;

    /* Load the size and start the transaction. */
    peri->SIZE_D2 = 0;
    peri->SIZE_D1 = 5 * 4;

    while (!dma_is_ready())
    {
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if (dma_is_ready() == 0)
        {
            wait_for_interrupt();
            // from here we wake up even if we did not jump to the ISR
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    for (int i = 0; i < 5; i++)
    {
        if (src[i] != dst[i])
        {
            printf("Error: src[%d] = %x, dst[%d] = %x\n", i, src[i], i, dst[i]);
            return EXIT_FAILURE;
        }
    }
    printf("Success\n");
    

    return EXIT_SUCCESS;
}

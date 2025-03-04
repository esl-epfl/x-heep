// Copyright 2024 EPFL and Politecnico di Torino.
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: dma_sdk.c
// Author: Michele Caon, Luigi Giuffrida
// Date: 19/06/2023
// Description: Utility functions for DMA peripheral.

#include "dma_sdk.h"
#include "dma.h"
#include "hart.h"
#include "handler.h"
#include "fast_intr_ctrl.h"
#include "core_v_mini_mcu.h"
#include "csr.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /******************************/
    /* ---- GLOBAL VARIABLES ---- */
    /******************************/

    volatile uint8_t dma_sdk_intr_flag;

#define DMA_REGISTER_SIZE_BYTES sizeof(int)
#define DMA_SELECTION_OFFSET_START 0

/* Mask for direct register operations */
#define DMA_CSR_REG_MIE_MASK ((1 << 19) | (1 << 11))

    /**********************************/
    /* ---- FUNCTION DEFINITIONS ---- */
    /**********************************/

    static __attribute__((always_inline)) void dma_start(dma *peri, uint32_t size, dma_data_type_t src_type)
    {
        peri->SIZE_D1 = (uint32_t)((size) & DMA_SIZE_D1_SIZE_MASK);
    }

    // Initialize the DMA
    void dma_sdk_init(void)
    {
        dma_init(NULL);

        /* Enable global interrupts */
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);

        /* Enable fast interrupts */
        CSR_SET_BITS(CSR_REG_MIE, DMA_CSR_REG_MIE_MASK);

        return;
    }

    void dma_copy(uint32_t dst_ptr, uint32_t src_ptr, uint32_t size, uint8_t channel, dma_data_type_t src_type, dma_data_type_t dst_type, uint8_t signed_data)
    {
        volatile dma *the_dma = dma_peri(channel);
        DMA_COPY(dst_ptr, src_ptr, size, src_type, dst_type, signed_data, the_dma);
        dma_start(the_dma, size, src_type);
        DMA_WAIT(channel);
        return;
    }

    void dma_copy_to_addr(uint32_t addr_ptr, uint32_t src_ptr, uint32_t size, uint8_t channel)
    {
        volatile dma *the_dma = dma_peri(channel);
        DMA_COPY_ADDR(addr_ptr, src_ptr, size, the_dma);
        dma_start(the_dma, size, DMA_DATA_TYPE_WORD);
        DMA_WAIT(channel);
        return;
    }

    void dma_fill(uint32_t dst_ptr, uint32_t value_ptr, uint32_t size, uint8_t channel, dma_data_type_t src_type, dma_data_type_t dst_type, uint8_t signed_data)
    {
        volatile dma *the_dma = dma_peri(channel);
        DMA_FILL(dst_ptr, value_ptr, size, src_type, dst_type, signed_data, the_dma);
        dma_start(the_dma, size, src_type);
        DMA_WAIT(channel);
        return;
    }

    void __attribute__ ((noinline)) dma_copy_async(uint32_t dst_ptr, uint32_t src_ptr, uint32_t size, uint8_t channel, dma_data_type_t src_type, dma_data_type_t dst_type, uint8_t signed_data)
    {
        volatile dma *the_dma = dma_peri(channel);
        DMA_COPY(dst_ptr, src_ptr, size, src_type, dst_type, signed_data, the_dma);
        dma_start(the_dma, size, src_type);
        return;
    }

    void __attribute__ ((noinline)) dma_fill_async(uint32_t dst_ptr, uint32_t value_ptr, uint32_t size, uint8_t channel, dma_data_type_t src_type, dma_data_type_t dst_type, uint8_t signed_data)
    {
        volatile dma *the_dma = dma_peri(channel);
        DMA_FILL(dst_ptr, value_ptr, size, src_type, dst_type, signed_data, the_dma);
        dma_start(the_dma, size, src_type);
        return;
    }

    void __attribute__ ((noinline)) dma_wait(uint8_t channel)
    {
        DMA_WAIT(channel);
        return;
    }

#ifdef __cplusplus
}
#endif

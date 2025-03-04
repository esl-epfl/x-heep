// Copyright 2024 EPFL and Politecnico di Torino.
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: dma_sdk.h
// Author: Michele Caon, Luigi Giuffrida
// Date: 19/06/2023
// Description: DMA utility functions

#ifndef DMA_SDK_H_
#define DMA_SDK_H_

#include <stdint.h>
#include <stddef.h> // for size_t
#include "csr.h"

#include "dma.h"
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define INCREMENT(TYPE) ((TYPE == DMA_DATA_TYPE_BYTE) ? 1 : (TYPE == DMA_DATA_TYPE_HALF_WORD) ? 2 \
                                                                                              : 4)

#define DMA_COPY(DST, SRC, SIZE, SRC_TYPE, DST_TYPE, SIGNED, DMA_PERI)                        \
    DMA_PERI->INTERRUPT_EN = (uint32_t)0x1;                                                   \
    DMA_PERI->SRC_PTR = (uint32_t)SRC;                                                        \
    DMA_PERI->DST_PTR = (uint32_t)DST;                                                        \
    DMA_PERI->SRC_PTR_INC_D1 = (uint32_t)(INCREMENT(SRC_TYPE) & DMA_SRC_PTR_INC_D1_INC_MASK); \
    DMA_PERI->DST_PTR_INC_D1 = (uint32_t)(INCREMENT(DST_TYPE) & DMA_DST_PTR_INC_D1_INC_MASK); \
    DMA_PERI->MODE = (uint32_t)(DMA_TRANS_MODE_SINGLE & DMA_MODE_MODE_MASK);                  \
    DMA_PERI->SRC_DATA_TYPE = (uint32_t)(SRC_TYPE & DMA_SRC_DATA_TYPE_DATA_TYPE_MASK);        \
    DMA_PERI->DST_DATA_TYPE = (uint32_t)(DST_TYPE & DMA_DST_DATA_TYPE_DATA_TYPE_MASK);        \
    DMA_PERI->SIGN_EXT = (uint32_t)SIGNED;

#define DMA_FILL(DST, VALUE_PTR, SIZE, SRC_TYPE, DST_TYPE, SIGNED, DMA_PERI)                  \
    DMA_PERI->INTERRUPT_EN = (uint32_t)0x1;                                                   \
    DMA_PERI->SRC_PTR = (uint32_t)VALUE_PTR;                                                  \
    DMA_PERI->DST_PTR = (uint32_t)DST;                                                        \
    DMA_PERI->SRC_PTR_INC_D1 = (uint32_t)0;                                                   \
    DMA_PERI->DST_PTR_INC_D1 = (uint32_t)(INCREMENT(DST_TYPE) & DMA_DST_PTR_INC_D1_INC_MASK); \
    DMA_PERI->MODE = (uint32_t)(DMA_TRANS_MODE_SINGLE & DMA_MODE_MODE_MASK);                  \
    DMA_PERI->SRC_DATA_TYPE = (uint32_t)(SRC_TYPE & DMA_SRC_DATA_TYPE_DATA_TYPE_MASK);        \
    DMA_PERI->DST_DATA_TYPE = (uint32_t)(DST_TYPE & DMA_DST_DATA_TYPE_DATA_TYPE_MASK);        \
    DMA_PERI->SIGN_EXT = (uint32_t)SIGNED;

#define DMA_COPY_ADDR(ADDR, SRC, SIZE, DMA_PERI)                                                        \
    DMA_PERI->INTERRUPT_EN = (uint32_t)0x1;                                                             \
    DMA_PERI->SRC_PTR  = (uint32_t)SRC;                                                                 \
    DMA_PERI->SRC_PTR_INC_D1 = (uint32_t)(INCREMENT(DMA_DATA_TYPE_WORD) & DMA_SRC_PTR_INC_D1_INC_MASK); \
    DMA_PERI->ADDR_PTR = (uint32_t)ADDR;                                                                \
    DMA_PERI->MODE = (uint32_t)(DMA_TRANS_MODE_ADDRESS & DMA_MODE_MODE_MASK);                           \
    DMA_PERI->SRC_DATA_TYPE = (uint32_t)(DMA_DATA_TYPE_WORD & DMA_SRC_DATA_TYPE_DATA_TYPE_MASK);


#define DMA_WAIT(CH)                          \
    while (!dma_is_ready(CH))                 \
    {                                         \
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8); \
        if (dma_is_ready(CH) == 0)            \
        {                                     \
            wait_for_interrupt();             \
        }                                     \
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);   \
    }

    /********************************/
    /* ---- EXPORTED VARIABLES ---- */
    /********************************/

    extern volatile uint8_t dma_sdk_intr_flag;

    /**
     * @brief Initializes the DMA SDK.
     *
     * This function initializes the DMA SDK and prepares it for use.
     */
    void __attribute__((noinline)) dma_sdk_init(void);

    /**
     * @brief Copies data from source to destination using DMA.
     *
     * @param dst_ptr   Pointer to the destination memory location.
     * @param src_ptr   Pointer to the source memory location.
     * @param size      Size of the data to be copied in bytes.
     * @param channel   DMA channel to be used for the transfer.
     * @param src_type  Source variable type (byte, half-word, word).
     * @param dst_type  Destination variable type (byte, half-word, word).
     * @param signed_data  Indicates whether the data is signed or unsigned.
     */
    void __attribute__((noinline)) dma_copy(uint32_t dst_ptr, uint32_t src_ptr, uint32_t size, uint8_t channel, dma_data_type_t src_type, dma_data_type_t dst_type, uint8_t signed_data);

    /**
     * @brief Fills a memory region with a specified value using DMA.
     *
     * @param dst_ptr   Pointer to the destination memory location.
     * @param value_ptr Pointer to the value to be filled.
     * @param size      Size of the memory region to be filled in bytes.
     * @param channel   DMA channel to be used for the transfer.
     * @param src_type  Source variable type (byte, half-word, word).
     * @param dst_type  Destination variable type (byte, half-word, word).
     * @param signed_data  Indicates whether the data is signed or unsigned.
     */
    void __attribute__((noinline)) dma_fill(uint32_t dst_ptr, uint32_t value_ptr, uint32_t size, uint8_t channel, dma_data_type_t src_type, dma_data_type_t dst_type, uint8_t signed_data);

    /**
     * @brief Copies 32-bit data from source to *addr_ptr using DMA.
     *
     * @param addr_ptr  Pointer to the memory location that contains destinations.
     * @param src_ptr   Pointer to the source memory location.
     * @param size      Size of the data to be copied in bytes.
     * @param channel   DMA channel to be used for the transfer.
     */
    void dma_copy_to_addr(uint32_t addr_ptr, uint32_t src_ptr, uint32_t size, uint8_t channel);

    void __attribute__((noinline)) dma_copy_async(uint32_t dst_ptr, uint32_t src_ptr, uint32_t size, uint8_t channel, dma_data_type_t src_type, dma_data_type_t dst_type, uint8_t signed_data);

    void __attribute__((noinline)) dma_fill_async(uint32_t dst_ptr, uint32_t value_ptr, uint32_t size, uint8_t channel, dma_data_type_t src_type, dma_data_type_t dst_type, uint8_t signed_data);

    void __attribute__((noinline)) dma_wait(uint8_t channel);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* DMA_SDK_H_ */

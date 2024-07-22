// Copyright 2024 EPFL and Politecnico di Torino.
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: dma_sdk.h
// Author: Michele Caon
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
#define INCREMENT(C_type) (sizeof(C_type))

#define C_TYPE_2_DMA_TYPE(C_type)                                                                 \
    ((sizeof(C_type)) == 1 ? DMA_DATA_TYPE_BYTE : (sizeof(C_type)) == 2 ? DMA_DATA_TYPE_HALF_WORD \
                                                                        : DMA_DATA_TYPE_WORD)

#define DMA_COPY(DST, SRC, SIZE, C_SRC_TYPE, C_DST_TYPE, SIGNED, DMA_PERI)                                  \
    DMA_PERI->INTERRUPT_EN = (uint32_t)0x1;                                                                 \
    DMA_PERI->SRC_PTR = (uint32_t)SRC;                                                                      \
    DMA_PERI->DST_PTR = (uint32_t)DST;                                                                      \
    DMA_PERI->SRC_PTR_INC_D1 = (uint32_t)(INCREMENT(C_SRC_TYPE) & DMA_SRC_PTR_INC_D1_INC_MASK);             \
    DMA_PERI->DST_PTR_INC_D1 = (uint32_t)(INCREMENT(C_DST_TYPE) & DMA_DST_PTR_INC_D1_INC_MASK);             \
    DMA_PERI->MODE = (uint32_t)(DMA_TRANS_MODE_SINGLE & DMA_MODE_MODE_MASK);                                \
    DMA_PERI->SRC_DATA_TYPE = (uint32_t)(C_TYPE_2_DMA_TYPE(C_SRC_TYPE) & DMA_SRC_DATA_TYPE_DATA_TYPE_MASK); \
    DMA_PERI->DST_DATA_TYPE = (uint32_t)(C_TYPE_2_DMA_TYPE(C_DST_TYPE) & DMA_DST_DATA_TYPE_DATA_TYPE_MASK); \
    DMA_PERI->SIGN_EXT = (uint32_t)SIGNED;                                                                  \
    DMA_PERI->SIZE_D1 = (uint32_t)((SIZE * DMA_DATA_TYPE_2_SIZE(C_TYPE_2_DMA_TYPE(C_SRC_TYPE))) & DMA_SIZE_D1_SIZE_MASK);

#define DMA_FILL(DST, VALUE_PTR, SIZE, C_SRC_TYPE, C_DST_TYPE, SIGNED, DMA_PERI)                            \
    DMA_PERI->INTERRUPT_EN = (uint32_t)0x1;                                                                 \
    DMA_PERI->SRC_PTR = (uint32_t)VALUE_PTR;                                                                \
    DMA_PERI->DST_PTR = (uint32_t)DST;                                                                      \
    DMA_PERI->SRC_PTR_INC_D1 = (uint32_t)0;                                                                 \
    DMA_PERI->DST_PTR_INC_D1 = (uint32_t)(INCREMENT(C_DST_TYPE) & DMA_DST_PTR_INC_D1_INC_MASK);             \
    DMA_PERI->MODE = (uint32_t)(DMA_TRANS_MODE_SINGLE & DMA_MODE_MODE_MASK);                                \
    DMA_PERI->SRC_DATA_TYPE = (uint32_t)(C_TYPE_2_DMA_TYPE(C_SRC_TYPE) & DMA_SRC_DATA_TYPE_DATA_TYPE_MASK); \
    DMA_PERI->DST_DATA_TYPE = (uint32_t)(C_TYPE_2_DMA_TYPE(C_DST_TYPE) & DMA_DST_DATA_TYPE_DATA_TYPE_MASK); \
    DMA_PERI->SIGN_EXT = (uint32_t)SIGNED;                                                                  \
    DMA_PERI->SIZE_D1 = (uint32_t)((SIZE * DMA_DATA_TYPE_2_SIZE(C_TYPE_2_DMA_TYPE(C_SRC_TYPE))) & DMA_SIZE_D1_SIZE_MASK);

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

    /*
     */
    void dma_sdk_init(void);

    /**
     * @brief Copy data words from source address to destination address
     *
     * @param dst Destination address
     * @param src Source address
     * @param size Number of words (not bytes) to copy
     * @param channel DMA channel to use
     */
    void dma_copy_32b(uint32_t *dst, uint32_t *src, uint32_t size, uint8_t channel);

    /**
     * @brief Copy data words from source address to destination address
     *
     * @param dst Destination address
     * @param src Source address
     * @param size Number of words (not bytes) to copy
     * @param channel DMA channel to use
     */
    void dma_copy_16b(uint16_t *dst, uint16_t *src, uint32_t size, uint8_t channel);

    /**
     * @brief Copy data words from source address to destination address
     *
     * @param dst Destination address
     * @param src Source address
     * @param size Number of words (not bytes) to copy
     * @param channel DMA channel to use
     */
    void dma_copy_8b(uint8_t *dst, uint8_t *src, uint32_t size, uint8_t channel);

    /**
     * @brief Copy data from source address to explicit destination addresses
     *
     * @param dst_addr Array of destination addresses
     * @param src Source address
     * @param bytes Number of words (not bytes) to copy
     * @param channel DMA channel to use
     */
    void dma_copy_to_addr_32b(uint32_t *dst_addr, uint32_t *src, uint32_t size, uint8_t channel);

    /**
     * @brief Fill a memory region with a 32-bit value
     *
     * @param dst Destination address
     * @param value Pointer to the value to fill the memory with
     * @param size Number of words (not bytes) to fill
     * @param channel DMA channel to use
     * @return int 0 if success, -1 if error
     */
    void dma_fill_32b(uint32_t *dst, uint32_t *value, uint32_t size, uint8_t channel);

    /**
     * @brief Fill a memory region with a 16-bit value
     *
     * @param dst Destination address
     * @param value Pointer to the value to fill the memory with
     * @param size Number of words (not bytes) to fill
     * @param channel DMA channel to use
     * @return int 0 if success, -1 if error
     */
    void dma_fill_16b(uint16_t *dst, uint16_t *value, uint32_t size, uint8_t channel);

    /**
     * @brief Fill a memory region with a 16-bit value
     *
     * @param dst Destination address
     * @param value Pointer to the value to fill the memory with
     * @param size Number of words (not bytes) to fill
     * @param channel DMA channel to use
     * @param sign_extend 1 to sign extend the 16-bit values, 0 otherwise
     * @return int 0 if success, -1 if error
     */
    void dma_fill_16b_32b(uint32_t *dst, uint16_t *value, uint32_t size, uint8_t channel, uint32_t sign_extend);

    /**
     * @brief Fill a memory region with an 8-bit value
     *
     * @param dst Destination address
     * @param value Pointer to the value to fill the memory with
     * @param size Number of words (not bytes) to fill
     * @param channel DMA channel to use
     * @return int 0 if success, -1 if error
     */
    void dma_fill_8b(uint8_t *dst, uint8_t *value, uint32_t size, uint8_t channel);

    /**
     * @brief Copy data from source address to destination address (16-bit aligned)
     *
     * @param size Number of words (not bytes) to fill
     * @param channel DMA channel to use
     * @param sign_extend 1 to sign extend the 16-bit values, 0 otherwise
     * @return int 0 if success, -1 if error
     */
    void dma_fill_16b_32b(uint32_t *dst, uint16_t *value, uint32_t size, uint8_t channel, uint32_t sign_extend);

    /**
     * @brief Fill a memory region with an 8-bit value
     *
     * @param dst Destination address
     * @param value Pointer to the value to fill the memory with
     * @param size Number of words (not bytes) to fill
     * @param channel DMA channel to use
     * @return int 0 if success, -1 if error
     */
    void dma_fill_8b(uint8_t *dst, uint8_t *value, uint32_t size, uint8_t channel);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* DMA_SDK_H_ */

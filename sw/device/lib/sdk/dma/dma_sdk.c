// Copyright 2024 EPFL and Politecnico di Torino.
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: dma_sdk.c
// Author: Michele Caon
// Date: 19/06/2023
// Description: Utility functions for DMA peripheral.

#include "dma_sdk.h"
#include "dma.h"
#include "hart.h"
#include "handler.h"
#include "fast_intr_ctrl.h"
#include "core_v_mini_mcu.h"
#include "csr.h"

/******************************/
/* ---- GLOBAL VARIABLES ---- */
/******************************/

volatile uint8_t dma_sdk_intr_flag;

/**********************************/
/* ---- FUNCTION DEFINITIONS ---- */
/**********************************/

#ifndef USE_HEEP_DMA_HAL

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

#endif

// Copy data from source to destination using DMA peripheral
void dma_copy_32b(uint32_t *dst, uint32_t *src, uint32_t size)
{

    dma_config_flags_t res;

    dma_target_t tgt_src = {
        .ptr = src,
        .inc_du = 1,
        .size_du = size,
        .trig = DMA_TRIG_MEMORY,
        .type = DMA_DATA_TYPE_WORD,
    };
    dma_target_t tgt_dst = {
        .ptr = dst,
        .inc_du = 1,
        .size_du = size,
        .trig = DMA_TRIG_MEMORY,
        .type = DMA_DATA_TYPE_WORD,
    };

    dma_trans_t trans = {
        .src = &tgt_src,
        .dst = &tgt_dst,
        .src_addr = NULL,
        .mode = DMA_TRANS_MODE_SINGLE,
        .win_du = 0,
        .end = DMA_TRANS_END_INTR,
    };

#ifdef USE_HEEP_DMA_HAL
    res = dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    res = dma_load_transaction(&trans);
    res = dma_launch(&trans);
#else

    dma *peri = dma_peri;

    uint8_t dataSize_b = DMA_DATA_TYPE_2_SIZE(trans.src->type);
    trans.size_b = trans.src->size_du * dataSize_b;
    /* By default, the source defines the data type.*/
    trans.src_type = trans.src->type;

    /*
     * SET THE POINTERS
     */
    peri->SRC_PTR = trans.src->ptr;
    peri->DST_PTR = trans.dst->ptr;

    /*
     * SET THE INCREMENTS
     */

    write_register(4,
                   DMA_SRC_PTR_INC_D1_REG_OFFSET,
                   DMA_SRC_PTR_INC_D1_INC_MASK,
                   DMA_SRC_PTR_INC_D1_INC_OFFSET,
                   peri);

    write_register(4,
                   DMA_DST_PTR_INC_D1_REG_OFFSET,
                   DMA_DST_PTR_INC_D1_INC_MASK,
                   DMA_DST_PTR_INC_D1_INC_OFFSET,
                   peri);

    /*
     * SET THE OPERATION MODE AND WINDOW SIZE
     */

    peri->MODE = trans.mode;

    write_register(trans.src_type,
                   DMA_SRC_DATA_TYPE_REG_OFFSET,
                   DMA_SRC_DATA_TYPE_DATA_TYPE_MASK,
                   DMA_SELECTION_OFFSET_START,
                   peri);

    peri->INTERRUPT_EN = 0x1;

    /* Load the size and start the transaction. */
    peri->SIZE_D1 = trans.size_b;

#endif

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

    return;
}

void dma_fill(uint32_t *dst, uint32_t *value, uint32_t size)
{

    dma *peri = dma_peri;

    /*
     * SET THE POINTERS
     */
    peri->SRC_PTR = value;
    peri->DST_PTR = dst;

    /*
     * SET THE INCREMENTS
     */

    write_register(0,
                   DMA_SRC_PTR_INC_D1_REG_OFFSET,
                   DMA_SRC_PTR_INC_D1_INC_MASK,
                   DMA_SRC_PTR_INC_D1_INC_OFFSET,
                   peri);

    write_register(4,
                   DMA_DST_PTR_INC_D1_REG_OFFSET,
                   DMA_DST_PTR_INC_D1_INC_MASK,
                   DMA_DST_PTR_INC_D1_INC_OFFSET,
                   peri);

    /*
     * SET THE OPERATION MODE AND WINDOW SIZE
     */

    peri->MODE = DMA_TRANS_MODE_SINGLE;

    write_register(DMA_DATA_TYPE_WORD,
                   DMA_SRC_DATA_TYPE_REG_OFFSET,
                   DMA_SRC_DATA_TYPE_DATA_TYPE_MASK,
                   DMA_SELECTION_OFFSET_START,
                   peri);

    peri->INTERRUPT_EN = 0x1;

    /* Load the size and start the transaction. */
    peri->SIZE_D1 = size * sizeof(uint32_t);

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

    return;
}

void dma_copy_16_32(uint32_t *dst, uint16_t *src, uint32_t size)
{

    dma *peri = dma_peri;

    uint8_t dataSize_b = DMA_DATA_TYPE_2_SIZE(DMA_DATA_TYPE_WORD);

    /*
     * SET THE POINTERS
     */
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
                   DMA_SRC_PTR_INC_D1_REG_OFFSET,
                   DMA_DST_PTR_INC_D1_INC_MASK,
                   DMA_DST_PTR_INC_D1_INC_OFFSET,
                   peri);
    /*
     * SET THE OPERATION MODE AND WINDOW SIZE
     */

    peri->MODE = DMA_TRANS_MODE_SINGLE;

    write_register(DMA_DATA_TYPE_HALF_WORD,
                   DMA_SRC_DATA_TYPE_REG_OFFSET,
                   DMA_SRC_DATA_TYPE_DATA_TYPE_MASK,
                   DMA_SELECTION_OFFSET_START,
                   peri);

    peri->INTERRUPT_EN = 0x1;

    /* Load the size and start the transaction. */
    peri->SIZE_D1 = dataSize_b * size;

    // #endif

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

    return;
}


// Copy data from source to destination using DMA peripheral
void dma_copy_to_addr_32b(uint32_t *dst_addr, uint32_t *src, uint32_t size)
{

    dma_config_flags_t res;

    dma_target_t tgt_src = {
        .ptr = src,
        .inc_du = 1,
        .size_du = size,
        .trig = DMA_TRIG_MEMORY,
        .type = DMA_DATA_TYPE_WORD,
    };
    dma_target_t tgt_addr = {
        .ptr = dst_addr,
        .inc_du = 1,
        .size_du = size,
        .trig = DMA_TRIG_MEMORY,
    };

    dma_trans_t trans = {
        .src = &tgt_src,
        .dst = NULL,
        .src_addr = &tgt_addr,
        .mode = DMA_TRANS_MODE_ADDRESS,
        .win_du = 0,
        .end = DMA_TRANS_END_INTR,
    };

#ifdef USE_HEEP_DMA_HAL
    /** TO BE DONE */
#else

    dma *peri = dma_peri;

    uint8_t dataSize_b = DMA_DATA_TYPE_2_SIZE(trans.src->type);
    trans.size_b = trans.src->size_du * dataSize_b;
    /* By default, the source defines the data type.*/
    trans.src_type = trans.src->type;

    /*
     * SET THE POINTERS
     */
    peri->SRC_PTR = trans.src->ptr;
    peri->ADDR_PTR = trans.src_addr->ptr;

    /*
     * SET THE INCREMENTS
     */

    write_register(4,
                   DMA_SRC_PTR_INC_D1_REG_OFFSET,
                   DMA_SRC_PTR_INC_D1_INC_MASK,
                   DMA_SRC_PTR_INC_D1_INC_OFFSET,
                   peri);

    /*
     * SET THE OPERATION MODE AND WINDOW SIZE
     */

    peri->MODE = trans.mode;

    write_register(trans.src_type,
                   DMA_SRC_DATA_TYPE_REG_OFFSET,
                   DMA_SRC_DATA_TYPE_DATA_TYPE_MASK,
                   DMA_SELECTION_OFFSET_START,
                   peri);

    peri->INTERRUPT_EN = 0x1;

    /* Load the size and start the transaction. */
    peri->SIZE_D1 = trans.size_b;

#endif

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

    return;
}

// Copy data from source to destination using DMA peripheral
int dma_copy(const uint8_t *dst, const uint8_t *src, const size_t bytes, const dma_data_type_t type)
{
    // Number of words
    size_t num_du = bytes >> 2;

    // Last bytes
    size_t last_bytes = bytes & 0x3;
    uint8_t *last_src = src + (num_du << 2);
    uint8_t *last_dst = dst + (num_du << 2);

    // DMA configuration
    dma_config_flags_t dma_ret;
    dma_data_type_t dma_type;

    // Check alignment
    if (((uintptr_t)src & 0x3) || ((uintptr_t)dst & 0x3))
    {
        // NOTE: use data units (half words or bytes) instead of words when
        // dealing with unaligned base addresses to avoid alignment issues
        // (unsupported by X-HEEP's DMA. This is 2x or 4x slower than words.
        switch (type)
        {
        case DMA_DATA_TYPE_WORD:
            num_du = bytes >> 2;
            last_bytes = bytes & 0x3;
            last_src = src + (num_du << 2);
            last_dst = dst + (num_du << 2);
            break;
        case DMA_DATA_TYPE_HALF_WORD:
            num_du = bytes >> 1;
            last_bytes = bytes & 0x1;
            last_src = src + (num_du << 1);
            last_dst = dst + (num_du << 1);
            break;
        default:
            num_du = bytes;
            last_bytes = 0;
            break;
        }
        dma_type = type;
    }
    else
    {
        // Use word transactions (faster)
        dma_type = DMA_DATA_TYPE_WORD;
    }

    // Source pointer
    dma_target_t tgt_src = {
        .ptr = src,
        .inc_du = 1,
        .size_du = num_du,
        .trig = DMA_TRIG_MEMORY,
        .type = dma_type,
    };

    // Destination pointer
    dma_target_t tgt_dst = {
        .ptr = dst,
        .inc_du = 1,
        .trig = DMA_TRIG_MEMORY,
    };

    // DMA transaction
    dma_trans_t trans = {
        .src = &tgt_src,
        .dst = &tgt_dst,
        .src_addr = NULL,
        .mode = DMA_TRANS_MODE_SINGLE,
        .win_du = 0,
        .end = DMA_TRANS_END_INTR,
    };

    // Configure and launch DMA transfer
    dma_ret = dma_validate_transaction(&trans, DMA_DO_NOT_ENABLE_REALIGN, DMA_PERFORM_CHECKS_ONLY_SANITY);
    if (dma_ret != DMA_CONFIG_OK)
    {
        return -1;
    }
    dma_ret = dma_load_transaction(&trans);
    if (dma_ret != DMA_CONFIG_OK)
    {
        return -1;
    }
    dma_ret = dma_launch(&trans);
    if (dma_ret != DMA_CONFIG_OK)
    {
        return -1;
    }

    // Wait for DMA transfer to finish
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

    // Manually copy the last bytes (unaligned transfers not supported by DMA)
    for (size_t i = 0; i < last_bytes; i++)
    {
        last_dst[i] = last_src[i];
    }

    return 0;
}

// DMA interrupt handler
void dma_sdk_intr_handler_trans_done()
{
    dma_sdk_intr_flag = 1;
}

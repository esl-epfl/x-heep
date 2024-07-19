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

#ifdef __cplusplus
extern "C" {
#endif

/******************************/
/* ---- GLOBAL VARIABLES ---- */
/******************************/

/* Uncomment to use the DMA HALs */
//#define USE_HEEP_DMA_HAL

volatile uint8_t dma_sdk_intr_flag;

#define DMA_REGISTER_SIZE_BYTES sizeof(int)
#define DMA_SELECTION_OFFSET_START 0

/* Mask for direct register operations */
#define DMA_CSR_REG_MIE_MASK (( 1 << 19 ) | (1 << 11 ))

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

// Initialize the DMA
void dma_sdk_init(void)
{    
    dma_init(NULL);

    #ifndef USE_HEEP_DMA_HAL
    
    /* Enable global interrupts */
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);

    /* Enable fast interrupts */
    CSR_SET_BITS(CSR_REG_MIE, DMA_CSR_REG_MIE_MASK);

    #endif

    return;
}

// Copy data from source to destination using DMA peripheral
void dma_copy_32b(uint32_t *dst, uint32_t *src, uint32_t size, uint8_t channel)
{

    dma_config_flags_t res;

    dma_target_t tgt_src = {
        .ptr = (uint8_t *) src,
        .inc_du = 1,
        .size_du = size,
        .type = DMA_DATA_TYPE_WORD,
        .trig = DMA_TRIG_MEMORY,   
    };
    dma_target_t tgt_dst = {
        .ptr = (uint8_t *)dst,
        .inc_du = 1,
        .size_du = size,
        .type = DMA_DATA_TYPE_WORD,
        .trig = DMA_TRIG_MEMORY,    
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

    dma *peri = dma_peri(channel);

    uint8_t dataSize_b = DMA_DATA_TYPE_2_SIZE(trans.src->type);
    trans.size_b = trans.src->size_du * dataSize_b;
    /* By default, the source defines the data type.*/
    trans.src_type = trans.src->type;

    /*
     * SET THE POINTERS
     */
    peri->SRC_PTR = (uint32_t) trans.src->ptr;
    peri->DST_PTR = (uint32_t) trans.dst->ptr;

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

    while (!dma_is_ready(channel))
    {
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if (dma_is_ready(channel) == 0)
        {
            wait_for_interrupt();
            // from here we wake up even if we did not jump to the ISR
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    return;
}

// Copy data from source to destination using DMA peripheral
void dma_copy_16b(uint32_t *dst, uint32_t *src, uint32_t size, uint8_t channel)
{

    dma_config_flags_t res;

    dma_target_t tgt_src = {
        .ptr = (uint8_t *) src,
        .inc_du = 1,
        .size_du = size,
        .trig = DMA_TRIG_MEMORY,
        .type = DMA_DATA_TYPE_HALF_WORD,
    };
    dma_target_t tgt_dst = {
        .ptr = (uint8_t *) dst,
        .inc_du = 1,
        .size_du = size,
        .trig = DMA_TRIG_MEMORY,
        .type = DMA_DATA_TYPE_HALF_WORD,
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

    dma *peri = dma_peri(channel);

    uint8_t dataSize_b = DMA_DATA_TYPE_2_SIZE(trans.src->type);
    trans.size_b = trans.src->size_du * dataSize_b;
    /* By default, the source defines the data type.*/
    trans.src_type = trans.src->type;
    trans.dst_type = trans.dst->type;

    /*
     * SET THE POINTERS
     */
    peri->SRC_PTR = (uint32_t) trans.src->ptr;
    peri->DST_PTR = (uint32_t) trans.dst->ptr;

    /*
     * SET THE INCREMENTS
     */

    write_register(2,
                   DMA_SRC_PTR_INC_D1_REG_OFFSET,
                   DMA_SRC_PTR_INC_D1_INC_MASK,
                   DMA_SRC_PTR_INC_D1_INC_OFFSET,
                   peri);

    write_register(2,
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
    
    write_register(trans.dst_type,
                   DMA_SRC_DATA_TYPE_REG_OFFSET,
                   DMA_SRC_DATA_TYPE_DATA_TYPE_MASK,
                   DMA_SELECTION_OFFSET_START,
                   peri);

    peri->INTERRUPT_EN = 0x1;

    /* Load the size and start the transaction. */
    peri->SIZE_D1 = trans.size_b;

#endif

    while (!dma_is_ready(channel))
    {
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if (dma_is_ready(channel) == 0)
        {
            wait_for_interrupt();
            // from here we wake up even if we did not jump to the ISR
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    return;
}

// Copy data from source to destination using DMA peripheral
void dma_copy_8b(uint32_t *dst, uint32_t *src, uint32_t size, uint8_t channel)
{

    dma_config_flags_t res;

    dma_target_t tgt_src = {
        .ptr = (uint8_t *) src,
        .inc_du = 1,
        .size_du = size,
        .trig = DMA_TRIG_MEMORY,
        .type = DMA_DATA_TYPE_BYTE,
    };
    dma_target_t tgt_dst = {
        .ptr = (uint8_t *) dst,
        .inc_du = 1,
        .size_du = size,
        .trig = DMA_TRIG_MEMORY,
        .type = DMA_DATA_TYPE_BYTE,
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

    dma *peri = dma_peri(channel);

    uint8_t dataSize_b = DMA_DATA_TYPE_2_SIZE(trans.src->type);
    trans.size_b = trans.src->size_du * dataSize_b;
    /* By default, the source defines the data type.*/
    trans.src_type = trans.src->type;

    /*
     * SET THE POINTERS
     */
    peri->SRC_PTR = (uint32_t) trans.src->ptr;
    peri->DST_PTR = (uint32_t) trans.dst->ptr;

    /*
     * SET THE INCREMENTS
     */

    write_register(1,
                   DMA_SRC_PTR_INC_D1_REG_OFFSET,
                   DMA_SRC_PTR_INC_D1_INC_MASK,
                   DMA_SRC_PTR_INC_D1_INC_OFFSET,
                   peri);

    write_register(1,
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

    while (!dma_is_ready(channel))
    {
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if (dma_is_ready(channel) == 0)
        {
            wait_for_interrupt();
            // from here we wake up even if we did not jump to the ISR
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    return;
}

void dma_fill_32b(uint32_t *dst, uint32_t *value, uint32_t size, uint8_t channel)
{
    dma_config_flags_t res;

    dma_target_t tgt_src = {
        .ptr = (uint8_t *) value,
        .inc_du = 0,
        .size_du = size,
        .type = DMA_DATA_TYPE_WORD,
        .trig = DMA_TRIG_MEMORY    
    };
    dma_target_t tgt_dst = {
        .ptr = (uint8_t *) dst,
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
        .end = DMA_TRANS_END_INTR
    };

    #ifdef USE_HEEP_DMA_HAL
    res = dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    res = dma_load_transaction(&trans);
    res = dma_launch(&trans);
    #else

    dma *peri = dma_peri(channel);

    /*
     * SET THE POINTERS
     */
    peri->SRC_PTR = (uint32_t) trans.src->ptr;
    peri->DST_PTR = (uint32_t) trans.dst->ptr;

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

    peri->MODE = trans.mode;

    write_register(trans.src_type,
                   DMA_SRC_DATA_TYPE_REG_OFFSET,
                   DMA_SRC_DATA_TYPE_DATA_TYPE_MASK,
                   DMA_SELECTION_OFFSET_START,
                   peri);

    peri->INTERRUPT_EN = 0x1;

    /* Load the size and start the transaction. */
    peri->SIZE_D1 = size * sizeof(uint32_t);

    #endif

    while (!dma_is_ready(channel))
    {
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if (dma_is_ready(channel) == 0)
        {
            wait_for_interrupt();
            // from here we wake up even if we did not jump to the ISR
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    return;
}

void dma_fill_16b(uint16_t *dst, uint16_t *value, uint32_t size, uint8_t channel)
{
    dma_config_flags_t res;

    dma_target_t tgt_src = {
        .ptr = (uint8_t *) value,
        .inc_du = 0,
        .size_du = size,
        .trig = DMA_TRIG_MEMORY,
        .type = DMA_DATA_TYPE_HALF_WORD,
    };
    dma_target_t tgt_dst = {
        .ptr = (uint8_t *) dst,
        .inc_du = 1,
        .size_du = size,
        .trig = DMA_TRIG_MEMORY,
        .type = DMA_DATA_TYPE_HALF_WORD,
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

    dma *peri = dma_peri(channel);

    trans.src_type = trans.src->type;
    trans.dst_type = trans.dst->type;

    /*
     * SET THE POINTERS
     */
    peri->SRC_PTR = (uint32_t) trans.src->ptr;
    peri->DST_PTR = (uint32_t) trans.dst->ptr;

    /*
     * SET THE INCREMENTS
     */

    write_register(0,
                   DMA_SRC_PTR_INC_D1_REG_OFFSET,
                   DMA_SRC_PTR_INC_D1_INC_MASK,
                   DMA_SRC_PTR_INC_D1_INC_OFFSET,
                   peri);

    write_register(2,
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

    write_register(trans.dst_type,
                   DMA_DST_DATA_TYPE_REG_OFFSET,
                   DMA_DST_DATA_TYPE_DATA_TYPE_MASK,
                   DMA_SELECTION_OFFSET_START,
                   peri);

    peri->INTERRUPT_EN = 0x1;

    /* Load the size and start the transaction. */
    peri->SIZE_D1 = size * sizeof(uint16_t);

    #endif

    while (!dma_is_ready(channel))
    {
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if (dma_is_ready(channel) == 0)
        {
            wait_for_interrupt();
            // from here we wake up even if we did not jump to the ISR
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    return;
}

void dma_fill_8b(uint8_t *dst, uint8_t *value, uint32_t size, uint8_t channel)
{
    dma_config_flags_t res;

    dma_target_t tgt_src = {
        .ptr = (uint8_t *) value,
        .inc_du = 0,
        .size_du = size,
        .trig = DMA_TRIG_MEMORY,
        .type = DMA_DATA_TYPE_BYTE,
    };
    dma_target_t tgt_dst = {
        .ptr = (uint8_t *) dst,
        .inc_du = 1,
        .size_du = size,
        .trig = DMA_TRIG_MEMORY,
        .type = DMA_DATA_TYPE_BYTE,
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

    dma *peri = dma_peri(channel);

    trans.src_type = trans.src->type;
    trans.dst_type = trans.dst->type;

    /*
     * SET THE POINTERS
     */
    peri->SRC_PTR = (uint32_t) trans.src->ptr;
    peri->DST_PTR = (uint32_t) trans.dst->ptr;

    /*
     * SET THE INCREMENTS
     */

    write_register(0,
                   DMA_SRC_PTR_INC_D1_REG_OFFSET,
                   DMA_SRC_PTR_INC_D1_INC_MASK,
                   DMA_SRC_PTR_INC_D1_INC_OFFSET,
                   peri);

    write_register(1,
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

    write_register(trans.dst_type,
                   DMA_DST_DATA_TYPE_REG_OFFSET,
                   DMA_DST_DATA_TYPE_DATA_TYPE_MASK,
                   DMA_SELECTION_OFFSET_START,
                   peri);

    peri->INTERRUPT_EN = 0x1;

    /* Load the size and start the transaction. */
    peri->SIZE_D1 = size * sizeof(uint8_t);

    #endif

    while (!dma_is_ready(channel))
    {
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if (dma_is_ready(channel) == 0)
        {
            wait_for_interrupt();
            // from here we wake up even if we did not jump to the ISR
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    return;
}



void dma_copy_16_32(uint32_t *dst, uint16_t *src, uint32_t size, uint8_t channel)
{

    dma *peri = dma_peri(channel);

    uint8_t dataSize_b = DMA_DATA_TYPE_2_SIZE(DMA_DATA_TYPE_WORD);

    /*
     * SET THE POINTERS
     */
    peri->SRC_PTR = (uint32_t) src;
    peri->DST_PTR = (uint32_t) dst;

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

    while (!dma_is_ready(channel))
    {
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if (dma_is_ready(channel) == 0)
        {
            wait_for_interrupt();
            // from here we wake up even if we did not jump to the ISR
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    return;
}


// Copy data from source to destination using DMA peripheral
void dma_copy_to_addr_32b(uint32_t *dst_addr, uint32_t *src, uint32_t size, uint8_t channel)
{

    dma_config_flags_t res;

    dma_target_t tgt_src = {
        .ptr = (uint8_t *) src,
        .inc_du = 1,
        .size_du = size,
        .trig = DMA_TRIG_MEMORY,
        .type = DMA_DATA_TYPE_WORD,
    };
    dma_target_t tgt_addr = {
        .ptr = (uint8_t *) dst_addr,
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

    dma *peri = dma_peri(channel);

    uint8_t dataSize_b = DMA_DATA_TYPE_2_SIZE(trans.src->type);
    trans.size_b = trans.src->size_du * dataSize_b;
    /* By default, the source defines the data type.*/
    trans.src_type = trans.src->type;

    /*
     * SET THE POINTERS
     */
    peri->SRC_PTR = (uint32_t) trans.src->ptr;
    peri->ADDR_PTR = (uint32_t) trans.src_addr->ptr;

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

    while (!dma_is_ready(channel))
    {
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if (dma_is_ready(channel) == 0)
        {
            wait_for_interrupt();
            // from here we wake up even if we did not jump to the ISR
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    return;
}

#ifdef __cplusplus
}
#endif

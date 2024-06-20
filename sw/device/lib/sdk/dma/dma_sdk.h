// Copyright 2024 EPFL and Politecnico di Torino.
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: dma_sdk.h
// Author: Michele Caon
// Date: 19/06/2023
// Description: DMA utility functions

#ifndef DMA_UTIL_H_
#define DMA_UTIL_H_

#include <stdint.h>
#include <stddef.h> // for size_t

#include "dma.h"
#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
/********************************/
/* ---- EXPORTED VARIABLES ---- */
/********************************/

extern volatile uint8_t dma_sdk_intr_flag;

/********************************/
/* ---- EXPORTED FUNCTIONS ---- */
/********************************/

/**
 * @brief Copy data words from source address to destination address
 * 
 * @param dst Destination address
 * @param src Source address
 * @param size Number of words (not bytes) to copy
 */
void dma_copy_32b(uint32_t *dst, uint32_t *src, uint32_t size);

/**
 * @brief Copy data from source address to explicit destination addresses
 * 
 * @param dst_addr Array of destination addresses
 * @param src Source address
 * @param bytes Number of words (not bytes) to copy
 */
void dma_copy_to_addr_32b(uint32_t *dst_addr, uint32_t *src, uint32_t size);

/**
 * @brief Copy data from source address to destination address
 * 
 * @param dst Destination address
 * @param src Source address
 * @param bytes Number of bytes to copy
 * @param type Data type (DMA_DATA_TYPE_WORD, DMA_DATA_TYPE_HALF_WORD, DMA_DATA_TYPE_BYTE)
 * @return int 0 if success, -1 if error
 */
int dma_copy(const uint8_t *dst, const uint8_t *src, const size_t bytes, const dma_data_type_t type);

/**
 * @brief Fill a memory region with a 32-bit value
 * 
 * @param dst Destination address
 * @param value Pointer to the value to fill the memory with
 * @param size Number of words (not bytes) to fill
 * @return int 0 if success, -1 if error
 */
void dma_fill(uint32_t *dst, uint32_t *value, uint32_t size);

/**
 * @brief Copy data from source address to destination address (16-bit aligned) [BROKEN until siigned DMA works]
 * 
 * @param dst Destination address (32-bit aligned)
 * @param src Source address (16-bit aligned)
 * @param size Number of bytes to copy
 */
void dma_copy_16_32(uint32_t *dst, uint16_t *src, uint32_t size);

/**
 * @brief DMA interrupt handler (overrides the weak one from dma.c)
 * 
 */
void dma_sdk_intr_handler_trans_done();
#ifdef __cplusplus
}
#endif  // __cplusplus
#endif /* DMA_UTIL_H_ */


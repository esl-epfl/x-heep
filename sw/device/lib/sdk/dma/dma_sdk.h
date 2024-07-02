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
#include "csr.h"

#include "dma.h"

/********************************/
/* ---- EXPORTED VARIABLES ---- */
/********************************/

extern volatile uint8_t dma_sdk_intr_flag;

/********************************/
/* ---- EXPORTED FUNCTIONS ---- */
/********************************/

/**
 * @brief Initialize the DMA controller
 * 
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
void dma_copy_16b(uint32_t *dst, uint32_t *src, uint32_t size, uint8_t channel);

/**
 * @brief Copy data words from source address to destination address
 * 
 * @param dst Destination address
 * @param src Source address
 * @param size Number of words (not bytes) to copy
 * @param channel DMA channel to use
 */
void dma_copy_8b(uint32_t *dst, uint32_t *src, uint32_t size, uint8_t channel);

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
 * @brief Copy data from source address to destination address (16-bit aligned) [BROKEN until siigned DMA works]
 * 
 * @param dst Destination address (32-bit aligned)
 * @param src Source address (16-bit aligned)
 * @param size Number of bytes to copy
 * @param channel DMA channel to use
 */
void dma_copy_16_32(uint32_t *dst, uint16_t *src, uint32_t size, uint8_t channel);

#endif /* DMA_UTIL_H_ */

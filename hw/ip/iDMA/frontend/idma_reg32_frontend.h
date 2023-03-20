// Generated register defines for idma_reg32_frontend

// Copyright information found in source file:
// Copyright 2022 ETH Zurich and University of Bologna.

// Licensing information found in source file:
// Licensed under Solderpad Hardware License, Version 0.51
// SPDX-License-Identifier: SHL-0.51

#ifndef _IDMA_REG32_FRONTEND_REG_DEFS_
#define _IDMA_REG32_FRONTEND_REG_DEFS_

#ifdef __cplusplus
extern "C" {
#endif
// Register width
#define IDMA_REG32_FRONTEND_PARAM_REG_WIDTH 32

// Source Address
#define IDMA_REG32_FRONTEND_SRC_ADDR_REG_OFFSET 0x0

// Destination Address
#define IDMA_REG32_FRONTEND_DST_ADDR_REG_OFFSET 0x4

// Number of bytes
#define IDMA_REG32_FRONTEND_NUM_BYTES_REG_OFFSET 0x8

// Configuration Register for DMA settings
#define IDMA_REG32_FRONTEND_CONF_REG_OFFSET 0xc
#define IDMA_REG32_FRONTEND_CONF_DECOUPLE_BIT 0
#define IDMA_REG32_FRONTEND_CONF_DEBURST_BIT 1

// DMA Status
#define IDMA_REG32_FRONTEND_STATUS_REG_OFFSET 0x10
#define IDMA_REG32_FRONTEND_STATUS_BUSY_BIT 0

// Next ID, launches transfer, returns 0 if transfer not set up properly.
#define IDMA_REG32_FRONTEND_NEXT_ID_REG_OFFSET 0x14

// Get ID of finished transactions.
#define IDMA_REG32_FRONTEND_DONE_REG_OFFSET 0x18

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _IDMA_REG32_FRONTEND_REG_DEFS_
// End generated register defines for idma_reg32_frontend
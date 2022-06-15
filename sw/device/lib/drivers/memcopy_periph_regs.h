// Generated register defines for memcopy_periph

// Copyright information found in source file:
// Copyright 2022 OpenHW Group

// Licensing information found in source file:
// None
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#ifndef _MEMCOPY_PERIPH_REG_DEFS_
#define _MEMCOPY_PERIPH_REG_DEFS_

#ifdef __cplusplus
extern "C" {
#endif
// Register width
#define MEMCOPY_PERIPH_PARAM_REG_WIDTH 32

// Read data pointer
#define MEMCOPY_PERIPH_PTR_READ_REG_OFFSET 0x0

// Write data pointer
#define MEMCOPY_PERIPH_PTR_WRITE_REG_OFFSET 0x4

// Number of data to copy - Once a value is written, the copy starts
#define MEMCOPY_PERIPH_CNT_START_REG_OFFSET 0x8

// Register set to 1 when copy is done.
#define MEMCOPY_PERIPH_DONE_REG_OFFSET 0xc

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _MEMCOPY_PERIPH_REG_DEFS_
// End generated register defines for memcopy_periph
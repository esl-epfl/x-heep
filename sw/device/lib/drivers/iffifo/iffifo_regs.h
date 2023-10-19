// Generated register defines for iffifo

// Copyright information found in source file:
// Copyright EPFL contributors.

// Licensing information found in source file:
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef _IFFIFO_REG_DEFS_
#define _IFFIFO_REG_DEFS_

#ifdef __cplusplus
extern "C" {
#endif
// Register width
#define IFFIFO_PARAM_REG_WIDTH 32

// Does nothing.
#define IFFIFO_DUMMYR_REG_OFFSET 0x0

// Does nothing.
#define IFFIFO_DUMMYW_REG_OFFSET 0x4

// Memory area: Data coming from the FIFO (Fifo Output/Software RX)
#define IFFIFO_FIFO_OUT_REG_OFFSET 0x8
#define IFFIFO_FIFO_OUT_SIZE_WORDS 1
#define IFFIFO_FIFO_OUT_SIZE_BYTES 4
// Memory area: Data sent to the FIFO (Fifo Input/Software TX)
#define IFFIFO_FIFO_IN_REG_OFFSET 0xc
#define IFFIFO_FIFO_IN_SIZE_WORDS 1
#define IFFIFO_FIFO_IN_SIZE_BYTES 4
#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _IFFIFO_REG_DEFS_
// End generated register defines for iffifo
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

// Data coming from the FIFO (Fifo Output/Software RX)
#define IFFIFO_FIFO_OUT_REG_OFFSET 0x0

// Data sent to the FIFO (Fifo Input/Software TX)
#define IFFIFO_FIFO_IN_REG_OFFSET 0x4

// General purpose status register
#define IFFIFO_STATUS_REG_OFFSET 0x8
#define IFFIFO_STATUS_EMPTY_BIT 0
#define IFFIFO_STATUS_AVAILABLE_BIT 1
#define IFFIFO_STATUS_REACHED_BIT 2
#define IFFIFO_STATUS_FULL_BIT 3

// Current number of occupied FIFO slots
#define IFFIFO_OCCUPANCY_REG_OFFSET 0xc

// FIFO occupancy at which the STATUS:REACHED bit is asserted
#define IFFIFO_WATERMARK_REG_OFFSET 0x10

// Write any value to assert an interrupt. Write 0 or 1 to disable or enable
// an interrupt.
#define IFFIFO_INTERRUPTS_REG_OFFSET 0x14
#define IFFIFO_INTERRUPTS_REACHED_BIT 0

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _IFFIFO_REG_DEFS_
// End generated register defines for iffifo
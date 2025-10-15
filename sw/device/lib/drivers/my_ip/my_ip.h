// Generated register defines for my_ip

// Copyright information found in source file:
// Copyright EPFL and Politecnico di Torino contributors.

// Licensing information found in source file:
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef _MY_IP_REG_DEFS_
#define _MY_IP_REG_DEFS_

#ifdef __cplusplus
extern "C" {
#endif
// Register width
#define MY_IP_PARAM_REG_WIDTH 32

// Common Interrupt Offsets
#define MY_IP_INTR_COMMON_MY_IP_EVENT_BIT 0

// Interrupt State Register
#define MY_IP_INTR_STATE_REG_OFFSET 0x0
#define MY_IP_INTR_STATE_MY_IP_EVENT_BIT 0

// Interrupt Enable Register
#define MY_IP_INTR_ENABLE_REG_OFFSET 0x4
#define MY_IP_INTR_ENABLE_MY_IP_EVENT_BIT 0

// Interrupt Test Register
#define MY_IP_INTR_TEST_REG_OFFSET 0x8
#define MY_IP_INTR_TEST_MY_IP_EVENT_BIT 0

// Test register for wr
#define MY_IP_TEST_REG_W_REG_OFFSET 0xc

// Test register for wr
#define MY_IP_TEST_REG_W2_REG_OFFSET 0x10

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _MY_IP_REG_DEFS_
// End generated register defines for my_ip
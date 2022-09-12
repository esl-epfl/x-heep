// Generated register defines for power_manager

// Copyright information found in source file:
// Copyright lowRISC contributors.

// Licensing information found in source file:
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef _POWER_MANAGER_REG_DEFS_
#define _POWER_MANAGER_REG_DEFS_

#ifdef __cplusplus
extern "C" {
#endif
// Register width
#define POWER_MANAGER_PARAM_REG_WIDTH 32

// Used to power gate domain
#define POWER_MANAGER_POWER_GATE_DOMAIN_REG_OFFSET 0x0

// Wake-up state of the system
#define POWER_MANAGER_WAKEUP_STATE_REG_OFFSET 0x4
#define POWER_MANAGER_WAKEUP_STATE_WAKEUP_STATE_BIT 0

// Restore xddress value
#define POWER_MANAGER_RESTORE_ADDRESS_REG_OFFSET 0x8

// Core reg x1 value
#define POWER_MANAGER_CORE_REG_X1_REG_OFFSET 0xc

// Core reg x2 value
#define POWER_MANAGER_CORE_REG_X2_REG_OFFSET 0x10

// Core reg x3 value
#define POWER_MANAGER_CORE_REG_X3_REG_OFFSET 0x14

// Core reg x4 value
#define POWER_MANAGER_CORE_REG_X4_REG_OFFSET 0x18

// Core reg x5 value
#define POWER_MANAGER_CORE_REG_X5_REG_OFFSET 0x1c

// Core reg x6 value
#define POWER_MANAGER_CORE_REG_X6_REG_OFFSET 0x20

// Core reg x7 value
#define POWER_MANAGER_CORE_REG_X7_REG_OFFSET 0x24

// Core reg x8 value
#define POWER_MANAGER_CORE_REG_X8_REG_OFFSET 0x28

// Core reg x9 value
#define POWER_MANAGER_CORE_REG_X9_REG_OFFSET 0x2c

// Core reg x10 value
#define POWER_MANAGER_CORE_REG_X10_REG_OFFSET 0x30

// Core reg x11 value
#define POWER_MANAGER_CORE_REG_X11_REG_OFFSET 0x34

// Core reg x12 value
#define POWER_MANAGER_CORE_REG_X12_REG_OFFSET 0x38

// Core reg x13 value
#define POWER_MANAGER_CORE_REG_X13_REG_OFFSET 0x3c

// Core reg x14 value
#define POWER_MANAGER_CORE_REG_X14_REG_OFFSET 0x40

// Core reg x15 value
#define POWER_MANAGER_CORE_REG_X15_REG_OFFSET 0x44

// Core reg x16 value
#define POWER_MANAGER_CORE_REG_X16_REG_OFFSET 0x48

// Core reg x17 value
#define POWER_MANAGER_CORE_REG_X17_REG_OFFSET 0x4c

// Core reg x18 value
#define POWER_MANAGER_CORE_REG_X18_REG_OFFSET 0x50

// Core reg x19 value
#define POWER_MANAGER_CORE_REG_X19_REG_OFFSET 0x54

// Core reg x20 value
#define POWER_MANAGER_CORE_REG_X20_REG_OFFSET 0x58

// Core reg x21 value
#define POWER_MANAGER_CORE_REG_X21_REG_OFFSET 0x5c

// Core reg x22 value
#define POWER_MANAGER_CORE_REG_X22_REG_OFFSET 0x60

// Core reg x23 value
#define POWER_MANAGER_CORE_REG_X23_REG_OFFSET 0x64

// Core reg x24 value
#define POWER_MANAGER_CORE_REG_X24_REG_OFFSET 0x68

// Core reg x25 value
#define POWER_MANAGER_CORE_REG_X25_REG_OFFSET 0x6c

// Core reg x26 value
#define POWER_MANAGER_CORE_REG_X26_REG_OFFSET 0x70

// Core reg x27 value
#define POWER_MANAGER_CORE_REG_X27_REG_OFFSET 0x74

// Core reg x28 value
#define POWER_MANAGER_CORE_REG_X28_REG_OFFSET 0x78

// Core reg x29 value
#define POWER_MANAGER_CORE_REG_X29_REG_OFFSET 0x7c

// Core reg x30 value
#define POWER_MANAGER_CORE_REG_X30_REG_OFFSET 0x80

// Core reg x31 value
#define POWER_MANAGER_CORE_REG_X31_REG_OFFSET 0x84

// Core csr c0 value
#define POWER_MANAGER_CORE_CSR_C0_REG_OFFSET 0x88

// Core csr c1 value
#define POWER_MANAGER_CORE_CSR_C1_REG_OFFSET 0x8c

// Core csr c2 value
#define POWER_MANAGER_CORE_CSR_C2_REG_OFFSET 0x90

// Core csr c3 value
#define POWER_MANAGER_CORE_CSR_C3_REG_OFFSET 0x94

// Core csr c4 value
#define POWER_MANAGER_CORE_CSR_C4_REG_OFFSET 0x98

// Core csr c5 value
#define POWER_MANAGER_CORE_CSR_C5_REG_OFFSET 0x9c

// Core csr c6 value
#define POWER_MANAGER_CORE_CSR_C6_REG_OFFSET 0xa0

// Core csr c7 value
#define POWER_MANAGER_CORE_CSR_C7_REG_OFFSET 0xa4

// Enable wait for interrupt
#define POWER_MANAGER_EN_WAIT_FOR_INTR_REG_OFFSET 0xa8

// Interrupt state
#define POWER_MANAGER_INTR_STATE_REG_OFFSET 0xac

// Counter before resetting the CPU
#define POWER_MANAGER_CPU_RESET_ASSERT_COUNTER_REG_OFFSET 0xb0

// Counter before unreset the CPU
#define POWER_MANAGER_CPU_RESET_DEASSERT_COUNTER_REG_OFFSET 0xb4

// Counter before switching off the CPU
#define POWER_MANAGER_CPU_SWITCH_OFF_COUNTER_REG_OFFSET 0xb8

// Counter before switching on the CPU
#define POWER_MANAGER_CPU_SWITCH_ON_COUNTER_REG_OFFSET 0xbc

// Bits to stop the counters keeping the done_o signal high
#define POWER_MANAGER_CPU_COUNTERS_STOP_REG_OFFSET 0xc0
#define POWER_MANAGER_CPU_COUNTERS_STOP_CPU_RESET_ASSERT_STOP_BIT_COUNTER_BIT 0
#define POWER_MANAGER_CPU_COUNTERS_STOP_CPU_RESET_DEASSERT_STOP_BIT_COUNTER_BIT \
  1
#define POWER_MANAGER_CPU_COUNTERS_STOP_CPU_SWITCH_OFF_STOP_BIT_COUNTER_BIT 2
#define POWER_MANAGER_CPU_COUNTERS_STOP_CPU_SWITCH_ON_STOP_BIT_COUNTER_BIT 3

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _POWER_MANAGER_REG_DEFS_
// End generated register defines for power_manager
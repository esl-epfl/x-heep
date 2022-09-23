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

// Used to power gate core
#define POWER_MANAGER_POWER_GATE_CORE_REG_OFFSET 0x0
#define POWER_MANAGER_POWER_GATE_CORE_POWER_GATE_CORE_BIT 0

// Used to power gate peripheral_subsystem
#define POWER_MANAGER_POWER_GATE_PERIPH_REG_OFFSET 0x4
#define POWER_MANAGER_POWER_GATE_PERIPH_POWER_GATE_PERIPH_BIT 0

// Used to power gate ram block 0
#define POWER_MANAGER_POWER_GATE_RAM_BLOCK_0_REG_OFFSET 0x8
#define POWER_MANAGER_POWER_GATE_RAM_BLOCK_0_POWER_GATE_RAM_BLOCK_0_BIT 0

// Used to power gate ram block 1
#define POWER_MANAGER_POWER_GATE_RAM_BLOCK_1_REG_OFFSET 0xc
#define POWER_MANAGER_POWER_GATE_RAM_BLOCK_1_POWER_GATE_RAM_BLOCK_1_BIT 0

// Used to power gate ram block 2
#define POWER_MANAGER_POWER_GATE_RAM_BLOCK_2_REG_OFFSET 0x10
#define POWER_MANAGER_POWER_GATE_RAM_BLOCK_2_POWER_GATE_RAM_BLOCK_2_BIT 0

// Used to power gate ram block 3
#define POWER_MANAGER_POWER_GATE_RAM_BLOCK_3_REG_OFFSET 0x14
#define POWER_MANAGER_POWER_GATE_RAM_BLOCK_3_POWER_GATE_RAM_BLOCK_3_BIT 0

// Wake-up state of the system
#define POWER_MANAGER_WAKEUP_STATE_REG_OFFSET 0x18
#define POWER_MANAGER_WAKEUP_STATE_WAKEUP_STATE_BIT 0

// Restore xddress value
#define POWER_MANAGER_RESTORE_ADDRESS_REG_OFFSET 0x1c

// Core reg x1 value
#define POWER_MANAGER_CORE_REG_X1_REG_OFFSET 0x20

// Core reg x2 value
#define POWER_MANAGER_CORE_REG_X2_REG_OFFSET 0x24

// Core reg x3 value
#define POWER_MANAGER_CORE_REG_X3_REG_OFFSET 0x28

// Core reg x4 value
#define POWER_MANAGER_CORE_REG_X4_REG_OFFSET 0x2c

// Core reg x5 value
#define POWER_MANAGER_CORE_REG_X5_REG_OFFSET 0x30

// Core reg x6 value
#define POWER_MANAGER_CORE_REG_X6_REG_OFFSET 0x34

// Core reg x7 value
#define POWER_MANAGER_CORE_REG_X7_REG_OFFSET 0x38

// Core reg x8 value
#define POWER_MANAGER_CORE_REG_X8_REG_OFFSET 0x3c

// Core reg x9 value
#define POWER_MANAGER_CORE_REG_X9_REG_OFFSET 0x40

// Core reg x10 value
#define POWER_MANAGER_CORE_REG_X10_REG_OFFSET 0x44

// Core reg x11 value
#define POWER_MANAGER_CORE_REG_X11_REG_OFFSET 0x48

// Core reg x12 value
#define POWER_MANAGER_CORE_REG_X12_REG_OFFSET 0x4c

// Core reg x13 value
#define POWER_MANAGER_CORE_REG_X13_REG_OFFSET 0x50

// Core reg x14 value
#define POWER_MANAGER_CORE_REG_X14_REG_OFFSET 0x54

// Core reg x15 value
#define POWER_MANAGER_CORE_REG_X15_REG_OFFSET 0x58

// Core reg x16 value
#define POWER_MANAGER_CORE_REG_X16_REG_OFFSET 0x5c

// Core reg x17 value
#define POWER_MANAGER_CORE_REG_X17_REG_OFFSET 0x60

// Core reg x18 value
#define POWER_MANAGER_CORE_REG_X18_REG_OFFSET 0x64

// Core reg x19 value
#define POWER_MANAGER_CORE_REG_X19_REG_OFFSET 0x68

// Core reg x20 value
#define POWER_MANAGER_CORE_REG_X20_REG_OFFSET 0x6c

// Core reg x21 value
#define POWER_MANAGER_CORE_REG_X21_REG_OFFSET 0x70

// Core reg x22 value
#define POWER_MANAGER_CORE_REG_X22_REG_OFFSET 0x74

// Core reg x23 value
#define POWER_MANAGER_CORE_REG_X23_REG_OFFSET 0x78

// Core reg x24 value
#define POWER_MANAGER_CORE_REG_X24_REG_OFFSET 0x7c

// Core reg x25 value
#define POWER_MANAGER_CORE_REG_X25_REG_OFFSET 0x80

// Core reg x26 value
#define POWER_MANAGER_CORE_REG_X26_REG_OFFSET 0x84

// Core reg x27 value
#define POWER_MANAGER_CORE_REG_X27_REG_OFFSET 0x88

// Core reg x28 value
#define POWER_MANAGER_CORE_REG_X28_REG_OFFSET 0x8c

// Core reg x29 value
#define POWER_MANAGER_CORE_REG_X29_REG_OFFSET 0x90

// Core reg x30 value
#define POWER_MANAGER_CORE_REG_X30_REG_OFFSET 0x94

// Core reg x31 value
#define POWER_MANAGER_CORE_REG_X31_REG_OFFSET 0x98

// Core csr c0 value
#define POWER_MANAGER_CORE_CSR_C0_REG_OFFSET 0x9c

// Core csr c1 value
#define POWER_MANAGER_CORE_CSR_C1_REG_OFFSET 0xa0

// Core csr c2 value
#define POWER_MANAGER_CORE_CSR_C2_REG_OFFSET 0xa4

// Core csr c3 value
#define POWER_MANAGER_CORE_CSR_C3_REG_OFFSET 0xa8

// Core csr c4 value
#define POWER_MANAGER_CORE_CSR_C4_REG_OFFSET 0xac

// Core csr c5 value
#define POWER_MANAGER_CORE_CSR_C5_REG_OFFSET 0xb0

// Core csr c6 value
#define POWER_MANAGER_CORE_CSR_C6_REG_OFFSET 0xb4

// Core csr c7 value
#define POWER_MANAGER_CORE_CSR_C7_REG_OFFSET 0xb8

// Enable wait for interrupt
#define POWER_MANAGER_EN_WAIT_FOR_INTR_REG_OFFSET 0xbc

// Interrupt state
#define POWER_MANAGER_INTR_STATE_REG_OFFSET 0xc0

// Counter before resetting the cpu_subsystem domain
#define POWER_MANAGER_CPU_RESET_ASSERT_COUNTER_REG_OFFSET 0xc4

// Counter before unreset the cpu_subsystem domain
#define POWER_MANAGER_CPU_RESET_DEASSERT_COUNTER_REG_OFFSET 0xc8

// Counter before switching off the cpu_subsystem domain
#define POWER_MANAGER_CPU_SWITCH_OFF_COUNTER_REG_OFFSET 0xcc

// Counter before switching on the cpu_subsystem domain
#define POWER_MANAGER_CPU_SWITCH_ON_COUNTER_REG_OFFSET 0xd0

// Bits to stop the counters keeping the done_o signal high
#define POWER_MANAGER_CPU_COUNTERS_STOP_REG_OFFSET 0xd4
#define POWER_MANAGER_CPU_COUNTERS_STOP_CPU_RESET_ASSERT_STOP_BIT_COUNTER_BIT 0
#define POWER_MANAGER_CPU_COUNTERS_STOP_CPU_RESET_DEASSERT_STOP_BIT_COUNTER_BIT \
  1
#define POWER_MANAGER_CPU_COUNTERS_STOP_CPU_SWITCH_OFF_STOP_BIT_COUNTER_BIT 2
#define POWER_MANAGER_CPU_COUNTERS_STOP_CPU_SWITCH_ON_STOP_BIT_COUNTER_BIT 3

// Counter before resetting the peripheral_subsystem domain
#define POWER_MANAGER_PERIPH_RESET_ASSERT_COUNTER_REG_OFFSET 0xd8

// Counter before unreset the peripheral_subsystem domain
#define POWER_MANAGER_PERIPH_RESET_DEASSERT_COUNTER_REG_OFFSET 0xdc

// Counter before switching off the peripheral_subsystem domain
#define POWER_MANAGER_PERIPH_SWITCH_OFF_COUNTER_REG_OFFSET 0xe0

// Counter before switching on the peripheral_subsystem domain
#define POWER_MANAGER_PERIPH_SWITCH_ON_COUNTER_REG_OFFSET 0xe4

// Bits to stop the counters keeping the done_o signal high
#define POWER_MANAGER_PERIPH_COUNTERS_STOP_REG_OFFSET 0xe8
#define POWER_MANAGER_PERIPH_COUNTERS_STOP_PERIPH_RESET_ASSERT_STOP_BIT_COUNTER_BIT \
  0
#define POWER_MANAGER_PERIPH_COUNTERS_STOP_PERIPH_RESET_DEASSERT_STOP_BIT_COUNTER_BIT \
  1
#define POWER_MANAGER_PERIPH_COUNTERS_STOP_PERIPH_SWITCH_OFF_STOP_BIT_COUNTER_BIT \
  2
#define POWER_MANAGER_PERIPH_COUNTERS_STOP_PERIPH_SWITCH_ON_STOP_BIT_COUNTER_BIT \
  3

// Counter before resetting the ram0 domain
#define POWER_MANAGER_RAM0_RESET_ASSERT_COUNTER_REG_OFFSET 0xec

// Counter before unreset the ram0 domain
#define POWER_MANAGER_RAM0_RESET_DEASSERT_COUNTER_REG_OFFSET 0xf0

// Counter before switching off the ram0 domain
#define POWER_MANAGER_RAM0_SWITCH_OFF_COUNTER_REG_OFFSET 0xf4

// Counter before switching on the ram0 domain
#define POWER_MANAGER_RAM0_SWITCH_ON_COUNTER_REG_OFFSET 0xf8

// Bits to stop the counters keeping the done_o signal high
#define POWER_MANAGER_RAM0_COUNTERS_STOP_REG_OFFSET 0xfc
#define POWER_MANAGER_RAM0_COUNTERS_STOP_RAM0_RESET_ASSERT_STOP_BIT_COUNTER_BIT \
  0
#define POWER_MANAGER_RAM0_COUNTERS_STOP_RAM0_RESET_DEASSERT_STOP_BIT_COUNTER_BIT \
  1
#define POWER_MANAGER_RAM0_COUNTERS_STOP_RAM0_SWITCH_OFF_STOP_BIT_COUNTER_BIT 2
#define POWER_MANAGER_RAM0_COUNTERS_STOP_RAM0_SWITCH_ON_STOP_BIT_COUNTER_BIT 3

// Counter before resetting the ram1 domain
#define POWER_MANAGER_RAM1_RESET_ASSERT_COUNTER_REG_OFFSET 0x100

// Counter before unreset the ram1 domain
#define POWER_MANAGER_RAM1_RESET_DEASSERT_COUNTER_REG_OFFSET 0x104

// Counter before switching off the ram1 domain
#define POWER_MANAGER_RAM1_SWITCH_OFF_COUNTER_REG_OFFSET 0x108

// Counter before switching on the ram1 domain
#define POWER_MANAGER_RAM1_SWITCH_ON_COUNTER_REG_OFFSET 0x10c

// Bits to stop the counters keeping the done_o signal high
#define POWER_MANAGER_RAM1_COUNTERS_STOP_REG_OFFSET 0x110
#define POWER_MANAGER_RAM1_COUNTERS_STOP_RAM1_RESET_ASSERT_STOP_BIT_COUNTER_BIT \
  0
#define POWER_MANAGER_RAM1_COUNTERS_STOP_RAM1_RESET_DEASSERT_STOP_BIT_COUNTER_BIT \
  1
#define POWER_MANAGER_RAM1_COUNTERS_STOP_RAM1_SWITCH_OFF_STOP_BIT_COUNTER_BIT 2
#define POWER_MANAGER_RAM1_COUNTERS_STOP_RAM1_SWITCH_ON_STOP_BIT_COUNTER_BIT 3

// Counter before resetting the ram2 domain
#define POWER_MANAGER_RAM2_RESET_ASSERT_COUNTER_REG_OFFSET 0x114

// Counter before unreset the ram2 domain
#define POWER_MANAGER_RAM2_RESET_DEASSERT_COUNTER_REG_OFFSET 0x118

// Counter before switching off the ram2 domain
#define POWER_MANAGER_RAM2_SWITCH_OFF_COUNTER_REG_OFFSET 0x11c

// Counter before switching on the ram2 domain
#define POWER_MANAGER_RAM2_SWITCH_ON_COUNTER_REG_OFFSET 0x120

// Bits to stop the counters keeping the done_o signal high
#define POWER_MANAGER_RAM2_COUNTERS_STOP_REG_OFFSET 0x124
#define POWER_MANAGER_RAM2_COUNTERS_STOP_RAM2_RESET_ASSERT_STOP_BIT_COUNTER_BIT \
  0
#define POWER_MANAGER_RAM2_COUNTERS_STOP_RAM2_RESET_DEASSERT_STOP_BIT_COUNTER_BIT \
  1
#define POWER_MANAGER_RAM2_COUNTERS_STOP_RAM2_SWITCH_OFF_STOP_BIT_COUNTER_BIT 2
#define POWER_MANAGER_RAM2_COUNTERS_STOP_RAM2_SWITCH_ON_STOP_BIT_COUNTER_BIT 3

// Counter before resetting the ram3 domain
#define POWER_MANAGER_RAM3_RESET_ASSERT_COUNTER_REG_OFFSET 0x128

// Counter before unreset the ram3 domain
#define POWER_MANAGER_RAM3_RESET_DEASSERT_COUNTER_REG_OFFSET 0x12c

// Counter before switching off the ram3 domain
#define POWER_MANAGER_RAM3_SWITCH_OFF_COUNTER_REG_OFFSET 0x130

// Counter before switching on the ram3 domain
#define POWER_MANAGER_RAM3_SWITCH_ON_COUNTER_REG_OFFSET 0x134

// Bits to stop the counters keeping the done_o signal high
#define POWER_MANAGER_RAM3_COUNTERS_STOP_REG_OFFSET 0x138
#define POWER_MANAGER_RAM3_COUNTERS_STOP_RAM3_RESET_ASSERT_STOP_BIT_COUNTER_BIT \
  0
#define POWER_MANAGER_RAM3_COUNTERS_STOP_RAM3_RESET_DEASSERT_STOP_BIT_COUNTER_BIT \
  1
#define POWER_MANAGER_RAM3_COUNTERS_STOP_RAM3_SWITCH_OFF_STOP_BIT_COUNTER_BIT 2
#define POWER_MANAGER_RAM3_COUNTERS_STOP_RAM3_SWITCH_ON_STOP_BIT_COUNTER_BIT 3

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _POWER_MANAGER_REG_DEFS_
// End generated register defines for power_manager
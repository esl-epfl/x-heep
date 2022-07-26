// Generated register defines for RV_PLIC

// Copyright information found in source file:
// Copyright lowRISC contributors.

// Licensing information found in source file:
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef _RV_PLIC_REG_DEFS_
#define _RV_PLIC_REG_DEFS_

#ifdef __cplusplus
extern "C" {
#endif
// Number of interrupt sources
#define RV_PLIC_PARAM_NUM_SRC 64

// Number of Targets (Harts)
#define RV_PLIC_PARAM_NUM_TARGET 1

// Width of priority signals
#define RV_PLIC_PARAM_PRIO_WIDTH 3

// Register width
#define RV_PLIC_PARAM_REG_WIDTH 32

// Interrupt Pending (common parameters)
#define RV_PLIC_IP_P_FIELD_WIDTH 1
#define RV_PLIC_IP_P_FIELDS_PER_REG 32
#define RV_PLIC_IP_MULTIREG_COUNT 2

// Interrupt Pending
#define RV_PLIC_IP_0_REG_OFFSET 0x0
#define RV_PLIC_IP_0_P_0_BIT 0
#define RV_PLIC_IP_0_P_1_BIT 1
#define RV_PLIC_IP_0_P_2_BIT 2
#define RV_PLIC_IP_0_P_3_BIT 3
#define RV_PLIC_IP_0_P_4_BIT 4
#define RV_PLIC_IP_0_P_5_BIT 5
#define RV_PLIC_IP_0_P_6_BIT 6
#define RV_PLIC_IP_0_P_7_BIT 7
#define RV_PLIC_IP_0_P_8_BIT 8
#define RV_PLIC_IP_0_P_9_BIT 9
#define RV_PLIC_IP_0_P_10_BIT 10
#define RV_PLIC_IP_0_P_11_BIT 11
#define RV_PLIC_IP_0_P_12_BIT 12
#define RV_PLIC_IP_0_P_13_BIT 13
#define RV_PLIC_IP_0_P_14_BIT 14
#define RV_PLIC_IP_0_P_15_BIT 15
#define RV_PLIC_IP_0_P_16_BIT 16
#define RV_PLIC_IP_0_P_17_BIT 17
#define RV_PLIC_IP_0_P_18_BIT 18
#define RV_PLIC_IP_0_P_19_BIT 19
#define RV_PLIC_IP_0_P_20_BIT 20
#define RV_PLIC_IP_0_P_21_BIT 21
#define RV_PLIC_IP_0_P_22_BIT 22
#define RV_PLIC_IP_0_P_23_BIT 23
#define RV_PLIC_IP_0_P_24_BIT 24
#define RV_PLIC_IP_0_P_25_BIT 25
#define RV_PLIC_IP_0_P_26_BIT 26
#define RV_PLIC_IP_0_P_27_BIT 27
#define RV_PLIC_IP_0_P_28_BIT 28
#define RV_PLIC_IP_0_P_29_BIT 29
#define RV_PLIC_IP_0_P_30_BIT 30
#define RV_PLIC_IP_0_P_31_BIT 31

// Interrupt Pending
#define RV_PLIC_IP_1_REG_OFFSET 0x4
#define RV_PLIC_IP_1_P_32_BIT 0
#define RV_PLIC_IP_1_P_33_BIT 1
#define RV_PLIC_IP_1_P_34_BIT 2
#define RV_PLIC_IP_1_P_35_BIT 3
#define RV_PLIC_IP_1_P_36_BIT 4
#define RV_PLIC_IP_1_P_37_BIT 5
#define RV_PLIC_IP_1_P_38_BIT 6
#define RV_PLIC_IP_1_P_39_BIT 7
#define RV_PLIC_IP_1_P_40_BIT 8
#define RV_PLIC_IP_1_P_41_BIT 9
#define RV_PLIC_IP_1_P_42_BIT 10
#define RV_PLIC_IP_1_P_43_BIT 11
#define RV_PLIC_IP_1_P_44_BIT 12
#define RV_PLIC_IP_1_P_45_BIT 13
#define RV_PLIC_IP_1_P_46_BIT 14
#define RV_PLIC_IP_1_P_47_BIT 15
#define RV_PLIC_IP_1_P_48_BIT 16
#define RV_PLIC_IP_1_P_49_BIT 17
#define RV_PLIC_IP_1_P_50_BIT 18
#define RV_PLIC_IP_1_P_51_BIT 19
#define RV_PLIC_IP_1_P_52_BIT 20
#define RV_PLIC_IP_1_P_53_BIT 21
#define RV_PLIC_IP_1_P_54_BIT 22
#define RV_PLIC_IP_1_P_55_BIT 23
#define RV_PLIC_IP_1_P_56_BIT 24
#define RV_PLIC_IP_1_P_57_BIT 25
#define RV_PLIC_IP_1_P_58_BIT 26
#define RV_PLIC_IP_1_P_59_BIT 27
#define RV_PLIC_IP_1_P_60_BIT 28
#define RV_PLIC_IP_1_P_61_BIT 29
#define RV_PLIC_IP_1_P_62_BIT 30
#define RV_PLIC_IP_1_P_63_BIT 31

// Interrupt Source mode. 0: Level, 1: Edge-triggered (common parameters)
#define RV_PLIC_LE_LE_FIELD_WIDTH 1
#define RV_PLIC_LE_LE_FIELDS_PER_REG 32
#define RV_PLIC_LE_MULTIREG_COUNT 2

// Interrupt Source mode. 0: Level, 1: Edge-triggered
#define RV_PLIC_LE_0_REG_OFFSET 0x8
#define RV_PLIC_LE_0_LE_0_BIT 0
#define RV_PLIC_LE_0_LE_1_BIT 1
#define RV_PLIC_LE_0_LE_2_BIT 2
#define RV_PLIC_LE_0_LE_3_BIT 3
#define RV_PLIC_LE_0_LE_4_BIT 4
#define RV_PLIC_LE_0_LE_5_BIT 5
#define RV_PLIC_LE_0_LE_6_BIT 6
#define RV_PLIC_LE_0_LE_7_BIT 7
#define RV_PLIC_LE_0_LE_8_BIT 8
#define RV_PLIC_LE_0_LE_9_BIT 9
#define RV_PLIC_LE_0_LE_10_BIT 10
#define RV_PLIC_LE_0_LE_11_BIT 11
#define RV_PLIC_LE_0_LE_12_BIT 12
#define RV_PLIC_LE_0_LE_13_BIT 13
#define RV_PLIC_LE_0_LE_14_BIT 14
#define RV_PLIC_LE_0_LE_15_BIT 15
#define RV_PLIC_LE_0_LE_16_BIT 16
#define RV_PLIC_LE_0_LE_17_BIT 17
#define RV_PLIC_LE_0_LE_18_BIT 18
#define RV_PLIC_LE_0_LE_19_BIT 19
#define RV_PLIC_LE_0_LE_20_BIT 20
#define RV_PLIC_LE_0_LE_21_BIT 21
#define RV_PLIC_LE_0_LE_22_BIT 22
#define RV_PLIC_LE_0_LE_23_BIT 23
#define RV_PLIC_LE_0_LE_24_BIT 24
#define RV_PLIC_LE_0_LE_25_BIT 25
#define RV_PLIC_LE_0_LE_26_BIT 26
#define RV_PLIC_LE_0_LE_27_BIT 27
#define RV_PLIC_LE_0_LE_28_BIT 28
#define RV_PLIC_LE_0_LE_29_BIT 29
#define RV_PLIC_LE_0_LE_30_BIT 30
#define RV_PLIC_LE_0_LE_31_BIT 31

// Interrupt Source mode. 0: Level, 1: Edge-triggered
#define RV_PLIC_LE_1_REG_OFFSET 0xc
#define RV_PLIC_LE_1_LE_32_BIT 0
#define RV_PLIC_LE_1_LE_33_BIT 1
#define RV_PLIC_LE_1_LE_34_BIT 2
#define RV_PLIC_LE_1_LE_35_BIT 3
#define RV_PLIC_LE_1_LE_36_BIT 4
#define RV_PLIC_LE_1_LE_37_BIT 5
#define RV_PLIC_LE_1_LE_38_BIT 6
#define RV_PLIC_LE_1_LE_39_BIT 7
#define RV_PLIC_LE_1_LE_40_BIT 8
#define RV_PLIC_LE_1_LE_41_BIT 9
#define RV_PLIC_LE_1_LE_42_BIT 10
#define RV_PLIC_LE_1_LE_43_BIT 11
#define RV_PLIC_LE_1_LE_44_BIT 12
#define RV_PLIC_LE_1_LE_45_BIT 13
#define RV_PLIC_LE_1_LE_46_BIT 14
#define RV_PLIC_LE_1_LE_47_BIT 15
#define RV_PLIC_LE_1_LE_48_BIT 16
#define RV_PLIC_LE_1_LE_49_BIT 17
#define RV_PLIC_LE_1_LE_50_BIT 18
#define RV_PLIC_LE_1_LE_51_BIT 19
#define RV_PLIC_LE_1_LE_52_BIT 20
#define RV_PLIC_LE_1_LE_53_BIT 21
#define RV_PLIC_LE_1_LE_54_BIT 22
#define RV_PLIC_LE_1_LE_55_BIT 23
#define RV_PLIC_LE_1_LE_56_BIT 24
#define RV_PLIC_LE_1_LE_57_BIT 25
#define RV_PLIC_LE_1_LE_58_BIT 26
#define RV_PLIC_LE_1_LE_59_BIT 27
#define RV_PLIC_LE_1_LE_60_BIT 28
#define RV_PLIC_LE_1_LE_61_BIT 29
#define RV_PLIC_LE_1_LE_62_BIT 30
#define RV_PLIC_LE_1_LE_63_BIT 31

// Interrupt Source 0 Priority
#define RV_PLIC_PRIO0_REG_OFFSET 0x10
#define RV_PLIC_PRIO0_PRIO0_MASK 0x7
#define RV_PLIC_PRIO0_PRIO0_OFFSET 0
#define RV_PLIC_PRIO0_PRIO0_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO0_PRIO0_MASK, .index = RV_PLIC_PRIO0_PRIO0_OFFSET })

// Interrupt Source 1 Priority
#define RV_PLIC_PRIO1_REG_OFFSET 0x14
#define RV_PLIC_PRIO1_PRIO1_MASK 0x7
#define RV_PLIC_PRIO1_PRIO1_OFFSET 0
#define RV_PLIC_PRIO1_PRIO1_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO1_PRIO1_MASK, .index = RV_PLIC_PRIO1_PRIO1_OFFSET })

// Interrupt Source 2 Priority
#define RV_PLIC_PRIO2_REG_OFFSET 0x18
#define RV_PLIC_PRIO2_PRIO2_MASK 0x7
#define RV_PLIC_PRIO2_PRIO2_OFFSET 0
#define RV_PLIC_PRIO2_PRIO2_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO2_PRIO2_MASK, .index = RV_PLIC_PRIO2_PRIO2_OFFSET })

// Interrupt Source 3 Priority
#define RV_PLIC_PRIO3_REG_OFFSET 0x1c
#define RV_PLIC_PRIO3_PRIO3_MASK 0x7
#define RV_PLIC_PRIO3_PRIO3_OFFSET 0
#define RV_PLIC_PRIO3_PRIO3_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO3_PRIO3_MASK, .index = RV_PLIC_PRIO3_PRIO3_OFFSET })

// Interrupt Source 4 Priority
#define RV_PLIC_PRIO4_REG_OFFSET 0x20
#define RV_PLIC_PRIO4_PRIO4_MASK 0x7
#define RV_PLIC_PRIO4_PRIO4_OFFSET 0
#define RV_PLIC_PRIO4_PRIO4_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO4_PRIO4_MASK, .index = RV_PLIC_PRIO4_PRIO4_OFFSET })

// Interrupt Source 5 Priority
#define RV_PLIC_PRIO5_REG_OFFSET 0x24
#define RV_PLIC_PRIO5_PRIO5_MASK 0x7
#define RV_PLIC_PRIO5_PRIO5_OFFSET 0
#define RV_PLIC_PRIO5_PRIO5_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO5_PRIO5_MASK, .index = RV_PLIC_PRIO5_PRIO5_OFFSET })

// Interrupt Source 6 Priority
#define RV_PLIC_PRIO6_REG_OFFSET 0x28
#define RV_PLIC_PRIO6_PRIO6_MASK 0x7
#define RV_PLIC_PRIO6_PRIO6_OFFSET 0
#define RV_PLIC_PRIO6_PRIO6_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO6_PRIO6_MASK, .index = RV_PLIC_PRIO6_PRIO6_OFFSET })

// Interrupt Source 7 Priority
#define RV_PLIC_PRIO7_REG_OFFSET 0x2c
#define RV_PLIC_PRIO7_PRIO7_MASK 0x7
#define RV_PLIC_PRIO7_PRIO7_OFFSET 0
#define RV_PLIC_PRIO7_PRIO7_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO7_PRIO7_MASK, .index = RV_PLIC_PRIO7_PRIO7_OFFSET })

// Interrupt Source 8 Priority
#define RV_PLIC_PRIO8_REG_OFFSET 0x30
#define RV_PLIC_PRIO8_PRIO8_MASK 0x7
#define RV_PLIC_PRIO8_PRIO8_OFFSET 0
#define RV_PLIC_PRIO8_PRIO8_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO8_PRIO8_MASK, .index = RV_PLIC_PRIO8_PRIO8_OFFSET })

// Interrupt Source 9 Priority
#define RV_PLIC_PRIO9_REG_OFFSET 0x34
#define RV_PLIC_PRIO9_PRIO9_MASK 0x7
#define RV_PLIC_PRIO9_PRIO9_OFFSET 0
#define RV_PLIC_PRIO9_PRIO9_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO9_PRIO9_MASK, .index = RV_PLIC_PRIO9_PRIO9_OFFSET })

// Interrupt Source 10 Priority
#define RV_PLIC_PRIO10_REG_OFFSET 0x38
#define RV_PLIC_PRIO10_PRIO10_MASK 0x7
#define RV_PLIC_PRIO10_PRIO10_OFFSET 0
#define RV_PLIC_PRIO10_PRIO10_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO10_PRIO10_MASK, .index = RV_PLIC_PRIO10_PRIO10_OFFSET })

// Interrupt Source 11 Priority
#define RV_PLIC_PRIO11_REG_OFFSET 0x3c
#define RV_PLIC_PRIO11_PRIO11_MASK 0x7
#define RV_PLIC_PRIO11_PRIO11_OFFSET 0
#define RV_PLIC_PRIO11_PRIO11_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO11_PRIO11_MASK, .index = RV_PLIC_PRIO11_PRIO11_OFFSET })

// Interrupt Source 12 Priority
#define RV_PLIC_PRIO12_REG_OFFSET 0x40
#define RV_PLIC_PRIO12_PRIO12_MASK 0x7
#define RV_PLIC_PRIO12_PRIO12_OFFSET 0
#define RV_PLIC_PRIO12_PRIO12_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO12_PRIO12_MASK, .index = RV_PLIC_PRIO12_PRIO12_OFFSET })

// Interrupt Source 13 Priority
#define RV_PLIC_PRIO13_REG_OFFSET 0x44
#define RV_PLIC_PRIO13_PRIO13_MASK 0x7
#define RV_PLIC_PRIO13_PRIO13_OFFSET 0
#define RV_PLIC_PRIO13_PRIO13_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO13_PRIO13_MASK, .index = RV_PLIC_PRIO13_PRIO13_OFFSET })

// Interrupt Source 14 Priority
#define RV_PLIC_PRIO14_REG_OFFSET 0x48
#define RV_PLIC_PRIO14_PRIO14_MASK 0x7
#define RV_PLIC_PRIO14_PRIO14_OFFSET 0
#define RV_PLIC_PRIO14_PRIO14_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO14_PRIO14_MASK, .index = RV_PLIC_PRIO14_PRIO14_OFFSET })

// Interrupt Source 15 Priority
#define RV_PLIC_PRIO15_REG_OFFSET 0x4c
#define RV_PLIC_PRIO15_PRIO15_MASK 0x7
#define RV_PLIC_PRIO15_PRIO15_OFFSET 0
#define RV_PLIC_PRIO15_PRIO15_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO15_PRIO15_MASK, .index = RV_PLIC_PRIO15_PRIO15_OFFSET })

// Interrupt Source 16 Priority
#define RV_PLIC_PRIO16_REG_OFFSET 0x50
#define RV_PLIC_PRIO16_PRIO16_MASK 0x7
#define RV_PLIC_PRIO16_PRIO16_OFFSET 0
#define RV_PLIC_PRIO16_PRIO16_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO16_PRIO16_MASK, .index = RV_PLIC_PRIO16_PRIO16_OFFSET })

// Interrupt Source 17 Priority
#define RV_PLIC_PRIO17_REG_OFFSET 0x54
#define RV_PLIC_PRIO17_PRIO17_MASK 0x7
#define RV_PLIC_PRIO17_PRIO17_OFFSET 0
#define RV_PLIC_PRIO17_PRIO17_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO17_PRIO17_MASK, .index = RV_PLIC_PRIO17_PRIO17_OFFSET })

// Interrupt Source 18 Priority
#define RV_PLIC_PRIO18_REG_OFFSET 0x58
#define RV_PLIC_PRIO18_PRIO18_MASK 0x7
#define RV_PLIC_PRIO18_PRIO18_OFFSET 0
#define RV_PLIC_PRIO18_PRIO18_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO18_PRIO18_MASK, .index = RV_PLIC_PRIO18_PRIO18_OFFSET })

// Interrupt Source 19 Priority
#define RV_PLIC_PRIO19_REG_OFFSET 0x5c
#define RV_PLIC_PRIO19_PRIO19_MASK 0x7
#define RV_PLIC_PRIO19_PRIO19_OFFSET 0
#define RV_PLIC_PRIO19_PRIO19_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO19_PRIO19_MASK, .index = RV_PLIC_PRIO19_PRIO19_OFFSET })

// Interrupt Source 20 Priority
#define RV_PLIC_PRIO20_REG_OFFSET 0x60
#define RV_PLIC_PRIO20_PRIO20_MASK 0x7
#define RV_PLIC_PRIO20_PRIO20_OFFSET 0
#define RV_PLIC_PRIO20_PRIO20_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO20_PRIO20_MASK, .index = RV_PLIC_PRIO20_PRIO20_OFFSET })

// Interrupt Source 21 Priority
#define RV_PLIC_PRIO21_REG_OFFSET 0x64
#define RV_PLIC_PRIO21_PRIO21_MASK 0x7
#define RV_PLIC_PRIO21_PRIO21_OFFSET 0
#define RV_PLIC_PRIO21_PRIO21_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO21_PRIO21_MASK, .index = RV_PLIC_PRIO21_PRIO21_OFFSET })

// Interrupt Source 22 Priority
#define RV_PLIC_PRIO22_REG_OFFSET 0x68
#define RV_PLIC_PRIO22_PRIO22_MASK 0x7
#define RV_PLIC_PRIO22_PRIO22_OFFSET 0
#define RV_PLIC_PRIO22_PRIO22_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO22_PRIO22_MASK, .index = RV_PLIC_PRIO22_PRIO22_OFFSET })

// Interrupt Source 23 Priority
#define RV_PLIC_PRIO23_REG_OFFSET 0x6c
#define RV_PLIC_PRIO23_PRIO23_MASK 0x7
#define RV_PLIC_PRIO23_PRIO23_OFFSET 0
#define RV_PLIC_PRIO23_PRIO23_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO23_PRIO23_MASK, .index = RV_PLIC_PRIO23_PRIO23_OFFSET })

// Interrupt Source 24 Priority
#define RV_PLIC_PRIO24_REG_OFFSET 0x70
#define RV_PLIC_PRIO24_PRIO24_MASK 0x7
#define RV_PLIC_PRIO24_PRIO24_OFFSET 0
#define RV_PLIC_PRIO24_PRIO24_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO24_PRIO24_MASK, .index = RV_PLIC_PRIO24_PRIO24_OFFSET })

// Interrupt Source 25 Priority
#define RV_PLIC_PRIO25_REG_OFFSET 0x74
#define RV_PLIC_PRIO25_PRIO25_MASK 0x7
#define RV_PLIC_PRIO25_PRIO25_OFFSET 0
#define RV_PLIC_PRIO25_PRIO25_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO25_PRIO25_MASK, .index = RV_PLIC_PRIO25_PRIO25_OFFSET })

// Interrupt Source 26 Priority
#define RV_PLIC_PRIO26_REG_OFFSET 0x78
#define RV_PLIC_PRIO26_PRIO26_MASK 0x7
#define RV_PLIC_PRIO26_PRIO26_OFFSET 0
#define RV_PLIC_PRIO26_PRIO26_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO26_PRIO26_MASK, .index = RV_PLIC_PRIO26_PRIO26_OFFSET })

// Interrupt Source 27 Priority
#define RV_PLIC_PRIO27_REG_OFFSET 0x7c
#define RV_PLIC_PRIO27_PRIO27_MASK 0x7
#define RV_PLIC_PRIO27_PRIO27_OFFSET 0
#define RV_PLIC_PRIO27_PRIO27_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO27_PRIO27_MASK, .index = RV_PLIC_PRIO27_PRIO27_OFFSET })

// Interrupt Source 28 Priority
#define RV_PLIC_PRIO28_REG_OFFSET 0x80
#define RV_PLIC_PRIO28_PRIO28_MASK 0x7
#define RV_PLIC_PRIO28_PRIO28_OFFSET 0
#define RV_PLIC_PRIO28_PRIO28_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO28_PRIO28_MASK, .index = RV_PLIC_PRIO28_PRIO28_OFFSET })

// Interrupt Source 29 Priority
#define RV_PLIC_PRIO29_REG_OFFSET 0x84
#define RV_PLIC_PRIO29_PRIO29_MASK 0x7
#define RV_PLIC_PRIO29_PRIO29_OFFSET 0
#define RV_PLIC_PRIO29_PRIO29_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO29_PRIO29_MASK, .index = RV_PLIC_PRIO29_PRIO29_OFFSET })

// Interrupt Source 30 Priority
#define RV_PLIC_PRIO30_REG_OFFSET 0x88
#define RV_PLIC_PRIO30_PRIO30_MASK 0x7
#define RV_PLIC_PRIO30_PRIO30_OFFSET 0
#define RV_PLIC_PRIO30_PRIO30_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO30_PRIO30_MASK, .index = RV_PLIC_PRIO30_PRIO30_OFFSET })

// Interrupt Source 31 Priority
#define RV_PLIC_PRIO31_REG_OFFSET 0x8c
#define RV_PLIC_PRIO31_PRIO31_MASK 0x7
#define RV_PLIC_PRIO31_PRIO31_OFFSET 0
#define RV_PLIC_PRIO31_PRIO31_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO31_PRIO31_MASK, .index = RV_PLIC_PRIO31_PRIO31_OFFSET })

// Interrupt Source 32 Priority
#define RV_PLIC_PRIO32_REG_OFFSET 0x90
#define RV_PLIC_PRIO32_PRIO32_MASK 0x7
#define RV_PLIC_PRIO32_PRIO32_OFFSET 0
#define RV_PLIC_PRIO32_PRIO32_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO32_PRIO32_MASK, .index = RV_PLIC_PRIO32_PRIO32_OFFSET })

// Interrupt Source 33 Priority
#define RV_PLIC_PRIO33_REG_OFFSET 0x94
#define RV_PLIC_PRIO33_PRIO33_MASK 0x7
#define RV_PLIC_PRIO33_PRIO33_OFFSET 0
#define RV_PLIC_PRIO33_PRIO33_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO33_PRIO33_MASK, .index = RV_PLIC_PRIO33_PRIO33_OFFSET })

// Interrupt Source 34 Priority
#define RV_PLIC_PRIO34_REG_OFFSET 0x98
#define RV_PLIC_PRIO34_PRIO34_MASK 0x7
#define RV_PLIC_PRIO34_PRIO34_OFFSET 0
#define RV_PLIC_PRIO34_PRIO34_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO34_PRIO34_MASK, .index = RV_PLIC_PRIO34_PRIO34_OFFSET })

// Interrupt Source 35 Priority
#define RV_PLIC_PRIO35_REG_OFFSET 0x9c
#define RV_PLIC_PRIO35_PRIO35_MASK 0x7
#define RV_PLIC_PRIO35_PRIO35_OFFSET 0
#define RV_PLIC_PRIO35_PRIO35_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO35_PRIO35_MASK, .index = RV_PLIC_PRIO35_PRIO35_OFFSET })

// Interrupt Source 36 Priority
#define RV_PLIC_PRIO36_REG_OFFSET 0xa0
#define RV_PLIC_PRIO36_PRIO36_MASK 0x7
#define RV_PLIC_PRIO36_PRIO36_OFFSET 0
#define RV_PLIC_PRIO36_PRIO36_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO36_PRIO36_MASK, .index = RV_PLIC_PRIO36_PRIO36_OFFSET })

// Interrupt Source 37 Priority
#define RV_PLIC_PRIO37_REG_OFFSET 0xa4
#define RV_PLIC_PRIO37_PRIO37_MASK 0x7
#define RV_PLIC_PRIO37_PRIO37_OFFSET 0
#define RV_PLIC_PRIO37_PRIO37_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO37_PRIO37_MASK, .index = RV_PLIC_PRIO37_PRIO37_OFFSET })

// Interrupt Source 38 Priority
#define RV_PLIC_PRIO38_REG_OFFSET 0xa8
#define RV_PLIC_PRIO38_PRIO38_MASK 0x7
#define RV_PLIC_PRIO38_PRIO38_OFFSET 0
#define RV_PLIC_PRIO38_PRIO38_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO38_PRIO38_MASK, .index = RV_PLIC_PRIO38_PRIO38_OFFSET })

// Interrupt Source 39 Priority
#define RV_PLIC_PRIO39_REG_OFFSET 0xac
#define RV_PLIC_PRIO39_PRIO39_MASK 0x7
#define RV_PLIC_PRIO39_PRIO39_OFFSET 0
#define RV_PLIC_PRIO39_PRIO39_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO39_PRIO39_MASK, .index = RV_PLIC_PRIO39_PRIO39_OFFSET })

// Interrupt Source 40 Priority
#define RV_PLIC_PRIO40_REG_OFFSET 0xb0
#define RV_PLIC_PRIO40_PRIO40_MASK 0x7
#define RV_PLIC_PRIO40_PRIO40_OFFSET 0
#define RV_PLIC_PRIO40_PRIO40_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO40_PRIO40_MASK, .index = RV_PLIC_PRIO40_PRIO40_OFFSET })

// Interrupt Source 41 Priority
#define RV_PLIC_PRIO41_REG_OFFSET 0xb4
#define RV_PLIC_PRIO41_PRIO41_MASK 0x7
#define RV_PLIC_PRIO41_PRIO41_OFFSET 0
#define RV_PLIC_PRIO41_PRIO41_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO41_PRIO41_MASK, .index = RV_PLIC_PRIO41_PRIO41_OFFSET })

// Interrupt Source 42 Priority
#define RV_PLIC_PRIO42_REG_OFFSET 0xb8
#define RV_PLIC_PRIO42_PRIO42_MASK 0x7
#define RV_PLIC_PRIO42_PRIO42_OFFSET 0
#define RV_PLIC_PRIO42_PRIO42_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO42_PRIO42_MASK, .index = RV_PLIC_PRIO42_PRIO42_OFFSET })

// Interrupt Source 43 Priority
#define RV_PLIC_PRIO43_REG_OFFSET 0xbc
#define RV_PLIC_PRIO43_PRIO43_MASK 0x7
#define RV_PLIC_PRIO43_PRIO43_OFFSET 0
#define RV_PLIC_PRIO43_PRIO43_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO43_PRIO43_MASK, .index = RV_PLIC_PRIO43_PRIO43_OFFSET })

// Interrupt Source 44 Priority
#define RV_PLIC_PRIO44_REG_OFFSET 0xc0
#define RV_PLIC_PRIO44_PRIO44_MASK 0x7
#define RV_PLIC_PRIO44_PRIO44_OFFSET 0
#define RV_PLIC_PRIO44_PRIO44_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO44_PRIO44_MASK, .index = RV_PLIC_PRIO44_PRIO44_OFFSET })

// Interrupt Source 45 Priority
#define RV_PLIC_PRIO45_REG_OFFSET 0xc4
#define RV_PLIC_PRIO45_PRIO45_MASK 0x7
#define RV_PLIC_PRIO45_PRIO45_OFFSET 0
#define RV_PLIC_PRIO45_PRIO45_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO45_PRIO45_MASK, .index = RV_PLIC_PRIO45_PRIO45_OFFSET })

// Interrupt Source 46 Priority
#define RV_PLIC_PRIO46_REG_OFFSET 0xc8
#define RV_PLIC_PRIO46_PRIO46_MASK 0x7
#define RV_PLIC_PRIO46_PRIO46_OFFSET 0
#define RV_PLIC_PRIO46_PRIO46_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO46_PRIO46_MASK, .index = RV_PLIC_PRIO46_PRIO46_OFFSET })

// Interrupt Source 47 Priority
#define RV_PLIC_PRIO47_REG_OFFSET 0xcc
#define RV_PLIC_PRIO47_PRIO47_MASK 0x7
#define RV_PLIC_PRIO47_PRIO47_OFFSET 0
#define RV_PLIC_PRIO47_PRIO47_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO47_PRIO47_MASK, .index = RV_PLIC_PRIO47_PRIO47_OFFSET })

// Interrupt Source 48 Priority
#define RV_PLIC_PRIO48_REG_OFFSET 0xd0
#define RV_PLIC_PRIO48_PRIO48_MASK 0x7
#define RV_PLIC_PRIO48_PRIO48_OFFSET 0
#define RV_PLIC_PRIO48_PRIO48_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO48_PRIO48_MASK, .index = RV_PLIC_PRIO48_PRIO48_OFFSET })

// Interrupt Source 49 Priority
#define RV_PLIC_PRIO49_REG_OFFSET 0xd4
#define RV_PLIC_PRIO49_PRIO49_MASK 0x7
#define RV_PLIC_PRIO49_PRIO49_OFFSET 0
#define RV_PLIC_PRIO49_PRIO49_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO49_PRIO49_MASK, .index = RV_PLIC_PRIO49_PRIO49_OFFSET })

// Interrupt Source 50 Priority
#define RV_PLIC_PRIO50_REG_OFFSET 0xd8
#define RV_PLIC_PRIO50_PRIO50_MASK 0x7
#define RV_PLIC_PRIO50_PRIO50_OFFSET 0
#define RV_PLIC_PRIO50_PRIO50_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO50_PRIO50_MASK, .index = RV_PLIC_PRIO50_PRIO50_OFFSET })

// Interrupt Source 51 Priority
#define RV_PLIC_PRIO51_REG_OFFSET 0xdc
#define RV_PLIC_PRIO51_PRIO51_MASK 0x7
#define RV_PLIC_PRIO51_PRIO51_OFFSET 0
#define RV_PLIC_PRIO51_PRIO51_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO51_PRIO51_MASK, .index = RV_PLIC_PRIO51_PRIO51_OFFSET })

// Interrupt Source 52 Priority
#define RV_PLIC_PRIO52_REG_OFFSET 0xe0
#define RV_PLIC_PRIO52_PRIO52_MASK 0x7
#define RV_PLIC_PRIO52_PRIO52_OFFSET 0
#define RV_PLIC_PRIO52_PRIO52_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO52_PRIO52_MASK, .index = RV_PLIC_PRIO52_PRIO52_OFFSET })

// Interrupt Source 53 Priority
#define RV_PLIC_PRIO53_REG_OFFSET 0xe4
#define RV_PLIC_PRIO53_PRIO53_MASK 0x7
#define RV_PLIC_PRIO53_PRIO53_OFFSET 0
#define RV_PLIC_PRIO53_PRIO53_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO53_PRIO53_MASK, .index = RV_PLIC_PRIO53_PRIO53_OFFSET })

// Interrupt Source 54 Priority
#define RV_PLIC_PRIO54_REG_OFFSET 0xe8
#define RV_PLIC_PRIO54_PRIO54_MASK 0x7
#define RV_PLIC_PRIO54_PRIO54_OFFSET 0
#define RV_PLIC_PRIO54_PRIO54_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO54_PRIO54_MASK, .index = RV_PLIC_PRIO54_PRIO54_OFFSET })

// Interrupt Source 55 Priority
#define RV_PLIC_PRIO55_REG_OFFSET 0xec
#define RV_PLIC_PRIO55_PRIO55_MASK 0x7
#define RV_PLIC_PRIO55_PRIO55_OFFSET 0
#define RV_PLIC_PRIO55_PRIO55_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO55_PRIO55_MASK, .index = RV_PLIC_PRIO55_PRIO55_OFFSET })

// Interrupt Source 56 Priority
#define RV_PLIC_PRIO56_REG_OFFSET 0xf0
#define RV_PLIC_PRIO56_PRIO56_MASK 0x7
#define RV_PLIC_PRIO56_PRIO56_OFFSET 0
#define RV_PLIC_PRIO56_PRIO56_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO56_PRIO56_MASK, .index = RV_PLIC_PRIO56_PRIO56_OFFSET })

// Interrupt Source 57 Priority
#define RV_PLIC_PRIO57_REG_OFFSET 0xf4
#define RV_PLIC_PRIO57_PRIO57_MASK 0x7
#define RV_PLIC_PRIO57_PRIO57_OFFSET 0
#define RV_PLIC_PRIO57_PRIO57_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO57_PRIO57_MASK, .index = RV_PLIC_PRIO57_PRIO57_OFFSET })

// Interrupt Source 58 Priority
#define RV_PLIC_PRIO58_REG_OFFSET 0xf8
#define RV_PLIC_PRIO58_PRIO58_MASK 0x7
#define RV_PLIC_PRIO58_PRIO58_OFFSET 0
#define RV_PLIC_PRIO58_PRIO58_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO58_PRIO58_MASK, .index = RV_PLIC_PRIO58_PRIO58_OFFSET })

// Interrupt Source 59 Priority
#define RV_PLIC_PRIO59_REG_OFFSET 0xfc
#define RV_PLIC_PRIO59_PRIO59_MASK 0x7
#define RV_PLIC_PRIO59_PRIO59_OFFSET 0
#define RV_PLIC_PRIO59_PRIO59_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO59_PRIO59_MASK, .index = RV_PLIC_PRIO59_PRIO59_OFFSET })

// Interrupt Source 60 Priority
#define RV_PLIC_PRIO60_REG_OFFSET 0x100
#define RV_PLIC_PRIO60_PRIO60_MASK 0x7
#define RV_PLIC_PRIO60_PRIO60_OFFSET 0
#define RV_PLIC_PRIO60_PRIO60_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO60_PRIO60_MASK, .index = RV_PLIC_PRIO60_PRIO60_OFFSET })

// Interrupt Source 61 Priority
#define RV_PLIC_PRIO61_REG_OFFSET 0x104
#define RV_PLIC_PRIO61_PRIO61_MASK 0x7
#define RV_PLIC_PRIO61_PRIO61_OFFSET 0
#define RV_PLIC_PRIO61_PRIO61_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO61_PRIO61_MASK, .index = RV_PLIC_PRIO61_PRIO61_OFFSET })

// Interrupt Source 62 Priority
#define RV_PLIC_PRIO62_REG_OFFSET 0x108
#define RV_PLIC_PRIO62_PRIO62_MASK 0x7
#define RV_PLIC_PRIO62_PRIO62_OFFSET 0
#define RV_PLIC_PRIO62_PRIO62_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO62_PRIO62_MASK, .index = RV_PLIC_PRIO62_PRIO62_OFFSET })

// Interrupt Source 63 Priority
#define RV_PLIC_PRIO63_REG_OFFSET 0x10c
#define RV_PLIC_PRIO63_PRIO63_MASK 0x7
#define RV_PLIC_PRIO63_PRIO63_OFFSET 0
#define RV_PLIC_PRIO63_PRIO63_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_PRIO63_PRIO63_MASK, .index = RV_PLIC_PRIO63_PRIO63_OFFSET })

// Interrupt Enable for Target 0 (common parameters)
#define RV_PLIC_IE0_E_FIELD_WIDTH 1
#define RV_PLIC_IE0_E_FIELDS_PER_REG 32
#define RV_PLIC_IE0_MULTIREG_COUNT 2

// Interrupt Enable for Target 0
#define RV_PLIC_IE0_0_REG_OFFSET 0x200
#define RV_PLIC_IE0_0_E_0_BIT 0
#define RV_PLIC_IE0_0_E_1_BIT 1
#define RV_PLIC_IE0_0_E_2_BIT 2
#define RV_PLIC_IE0_0_E_3_BIT 3
#define RV_PLIC_IE0_0_E_4_BIT 4
#define RV_PLIC_IE0_0_E_5_BIT 5
#define RV_PLIC_IE0_0_E_6_BIT 6
#define RV_PLIC_IE0_0_E_7_BIT 7
#define RV_PLIC_IE0_0_E_8_BIT 8
#define RV_PLIC_IE0_0_E_9_BIT 9
#define RV_PLIC_IE0_0_E_10_BIT 10
#define RV_PLIC_IE0_0_E_11_BIT 11
#define RV_PLIC_IE0_0_E_12_BIT 12
#define RV_PLIC_IE0_0_E_13_BIT 13
#define RV_PLIC_IE0_0_E_14_BIT 14
#define RV_PLIC_IE0_0_E_15_BIT 15
#define RV_PLIC_IE0_0_E_16_BIT 16
#define RV_PLIC_IE0_0_E_17_BIT 17
#define RV_PLIC_IE0_0_E_18_BIT 18
#define RV_PLIC_IE0_0_E_19_BIT 19
#define RV_PLIC_IE0_0_E_20_BIT 20
#define RV_PLIC_IE0_0_E_21_BIT 21
#define RV_PLIC_IE0_0_E_22_BIT 22
#define RV_PLIC_IE0_0_E_23_BIT 23
#define RV_PLIC_IE0_0_E_24_BIT 24
#define RV_PLIC_IE0_0_E_25_BIT 25
#define RV_PLIC_IE0_0_E_26_BIT 26
#define RV_PLIC_IE0_0_E_27_BIT 27
#define RV_PLIC_IE0_0_E_28_BIT 28
#define RV_PLIC_IE0_0_E_29_BIT 29
#define RV_PLIC_IE0_0_E_30_BIT 30
#define RV_PLIC_IE0_0_E_31_BIT 31

// Interrupt Enable for Target 0
#define RV_PLIC_IE0_1_REG_OFFSET 0x204
#define RV_PLIC_IE0_1_E_32_BIT 0
#define RV_PLIC_IE0_1_E_33_BIT 1
#define RV_PLIC_IE0_1_E_34_BIT 2
#define RV_PLIC_IE0_1_E_35_BIT 3
#define RV_PLIC_IE0_1_E_36_BIT 4
#define RV_PLIC_IE0_1_E_37_BIT 5
#define RV_PLIC_IE0_1_E_38_BIT 6
#define RV_PLIC_IE0_1_E_39_BIT 7
#define RV_PLIC_IE0_1_E_40_BIT 8
#define RV_PLIC_IE0_1_E_41_BIT 9
#define RV_PLIC_IE0_1_E_42_BIT 10
#define RV_PLIC_IE0_1_E_43_BIT 11
#define RV_PLIC_IE0_1_E_44_BIT 12
#define RV_PLIC_IE0_1_E_45_BIT 13
#define RV_PLIC_IE0_1_E_46_BIT 14
#define RV_PLIC_IE0_1_E_47_BIT 15
#define RV_PLIC_IE0_1_E_48_BIT 16
#define RV_PLIC_IE0_1_E_49_BIT 17
#define RV_PLIC_IE0_1_E_50_BIT 18
#define RV_PLIC_IE0_1_E_51_BIT 19
#define RV_PLIC_IE0_1_E_52_BIT 20
#define RV_PLIC_IE0_1_E_53_BIT 21
#define RV_PLIC_IE0_1_E_54_BIT 22
#define RV_PLIC_IE0_1_E_55_BIT 23
#define RV_PLIC_IE0_1_E_56_BIT 24
#define RV_PLIC_IE0_1_E_57_BIT 25
#define RV_PLIC_IE0_1_E_58_BIT 26
#define RV_PLIC_IE0_1_E_59_BIT 27
#define RV_PLIC_IE0_1_E_60_BIT 28
#define RV_PLIC_IE0_1_E_61_BIT 29
#define RV_PLIC_IE0_1_E_62_BIT 30
#define RV_PLIC_IE0_1_E_63_BIT 31

// Threshold of priority for Target 0
#define RV_PLIC_THRESHOLD0_REG_OFFSET 0x208
#define RV_PLIC_THRESHOLD0_THRESHOLD0_MASK 0x7
#define RV_PLIC_THRESHOLD0_THRESHOLD0_OFFSET 0
#define RV_PLIC_THRESHOLD0_THRESHOLD0_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_THRESHOLD0_THRESHOLD0_MASK, .index = RV_PLIC_THRESHOLD0_THRESHOLD0_OFFSET })

// Claim interrupt by read, complete interrupt by write for Target 0.
#define RV_PLIC_CC0_REG_OFFSET 0x20c
#define RV_PLIC_CC0_CC0_MASK 0x3f
#define RV_PLIC_CC0_CC0_OFFSET 0
#define RV_PLIC_CC0_CC0_FIELD \
  ((bitfield_field32_t) { .mask = RV_PLIC_CC0_CC0_MASK, .index = RV_PLIC_CC0_CC0_OFFSET })

// msip for Hart 0.
#define RV_PLIC_MSIP0_REG_OFFSET 0x210
#define RV_PLIC_MSIP0_MSIP0_BIT 0

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _RV_PLIC_REG_DEFS_
// End generated register defines for RV_PLIC
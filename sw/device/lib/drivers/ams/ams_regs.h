// Generated register defines for ams

// Copyright information found in source file:
// Copyright EPFL contributors.

// Licensing information found in source file:
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef _AMS_REG_DEFS_
#define _AMS_REG_DEFS_

#ifdef __cplusplus
extern "C" {
#endif
// Register width
#define AMS_PARAM_REG_WIDTH 32

// Select the ADC threshold value (20%, 40%, 60%, 80% of VDD)
#define AMS_SEL_REG_OFFSET 0x0
#define AMS_SEL_VALUE_MASK 0x3
#define AMS_SEL_VALUE_OFFSET 0
#define AMS_SEL_VALUE_FIELD \
  ((bitfield_field32_t) { .mask = AMS_SEL_VALUE_MASK, .index = AMS_SEL_VALUE_OFFSET })
#define AMS_SEL_UNUSED_MASK 0x3fffffff
#define AMS_SEL_UNUSED_OFFSET 2
#define AMS_SEL_UNUSED_FIELD \
  ((bitfield_field32_t) { .mask = AMS_SEL_UNUSED_MASK, .index = AMS_SEL_UNUSED_OFFSET })

// Get the ADC output
#define AMS_GET_REG_OFFSET 0x4
#define AMS_GET_VALUE_BIT 0
#define AMS_GET_UNUSED_MASK 0x7fffffff
#define AMS_GET_UNUSED_OFFSET 1
#define AMS_GET_UNUSED_FIELD \
  ((bitfield_field32_t) { .mask = AMS_GET_UNUSED_MASK, .index = AMS_GET_UNUSED_OFFSET })

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _AMS_REG_DEFS_
// End generated register defines for ams
// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef _SOC_CTRL_REG_DEFS_
#define _SOC_CTRL_REG_DEFS_

#ifdef __cplusplus
extern "C" {
#endif
// Register width
#define SOC_CTRL_PARAM_REG_WIDTH 32

// Exit Valid - Used to write exit valid bit
#define SOC_CTRL_EXIT_VALID_REG_OFFSET 0x0
#define SOC_CTRL_EXIT_VALID_EXIT_VALID_BIT 0

// Exit Value - Used to write exit value register
#define SOC_CTRL_EXIT_VALUE_REG_OFFSET 0x4

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _SOC_CTRL_REG_DEFS_
// End generated register defines for soc_ctrl

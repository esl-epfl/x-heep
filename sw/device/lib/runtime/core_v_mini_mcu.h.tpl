// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#ifndef COREV_MINI_MCU_H_
#define COREV_MINI_MCU_H_

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define DEBUG_START_ADDRESS 0x${debug_start_address}
#define DEBUG_SIZE 0x${debug_size_address}
#define DEBUG_END_ADDRESS (DEBUG_START_ADDRESS + DEBUG_SIZE)

#define PERIPHERAL_START_ADDRESS 0x${debug_start_address}
#define PERIPHERAL_SIZE 0x${peripheral_size_address}
#define PERIPHERAL_END_ADDRESS (PERIPHERAL_START_ADDRESS + PERIPHERAL_SIZE)

#define SOC_CTRL_IDX_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${soc_ctrl_start_offset})
#define SOC_CTRL_IDX_SIZE 0x${soc_ctrl_size_address}
#define SOC_CTRL_IDX_END_ADDRESS (SOC_CTRL_IDX_START_ADDRESS + SOC_CTRL_IDX_SIZE)

#define UART_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${uart_start_offset})
#define UART_SIZE 0x${uart_size_address}
#define UART_END_ADDRESS (UART_START_ADDRESS + UART_SIZE)

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // COREV_MINI_MCU_H_
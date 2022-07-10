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

#define PERIPHERAL_START_ADDRESS 0x${peripheral_start_address}
#define PERIPHERAL_SIZE 0x${peripheral_size_address}
#define PERIPHERAL_END_ADDRESS (PERIPHERAL_START_ADDRESS + PERIPHERAL_SIZE)

#define EXT_SLAVE_START_ADDRESS 0x${ext_slave_start_address}
#define EXT_SLAVE_SIZE 0x${ext_slave_size_address}
#define EXT_SLAVE_END_ADDRESS (EXT_SLAVE_START_ADDRESS + EXT_SLAVE_SIZE)

#define SOC_CTRL_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${soc_ctrl_start_offset})
#define SOC_CTRL_SIZE 0x${soc_ctrl_size_address}
#define SOC_CTRL_END_ADDRESS (SOC_CTRL_IDX_START_ADDRESS + SOC_CTRL_IDX_SIZE)

#define UART_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${uart_start_offset})
#define UART_SIZE 0x${uart_size_address}
#define UART_END_ADDRESS (UART_START_ADDRESS + UART_SIZE)

#define EXT_PERIPHERAL_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${ext_periph_start_offset})
#define EXT_PERIPHERAL_SIZE 0x${ext_periph_size_address}
#define EXT_PERIPHERAL_END_ADDRESS (EXT_PERIPHERAL_START_ADDRESS + EXT_PERIPHERAL_SIZE)

#define PLIC_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${plic_start_offset})
#define PLIC_SIZE 0x${plic_size_address}
#define PLIC_END_ADDRESS (PLIC_START_ADDRESS + PLIC_SIZE)

#define NULL_INTR ${null_intr}
#define UART_INTR_TX_WATERMARK ${uart_intr_tx_watermark}
#define UART_INTR_RX_WATERMARK ${uart_intr_rx_watermark}
#define UART_INTR_TX_EMPT ${uart_intr_tx_empty}
#define UART_INTR_RX_OVERFLOW ${uart_intr_rx_overflow}
#define UART_INTR_RX_FRAME_ERR ${uart_intr_rx_frame_err}
#define UART_INTR_RX_BREAK_ERR ${uart_intr_rx_break_err}
#define UART_INTR_RX_TIMEOUT ${uart_intr_rx_timeout}
#define UART_INTR_RX_PARITY_ERR ${uart_intr_rx_parity_err}
#define MEMCOPY_INTR_DONE ${memcopy_intr_done}

#define RV_TIMER_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${rv_timer_start_offset})
#define RV_TIMER_SIZE 0x${rv_timer_size_address}
#define RV_TIMER_END_ADDRESS (RV_TIMER_START_ADDRESS + RV_TIMER_SIZE)

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // COREV_MINI_MCU_H_

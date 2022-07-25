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

#define EXT_PERIPHERAL_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${ext_periph_start_offset})
#define EXT_PERIPHERAL_SIZE 0x${ext_periph_size_address}
#define EXT_PERIPHERAL_END_ADDRESS (EXT_PERIPHERAL_START_ADDRESS + EXT_PERIPHERAL_SIZE)

#define SOC_CTRL_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${soc_ctrl_start_offset})
#define SOC_CTRL_SIZE 0x${soc_ctrl_size_address}
#define SOC_CTRL_END_ADDRESS (SOC_CTRL_IDX_START_ADDRESS + SOC_CTRL_IDX_SIZE)

#define UART_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${uart_start_offset})
#define UART_SIZE 0x${uart_size_address}
#define UART_END_ADDRESS (UART_START_ADDRESS + UART_SIZE)

#define PLIC_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${plic_start_offset})
#define PLIC_SIZE 0x${plic_size_address}
#define PLIC_END_ADDRESS (PLIC_START_ADDRESS + PLIC_SIZE)

#define RV_TIMER_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${rv_timer_start_offset})
#define RV_TIMER_SIZE 0x${rv_timer_size_address}
#define RV_TIMER_END_ADDRESS (RV_TIMER_START_ADDRESS + RV_TIMER_SIZE)

#define GPIO_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${gpio_start_offset})
#define GPIO_SIZE 0x${gpio_size_address}
#define GPIO_END_ADDRESS (GPIO_START_ADDRESS + GPIO_SIZE)

#define SPI_HOST_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${spi_host_start_offset})
#define SPI_HOST_SIZE 0x${spi_host_size_address}
#define SPI_HOST_END_ADDRESS (SPI_HOST_START_ADDRESS + SPI_HOST_SIZE)

#define NULL_INTR ${null_intr}
#define UART_INTR_TX_WATERMARK ${uart_intr_tx_watermark}
#define UART_INTR_RX_WATERMARK ${uart_intr_rx_watermark}
#define UART_INTR_TX_EMPT ${uart_intr_tx_empty}
#define UART_INTR_RX_OVERFLOW ${uart_intr_rx_overflow}
#define UART_INTR_RX_FRAME_ERR ${uart_intr_rx_frame_err}
#define UART_INTR_RX_BREAK_ERR ${uart_intr_rx_break_err}
#define UART_INTR_RX_TIMEOUT ${uart_intr_rx_timeout}
#define UART_INTR_RX_PARITY_ERR ${uart_intr_rx_parity_err}
#define GPIO_INTR_0 ${gpio_intr_0}
#define GPIO_INTR_1 ${gpio_intr_1}
#define GPIO_INTR_2 ${gpio_intr_2}
#define GPIO_INTR_3 ${gpio_intr_3}
#define GPIO_INTR_4 ${gpio_intr_4}
#define GPIO_INTR_5 ${gpio_intr_5}
#define GPIO_INTR_6 ${gpio_intr_6}
#define GPIO_INTR_7 ${gpio_intr_7}
#define GPIO_INTR_8 ${gpio_intr_8}
#define GPIO_INTR_9 ${gpio_intr_9}
#define GPIO_INTR_10 ${gpio_intr_10}
#define GPIO_INTR_11 ${gpio_intr_11}
#define GPIO_INTR_12 ${gpio_intr_12}
#define GPIO_INTR_13 ${gpio_intr_13}
#define GPIO_INTR_14 ${gpio_intr_14}
#define GPIO_INTR_15 ${gpio_intr_15}
#define GPIO_INTR_16 ${gpio_intr_16}
#define GPIO_INTR_17 ${gpio_intr_17}
#define GPIO_INTR_18 ${gpio_intr_18}
#define GPIO_INTR_19 ${gpio_intr_19}
#define GPIO_INTR_20 ${gpio_intr_20}
#define GPIO_INTR_21 ${gpio_intr_21}
#define GPIO_INTR_22 ${gpio_intr_22}
#define GPIO_INTR_23 ${gpio_intr_23}
#define GPIO_INTR_24 ${gpio_intr_24}
#define GPIO_INTR_25 ${gpio_intr_25}
#define GPIO_INTR_26 ${gpio_intr_26}
#define GPIO_INTR_27 ${gpio_intr_27}
#define GPIO_INTR_28 ${gpio_intr_28}
#define GPIO_INTR_29 ${gpio_intr_29}
#define GPIO_INTR_30 ${gpio_intr_30}
#define GPIO_INTR_31 ${gpio_intr_31}
#define MEMCOPY_INTR_DONE ${memcopy_intr_done}

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // COREV_MINI_MCU_H_

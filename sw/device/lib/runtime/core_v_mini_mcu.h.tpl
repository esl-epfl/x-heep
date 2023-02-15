// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#ifndef COREV_MINI_MCU_H_
#define COREV_MINI_MCU_H_

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define MEMORY_BANKS ${ram_numbanks}

#define DEBUG_START_ADDRESS 0x${debug_start_address}
#define DEBUG_SIZE 0x${debug_size_address}
#define DEBUG_END_ADDRESS (DEBUG_START_ADDRESS + DEBUG_SIZE)

//always-on peripherals
#define AO_PERIPHERAL_START_ADDRESS 0x${ao_peripheral_start_address}
#define AO_PERIPHERAL_SIZE 0x${ao_peripheral_size_address}
#define AO_PERIPHERAL_END_ADDRESS (AO_PERIPHERAL_START_ADDRESS + AO_PERIPHERAL_SIZE)

% for name, peripheral in ao_peripherals.items():
#define ${name.upper()}_START_ADDRESS (AO_PERIPHERAL_START_ADDRESS + 0x${peripheral['offset']})
#define ${name.upper()}_SIZE 0x${peripheral['length']}
#define ${name.upper()}_END_ADDRESS (${name.upper()}_START_ADDRESS + ${name.upper()}_SIZE)

%endfor

//switch-on/off peripherals
#define PERIPHERAL_START_ADDRESS 0x${peripheral_start_address}
#define PERIPHERAL_SIZE 0x${peripheral_size_address}
#define PERIPHERAL_END_ADDRESS (PERIPHERAL_START_ADDRESS + PERIPHERAL_SIZE)

% for name, peripheral in peripherals.items():
#define ${name.upper()}_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${peripheral['offset']})
#define ${name.upper()}_SIZE 0x${peripheral['length']}
#define ${name.upper()}_END_ADDRESS (${name.upper()}_START_ADDRESS + ${name.upper()}_SIZE)

%endfor

#define EXT_SLAVE_START_ADDRESS 0x${ext_slave_start_address}
#define EXT_SLAVE_SIZE 0x${ext_slave_size_address}
#define EXT_SLAVE_END_ADDRESS (EXT_SLAVE_START_ADDRESS + EXT_SLAVE_SIZE)

#define FLASH_MEM_START_ADDRESS 0x${flash_mem_start_address}
#define FLASH_MEM_SIZE 0x${flash_mem_size_address}
#define FLASH_MEM_END_ADDRESS (FLASH_MEM_START_ADDRESS + FLASH_MEM_SIZE)

#define NULL_INTR ${null_intr}
#define UART_INTR_TX_WATERMARK ${uart_intr_tx_watermark}
#define UART_INTR_RX_WATERMARK ${uart_intr_rx_watermark}
#define UART_INTR_TX_EMPT ${uart_intr_tx_empty}
#define UART_INTR_RX_OVERFLOW ${uart_intr_rx_overflow}
#define UART_INTR_RX_FRAME_ERR ${uart_intr_rx_frame_err}
#define UART_INTR_RX_BREAK_ERR ${uart_intr_rx_break_err}
#define UART_INTR_RX_TIMEOUT ${uart_intr_rx_timeout}
#define UART_INTR_RX_PARITY_ERR ${uart_intr_rx_parity_err}
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
#define INTR_FMT_WATERMARK ${intr_fmt_watermark}
#define INTR_RX_WATERMARK ${intr_rx_watermark}
#define INTR_FMT_OVERFLOW ${intr_fmt_overflow}
#define INTR_RX_OVERFLOW ${intr_rx_overflow}
#define INTR_NAK ${intr_nak}
#define INTR_SCL_INTERFERENCE ${intr_scl_interference}
#define INTR_SDA_INTERFERENCE ${intr_sda_interference}
#define INTR_STRETCH_TIMEOUT ${intr_stretch_timeout}
#define INTR_SDA_UNSTABLE ${intr_sda_unstable}
#define INTR_TRANS_COMPLETE ${intr_trans_complete}
#define INTR_TX_EMPTY ${intr_tx_empty}
#define INTR_TX_NONEMPTY ${intr_tx_nonempty}
#define INTR_TX_OVERFLOW ${intr_tx_overflow}
#define INTR_ACQ_OVERFLOW ${intr_acq_overflow}
#define INTR_ACK_STOP ${intr_ack_stop}
#define INTR_HOST_TIMEOUT ${intr_host_timeout}
#define SPI2_INTR_EVENT ${spi2_intr_event}
% for ext_int_cnt, ext_int_val in zip(range(0,len(ext_int_list)), ext_int_list):
#define EXT_INTR_${ext_int_cnt} ${ext_int_val}
% endfor

% for pad in pad_list:
#define ${pad.localparam}_ATTRIBUTE ${pad.index}
% endfor

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // COREV_MINI_MCU_H_

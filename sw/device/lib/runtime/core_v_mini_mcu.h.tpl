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

#define SOC_CTRL_START_ADDRESS (AO_PERIPHERAL_START_ADDRESS + 0x${soc_ctrl_start_offset})
#define SOC_CTRL_SIZE 0x${soc_ctrl_size_address}
#define SOC_CTRL_END_ADDRESS (SOC_CTRL_IDX_START_ADDRESS + SOC_CTRL_IDX_SIZE)

#define BOOTROM_START_ADDRESS (AO_PERIPHERAL_START_ADDRESS + 0x${bootrom_start_offset})
#define BOOTROM_SIZE 0x${bootrom_size_address}
#define BOOTROM_END_ADDRESS (BOOTROM_START_ADDRESS + BOOTROM_SIZE)

#define SPI_FLASH_START_ADDRESS (AO_PERIPHERAL_START_ADDRESS + 0x${spi_flash_start_offset})
#define SPI_FLASH_SIZE 0x${spi_flash_size_address}
#define SPI_FLASH_END_ADDRESS (SPI_FLASH_START_ADDRESS + SPI_FLASH_SIZE)

#define SPI_MEMIO_START_ADDRESS (AO_PERIPHERAL_START_ADDRESS + 0x${spi_memio_start_offset})
#define SPI_MEMIO_SIZE 0x${spi_memio_size_address}
#define SPI_MEMIO_END_ADDRESS (SPI_MEMIO_START_ADDRESS + SPI_MEMIO_SIZE)

#define SPI_START_ADDRESS (AO_PERIPHERAL_START_ADDRESS + 0x${spi_start_offset})
#define SPI_SIZE 0x${spi_size_address}
#define SPI_END_ADDRESS (SPI_START_ADDRESS + SPI_SIZE)

#define POWER_MANAGER_START_ADDRESS (AO_PERIPHERAL_START_ADDRESS + 0x${power_manager_start_offset})
#define POWER_MANAGER_SIZE 0x${power_manager_size_address}
#define POWER_MANAGER_END_ADDRESS (POWER_MANAGER_START_ADDRESS + POWER_MANAGER_SIZE)

#define RV_TIMER_AO_START_ADDRESS (AO_PERIPHERAL_START_ADDRESS + 0x${rv_timer_ao_start_offset})
#define RV_TIMER_AO_SIZE 0x${rv_timer_ao_size_address}
#define RV_TIMER_AO_END_ADDRESS (RV_TIMER_AO_START_ADDRESS + RV_TIMER_AO_SIZE)

#define DMA_START_ADDRESS (AO_PERIPHERAL_START_ADDRESS + 0x${dma_start_offset})
#define DMA_SIZE 0x${dma_size_address}
#define DMA_END_ADDRESS (DMA_START_ADDRESS + DMA_SIZE)

#define FAST_INTR_CTRL_START_ADDRESS (AO_PERIPHERAL_START_ADDRESS + 0x${fast_intr_ctrl_start_offset})
#define FAST_INTR_CTRL_SIZE 0x${fast_intr_ctrl_size_address}
#define FAST_INTR_CTRL_END_ADDRESS (FAST_INTR_CTRL_START_ADDRESS + DMA_SIZE)

#define EXT_PERIPHERAL_START_ADDRESS (AO_PERIPHERAL_START_ADDRESS + 0x${ext_periph_start_offset})
#define EXT_PERIPHERAL_SIZE 0x${ext_periph_size_address}
#define EXT_PERIPHERAL_END_ADDRESS (EXT_PERIPHERAL_START_ADDRESS + EXT_PERIPHERAL_SIZE)

#define PAD_CONTROL_START_ADDRESS (AO_PERIPHERAL_START_ADDRESS + 0x${pad_control_start_offset})
#define PAD_CONTROL_SIZE 0x${pad_control_size_address}
#define PAD_CONTROL_END_ADDRESS (PAD_CONTROL_START_ADDRESS + PAD_CONTROL_SIZE)

#define GPIO_AO_START_ADDRESS (AO_PERIPHERAL_START_ADDRESS + 0x${gpio_ao_start_offset})
#define GPIO_AO_SIZE 0x${gpio_ao_size_address}
#define GPIO_AO_END_ADDRESS (GPIO_AO_START_ADDRESS + GPIO_AO_SIZE)

#define UART_START_ADDRESS (AO_PERIPHERAL_START_ADDRESS + 0x${uart_start_offset})
#define UART_SIZE 0x${uart_size_address}
#define UART_END_ADDRESS (UART_START_ADDRESS + UART_SIZE)

//switch-on/off peripherals
#define PERIPHERAL_START_ADDRESS 0x${peripheral_start_address}
#define PERIPHERAL_SIZE 0x${peripheral_size_address}
#define PERIPHERAL_END_ADDRESS (PERIPHERAL_START_ADDRESS + PERIPHERAL_SIZE)

#define PLIC_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${plic_start_offset})
#define PLIC_SIZE 0x${plic_size_address}
#define PLIC_END_ADDRESS (PLIC_START_ADDRESS + PLIC_SIZE)

#define GPIO_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${gpio_start_offset})
#define GPIO_SIZE 0x${gpio_size_address}
#define GPIO_END_ADDRESS (GPIO_START_ADDRESS + GPIO_SIZE)

#define I2C_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${i2c_start_offset})
#define I2C_SIZE 0x${i2c_size_address}
#define I2C_END_ADDRESS (I2C_START_ADDRESS + I2C_SIZE)

#define RV_TIMER_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${rv_timer_start_offset})
#define RV_TIMER_SIZE 0x${rv_timer_size_address}
#define RV_TIMER_END_ADDRESS (RV_TIMER_START_ADDRESS + RV_TIMER_SIZE)

#define SPI2_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${spi2_start_offset})
#define SPI2_SIZE 0x${spi2_size_address}
#define SPI2_END_ADDRESS (SPI2_START_ADDRESS + SPI2_SIZE)

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
#define EXT_INTR_0 ${ext_intr_0}
#define EXT_INTR_1 ${ext_intr_1}
#define EXT_INTR_2 ${ext_intr_2}
#define EXT_INTR_3 ${ext_intr_3}
#define EXT_INTR_4 ${ext_intr_4}
#define EXT_INTR_5 ${ext_intr_5}
#define EXT_INTR_6 ${ext_intr_6}
#define EXT_INTR_7 ${ext_intr_7}
#define EXT_INTR_8 ${ext_intr_8}
#define EXT_INTR_9 ${ext_intr_9}
#define EXT_INTR_10 ${ext_intr_10}
#define EXT_INTR_11 ${ext_intr_11}
#define EXT_INTR_12 ${ext_intr_12}
#define EXT_INTR_13 ${ext_intr_13}

% for pad in pad_list:
#define ${pad.localparam}_ATTRIBUTE ${pad.index}
% endfor

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // COREV_MINI_MCU_H_

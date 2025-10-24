// Generated register defines for soc_ctrl

// Copyright information found in source file:
// Copyright lowRISC contributors.

// Licensing information found in source file:
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

// Boot Select Value - Used to decide whether to boot from JTAG or FLASH
#define SOC_CTRL_BOOT_SELECT_REG_OFFSET 0x8
#define SOC_CTRL_BOOT_SELECT_BOOT_SELECT_BIT 0

// Boot Exit Loop Value - Set externally (e.g. JTAG, TESTBENCH, or another
// MASTER) to make the CPU jump to the main function entry
#define SOC_CTRL_BOOT_EXIT_LOOP_REG_OFFSET 0xc
#define SOC_CTRL_BOOT_EXIT_LOOP_BOOT_EXIT_LOOP_BIT 0

// Boot Address Value - Used in the boot rom or power-on-reset functions
#define SOC_CTRL_BOOT_ADDRESS_REG_OFFSET 0x10

// Spi Module Select Value - Used to decide whether to use the SPI from Yosys
// or OpenTitan
#define SOC_CTRL_USE_SPIMEMIO_REG_OFFSET 0x14
#define SOC_CTRL_USE_SPIMEMIO_USE_SPIMEMIO_BIT 0

// Enable Spi module selection from software
#define SOC_CTRL_ENABLE_SPI_SEL_REG_OFFSET 0x18
#define SOC_CTRL_ENABLE_SPI_SEL_ENABLE_SPI_SEL_BIT 0

// System Frequency Value - Used to know and set at which frequency the
// system is running (in Hz)
#define SOC_CTRL_SYSTEM_FREQUENCY_HZ_REG_OFFSET 0x1c

// In case of multiple X-HEEP instances, it tells you the X-HEEP instance id
#define SOC_CTRL_XHEEP_ID_REG_OFFSET 0x20

// Tells you about the ao_peripheral config
#define SOC_CTRL_XHEEP_AO_PERIPHERAL_CONFIG_REG_OFFSET 0x24
#define SOC_CTRL_XHEEP_AO_PERIPHERAL_CONFIG_SPI_FLASH_BIT 0
#define SOC_CTRL_XHEEP_AO_PERIPHERAL_CONFIG_DMA_BIT 1
#define SOC_CTRL_XHEEP_AO_PERIPHERAL_CONFIG_PAD_CONTROL_BIT 2
#define SOC_CTRL_XHEEP_AO_PERIPHERAL_CONFIG_GPIO_AO_BIT 3

// Tells you about the peripheral config
#define SOC_CTRL_XHEEP_PERIPHERAL_CONFIG_REG_OFFSET 0x28
#define SOC_CTRL_XHEEP_PERIPHERAL_CONFIG_RV_PLIC_BIT 0
#define SOC_CTRL_XHEEP_PERIPHERAL_CONFIG_SPI_HOST_BIT 1
#define SOC_CTRL_XHEEP_PERIPHERAL_CONFIG_GPIO_BIT 2
#define SOC_CTRL_XHEEP_PERIPHERAL_CONFIG_I2C_BIT 3
#define SOC_CTRL_XHEEP_PERIPHERAL_CONFIG_RV_TIMER_BIT 4
#define SOC_CTRL_XHEEP_PERIPHERAL_CONFIG_SPI2_BIT 5
#define SOC_CTRL_XHEEP_PERIPHERAL_CONFIG_PDM2PCM_BIT 6
#define SOC_CTRL_XHEEP_PERIPHERAL_CONFIG_I2S_BIT 7
#define SOC_CTRL_XHEEP_PERIPHERAL_CONFIG_UART_BIT 8

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _SOC_CTRL_REG_DEFS_
// End generated register defines for soc_ctrl
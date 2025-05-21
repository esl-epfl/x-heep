// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

<%
    user_peripheral_domain = xheep.get_user_peripheral_domain()
    base_peripheral_domain = xheep.get_base_peripheral_domain()
    dma = base_peripheral_domain.get_dma()
%>

#ifndef COREV_MINI_MCU_H_
#define COREV_MINI_MCU_H_

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define MEMORY_BANKS ${xheep.ram_numbanks()}
% if xheep.has_il_ram():
#define HAS_MEMORY_BANKS_IL
% endif

% for bank in xheep.iter_ram_banks():
#define RAM${bank.name()}_START_ADDRESS 0x${f'{bank.start_address():08X}'}
#define RAM${bank.name()}_END_ADDRESS 0x${f'{bank.end_address():08X}'}
% endfor


#define EXTERNAL_DOMAINS ${external_domains}

#define DEBUG_START_ADDRESS 0x${debug_start_address}
#define DEBUG_SIZE 0x${debug_size_address}
#define DEBUG_END_ADDRESS (DEBUG_START_ADDRESS + DEBUG_SIZE)

// base peripherals
#define AO_PERIPHERAL_START_ADDRESS ${hex(base_peripheral_domain.get_start_address())}
#define AO_PERIPHERAL_SIZE ${hex(base_peripheral_domain.get_length())}
#define AO_PERIPHERAL_END_ADDRESS (AO_PERIPHERAL_START_ADDRESS + AO_PERIPHERAL_SIZE)

% for peripheral in base_peripheral_domain.get_peripherals():
#define ${peripheral.get_name().upper()}_START_ADDRESS (AO_PERIPHERAL_START_ADDRESS + ${hex(peripheral.get_address())})
#define ${peripheral.get_name().upper()}_SIZE ${hex(peripheral.get_length())}
#define ${peripheral.get_name().upper()}_END_ADDRESS (${peripheral.get_name().upper()}_START_ADDRESS + ${peripheral.get_name().upper()}_SIZE)
#define ${peripheral.get_name().upper()}_IDX ${loop.index}

%endfor

#define DMA_CH_NUM ${hex(dma.get_num_channels())[2:]}
#define DMA_CH_SIZE 0x${hex(dma.get_ch_length())[2:]}
#define DMA_NUM_MASTER_PORTS ${hex(dma.get_num_master_ports())[2:]}
#define DMA_ADDR_MODE ${dma.get_addr_mode()}
#define DMA_SUBADDR_MODE ${dma.get_subaddr_mode()}
#define DMA_HW_FIFO_MODE ${dma.get_hw_fifo_mode()}
#define DMA_ZERO_PADDING ${dma.get_zero_padding()}

// user peripherals
#define PERIPHERAL_START_ADDRESS ${hex(user_peripheral_domain.get_start_address())}
#define PERIPHERAL_SIZE ${hex(user_peripheral_domain.get_length())}
#define PERIPHERAL_END_ADDRESS (PERIPHERAL_START_ADDRESS + PERIPHERAL_SIZE)

% for peripheral in user_peripheral_domain.get_peripherals():
#define ${peripheral.get_name().upper()}_START_ADDRESS (PERIPHERAL_START_ADDRESS + ${hex(peripheral.get_address())})
#define ${peripheral.get_name().upper()}_SIZE ${hex(peripheral.get_length())}
#define ${peripheral.get_name().upper()}_END_ADDRESS (${peripheral.get_name().upper()}_START_ADDRESS + ${peripheral.get_name().upper()}_SIZE)
#define ${peripheral.get_name().upper()}_IDX ${loop.index + len(base_peripheral_domain.get_peripherals())}
#define ${peripheral.get_name().upper()}_IS_INCLUDED
%endfor

// This section is here to have default values for the peripherals that are not included in the user peripheral domain. Their are used in their respective structs.h files.
// Some other files, like applications main c file, use also some peripheral attributes but the file is not generated if the peripheral is not included in the user peripheral domain.
% if not user_peripheral_domain.contains_peripheral('rv_plic'):
#define RV_PLIC_START_ADDRESS 0
% endif
% if not user_peripheral_domain.contains_peripheral('spi_host'):
#define SPI_HOST_START_ADDRESS 0
% endif
% if not user_peripheral_domain.contains_peripheral('gpio'):
#define GPIO_START_ADDRESS 0
% endif
% if not user_peripheral_domain.contains_peripheral('i2c'):
#define I2C_START_ADDRESS 0
% endif
% if not user_peripheral_domain.contains_peripheral('rv_timer'):
#define RV_TIMER_START_ADDRESS 0
% endif
% if not user_peripheral_domain.contains_peripheral('spi2'):
#define SPI2_START_ADDRESS 0
% endif
% if not user_peripheral_domain.contains_peripheral('pdm2pcm'):
#define PDM2PCM_START_ADDRESS 0
% endif
% if not user_peripheral_domain.contains_peripheral('i2s'):
#define I2S_START_ADDRESS 0
% endif
// End of the section

#define EXT_SLAVE_START_ADDRESS 0x${ext_slave_start_address}
#define EXT_SLAVE_SIZE 0x${ext_slave_size_address}
#define EXT_SLAVE_END_ADDRESS (EXT_SLAVE_START_ADDRESS + EXT_SLAVE_SIZE)

#define FLASH_MEM_START_ADDRESS 0x${flash_mem_start_address}
#define FLASH_MEM_SIZE 0x${flash_mem_size_address}
#define FLASH_MEM_END_ADDRESS (FLASH_MEM_START_ADDRESS + FLASH_MEM_SIZE)

#define QTY_INTR ${len(interrupts)}
% for key, value in interrupts.items():
#define ${key.upper()} ${value}
% endfor

% if pads_attributes != None:
% for pad in pad_list:
#define ${pad.localparam}_ATTRIBUTE ${pad.index}
% endfor
% endif

#define GPIO_AO_DOMAIN_LIMIT 8


#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // COREV_MINI_MCU_H_

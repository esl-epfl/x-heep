// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

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

//always-on peripherals
#define AO_PERIPHERAL_START_ADDRESS 0x${f"{xheep.get_base_peripherals_base_address() & 0xFFFFFFFF:08X}"}
#define AO_PERIPHERAL_SIZE 0x${f"{xheep.get_base_peripherals_length() & 0xFFFFFFFF:08X}"}
#define AO_PERIPHERAL_END_ADDRESS (AO_PERIPHERAL_START_ADDRESS + AO_PERIPHERAL_SIZE)

% for peripheral in xheep.get_base_peripherals():
#define ${peripheral.get_name().upper()}_START_ADDRESS (AO_PERIPHERAL_START_ADDRESS + 0x${f"{peripheral.get_address() & 0xFFFFFFFF:08X}"})
#define ${peripheral.get_name().upper()}_SIZE 0x${f"{peripheral.get_length() & 0xFFFFFFFF:08X}"}
#define ${peripheral.get_name().upper()}_END_ADDRESS (${peripheral.get_name().upper()}_START_ADDRESS + ${peripheral.get_name().upper()}_SIZE)
#define ${peripheral.get_name().upper()}_IDX ${loop.index}

%endfor

#define DMA_CH_NUM ${hex(xheep.get_dma()[0].get_num_channels())[2:]}
#define DMA_CH_SIZE 0x${hex(xheep.get_dma()[0].get_ch_length())[2:]}
#define DMA_NUM_MASTER_PORTS ${hex(xheep.get_dma()[0].get_num_master_ports())[2:]}

//switch-on/off peripherals
#define PERIPHERAL_START_ADDRESS 0x${f"{xheep.get_user_peripherals_base_address() & 0xFFFFFFFF:08X}"}
#define PERIPHERAL_SIZE 0x${f"{xheep.get_user_peripherals_length() & 0xFFFFFFFF:08X}"}
#define PERIPHERAL_END_ADDRESS (PERIPHERAL_START_ADDRESS + PERIPHERAL_SIZE)

% for peripheral in xheep.get_user_peripherals():
#define ${peripheral.get_name().upper()}_START_ADDRESS (PERIPHERAL_START_ADDRESS + 0x${f"{peripheral.get_address() & 0xFFFFFFFF:08X}"})
#define ${peripheral.get_name().upper()}_SIZE 0x${f"{peripheral.get_length() & 0xFFFFFFFF:08X}"}
#define ${peripheral.get_name().upper()}_END_ADDRESS (${peripheral.get_name().upper()}_START_ADDRESS + ${peripheral.get_name().upper()}_SIZE)
#define ${peripheral.get_name().upper()}_IDX ${loop.index + len(xheep.get_base_peripherals())}
#define ${peripheral.get_name().upper()}_IS_INCLUDED
%endfor

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

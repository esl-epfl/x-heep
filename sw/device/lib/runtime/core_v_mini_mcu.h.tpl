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


% for domain in xheep.iter_peripheral_domains():
#define ${domain.get_name().upper()}_START_ADDRESS ${f"0x{domain.get_address():08X}"}
#define ${domain.get_name().upper()}_SIZE ${f"0x{domain.get_address_length():08X}"}
#define ${domain.get_name().upper()}_END_ADDRESS (${domain.get_name().upper()}_START_ADDRESS + ${domain.get_name().upper()}_SIZE)

% for periph in domain.iter_peripherals():
#define ${periph.full_name.upper()}_START_ADDRESS (${domain.get_name().upper()}_START_ADDRESS + ${f"0x{periph.get_offset_checked():08X}"})
#define ${periph.full_name.upper()}_SIZE ${f"0x{periph.get_address_length_checked():08X}"}
#define ${periph.full_name.upper()}_END_ADDRESS (${periph.full_name.upper()}_START_ADDRESS + ${periph.full_name.upper()}_SIZE)
#define ${periph.full_name.upper()}_IS_INCLUDED
// FIXME: Do we need the _IDX from https://github.com/esl-epfl/x-heep/pull/523 ??
%endfor

%endfor

// FIXME: This is not working  as it's not itegrated with the new system
// Just copy-pasted from https://github.com/esl-epfl/x-heep/pull/517 and https://github.com/esl-epfl/x-heep/pull/581
#define DMA_CH_NUM ${dma_ch_count}
#define DMA_CH_SIZE 0x${dma_ch_size}
#define DMA_NUM_MASTER_PORTS ${num_dma_master_ports}

#define EXT_SLAVE_START_ADDRESS 0x${ext_slave_start_address}
#define EXT_SLAVE_SIZE 0x${ext_slave_size_address}
#define EXT_SLAVE_END_ADDRESS (EXT_SLAVE_START_ADDRESS + EXT_SLAVE_SIZE)

#define FLASH_MEM_START_ADDRESS 0x${flash_mem_start_address}
#define FLASH_MEM_SIZE 0x${flash_mem_size_address}
#define FLASH_MEM_END_ADDRESS (FLASH_MEM_START_ADDRESS + FLASH_MEM_SIZE)

##% for key, value in interrupts.items():
###define ${key.upper()} ${value}
##% endfor
<%
    from x_heep_gen.peripherals.rv_plic import RvPlicPeripheral
%>

#define NULL_INTR 0
% for domain in xheep.iter_peripheral_domains():
% for periph in domain.iter_peripherals():
% if isinstance(periph, RvPlicPeripheral):
${periph.make_intr_defs(xheep.get_rh())}
% endif
% endfor
% endfor


% if xheep.get_pad_manager().get_attr_bits() != 0:
% for i, pad in enumerate(xheep.get_pad_manager().iterate_pad_index()):
#define ${pad}_ATTRIBUTE ${i}
% endfor
% endif


#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // COREV_MINI_MCU_H_

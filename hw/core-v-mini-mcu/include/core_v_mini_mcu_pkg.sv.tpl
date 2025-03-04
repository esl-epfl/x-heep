/* Copyright 2018 ETH Zurich and University of Bologna.
 * Copyright and related rights are licensed under the Solderpad Hardware
 * License, Version 0.51 (the “License”); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://solderpad.org/licenses/SHL-0.51. Unless required by applicable law
 * or agreed to in writing, software, hardware and materials distributed under
 * this License is distributed on an “AS IS” BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 *
 * Description: Contains common system definitions.
 *
 */

package core_v_mini_mcu_pkg;

  import addr_map_rule_pkg::*;
  import power_manager_pkg::*;

  typedef enum logic [1:0] {
    cv32e40p,
    cv32e20,
    cv32e40x,
    cv32e40px
  } cpu_type_e;

  localparam cpu_type_e CpuType = ${cpu_type};

  typedef enum logic {
    NtoM,
    onetoM
  } bus_type_e;

  localparam bus_type_e BusType = ${xheep.bus_type().value};

  //master idx
  localparam logic [31:0] CORE_INSTR_IDX = 0;
  localparam logic [31:0] CORE_DATA_IDX = 1;
  localparam logic [31:0] DEBUG_MASTER_IDX = 2;
  localparam logic [31:0] DMA_READ_P0_IDX = 3;
  localparam logic [31:0] DMA_WRITE_P0_IDX = 4;
  localparam logic [31:0] DMA_ADDR_P0_IDX = 5;

  localparam SYSTEM_XBAR_NMASTER = ${3 + int(num_dma_master_ports)*3};

  // Internal slave memory map and index
  // -----------------------------------
  //must be power of two
  localparam int unsigned MEM_SIZE = 32'h${f'{xheep.ram_size_address():08X}'};

  localparam SYSTEM_XBAR_NSLAVE = ${xheep.ram_numbanks() + 5};

  localparam int unsigned LOG_SYSTEM_XBAR_NMASTER = SYSTEM_XBAR_NMASTER > 1 ? $clog2(SYSTEM_XBAR_NMASTER) : 32'd1;
  localparam int unsigned LOG_SYSTEM_XBAR_NSLAVE = SYSTEM_XBAR_NSLAVE > 1 ? $clog2(SYSTEM_XBAR_NSLAVE) : 32'd1;

  localparam int unsigned NUM_BANKS = ${xheep.ram_numbanks()};
  localparam int unsigned NUM_BANKS_IL = ${xheep.ram_numbanks_il()};
  localparam int unsigned EXTERNAL_DOMAINS = ${external_domains};

  localparam logic[31:0] ERROR_START_ADDRESS = 32'hBADACCE5;
  localparam logic[31:0] ERROR_SIZE = 32'h00000001;
  localparam logic[31:0] ERROR_END_ADDRESS = ERROR_START_ADDRESS + ERROR_SIZE;
  localparam logic[31:0] ERROR_IDX = 32'd0;

% for bank in xheep.iter_ram_banks():
  localparam logic [31:0] RAM${bank.name()}_IDX = 32'd${bank.map_idx()};
  localparam logic [31:0] RAM${bank.name()}_SIZE = 32'h${f'{bank.size():08X}'};
  localparam logic [31:0] RAM${bank.name()}_START_ADDRESS = 32'h${f'{bank.start_address():08X}'};
  localparam logic [31:0] RAM${bank.name()}_END_ADDRESS = 32'h${f'{bank.end_address():08X}'};
% endfor

% for i, group in enumerate(xheep.iter_il_groups()):
  localparam logic [31:0] RAM_IL${i}_START_ADDRESS = 32'h${f'{group.start:08X}'};
  localparam logic [31:0] RAM_IL${i}_SIZE = 32'h${f'{group.size:08X}'};
  localparam logic [31:0] RAM_IL${i}_END_ADDRESS = RAM_IL${i}_START_ADDRESS + RAM_IL${i}_SIZE;
  localparam logic [31:0] RAM_IL${i}_IDX = RAM${group.first_name}_IDX;
% endfor

  localparam logic[31:0] DEBUG_START_ADDRESS = 32'h${debug_start_address};
  localparam logic[31:0] DEBUG_SIZE = 32'h${debug_size_address};
  localparam logic[31:0] DEBUG_END_ADDRESS = DEBUG_START_ADDRESS + DEBUG_SIZE;
  localparam logic[31:0] DEBUG_IDX = 32'd${xheep.ram_numbanks() + 1};

  localparam logic[31:0] AO_PERIPHERAL_START_ADDRESS = 32'h${ao_peripheral_start_address};
  localparam logic[31:0] AO_PERIPHERAL_SIZE = 32'h${ao_peripheral_size_address};
  localparam logic[31:0] AO_PERIPHERAL_END_ADDRESS = AO_PERIPHERAL_START_ADDRESS + AO_PERIPHERAL_SIZE;
  localparam logic[31:0] AO_PERIPHERAL_IDX = 32'd${xheep.ram_numbanks() + 2};

  localparam logic[31:0] PERIPHERAL_START_ADDRESS = 32'h${peripheral_start_address};
  localparam logic[31:0] PERIPHERAL_SIZE = 32'h${peripheral_size_address};
  localparam logic[31:0] PERIPHERAL_END_ADDRESS = PERIPHERAL_START_ADDRESS + PERIPHERAL_SIZE;
  localparam logic[31:0] PERIPHERAL_IDX = 32'd${xheep.ram_numbanks() + 3};

  localparam logic[31:0] FLASH_MEM_START_ADDRESS = 32'h${flash_mem_start_address};
  localparam logic[31:0] FLASH_MEM_SIZE = 32'h${flash_mem_size_address};
  localparam logic[31:0] FLASH_MEM_END_ADDRESS = FLASH_MEM_START_ADDRESS + FLASH_MEM_SIZE;
  localparam logic[31:0] FLASH_MEM_IDX = 32'd${xheep.ram_numbanks() + 4};

  localparam addr_map_rule_t [SYSTEM_XBAR_NSLAVE-1:0] XBAR_ADDR_RULES = '{
      '{ idx: ERROR_IDX, start_addr: ERROR_START_ADDRESS, end_addr: ERROR_END_ADDRESS },
% for bank in xheep.iter_ram_banks():
      '{ idx: RAM${bank.name()}_IDX, start_addr: RAM${bank.name()}_START_ADDRESS, end_addr: RAM${bank.name()}_END_ADDRESS },
% endfor
      '{ idx: DEBUG_IDX, start_addr: DEBUG_START_ADDRESS, end_addr: DEBUG_END_ADDRESS },
      '{ idx: AO_PERIPHERAL_IDX, start_addr: AO_PERIPHERAL_START_ADDRESS, end_addr: AO_PERIPHERAL_END_ADDRESS },
      '{ idx: PERIPHERAL_IDX, start_addr: PERIPHERAL_START_ADDRESS, end_addr: PERIPHERAL_END_ADDRESS },
      '{ idx: FLASH_MEM_IDX, start_addr: FLASH_MEM_START_ADDRESS, end_addr: FLASH_MEM_END_ADDRESS }
  };

  // External slave address map
  // --------------------------
  localparam logic [31:0] EXT_SLAVE_START_ADDRESS = 32'h${ext_slave_start_address};
  localparam logic [31:0] EXT_SLAVE_SIZE = 32'h${ext_slave_size_address};
  localparam logic [31:0] EXT_SLAVE_END_ADDRESS = EXT_SLAVE_START_ADDRESS + EXT_SLAVE_SIZE;

  // Forward crossbars address map and index
  // ---------------------------------------
  // These crossbar connect each muster to the internal crossbar and to the
  // corresponding external master port.
  localparam logic [31:0] DEMUX_XBAR_INT_SLAVE_IDX = 32'd0;
  localparam logic[31:0] DEMUX_XBAR_EXT_SLAVE_IDX = 32'd1;

  // Address map
  // NOTE: the internal address space is chosen by default by the system bus,
  // so it is not defined here.
  localparam addr_map_rule_t [0:0] DEMUX_XBAR_ADDR_RULES = '{
    '{
      idx: DEMUX_XBAR_EXT_SLAVE_IDX,
      start_addr: EXT_SLAVE_START_ADDRESS,
      end_addr: EXT_SLAVE_END_ADDRESS
    }
  };

######################################################################
## Automatically add all always on peripherals listed
######################################################################
  // always-on peripherals
  // ---------------------
  localparam AO_PERIPHERALS = ${ao_peripherals_count};
  localparam DMA_CH_NUM = ${dma_ch_count};
  localparam DMA_CH_SIZE = 32'h${dma_ch_size};
  localparam DMA_NUM_MASTER_PORTS = ${num_dma_master_ports};
% if int(num_dma_master_ports) > 1:
  localparam int DMA_XBAR_MASTERS [DMA_NUM_MASTER_PORTS] = '{${dma_xbar_masters_array[::-1]}};
% else:
  localparam int DMA_XBAR_MASTERS [DMA_NUM_MASTER_PORTS] = '{${dma_xbar_masters_array}};
% endif
  
% for peripheral, addr in ao_peripherals.items():
  localparam logic [31:0] ${peripheral.upper()}_START_ADDRESS = AO_PERIPHERAL_START_ADDRESS + 32'h${addr["offset"]};
  localparam logic [31:0] ${peripheral.upper()}_SIZE = 32'h${addr["length"]};
  localparam logic [31:0] ${peripheral.upper()}_END_ADDRESS = ${peripheral.upper()}_START_ADDRESS + ${peripheral.upper()}_SIZE;
  localparam logic [31:0] ${peripheral.upper()}_IDX = 32'd${loop.index};
  
% endfor

  localparam addr_map_rule_t [AO_PERIPHERALS-1:0] AO_PERIPHERALS_ADDR_RULES = '{
% for peripheral, addr in ao_peripherals.items():
      '{ idx: ${peripheral.upper()}_IDX, start_addr: ${peripheral.upper()}_START_ADDRESS, end_addr: ${peripheral.upper()}_END_ADDRESS }${"," if not loop.last else ""}
% endfor
  };

  localparam int unsigned AO_PERIPHERALS_PORT_SEL_WIDTH = AO_PERIPHERALS > 1 ? $clog2(AO_PERIPHERALS) : 32'd1;

  // Relative DMA channels addresses
% for i in range(int(dma_ch_count)):
  localparam logic [7:0] DMA_CH${i}_START_ADDRESS = 8'h${hex(int(ao_peripherals["dma"]["ch_length"], 16)*(i) >> 8)[2:]};
  localparam logic [7:0] DMA_CH${i}_SIZE = 8'h${hex(int(ao_peripherals["dma"]["ch_length"], 16) >> 8)[2:]};
  localparam logic [7:0] DMA_CH${i}_END_ADDRESS = DMA_CH${i}_START_ADDRESS + DMA_CH${i}_SIZE;
  localparam logic [7:0] DMA_CH${i}_IDX = 8'd${i};

% endfor

  localparam addr_map_rule_8bit_t [DMA_CH_NUM-1:0] DMA_ADDR_RULES = '{
  % for i in range(int(dma_ch_count)):
      '{ idx: DMA_CH${i}_IDX, start_addr: DMA_CH${i}_START_ADDRESS, end_addr: DMA_CH${i}_END_ADDRESS }${"," if not loop.last else ""}
% endfor
  };
  
  localparam int unsigned DMA_CH_PORT_SEL_WIDTH = DMA_CH_NUM > 1 ? $clog2(DMA_CH_NUM) : 32'd1;

######################################################################
## Automatically add all peripherals listed
######################################################################
  // switch-on/off peripherals
  // -------------------------
  localparam PERIPHERALS = ${peripherals_count};

% for peripheral, addr in peripherals.items():
  localparam logic [31:0] ${peripheral.upper()}_START_ADDRESS = PERIPHERAL_START_ADDRESS + 32'h${addr["offset"]};
  localparam logic [31:0] ${peripheral.upper()}_SIZE = 32'h${addr["length"]};
  localparam logic [31:0] ${peripheral.upper()}_END_ADDRESS = ${peripheral.upper()}_START_ADDRESS + ${peripheral.upper()}_SIZE;
  localparam logic [31:0] ${peripheral.upper()}_IDX = 32'd${loop.index};
  
% endfor
  localparam addr_map_rule_t [PERIPHERALS-1:0] PERIPHERALS_ADDR_RULES = '{
% for peripheral, addr in peripherals.items():
      '{ idx: ${peripheral.upper()}_IDX, start_addr: ${peripheral.upper()}_START_ADDRESS, end_addr: ${peripheral.upper()}_END_ADDRESS }${"," if not loop.last else ""}
% endfor
  };

  localparam int unsigned PERIPHERALS_PORT_SEL_WIDTH = PERIPHERALS > 1 ? $clog2(PERIPHERALS) : 32'd1;

  // Interrupts
  // ----------
  localparam PLIC_NINT = ${plit_n_interrupts};
  localparam PLIC_USED_NINT = ${plic_used_n_interrupts};
  localparam NEXT_INT = PLIC_NINT - PLIC_USED_NINT;

% for pad in total_pad_list:
  localparam ${pad.localparam} = ${pad.index};
% endfor

  localparam NUM_PAD = ${total_pad};
  localparam NUM_PAD_MUXED = ${total_pad_muxed};

  localparam int unsigned NUM_PAD_PORT_SEL_WIDTH = NUM_PAD > 1 ? $clog2(NUM_PAD) : 32'd1;

  typedef enum logic [1:0] {
    TOP,
    RIGHT,
    BOTTOM,
    LEFT
  } pad_side_e;

endpackage

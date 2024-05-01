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

`include "axi/typedef.svh"

package core_v_mini_mcu_pkg;

  import addr_map_rule_pkg::*;

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
  localparam logic [31:0] DMA_READ_CH0_IDX = 3;
  localparam logic [31:0] DMA_WRITE_CH0_IDX = 4;
  localparam logic [31:0] DMA_ADDR_CH0_IDX = 5;
  localparam logic [31:0] AXI_SL_M_IDX = 6;

  localparam SYSTEM_XBAR_NMASTER = 7;

  // Internal slave memory map and index
  // -----------------------------------
  //must be power of two
  localparam int unsigned MEM_SIZE = 32'h${f'{xheep.ram_size_address():08X}'};

  localparam SYSTEM_XBAR_NSLAVE = ${xheep.ram_numbanks() + 6};

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

  localparam logic[31:0] AXI_SL_SLAVE_START_ADDRESS = 32'h${axi_sl_slave_start_address};
  localparam logic[31:0] AXI_SL_SLAVE_SIZE = 32'h${axi_sl_slave_size_address};
  localparam logic[31:0] AXI_SL_SLAVE_END_ADDRESS = AXI_SL_SLAVE_START_ADDRESS + AXI_SL_SLAVE_SIZE;
  localparam logic[31:0] AXI_SL_SLAVE_IDX = 32'd${int(xheep.ram_numbanks()) + 5};

  localparam addr_map_rule_t [SYSTEM_XBAR_NSLAVE-1:0] XBAR_ADDR_RULES = '{
      '{ idx: ERROR_IDX, start_addr: ERROR_START_ADDRESS, end_addr: ERROR_END_ADDRESS },
% for bank in xheep.iter_ram_banks():
      '{ idx: RAM${bank.name()}_IDX, start_addr: RAM${bank.name()}_START_ADDRESS, end_addr: RAM${bank.name()}_END_ADDRESS },
% endfor
      '{ idx: DEBUG_IDX, start_addr: DEBUG_START_ADDRESS, end_addr: DEBUG_END_ADDRESS },
      '{ idx: AO_PERIPHERAL_IDX, start_addr: AO_PERIPHERAL_START_ADDRESS, end_addr: AO_PERIPHERAL_END_ADDRESS },
      '{ idx: PERIPHERAL_IDX, start_addr: PERIPHERAL_START_ADDRESS, end_addr: PERIPHERAL_END_ADDRESS },
      '{ idx: FLASH_MEM_IDX, start_addr: FLASH_MEM_START_ADDRESS, end_addr: FLASH_MEM_END_ADDRESS },
      '{ idx: AXI_SL_SLAVE_IDX, start_addr: AXI_SL_SLAVE_START_ADDRESS, end_addr: AXI_SL_SLAVE_END_ADDRESS }
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
    parameter int unsigned AXI_ADDR_WIDTH = 32;
    parameter int unsigned AXI_DATA_WIDTH = 32;
    parameter int unsigned AxiDataWidth = 32;
    parameter int unsigned RegDataWidth = 32;
    parameter int unsigned StreamDataBytes = 32;
    localparam int unsigned AxiStrbWidth = AxiDataWidth / 8;
    localparam int unsigned RegStrbWidth = RegDataWidth / 8;
    typedef logic [StreamDataBytes*8-1:0] tdata_t;
    typedef logic [StreamDataBytes-1:0] tstrb_t;
    typedef logic [StreamDataBytes-1:0] tkeep_t;
    typedef logic tlast_t;
    typedef logic id_t;
    typedef logic tdest_t;
    typedef logic tuser_t;
    typedef logic tready_t;
    typedef logic[AXI_ADDR_WIDTH-1:0] axi_addr_t;
    typedef logic[AXI_DATA_WIDTH-1:0] axi_data_t;
    typedef logic[AxiStrbWidth-1:0] axi_strb_t;
    typedef logic[7:0] axi_user_t;
    typedef logic[7:0] axi_id_t;
    `AXI_TYPEDEF_AW_CHAN_T(axi_aw_t, axi_addr_t, axi_id_t, axi_user_t)
    `AXI_TYPEDEF_W_CHAN_T(axi_w_t, axi_data_t, axi_strb_t, axi_user_t)
    `AXI_TYPEDEF_B_CHAN_T(axi_b_t, axi_id_t, axi_user_t)
    `AXI_TYPEDEF_AR_CHAN_T(axi_ar_t, axi_addr_t, axi_id_t, axi_user_t)
    `AXI_TYPEDEF_R_CHAN_T(axi_r_t, axi_data_t, axi_id_t, axi_user_t)
    `AXI_TYPEDEF_REQ_T(axi_req_t, axi_aw_t, axi_w_t, axi_ar_t)
    `AXI_TYPEDEF_RESP_T(axi_resp_t, axi_b_t, axi_r_t)
  typedef enum logic [1:0] {
    TOP,
    RIGHT,
    BOTTOM,
    LEFT
  } pad_side_e;

endpackage

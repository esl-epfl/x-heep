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

  typedef enum logic {
    cv32e40p,
    cv32e20
  } cpu_type_e;

  localparam cpu_type_e CpuType = cv32e20;

  //must be power of two
  localparam int unsigned MEM_SIZE = 2 ** 16;

  localparam SYSTEM_XBAR_NMASTER = 3;
  localparam SYSTEM_XBAR_NSLAVE = 6;

  localparam int unsigned LOG_SYSTEM_XBAR_NMASTER = SYSTEM_XBAR_NMASTER > 1 ? $clog2(
      SYSTEM_XBAR_NMASTER
  ) : 32'd1;
  localparam int unsigned LOG_SYSTEM_XBAR_NSLAVE = SYSTEM_XBAR_NSLAVE > 1 ? $clog2(
      SYSTEM_XBAR_NSLAVE
  ) : 32'd1;

  typedef enum logic {
    NtoM,
    onetoM
  } bus_type_e;

  localparam bus_type_e BusType = onetoM;

  //master idx
  localparam logic [31:0] CORE_INSTR_IDX = 0;
  localparam logic [31:0] CORE_DATA_IDX = 1;
  localparam logic [31:0] DEBUG_MASTER_IDX = 2;

  //slave mmap and idx
  localparam logic [31:0] ERROR_START_ADDRESS = 32'hBADACCE5;
  localparam logic [31:0] ERROR_SIZE = 32'h00000001;
  localparam logic [31:0] ERROR_END_ADDRESS = ERROR_START_ADDRESS + ERROR_SIZE;
  localparam logic [31:0] ERROR_IDX = 32'd0;

  localparam logic [31:0] RAM0_START_ADDRESS = 32'h00000000;
  localparam logic [31:0] RAM0_SIZE = (MEM_SIZE / 2);
  localparam logic [31:0] RAM0_END_ADDRESS = RAM0_START_ADDRESS + RAM0_SIZE;
  localparam logic [31:0] RAM0_IDX = 32'd1;

  localparam logic [31:0] RAM1_START_ADDRESS = (MEM_SIZE / 2);
  localparam logic [31:0] RAM1_SIZE = MEM_SIZE;
  localparam logic [31:0] RAM1_END_ADDRESS = RAM1_START_ADDRESS + RAM1_SIZE;
  localparam logic [31:0] RAM1_IDX = 32'd2;

  localparam logic [31:0] DEBUG_START_ADDRESS = 32'h10000000;
  localparam logic [31:0] DEBUG_SIZE = 32'h100000;
  localparam logic [31:0] DEBUG_END_ADDRESS = DEBUG_START_ADDRESS + DEBUG_SIZE;
  localparam logic [31:0] DEBUG_IDX = 32'd3;

  localparam logic [31:0] PERIPHERAL_START_ADDRESS = 32'h20000000;
  localparam logic [31:0] PERIPHERAL_SIZE = 32'h100000;
  localparam logic [31:0] PERIPHERAL_END_ADDRESS = PERIPHERAL_START_ADDRESS + PERIPHERAL_SIZE;
  localparam logic [31:0] PERIPHERAL_IDX = 32'd4;

  localparam logic [31:0] EXT_SLAVE_START_ADDRESS = 32'h30000000;
  localparam logic [31:0] EXT_SLAVE_SIZE = 32'h1000000;
  localparam logic [31:0] EXT_SLAVE_END_ADDRESS = EXT_SLAVE_START_ADDRESS + EXT_SLAVE_SIZE;
  localparam logic [31:0] EXT_SLAVE_IDX = 32'd5;

  localparam addr_map_rule_t [SYSTEM_XBAR_NSLAVE-1:0] XBAR_ADDR_RULES = '{
      '{idx: RAM0_IDX, start_addr: RAM0_START_ADDRESS, end_addr: RAM0_END_ADDRESS},
      '{idx: RAM1_IDX, start_addr: RAM1_START_ADDRESS, end_addr: RAM1_END_ADDRESS},
      '{idx: DEBUG_IDX, start_addr: DEBUG_START_ADDRESS, end_addr: DEBUG_END_ADDRESS},
      '{
          idx: PERIPHERAL_IDX,
          start_addr: PERIPHERAL_START_ADDRESS,
          end_addr: PERIPHERAL_END_ADDRESS
      },
      '{
          idx: EXT_SLAVE_IDX,
          start_addr: EXT_SLAVE_START_ADDRESS,
          end_addr: EXT_SLAVE_END_ADDRESS
      },
      '{idx: ERROR_IDX, start_addr: ERROR_START_ADDRESS, end_addr: ERROR_END_ADDRESS}
  };

  //slave encoder
  localparam SYSTEM_NPERIPHERALS = 6;

  localparam logic [31:0] SOC_CTRL_START_ADDRESS = PERIPHERAL_START_ADDRESS + 32'h0000000;
  localparam logic [31:0] SOC_CTRL_SIZE = 32'h0010000;
  localparam logic [31:0] SOC_CTRL_END_ADDRESS = SOC_CTRL_START_ADDRESS + SOC_CTRL_SIZE;
  localparam logic [31:0] SOC_CTRL_IDX = 32'd0;

  localparam logic [31:0] UART_START_ADDRESS = PERIPHERAL_START_ADDRESS + 32'h0010000;
  localparam logic [31:0] UART_SIZE = 32'h0010000;
  localparam logic [31:0] UART_END_ADDRESS = UART_START_ADDRESS + UART_SIZE;
  localparam logic [31:0] UART_IDX = 32'd1;

  localparam logic [31:0] EXT_PERIPH_START_ADDRESS = PERIPHERAL_START_ADDRESS + 32'h0020000;
  localparam logic [31:0] EXT_PERIPH_SIZE = 32'h0010000;
  localparam logic [31:0] EXT_PERIPH_END_ADDRESS = EXT_PERIPH_START_ADDRESS + EXT_PERIPH_SIZE;
  localparam logic [31:0] EXT_PERIPH_IDX = 32'd2;

  localparam logic [31:0] PLIC_START_ADDRESS = PERIPHERAL_START_ADDRESS + 32'h0030000;
  localparam logic [31:0] PLIC_SIZE = 32'h0010000;
  localparam logic [31:0] PLIC_END_ADDRESS = PLIC_START_ADDRESS + PLIC_SIZE;
  localparam logic [31:0] PLIC_IDX = 32'd3;

  localparam logic [31:0] RV_TIMER_START_ADDRESS = PERIPHERAL_START_ADDRESS + 32'h0040000;
  localparam logic [31:0] RV_TIMER_SIZE = 32'h0010000;
  localparam logic [31:0] RV_TIMER_END_ADDRESS = RV_TIMER_START_ADDRESS + RV_TIMER_SIZE;
  localparam logic [31:0] RV_TIMER_IDX = 32'd4;

  localparam logic [31:0] GPIO_START_ADDRESS = PERIPHERAL_START_ADDRESS + 32'h0050000;
  localparam logic [31:0] GPIO_SIZE = 32'h0010000;
  localparam logic [31:0] GPIO_END_ADDRESS = GPIO_START_ADDRESS + GPIO_SIZE;
  localparam logic [31:0] GPIO_IDX = 32'd5;

  localparam addr_map_rule_t [SYSTEM_NPERIPHERALS-1:0] PERIPHERALS_ADDR_RULES = '{
      '{idx: SOC_CTRL_IDX, start_addr: SOC_CTRL_START_ADDRESS, end_addr: SOC_CTRL_END_ADDRESS},
      '{idx: UART_IDX, start_addr: UART_START_ADDRESS, end_addr: UART_END_ADDRESS},
      '{
          idx: EXT_PERIPH_IDX,
          start_addr: EXT_PERIPH_START_ADDRESS,
          end_addr: EXT_PERIPH_END_ADDRESS
      },
      '{idx: PLIC_IDX, start_addr: PLIC_START_ADDRESS, end_addr: PLIC_END_ADDRESS},
      '{idx: RV_TIMER_IDX, start_addr: RV_TIMER_START_ADDRESS, end_addr: RV_TIMER_END_ADDRESS},
      '{idx: GPIO_IDX, start_addr: GPIO_START_ADDRESS, end_addr: GPIO_END_ADDRESS}
  };

  localparam int unsigned PERIPHERALS_PORT_SEL_WIDTH = SYSTEM_NPERIPHERALS > 1 ? $clog2(
      SYSTEM_NPERIPHERALS
  ) : 32'd1;

endpackage

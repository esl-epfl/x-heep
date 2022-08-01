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
  localparam int unsigned MEM_SIZE = 2 ** 19;

  localparam SYSTEM_XBAR_NMASTER = 3;
  localparam SYSTEM_XBAR_NSLAVE = 14;

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
  localparam logic [31:0] RAM0_SIZE = (MEM_SIZE / 8);
  localparam logic [31:0] RAM0_END_ADDRESS = RAM0_START_ADDRESS + RAM0_SIZE - 1;
  localparam logic [31:0] RAM0_IDX = 32'd1;

  localparam logic [31:0] RAM1_START_ADDRESS = RAM0_END_ADDRESS + 1;
  localparam logic [31:0] RAM1_SIZE = (MEM_SIZE / 8);
  localparam logic [31:0] RAM1_END_ADDRESS = RAM1_START_ADDRESS + RAM1_SIZE - 1;
  localparam logic [31:0] RAM1_IDX = 32'd2;

  localparam logic [31:0] RAM2_START_ADDRESS = RAM1_END_ADDRESS + 1;
  localparam logic [31:0] RAM2_SIZE = (MEM_SIZE / 8);
  localparam logic [31:0] RAM2_END_ADDRESS = RAM2_START_ADDRESS + RAM2_SIZE - 1;
  localparam logic [31:0] RAM2_IDX = 32'd3;

  localparam logic [31:0] RAM3_START_ADDRESS = RAM2_END_ADDRESS + 1;
  localparam logic [31:0] RAM3_SIZE = (MEM_SIZE / 8);
  localparam logic [31:0] RAM3_END_ADDRESS = RAM3_START_ADDRESS + RAM3_SIZE - 1;
  localparam logic [31:0] RAM3_IDX = 32'd4;

  localparam logic [31:0] RAM4_START_ADDRESS = RAM3_END_ADDRESS + 1;
  localparam logic [31:0] RAM4_SIZE = (MEM_SIZE / 8);
  localparam logic [31:0] RAM4_END_ADDRESS = RAM4_START_ADDRESS + RAM4_SIZE - 1;
  localparam logic [31:0] RAM4_IDX = 32'd5;

  localparam logic [31:0] RAM5_START_ADDRESS = RAM4_END_ADDRESS + 1;
  localparam logic [31:0] RAM5_SIZE = (MEM_SIZE / 8);
  localparam logic [31:0] RAM5_END_ADDRESS = RAM5_START_ADDRESS + RAM5_SIZE - 1;
  localparam logic [31:0] RAM5_IDX = 32'd6;

  localparam logic [31:0] RAM6_START_ADDRESS = RAM5_END_ADDRESS + 1;
  localparam logic [31:0] RAM6_SIZE = (MEM_SIZE / 8);
  localparam logic [31:0] RAM6_END_ADDRESS = RAM6_START_ADDRESS + RAM6_SIZE - 1;
  localparam logic [31:0] RAM6_IDX = 32'd7;

  localparam logic [31:0] RAM7_START_ADDRESS = RAM6_END_ADDRESS + 1;
  localparam logic [31:0] RAM7_SIZE = (MEM_SIZE / 8);
  localparam logic [31:0] RAM7_END_ADDRESS = RAM7_START_ADDRESS + RAM7_SIZE - 1;
  localparam logic [31:0] RAM7_IDX = 32'd8;

  localparam logic [31:0] DEBUG_START_ADDRESS = 32'h10000000;
  localparam logic [31:0] DEBUG_SIZE = 32'h00100000;
  localparam logic [31:0] DEBUG_END_ADDRESS = DEBUG_START_ADDRESS + DEBUG_SIZE - 1;
  localparam logic [31:0] DEBUG_IDX = 32'd9;

  localparam logic [31:0] AO_PERIPHERAL_START_ADDRESS = 32'h20000000;
  localparam logic [31:0] AO_PERIPHERAL_SIZE = 32'h00100000;
  localparam logic[31:0] AO_PERIPHERAL_END_ADDRESS = AO_PERIPHERAL_START_ADDRESS + AO_PERIPHERAL_SIZE - 1;
  localparam logic [31:0] AO_PERIPHERAL_IDX = 32'd10;

  localparam logic [31:0] PERIPHERAL_START_ADDRESS = 32'h30000000;
  localparam logic [31:0] PERIPHERAL_SIZE = 32'h00100000;
  localparam logic [31:0] PERIPHERAL_END_ADDRESS = PERIPHERAL_START_ADDRESS + PERIPHERAL_SIZE - 1;
  localparam logic [31:0] PERIPHERAL_IDX = 32'd11;

  localparam logic [31:0] SPI_FLASH_START_ADDRESS = 32'h40000000;
  localparam logic [31:0] SPI_FLASH_SIZE = 32'h01000000;
  localparam logic [31:0] SPI_FLASH_END_ADDRESS = SPI_FLASH_START_ADDRESS + SPI_FLASH_SIZE - 1;
  localparam logic [31:0] SPI_FLASH_IDX = 32'd12;

  localparam logic [31:0] EXT_SLAVE_START_ADDRESS = 32'h40000000;
  localparam logic [31:0] EXT_SLAVE_SIZE = 32'h01000000;
  localparam logic [31:0] EXT_SLAVE_END_ADDRESS = EXT_SLAVE_START_ADDRESS + EXT_SLAVE_SIZE - 1;
  localparam logic [31:0] EXT_SLAVE_IDX = 32'd13;

  localparam addr_map_rule_t [SYSTEM_XBAR_NSLAVE-1:0] XBAR_ADDR_RULES = '{
      '{idx: ERROR_IDX, start_addr: ERROR_START_ADDRESS, end_addr: ERROR_END_ADDRESS},
      '{idx: RAM0_IDX, start_addr: RAM0_START_ADDRESS, end_addr: RAM0_END_ADDRESS},
      '{idx: RAM1_IDX, start_addr: RAM1_START_ADDRESS, end_addr: RAM1_END_ADDRESS},
      '{idx: RAM2_IDX, start_addr: RAM2_START_ADDRESS, end_addr: RAM2_END_ADDRESS},
      '{idx: RAM3_IDX, start_addr: RAM3_START_ADDRESS, end_addr: RAM3_END_ADDRESS},
      '{idx: RAM4_IDX, start_addr: RAM4_START_ADDRESS, end_addr: RAM4_END_ADDRESS},
      '{idx: RAM5_IDX, start_addr: RAM5_START_ADDRESS, end_addr: RAM5_END_ADDRESS},
      '{idx: RAM6_IDX, start_addr: RAM6_START_ADDRESS, end_addr: RAM6_END_ADDRESS},
      '{idx: RAM7_IDX, start_addr: RAM7_START_ADDRESS, end_addr: RAM7_END_ADDRESS},
      '{idx: DEBUG_IDX, start_addr: DEBUG_START_ADDRESS, end_addr: DEBUG_END_ADDRESS},
      '{
          idx: AO_PERIPHERAL_IDX,
          start_addr: AO_PERIPHERAL_START_ADDRESS,
          end_addr: AO_PERIPHERAL_END_ADDRESS
      },
      '{
          idx: PERIPHERAL_IDX,
          start_addr: PERIPHERAL_START_ADDRESS,
          end_addr: PERIPHERAL_END_ADDRESS
      },
      '{
          idx: SPI_FLASH_IDX,
          start_addr: SPI_FLASH_START_ADDRESS,
          end_addr: SPI_FLASH_END_ADDRESS
      },
      '{idx: EXT_SLAVE_IDX, start_addr: EXT_SLAVE_START_ADDRESS, end_addr: EXT_SLAVE_END_ADDRESS}
  };

  //slave encoder
  localparam AO_PERIPHERALS = 6;
  localparam PERIPHERALS = 4;

  localparam logic [31:0] SOC_CTRL_START_ADDRESS = AO_PERIPHERAL_START_ADDRESS + 32'h00000000;
  localparam logic [31:0] SOC_CTRL_SIZE = 32'h00010000;
  localparam logic [31:0] SOC_CTRL_END_ADDRESS = SOC_CTRL_START_ADDRESS + SOC_CTRL_SIZE - 1;
  localparam logic [31:0] SOC_CTRL_IDX = 32'd0;

  localparam logic [31:0] BOOTROM_START_ADDRESS = AO_PERIPHERAL_START_ADDRESS + 32'h00010000;
  localparam logic [31:0] BOOTROM_SIZE = 32'h00010000;
  localparam logic [31:0] BOOTROM_END_ADDRESS = BOOTROM_START_ADDRESS + BOOTROM_SIZE - 1;
  localparam logic [31:0] BOOTROM_IDX = 32'd1;

  localparam logic [31:0] SPI_HOST_START_ADDRESS = AO_PERIPHERAL_START_ADDRESS + 32'h00020000;
  localparam logic [31:0] SPI_HOST_SIZE = 32'h00008000;
  localparam logic [31:0] SPI_HOST_END_ADDRESS = SPI_HOST_START_ADDRESS + SPI_HOST_SIZE - 1;
  localparam logic [31:0] SPI_HOST_IDX = 32'd2;

  localparam logic [31:0] SPI_MEMIO_START_ADDRESS = AO_PERIPHERAL_START_ADDRESS + 32'h00028000;
  localparam logic [31:0] SPI_MEMIO_SIZE = 32'h00008000;
  localparam logic [31:0] SPI_MEMIO_END_ADDRESS = SPI_MEMIO_START_ADDRESS + SPI_MEMIO_SIZE - 1;
  localparam logic [31:0] SPI_MEMIO_IDX = 32'd3;

  localparam logic [31:0] POWER_MANAGER_START_ADDRESS = AO_PERIPHERAL_START_ADDRESS + 32'h00030000;
  localparam logic [31:0] POWER_MANAGER_SIZE = 32'h00010000;
  localparam logic [31:0] POWER_MANAGER_END_ADDRESS = POWER_MANAGER_START_ADDRESS + POWER_MANAGER_SIZE - 1;
  localparam logic [31:0] POWER_MANAGER_IDX = 32'd4;

  localparam logic [31:0] RV_TIMER_START_ADDRESS = AO_PERIPHERAL_START_ADDRESS + 32'h00040000;
  localparam logic [31:0] RV_TIMER_SIZE = 32'h00010000;
  localparam logic [31:0] RV_TIMER_END_ADDRESS = RV_TIMER_START_ADDRESS + RV_TIMER_SIZE - 1;
  localparam logic [31:0] RV_TIMER_IDX = 32'd5;

  localparam logic [31:0] PLIC_START_ADDRESS = PERIPHERAL_START_ADDRESS + 32'h00000000;
  localparam logic [31:0] PLIC_SIZE = 32'h00010000;
  localparam logic [31:0] PLIC_END_ADDRESS = PLIC_START_ADDRESS + PLIC_SIZE - 1;
  localparam logic [31:0] PLIC_IDX = 32'd0;

  localparam logic [31:0] UART_START_ADDRESS = PERIPHERAL_START_ADDRESS + 32'h00010000;
  localparam logic [31:0] UART_SIZE = 32'h00010000;
  localparam logic [31:0] UART_END_ADDRESS = UART_START_ADDRESS + UART_SIZE - 1;
  localparam logic [31:0] UART_IDX = 32'd1;

  localparam logic [31:0] GPIO_START_ADDRESS = PERIPHERAL_START_ADDRESS + 32'h00020000;
  localparam logic [31:0] GPIO_SIZE = 32'h00010000;
  localparam logic [31:0] GPIO_END_ADDRESS = GPIO_START_ADDRESS + GPIO_SIZE - 1;
  localparam logic [31:0] GPIO_IDX = 32'd2;

  localparam logic [31:0] EXT_PERIPH_START_ADDRESS = PERIPHERAL_START_ADDRESS + 32'h00030000;
  localparam logic [31:0] EXT_PERIPH_SIZE = 32'h00010000;
  localparam logic [31:0] EXT_PERIPH_END_ADDRESS = EXT_PERIPH_START_ADDRESS + EXT_PERIPH_SIZE - 1;
  localparam logic [31:0] EXT_PERIPH_IDX = 32'd3;

  localparam addr_map_rule_t [AO_PERIPHERALS-1:0] AO_PERIPHERALS_ADDR_RULES = '{
      '{idx: SOC_CTRL_IDX, start_addr: SOC_CTRL_START_ADDRESS, end_addr: SOC_CTRL_END_ADDRESS},
      '{idx: BOOTROM_IDX, start_addr: BOOTROM_START_ADDRESS, end_addr: BOOTROM_END_ADDRESS},
      '{idx: SPI_HOST_IDX, start_addr: SPI_HOST_START_ADDRESS, end_addr: SPI_HOST_END_ADDRESS},
      '{
          idx: SPI_MEMIO_IDX,
          start_addr: SPI_MEMIO_START_ADDRESS,
          end_addr: SPI_MEMIO_END_ADDRESS
      },
      '{
          idx: POWER_MANAGER_IDX,
          start_addr: POWER_MANAGER_START_ADDRESS,
          end_addr: POWER_MANAGER_END_ADDRESS
      },
      '{idx: RV_TIMER_IDX, start_addr: RV_TIMER_START_ADDRESS, end_addr: RV_TIMER_END_ADDRESS}
  };

  localparam int unsigned AO_PERIPHERALS_PORT_SEL_WIDTH = AO_PERIPHERALS > 1 ? $clog2(
      AO_PERIPHERALS
  ) : 32'd1;

  localparam addr_map_rule_t [PERIPHERALS-1:0] PERIPHERALS_ADDR_RULES = '{
      '{idx: PLIC_IDX, start_addr: PLIC_START_ADDRESS, end_addr: PLIC_END_ADDRESS},
      '{idx: UART_IDX, start_addr: UART_START_ADDRESS, end_addr: UART_END_ADDRESS},
      '{idx: GPIO_IDX, start_addr: GPIO_START_ADDRESS, end_addr: GPIO_END_ADDRESS},
      '{
          idx: EXT_PERIPH_IDX,
          start_addr: EXT_PERIPH_START_ADDRESS,
          end_addr: EXT_PERIPH_END_ADDRESS
      }
  };

  localparam int unsigned PERIPHERALS_PORT_SEL_WIDTH = PERIPHERALS > 1 ? $clog2(
      PERIPHERALS
  ) : 32'd1;

endpackage

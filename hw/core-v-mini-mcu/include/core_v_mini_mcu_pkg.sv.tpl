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

  localparam SYSTEM_XBAR_NMASTER      = 3;
  localparam SYSTEM_XBAR_NSLAVE       = 4;

  //master idx
  localparam CORE_INSTR_IDX           = 0;
  localparam CORE_DATA_IDX            = 1;
  localparam DEBUG_MASTER_IDX         = 2;

  //slave mmap and idx
  localparam ERROR_START_ADDRESS      = 32'hBADACCE5;
  localparam ERROR_SIZE               = 32'h00000001;
  localparam ERROR_END_ADDRESS        = ERROR_START_ADDRESS + ERROR_SIZE;
  localparam ERROR_IDX                = 0;

  localparam RAM0_START_ADDRESS       = 32'h00000000;
  localparam RAM0_SIZE                = 32'h00008000;
  localparam RAM0_END_ADDRESS         = RAM0_START_ADDRESS + RAM0_SIZE;
  localparam RAM0_IDX                 = 1;

  localparam RAM1_START_ADDRESS       = 32'h00008000;
  localparam RAM1_SIZE                = 32'h00008000;
  localparam RAM1_END_ADDRESS         = RAM1_START_ADDRESS + RAM1_SIZE;
  localparam RAM1_IDX                 = 2;

  localparam PERIPHERAL_START_ADDRESS = 32'h${peripheral_start_address};
  localparam PERIPHERAL_SIZE          = 32'h${peripheral_size_address};
  localparam PERIPHERAL_END_ADDRESS   = PERIPHERAL_START_ADDRESS + PERIPHERAL_SIZE;
  localparam PERIPHERAL_IDX           = 3;

      //slave encoder
      localparam SYSTEM_NPERIPHERALS      = 2;

      localparam DEBUG_START_ADDRESS      = PERIPHERAL_START_ADDRESS + 32'h${debug_start_offset};
      localparam DEBUG_SIZE               = 32'h${debug_size_address};
      localparam DEBUG_END_ADDRESS        = DEBUG_START_ADDRESS + DEBUG_SIZE;
      localparam DEBUG_IDX                = 0;

      localparam UART_START_ADDRESS       = PERIPHERAL_START_ADDRESS + 32'h${uart_start_offset};
      localparam UART_SIZE                = 32'h${uart_size_address};
      localparam UART_END_ADDRESS         = UART_START_ADDRESS + UART_SIZE;
      localparam UART_IDX                 = 1;






endpackage
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

  parameter SYSTEM_XBAR_NMASTER = 3;
  parameter SYSTEM_XBAR_NSLAVE  = 4;

  parameter RAM0_START_ADDRESS  = 32'h00000000;
  parameter RAM0_SIZE           = 32'h00008000;
  parameter RAM0_END_ADDRESS    = RAM0_START_ADDRESS + RAM0_SIZE;
  parameter RAM0_IDX            = 0;

  parameter RAM1_START_ADDRESS  = 32'h00008000;
  parameter RAM1_SIZE           = 32'h00008000;
  parameter RAM1_END_ADDRESS    = RAM1_START_ADDRESS + RAM1_SIZE;
  parameter RAM1_IDX            = 1;

  parameter DEBUG_START_ADDRESS = 32'h30000000;
  parameter DEBUG_SIZE          = 32'h00010000;
  parameter DEBUG_END_ADDRESS   = DEBUG_START_ADDRESS + DEBUG_SIZE;
  parameter DEBUG_IDX           = 2;

  parameter ERROR_START_ADDRESS = 32'hBADACCE5;
  parameter ERROR_SIZE          = 32'h00000001;
  parameter ERROR_END_ADDRESS   = ERROR_START_ADDRESS + ERROR_SIZE;
  parameter ERROR_IDX           = 3;


endpackage
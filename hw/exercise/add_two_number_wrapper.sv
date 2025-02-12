// Copyright 2025 Francesco Poluzzi
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// Licensed under the Solderpad Hardware License v 2.1 (the “License”); you 
// may not use this file except in compliance with the License, or, at your
// option, the Apache License version 2.0. You may obtain a copy of the 
// License at https://solderpad.org/licenses/SHL-2.1/
//
// Unless required by applicable law or agreed to in writing, any work 
// distributed under the License is distributed on an “AS IS” BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the 
// License for the specific language governing permissions and limitations 
// under the License.
//
// Author: Francesco Poluzzi
// Description: Wrapper for the X-Heep, adapting the XIF signals

module add_two_number_wrapper #(
  parameter type reg_req_t = logic,
  parameter type reg_rsp_t = logic
) (
  // Clock and Reset
  input  logic clk_i,
  input  logic rst_ni,

  // eXtension interface signals.
  // We assume that the peripheral is accessed through the memory–mapped
  // interface. The types and fields of these interfaces are defined in the
  // package (if_xif) used in the X-Heep design.
  if_xif.coproc_mem        xif_mem_if,
  if_xif.coproc_mem_result xif_mem_result_if,
  if_xif.coproc_result     xif_result_if
);

  //-------------------------------------------------------------------------
  // Internal Connections
  //-------------------------------------------------------------------------
  // We assume that the coprocessor memory interface carries the register 
  // request/response signals as follows:
  //   - xif_mem_if.mem_req  : (of type reg_req_t) carries the incoming
  //                           register read/write request.
  //   - xif_mem_if.mem_resp : (of type reg_rsp_t) carries the response.
  // The peripheral is always ready to accept a new request (so we drive
  // mem_ready high). The other coprocessor interfaces are not used by this
  // peripheral.
  //-------------------------------------------------------------------------

typedef struct packed {
  logic        valid;  // Indicates a valid request
  logic        write;  // 1 for write, 0 for read
  logic [31:0] addr;   // Target register address
  logic [31:0] wdata;  // Write data (if any)
} reg_req_t;

typedef struct packed {
  logic [31:0] rdata;  // Read data returned on a read operation
} reg_rsp_t;

  // Instantiate the add_two_number peripheral.
  add_two_number #(
    .reg_req_t(reg_req_t),
    .reg_rsp_t(reg_rsp_t)
  ) add_two_number_i (
    .clk_i    (clk_i),
    .rst_ni   (rst_ni),
    // Directly connect the memory interface signals:
    .reg_req_i(xif_mem_if.mem_req),
    .reg_rsp_o(xif_mem_if.mem_resp)
  );

  // The peripheral is always ready for memory–mapped requests.
  assign xif_mem_if.mem_ready = 1'b1;

  // Tie off the unused interfaces.
  assign xif_mem_result_if.mem_result_valid = 1'b0;
  assign xif_mem_result_if.mem_result       = '0;
  assign xif_result_if.result_valid         = 1'b0;
  assign xif_result_if.result               = '0;

endmodule
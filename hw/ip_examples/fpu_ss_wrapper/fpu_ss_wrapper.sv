// Copyright 2023 David Mallasén Quintana
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
// Author: David Mallasén <dmallase@ucm.es>
// Description: Wrapper for the fpu_ss, adapting the XIF signals

module fpu_ss_wrapper #(
    parameter                                 PULP_ZFINX         = 0,
    parameter                                 INPUT_BUFFER_DEPTH = 1,
    parameter                                 OUT_OF_ORDER       = 0,
    parameter                                 FORWARDING         = 1,
    parameter fpnew_pkg::fpu_features_t       FPU_FEATURES       = fpu_ss_pkg::FPU_FEATURES,
    parameter fpnew_pkg::fpu_implementation_t FPU_IMPLEMENTATION = fpu_ss_pkg::FPU_IMPLEMENTATION
) (
    // Clock and Reset
    input logic clk_i,
    input logic rst_ni,

    // eXtension interface
    if_xif.coproc_compressed xif_compressed_if,
    if_xif.coproc_issue      xif_issue_if,
    if_xif.coproc_commit     xif_commit_if,
    if_xif.coproc_mem        xif_mem_if,
    if_xif.coproc_mem_result xif_mem_result_if,
    if_xif.coproc_result     xif_result_if
);

  fpu_ss #(
      .PulpDivsqrt(1'b0),
      .PULP_ZFINX(PULP_ZFINX),
      .INPUT_BUFFER_DEPTH(INPUT_BUFFER_DEPTH),
      .OUT_OF_ORDER(OUT_OF_ORDER),
      .FORWARDING(FORWARDING),
      .FPU_FEATURES(FPU_FEATURES),
      .FPU_IMPLEMENTATION(FPU_IMPLEMENTATION)
  ) fpu_ss_i (
      // Clock and reset
      .clk_i (clk_i),
      .rst_ni(rst_ni),

      // Compressed Interface
      .x_compressed_valid_i(xif_compressed_if.compressed_valid),
      .x_compressed_ready_o(xif_compressed_if.compressed_ready),
      .x_compressed_req_i  (xif_compressed_if.compressed_req),
      .x_compressed_resp_o (xif_compressed_if.compressed_resp),

      // Issue Interface
      .x_issue_valid_i(xif_issue_if.issue_valid),
      .x_issue_ready_o(xif_issue_if.issue_ready),
      .x_issue_req_i  (xif_issue_if.issue_req),
      .x_issue_resp_o (xif_issue_if.issue_resp),

      // Commit Interface
      .x_commit_valid_i(xif_commit_if.commit_valid),
      .x_commit_i(xif_commit_if.commit),

      // Memory Request/Response Interface
      .x_mem_valid_o(xif_mem_if.mem_valid),
      .x_mem_ready_i(xif_mem_if.mem_ready),
      .x_mem_req_o  (xif_mem_if.mem_req),
      .x_mem_resp_i (xif_mem_if.mem_resp),

      // Memory Result Interface
      .x_mem_result_valid_i(xif_mem_result_if.mem_result_valid),
      .x_mem_result_i(xif_mem_result_if.mem_result),

      // Result Interface
      .x_result_valid_o(xif_result_if.result_valid),
      .x_result_ready_i(xif_result_if.result_ready),
      .x_result_o(xif_result_if.result)
  );

endmodule

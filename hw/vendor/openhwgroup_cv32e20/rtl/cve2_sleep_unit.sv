// Copyright 2020 Silicon Labs, Inc.
//
// This file, and derivatives thereof are licensed under the
// Solderpad License, Version 2.0 (the "License").
//
// Use of this file means you agree to the terms and conditions
// of the license and are in full compliance with the License.
//
// You may obtain a copy of the License at:
//
//     https://solderpad.org/licenses/SHL-2.0/
//
// Unless required by applicable law or agreed to in writing, software
// and hardware implementations thereof distributed under the License
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED.
//
// See the License for the specific language governing permissions and
// limitations under the License.

////////////////////////////////////////////////////////////////////////////////
// Engineer:       Arjan Bink - arjan.bink@silabs.com                         //
//                                                                            //
// Design Name:    Sleep Unit                                                 //
// Project Name:   CVE2                                                       //
// Language:       SystemVerilog                                              //
//                                                                            //
// Description:    Sleep unit containing the instantiated clock gate which    //
//                 provides the gated clock (clk_gated_o) for the rest        //
//                 of the design. Forked version of cv32e40p                  //
//                                                                            //
//                 The clock is gated for the following scenarios:            //
//                                                                            //
//                 - While waiting for fetch to become enabled                //
//                 - While blocked on a WFI (PULP_CLUSTER = 0)                //
//                 - While clock_en_i = 0 during a p.elw (PULP_CLUSTER = 1)   //
//                                                                            //
//                 Sleep is signaled via core_sleep_o when:                   //
//                 - During a WFI (except in debug)                           //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

module cve2_sleep_unit (
    // Clock, reset interface
    input  logic clk_ungated_i,  // Free running clock
    input  logic rst_n,
    output logic clk_gated_o,  // Gated clock
    input  logic scan_cg_en_i,  // Enable all clock gates for testing

    // Core sleep
    output logic core_sleep_o,

    // Fetch enable
    input  logic fetch_enable_i,
    output logic fetch_enable_o,

    // Core status
    input logic if_busy_i,
    input logic ctrl_busy_i,
    input logic lsu_busy_i,

    // WFI wake
    input logic wake_from_sleep_i
);

  logic fetch_enable_q;  // Sticky version of fetch_enable_i
  logic fetch_enable_d;
  logic core_busy_q; // Is core still busy (and requires a clock) with what needs to finish before entering sleep?
  logic core_busy_d;
  logic clock_en;  // Final clock enable

  //////////////////////////////////////////////////////////////////////////////
  // Sleep FSM
  //////////////////////////////////////////////////////////////////////////////

  // Make sticky version of fetch_enable_i
  assign fetch_enable_d = fetch_enable_i ? 1'b1 : fetch_enable_q;


  // Busy when any of the sub units is busy (typically wait for the instruction buffer to fill up)
  assign core_busy_d = if_busy_i || ctrl_busy_i || lsu_busy_i;

  // Enable the clock only after the initial fetch enable while busy or waking up to become busy
  assign clock_en = fetch_enable_q && (wake_from_sleep_i || core_busy_q);

  // Sleep only in response to WFI which leads to clock disable; debug_wfi_no_sleep_o in
  // cv32e40p_controller determines the scenarios for which WFI can(not) cause sleep.
  assign core_sleep_o = fetch_enable_q && !clock_en;


  always_ff @(posedge clk_ungated_i, negedge rst_n) begin
    if (rst_n == 1'b0) begin
      core_busy_q    <= 1'b0;
      fetch_enable_q <= 1'b0;
    end else begin
      core_busy_q    <= core_busy_d;
      fetch_enable_q <= fetch_enable_d;
    end
  end

  // Fetch enable for Controller
  assign fetch_enable_o = fetch_enable_q;

  // Main clock gate of CV32E40P
  cve2_clock_gate core_clock_gate_i (
      .clk_i       (clk_ungated_i),
      .en_i        (clock_en),
      .scan_cg_en_i(scan_cg_en_i),
      .clk_o       (clk_gated_o)
  );

  //----------------------------------------------------------------------------
  // Assertions - TODO
  //----------------------------------------------------------------------------


endmodule  // cve2_sleep_unit

// Copyright 2022 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51

// Author: Tim Fischer <fischeti@iis.ee.ethz.ch>

`include "common_cells/registers.svh"
`include "common_cells/assertions.svh"

// Implements the Physical Layer of the Serial Link
// The number of Channels and Lanes per Channel is parametrizable
module serial_link_physical #(
  parameter type phy_data_t = serial_link_pkg::phy_data_t,
  // Number of Wires in one channel
  parameter int NumLanes   = 8,
  // Fifo Depth of CDC, dependent on
  // Num Credit for Flow control
  parameter int FifoDepth  = 8,
  // Maximum factor of ClkDiv
  parameter int MaxClkDiv  = 32
) (
  input  logic                          clk_i,
  input  logic                          rst_ni,
  input  logic [$clog2(MaxClkDiv):0]    clk_div_i,
  input  logic [$clog2(MaxClkDiv):0]    clk_shift_start_i,
  input  logic [$clog2(MaxClkDiv):0]    clk_shift_end_i,
  input  logic                          ddr_rcv_clk_i,
  output logic                          ddr_rcv_clk_o,
  input  logic [NumLanes*2-1:0]         data_out_i,
  input  logic                          data_out_valid_i,
  output logic                          data_out_ready_o,
  output logic [NumLanes*2-1:0]         data_in_o,
  output logic                          data_in_valid_o,
  input  logic                          data_in_ready_i,
  input  logic [NumLanes-1:0]           ddr_i,
  output logic [NumLanes-1:0]           ddr_o
);

  ////////////////
  //   PHY TX   //
  ////////////////
  serial_link_physical_tx #(
    .NumLanes   ( NumLanes  ),
    .MaxClkDiv  ( MaxClkDiv )
  ) i_serial_link_physical_tx (
    .rst_ni,
    .clk_i,
    .clk_div_i,
    .clk_shift_start_i,
    .clk_shift_end_i,
    .data_out_i,
    .data_out_valid_i,
    .data_out_ready_o,
    .ddr_rcv_clk_o,
    .ddr_o
  );

  ////////////////
  //   PHY RX   //
  ////////////////
  serial_link_physical_rx #(
    .phy_data_t ( phy_data_t  ),
    .NumLanes   ( NumLanes    ),
    .FifoDepth  ( FifoDepth   )
  ) i_serial_link_physical_rx (
    .clk_i,
    .rst_ni,
    .ddr_rcv_clk_i,
    .data_in_o,
    .data_in_valid_o,
    .data_in_ready_i,
    .ddr_i
  );

endmodule

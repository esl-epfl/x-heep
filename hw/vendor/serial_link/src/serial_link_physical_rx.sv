// Copyright 2022 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51

// Author: Tim Fischer <fischeti@iis.ee.ethz.ch>

`include "common_cells/registers.svh"
`include "common_cells/assertions.svh"

// Implements the Physical Layer of the Serial Link
// The number of Channels and Lanes per Channel is parametrizable


// Implements a single TX channel which forwards the source-synchronous clock

// Impelements a single RX channel which samples the data with the received clock
// Synchronizes the data with the System clock with a CDC
module serial_link_physical_rx #(
  parameter type phy_data_t   = serial_link_pkg::phy_data_t,
  parameter int NumLanes      = 8,
  parameter int FifoDepth     = 8,
  parameter int CdcSyncStages = 2
) (
  input  logic                  clk_i,
  input  logic                  rst_ni,
  input  logic                  ddr_rcv_clk_i,
  output logic [NumLanes*2-1:0] data_in_o,
  output logic                  data_in_valid_o,
  input  logic                  data_in_ready_i,
  input  logic [NumLanes-1:0]   ddr_i
);

  phy_data_t            data_in;
  logic [NumLanes-1:0]  ddr_q;

  ///////////////////////////////
  //   CLOCK DOMAIN CROSSING   //
  ///////////////////////////////

  cdc_fifo_gray #(
    .T            ( phy_data_t                        ),
    .LOG_DEPTH    ( $clog2(FifoDepth) + CdcSyncStages ),
    .SYNC_STAGES  ( CdcSyncStages                     )
  ) i_cdc_in (
    .src_clk_i   ( ddr_rcv_clk_i    ),
    .src_rst_ni  ( rst_ni           ),
    .src_data_i  ( data_in          ),
    .src_valid_i ( 1'b1             ),
    .src_ready_o (                  ),

    .dst_clk_i   ( clk_i            ),
    .dst_rst_ni  ( rst_ni           ),
    .dst_data_o  ( data_in_o        ),
    .dst_valid_o ( data_in_valid_o  ),
    .dst_ready_i ( data_in_ready_i  )
  );

  // TODO: Fix assertion during reset
  // `ASSERT(CdcRxFifoFull, !(i_cdc_in.src_valid_i & ~i_cdc_in.src_ready_o), ddr_rcv_clk_i, !rst_ni)

  ////////////////
  //   DDR IN   //
  ////////////////
  always_ff @(negedge ddr_rcv_clk_i, negedge rst_ni) ddr_q <= !rst_ni ? '0 : ddr_i;
  assign data_in = {ddr_i, ddr_q};

endmodule



// Copyright 2022 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51

// Author: Tim Fischer <fischeti@iis.ee.ethz.ch>

`include "common_cells/registers.svh"
`include "common_cells/assertions.svh"

// Implements the Physical Layer of the Serial Link
// The number of Channels and Lanes per Channel is parametrizable


// Implements a single TX channel which forwards the source-synchronous clock
module serial_link_physical_tx #(
  parameter int NumLanes   = 8,
  parameter int MaxClkDiv  = 32
) (
  input  logic                          clk_i,
  input  logic                          rst_ni,
  input  logic [$clog2(MaxClkDiv):0]    clk_div_i,
  input  logic [$clog2(MaxClkDiv):0]    clk_shift_start_i,
  input  logic [$clog2(MaxClkDiv):0]    clk_shift_end_i,
  input  logic [NumLanes*2-1:0]         data_out_i,
  input  logic                          data_out_valid_i,
  output logic                          data_out_ready_o,
  output logic                          ddr_rcv_clk_o,
  output logic [NumLanes-1:0]           ddr_o
);

  logic [NumLanes*2-1:0]      data_out_q;

  logic [$clog2(MaxClkDiv):0] clk_cnt_q, clk_cnt_d;
  logic clk_enable;
  logic clk_toggle, clk_slow_toggle;
  logic clk_slow;
  logic ddr_sel;

  // Valid is always set, but
  // src_clk is clock gated
  assign data_out_ready_o = data_out_valid_i & (clk_cnt_q == clk_div_i - 1);

  ///////////////////////////////////////
  //   CLOCK DIVIDER + PHASE SHIFTER   //
  ///////////////////////////////////////

  always_comb begin

    clk_cnt_d = 0;

    if (data_out_valid_i) begin
      clk_cnt_d = (clk_cnt_q == clk_div_i - 1)? '0 : clk_cnt_q + 1;
    end

    clk_enable = data_out_valid_i;
    clk_toggle = (clk_cnt_q == clk_shift_start_i) | (clk_cnt_q == clk_shift_end_i);
    clk_slow_toggle = (clk_cnt_q == 0) | (clk_cnt_q == clk_div_i/2);
  end

  `FF(clk_cnt_q, clk_cnt_d, '0)

  // The ddr_rcv_clk_o T-Flip-Flop intentionally uses blocking assignments! If we were to use
  // non-blocking assignment like we normally do for flip-flops, we would create
  // a race condition when sampling data from the fast clock domain into
  // flip-flops clocked by ddr_rcv_clk_o. To avoid this, we use blocking assignments
  // which is the reccomended method acording to:
  // S. Sutherland and D. Mills,
  // Verilog and System Verilog gotchas: 101 common coding errors and how to
  // avoid them. New York: Springer, 2007. page 64.

  always_ff @(posedge clk_i, negedge rst_ni) begin
    if (~rst_ni) begin
      ddr_rcv_clk_o = 1'b1;
      clk_slow <= 1'b0;
      ddr_sel <= 1'b0;
    end else begin
      if (clk_enable) begin
        if (clk_toggle) begin
          ddr_rcv_clk_o = !ddr_rcv_clk_o;
        end
        if (clk_slow_toggle) begin
          clk_slow <= !clk_slow;
          ddr_sel <= !ddr_sel;
        end
      end else begin
        ddr_rcv_clk_o = 1'b1;
        clk_slow <= 1'b0;
        ddr_sel <= 1'b0;
      end
    end
  end

  /////////////////
  //   DDR OUT   //
  /////////////////
  `FF(data_out_q, data_out_i, '0, clk_slow, rst_ni)
  assign ddr_o = (ddr_sel)? data_out_q[NumLanes-1:0] : data_out_q[NumLanes*2-1:NumLanes];

endmodule

// Impelements a single RX channel which samples the data with the received clock
// Synchronizes the data with the System clock with a CDC



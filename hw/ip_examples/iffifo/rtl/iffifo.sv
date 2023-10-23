// Copyright 2023 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// Author: Pierre Guillod <pierre.guillod@epfl.ch>, EPFL, STI-SEL
// Date: 18.10.2023

module iffifo #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic,
    localparam int WIDTH = 32,
    localparam int DEPTH = 4
) (
    input logic clk_i,
    input logic rst_ni,
    input reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,
    output logic iffifo_in_ready_o,
    output logic iffifo_out_valid_o
);

  import iffifo_reg_pkg::*;

  iffifo_reg2hw_t reg2hw;
  iffifo_hw2reg_t hw2reg;

  logic [WIDTH-1:0] fifout;

  assign iffifo_in_ready_o  = 1;
  assign iffifo_out_valid_o = 1;

  assign hw2reg.fifo_out.d  = fifout + 1;

  iffifo_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) iffifo_reg_top_i (
      .clk_i,
      .rst_ni,
      .reg2hw,
      .hw2reg,
      .reg_req_i,
      .reg_rsp_o,
      .devmode_i(1'b0)
  );

  fifo_v3 #(
      .DEPTH(DEPTH),
      .DATA_WIDTH(WIDTH)
  ) iffifo_fifo_i (
      .clk_i,
      .rst_ni,
      .flush_i(1'b0),
      .testmode_i(1'b0),
      .full_o(),
      .empty_o(),
      .usage_o(),
      .data_i(reg2hw.fifo_in.q),
      .push_i(reg2hw.fifo_in.qe),
      .data_o(fifout),
      .pop_i(reg2hw.fifo_out.re)
  );

endmodule : iffifo


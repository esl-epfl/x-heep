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

  logic pop, push;
  logic [WIDTH-1:0] fifin, fifout;

  assign iffifo_in_ready_o  = 1;
  assign iffifo_out_valid_o = 1;

  reg_req_t [1:0] fifo_win_h2d;
  reg_rsp_t [1:0] fifo_win_d2h;

  iffifo_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) iffifo_reg_top_i (
      .clk_i,
      .rst_ni,
      .reg2hw(),
      .hw2reg(),
      .reg_req_i,
      .reg_rsp_o,
      .reg_req_win_o(fifo_win_h2d),
      .reg_rsp_win_i(fifo_win_d2h),
      .devmode_i(1'b0)
  );


  fifo_v3 #(
      .DEPTH(DEPTH),
      .DATA_WIDTH(WIDTH)
  ) pdm2pcm_fifo_i (
      .clk_i,
      .rst_ni,
      .flush_i(1'b0),
      .testmode_i(1'b0),
      .full_o(),
      .empty_o(),
      .usage_o(),
      .data_i(fifin),
      .push_i(push),
      .data_o(fifout),
      .pop_i(pop)
  );

  iffifo_window #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) u_window (
      .rx_win_i  (fifo_win_h2d[0]),
      .rx_win_o  (fifo_win_d2h[0]),
      .tx_win_i  (fifo_win_h2d[1]),
      .tx_win_o  (fifo_win_d2h[1]),
      .tx_data_o (fifin),
      .tx_be_o   (),
      .tx_valid_o(push),
      .rx_data_i (fifout),
      .rx_ready_o(pop)
  );

endmodule : iffifo


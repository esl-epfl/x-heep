// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module pdm2pcm #(
  parameter int unsigned FIFO_DEPTH = 4,
  parameter type reg_req_t = logic,
  parameter type reg_rsp_t = logic,
) (
  input logic clk_i,
  input logic rst_ni,

  // Register interface
  input  reg_req_t reg_req_i,
  output reg_rsp_t reg_rsp_o,

  // PDM interface
  input  logic pdm_i;
  output logic pdm_clk_o
);

  import pdm2pcm_reg_pkg::*;

  logic [15:0] par_clkdiv_idx;
  logic [ 3:0] par_decim_idx_combs;
  logic [ 4:0] par_decim_idx_hfbd2;
  logic [ 5:0] par_decim_idx_fir;
  logic [15:0] coeffs_hb1[0: 5];
  logic [15:0] coeffs_hb2[0:11];
  logic [15:0] coeffs_fir[0:13];

  logic [5:0] fifo_usage;

  logic [19:0] pcm_o;

  // FIFO/window related signals
  logic [31:0] rx_data;
  logic        rx_valid;

  // Interface signals
  pdm2pcm_reg2hw_t reg2hw;
  pdm2pcm_hw2reg_t hw2reg;

  reg_req_t fifo_win_h2d;
  reg_rsp_t fifo_win_d2h;

  assign hw2reg.status.reach.d = (fifo_usage + 1) > reg2hw.reachcount.count.q;

  assign par_clkdiv_idx = reg2hw.clkdividx.count.q
  assign par_decim_idx_combs = reg2hw.decimcic.count.q
  assign par_decim_idx_hfbd2 = reg2hw.decimhb1.count.q
  assign par_decim_idx_fir   = reg2hw.decimhb2.count.q

  assign coeffs_hb1 = '{
    reg2hw.hb1coef00.coeff.q,
    reg2hw.hb1coef01.coeff.q,
    reg2hw.hb1coef02.coeff.q,
    reg2hw.hb1coef03.coeff.q,
    reg2hw.hb1coef04.coeff.q,
    reg2hw.hb1coef05.coeff.q
  };

  assign coeffs_hb2 = '{
    reg2hw.hb2coef00.coeff.q,
    reg2hw.hb2coef01.coeff.q,
    reg2hw.hb2coef02.coeff.q,
    reg2hw.hb2coef03.coeff.q,
    reg2hw.hb2coef04.coeff.q,
    reg2hw.hb2coef05.coeff.q,
    reg2hw.hb2coef06.coeff.q,
    reg2hw.hb2coef07.coeff.q,
    reg2hw.hb2coef08.coeff.q,
    reg2hw.hb2coef09.coeff.q,
    reg2hw.hb2coef10.coeff.q,
    reg2hw.hb2coef11.coeff.q,
  };

  assign coeffs_fir = '{
    reg2hw.fircoef00.coeff.q,
    reg2hw.fircoef01.coeff.q,
    reg2hw.fircoef02.coeff.q,
    reg2hw.fircoef03.coeff.q,
    reg2hw.fircoef04.coeff.q,
    reg2hw.fircoef05.coeff.q,
    reg2hw.fircoef06.coeff.q,
    reg2hw.fircoef07.coeff.q,
    reg2hw.fircoef08.coeff.q,
    reg2hw.fircoef09.coeff.q,
    reg2hw.fircoef10.coeff.q,
    reg2hw.fircoef11.coeff.q,
    reg2hw.fircoef12.coeff.q,
    reg2hw.fircoef13.coeff.q,
  };

  pdm_core #() pdm_core_i (
    .clk_i,
    .rstn_i,
    .en_i(reg2hw.control.enabl.q),
    .par_decim_idx_combs,
    .par_decim_idx_hfbd2,
    .par_decim_idx_fir,
    .par_clkdiv_idx,
    .coeffs_hb1,
    .coeffs_hb2,
    .coeffs_fir,
    .pdm_clk_o,
    .pdm_i,
    .pcm_o,
    .pcm_data_valid_o
  );

  fifo_v3 #(
      .DEPTH(FIFO_DEPTH)
  ) pdm2pcm_fifo_i (
      .clk_i,
      .rst_ni,
      .flush_i(reg2hw.control.clear.qe),
      .testmode_i(1'b0),
      .full_o(hw2reg.status.fulll.d),
      .empty_o(hw2reg.status.empty.d),
      .usage_o(fifo_usage),
      .data_i(pcm_o),
      .push_i(pcm_data_valid_o),
      .data_o(rx_data),
      .pop_i(rx_ready)
  );

  pdm2pcm_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) dma_reg_top_i (
      .clk_i,
      .rst_ni,
      .reg_req_i(fifo_win_h2d),
      .reg_rsp_o(fifo_win_d2h),
      .reg2hw,
      .hw2reg,
      .devmode_i(1'b1)
  );

  pdm2pcm_window #(
    .reg_req_t  (reg_req_t),
    .reg_rsp_t  (reg_rsp_t)
  ) u_window (
    .clk_i,
    .rst_ni,
    .rx_win_i   (fifo_win_h2d),
    .rx_win_o   (fifo_win_d2h),
    .rx_data_i  (rx_data),
    .rx_ready_o (rx_ready)
  );

endmodule : pdm2pcm

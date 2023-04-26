// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Pierre Guillod <pierre.guillod@epfl.ch>, EPFL, STI-SEL
// Date: 19.02.2022
// Description: Top wrapper for the PDM2PCM acquisition peripheral

module pdm2pcm #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic,
    parameter int unsigned FIFO_DEPTH = 4,
    parameter int unsigned FIFO_WIDTH = 18,
    localparam int unsigned FIFO_ADDR_WIDTH = $clog2(FIFO_DEPTH)
) (
    input logic clk_i,
    input logic rst_ni,

    // Register interface
    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    // PDM interface
    input  logic pdm_i,
    output logic pdm_clk_o
);

  import pdm2pcm_reg_pkg::*;

  logic              [               15:0]     par_clkdiv_idx;
  logic              [                3:0]     par_decim_idx_combs;
  logic              [                4:0]     par_decim_idx_hfbd2;
  logic              [                5:0]     par_decim_idx_fir;
  logic              [               17:0]     coeffs_hb1          [ 0:3];
  logic              [               17:0]     coeffs_hb2          [ 0:6];
  logic              [               17:0]     coeffs_fir          [0:13];

  logic              [FIFO_ADDR_WIDTH-1:0]     fifo_usage;

  logic                                        pcm_data_valid;

  logic                                        rx_ready;

  logic              [               17:0]     pcm;

  // FIFO/window related signals
  logic              [               31:0]     rx_data;
  logic              [     FIFO_WIDTH-1:0]     rx_fifo;

  // Interface signals
  pdm2pcm_reg2hw_t                             reg2hw;
  pdm2pcm_hw2reg_t                             hw2reg;

  reg_req_t        [                      0:0] fifo_win_h2d;
  reg_rsp_t        [                      0:0] fifo_win_d2h;

  logic push, pop;
  logic empty, full;

  assign rx_data = ({{{32 - FIFO_WIDTH} {1'b0}}, rx_fifo});

  assign hw2reg.status.reach.d  = ({{{32-FIFO_ADDR_WIDTH}{1'b0}},fifo_usage}) > {{26{1'b0}},reg2hw.reachcount.q};
  assign hw2reg.status.reach.de = 1;
  assign hw2reg.status.fulll.de = 1;
  assign hw2reg.status.empty.de = 1;
  assign par_clkdiv_idx = reg2hw.clkdividx.q;
  assign par_decim_idx_combs = reg2hw.decimcic.q;
  assign par_decim_idx_hfbd2 = reg2hw.decimhb1.q;
  assign par_decim_idx_fir = reg2hw.decimhb2.q;

  assign coeffs_hb1 = '{
          reg2hw.hb1coef00.q,
          reg2hw.hb1coef01.q,
          reg2hw.hb1coef02.q,
          reg2hw.hb1coef03.q
      };

  assign coeffs_hb2 = '{
          reg2hw.hb2coef00.q,
          reg2hw.hb2coef01.q,
          reg2hw.hb2coef02.q,
          reg2hw.hb2coef03.q,
          reg2hw.hb2coef04.q,
          reg2hw.hb2coef05.q,
          reg2hw.hb2coef06.q
      };

  assign coeffs_fir = '{
          reg2hw.fircoef00.q,
          reg2hw.fircoef01.q,
          reg2hw.fircoef02.q,
          reg2hw.fircoef03.q,
          reg2hw.fircoef04.q,
          reg2hw.fircoef05.q,
          reg2hw.fircoef06.q,
          reg2hw.fircoef07.q,
          reg2hw.fircoef08.q,
          reg2hw.fircoef09.q,
          reg2hw.fircoef10.q,
          reg2hw.fircoef11.q,
          reg2hw.fircoef12.q,
          reg2hw.fircoef13.q
      };

  pdm_core #() pdm_core_i (
      .clk_i,
      .rstn_i(rst_ni),
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
      .pcm_o(pcm),
      .pcm_data_valid_o(pcm_data_valid)
  );

  assign push                  = pcm_data_valid & ~full;
  assign pop                   = rx_ready & ~empty;

  assign hw2reg.status.fulll.d = full;
  assign hw2reg.status.empty.d = empty;

  fifo_v3 #(
      .DEPTH(FIFO_DEPTH),
      .DATA_WIDTH(FIFO_WIDTH)
  ) pdm2pcm_fifo_i (
      .clk_i,
      .rst_ni,
      .flush_i(reg2hw.control.clear.q),
      .testmode_i(1'b0),
      .full_o(full),
      .empty_o(empty),
      .usage_o(fifo_usage),
      .data_i(pcm),
      .push_i(push),
      .data_o(rx_fifo),
      .pop_i(pop)
  );

  pdm2pcm_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) pdm2pcm_reg_top_i (
      .clk_i,
      .rst_ni,
      .reg_req_win_o(fifo_win_h2d),
      .reg_rsp_win_i(fifo_win_d2h),
      .reg_req_i,
      .reg_rsp_o,
      .reg2hw,
      .hw2reg,
      .devmode_i(1'b1)
  );

  pdm2pcm_window #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) u_window (
      .rx_win_i  (fifo_win_h2d),
      .rx_win_o  (fifo_win_d2h),
      .rx_data_i (rx_data),
      .rx_ready_o(rx_ready)
  );

endmodule : pdm2pcm

// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Tim Frey <tim.frey@epfl.ch>, EPFL, STI-SEL
// Date: 13.02.2023
// Description: I2s peripheral

module i2s_core #(
    parameter SampleWidth,
    parameter ClkDivSize
) (
    input logic clk_i,
    input logic rst_ni,
    input logic en_i,


    // IO interface
    output logic i2s_sck_o,
    output logic i2s_sck_oe_o,
    input  logic i2s_sck_i,
    output logic i2s_ws_o,
    output logic i2s_ws_oe_o,
    input  logic i2s_ws_i,
    output logic i2s_sd_o,
    output logic i2s_sd_oe_o,
    input  logic i2s_sd_i,

    // config
    input logic                     cfg_lsb_first_i,
    input logic [ClkDivSize-1:0] cfg_clock_div_i,
    input logic                     cfg_clk_ws_en_i,
    input logic [$clog2(SampleWidth+1)-1:0] cfg_sample_width_i,

    // FIFO
    output logic [SampleWidth-1:0] fifo_rx_data_o,
    output logic             fifo_rx_data_valid_o,
    input  logic             fifo_rx_data_ready_i,
    output logic             fifo_rx_err_o
);


  logic ws;
  assign ws = i2s_ws_oe_o ? i2s_ws_o : i2s_ws_i;


  i2s_ws_gen #(
      .SampleWidth(SampleWidth)
  ) i2s_ws_gen_i (
      .sck_i(sck),
      .rst_ni(rst_ni),
      .en_i(en_i & cfg_clk_ws_en_i),
      .ws_o(i2s_ws_o),
      .ws_oe_o(i2s_ws_oe_o),
      .cfg_sample_width_i(cfg_sample_width_i)
  );

  logic sck;
  assign i2s_sck_oe_o = en_i & cfg_clk_ws_en_i;
  assign sck = i2s_sck_oe_o ? i2s_sck_o : i2s_sck_i;

  i2s_clk_gen #(
      .ClkDivSize(ClkDivSize)
  ) i2s_clk_gen_i (
      .clk_i(clk_i),
      .rst_ni(rst_ni),
      .en_i(en_i & cfg_clk_ws_en_i),
      .sck_o(i2s_sck_o),
      .cfg_clock_div_i(cfg_clock_div_i)
  );

  assign i2s_sd_oe_o = 1'b0;
  assign i2s_sd_o    = 1'b0;

  i2s_rx_channel #(
      .SampleWidth(SampleWidth)
  ) i2s_rx_channel_i (
      .sck_i(sck),
      .rst_ni(rst_ni),
      .en_i(en_i),
      .ws_i(ws),
      .sd_i(i2s_sd_i),
      .cfg_lsb_first_i(cfg_lsb_first_i),
      .cfg_sample_width_i(cfg_sample_width_i),
      .fifo_rx_data_o(fifo_rx_data_o),
      .fifo_rx_data_valid_o(fifo_rx_data_valid_o),
      .fifo_rx_data_ready_i(fifo_rx_data_ready_i),
      .fifo_rx_err_o(fifo_rx_err_o)
  );


endmodule : i2s_core

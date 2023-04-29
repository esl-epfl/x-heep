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
    input logic en_ws_i,
    input logic en_rx_left_i,
    input logic en_rx_right_i,

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
    input logic [         ClkDivSize-1:0] cfg_clock_div_i,
    input logic [$clog2(SampleWidth)-1:0] cfg_sample_width_i,

    // FIFO
    output logic [SampleWidth-1:0] data_rx_o,
    output logic                   data_rx_valid_o,
    input  logic                   data_rx_ready_i,

    output logic data_rx_overflow_o
);

  logic                   ws_gen;
  logic                   ws;

  logic                   sck_gen;
  logic                   sck;

  logic                   clk_div_valid;
  logic                   clk_div_ready;
  logic                   clk_div_running;

  logic [SampleWidth-1:0] data_rx_dc;
  logic                   data_rx_dc_valid;
  logic                   data_rx_dc_ready;

  assign i2s_ws_oe_o = en_i & en_ws_i;
  assign i2s_ws_o = ws_gen;
  assign ws = i2s_ws_oe_o ? ws_gen : i2s_ws_i;

  assign i2s_sck_oe_o = en_i & clk_div_running;
  assign i2s_sck_o = sck_gen;

  assign i2s_sd_oe_o = 1'b0;
  assign i2s_sd_o    = 1'b0;

  assign clk_div_valid = |cfg_clock_div_i; // workaround

  logic data_rx_overflow_async;
  logic data_rx_overflow_q;

  i2s_ws_gen #(
      .SampleWidth(SampleWidth)
  ) i2s_ws_gen_i (
      .sck_i(sck),
      .rst_ni(rst_ni),
      .en_i(en_ws_i),
      .ws_o(ws_gen),
      .cfg_sample_width_i(cfg_sample_width_i)
  );

  tc_clk_mux2 i_clk_bypass_mux (
      .clk0_i   (i2s_sck_i),
      .clk1_i   (sck_gen),
      .clk_sel_i(1'b1),       // disable external sck
      .clk_o    (sck)
  );


  // This is a workaround
  // Such that it starts with the demanded div value.
  always_ff @(posedge clk_i, negedge rst_ni) begin
    if (~rst_ni) begin
      clk_div_running <= 1'b0;
    end else begin
      if (clk_div_ready) begin
        clk_div_running <= 1'b1;
      end
    end
  end

  clk_int_div #(
      .DIV_VALUE_WIDTH(ClkDivSize),
      .DEFAULT_DIV_VALUE(2)  // HAS TO BE BIGGER THAN ONE TO GET THE START RIGHT
  ) i2s_clk_gen_i (
      .clk_i(clk_i),
      .rst_ni(rst_ni),
      .en_i(en_i),
      .test_mode_en_i(1'b0),
      .clk_o(sck_gen),
      .div_i(cfg_clock_div_i),
      .div_valid_i(clk_div_valid),
      .div_ready_o(clk_div_ready)
  );

  i2s_rx_channel #(
      .SampleWidth(SampleWidth)
  ) i2s_rx_channel_i (
      .sck_i(sck),
      .rst_ni(rst_ni),
      .en_left_i(en_rx_left_i),
      .en_right_i(en_rx_right_i),
      .ws_i(ws),
      .sd_i(i2s_sd_i),

      .cfg_sample_width_i(cfg_sample_width_i),

      .data_o(data_rx_dc),
      .data_valid_o(data_rx_dc_valid),
      .data_ready_i(data_rx_dc_ready),
      .overflow_o(data_rx_overflow_async)
  );

  // cdc
  cdc_fifo_2phase #(
      .T(logic [31:0]),
      .LOG_DEPTH(2)
  ) rx_cdc_i (
      .src_clk_i  (sck),
      .src_rst_ni (rst_ni),
      .src_ready_o(data_rx_dc_ready),
      .src_data_i (data_rx_dc),
      .src_valid_i(data_rx_dc_valid),

      .dst_rst_ni (rst_ni),
      .dst_clk_i  (clk_i),
      .dst_data_o (data_rx_o),
      .dst_valid_o(data_rx_valid_o),
      .dst_ready_i(data_rx_ready_i)
  );

  // SYNC rx overflow signal
  always_ff @(posedge clk_i, negedge rst_ni) begin
    if (~rst_ni) begin
      data_rx_overflow_o <= 1'b0;
      data_rx_overflow_q <= 1'b0;
    end else begin
      data_rx_overflow_o <= data_rx_overflow_q;
      data_rx_overflow_q <= data_rx_overflow_async;
    end
  end

endmodule : i2s_core

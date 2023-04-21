// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Tim Frey <tim.frey@epfl.ch>, EPFL, STI-SEL
// Date: 07.02.2023
// Description: I2s peripheral

module i2s #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic
) (
    input logic clk_i,
    input logic rst_ni,

    // Register interface
    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    // I2s interface
    output logic i2s_sck_o,
    output logic i2s_sck_oe_o,
    input  logic i2s_sck_i,
    output logic i2s_ws_o,
    output logic i2s_ws_oe_o,
    input  logic i2s_ws_i,
    output logic i2s_sd_o,
    output logic i2s_sd_oe_o,
    input  logic i2s_sd_i,

    // Interrupt
    output logic intr_i2s_event_o,

    // DMA signal
    output logic i2s_rx_valid_o
);

  import i2s_reg_pkg::*;

  localparam SampleWidth = (1 << BytePerSampleWidth) * 8;
  localparam CounterWidth = BytePerSampleWidth + 3;

  // Interface signals
  i2s_reg2hw_t reg2hw;
  i2s_hw2reg_t hw2reg;

  // RX Window Interface signals
  reg_req_t    rx_win_h2d;
  reg_rsp_t    rx_win_d2h;

  logic en;
  assign en = |reg2hw.cfg.en.q;

  logic [SampleWidth-1:0] data_rx;
  logic data_rx_valid;
  logic data_rx_ready;

  logic [CounterWidth-1:0] sample_width;
  assign sample_width = {reg2hw.cfg.data_width.q, 3'h7};


  // I2s RX -> Bus
  assign rx_win_d2h.ready = rx_win_h2d.valid && rx_win_h2d.addr[BlockAw-1:0] == i2s_reg_pkg::I2S_RXDATA_OFFSET && !rx_win_h2d.write;
  assign rx_win_d2h.rdata = data_rx;
  assign rx_win_d2h.error = !data_rx_valid;

  assign data_rx_ready = rx_win_d2h.ready;
  assign hw2reg.status.d = data_rx_ready;

  // DMA signal
  assign i2s_rx_valid_o = data_rx_valid;


  // Register logic
  i2s_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) i2s_reg_top_i (
      .clk_i,
      .rst_ni,
      .reg_req_i,
      .reg_rsp_o,
      .reg_req_win_o(rx_win_h2d),  // host to device
      .reg_rsp_win_i(rx_win_d2h),  // device to host
      .reg2hw,
      .hw2reg,
      .devmode_i(1'b1)
  );



  // Core logic
  i2s_core #(
      .SampleWidth(SampleWidth),
      .ClkDivSize (ClkDivSize)
  ) i2s_core_i (
      .clk_i(clk_i),
      .rst_ni(rst_ni),
      .en_i(en),
      .en_left_i(reg2hw.cfg.en.q[0]),
      .en_right_i(reg2hw.cfg.en.q[1]),

      .i2s_sck_o   (i2s_sck_o),
      .i2s_sck_oe_o(i2s_sck_oe_o),
      .i2s_sck_i   (i2s_sck_i),
      .i2s_ws_o    (i2s_ws_o),
      .i2s_ws_oe_o (i2s_ws_oe_o),
      .i2s_ws_i    (i2s_ws_i),
      .i2s_sd_o    (i2s_sd_o),
      .i2s_sd_oe_o (i2s_sd_oe_o),
      .i2s_sd_i    (i2s_sd_i),

      .cfg_clk_ws_en_i(reg2hw.cfg.gen_clk_ws.q),
      .cfg_lsb_first_i(reg2hw.cfg.lsb_first.q),
      .cfg_clock_div_i(reg2hw.clkdividx.q),
      .cfg_sample_width_i(sample_width),

      .data_rx_o(data_rx),
      .data_rx_valid_o(data_rx_valid),
      .data_rx_ready_i(data_rx_ready)
  );

  logic event_i2s_event;
  assign intr_i2s_event_o = event_i2s_event & reg2hw.cfg.intr_en.q;


  // interrupt reach count event
  // count bus reads to be on clk_i
  logic [31:0] intr_reach_counter;
  always_ff @(posedge clk_i, negedge rst_ni) begin
    if (~rst_ni) begin
      intr_reach_counter <= 32'h0;
    end else begin
      if (intr_reach_counter == reg2hw.watermark.q) begin
        if (en & data_rx_ready & data_rx_valid) begin
          intr_reach_counter <= 32'h1;
        end else begin
          intr_reach_counter <= 32'h0;
        end
      end else if (en & data_rx_ready & data_rx_valid) begin
        intr_reach_counter <= intr_reach_counter + 32'h1;
      end
    end
  end

  assign event_i2s_event = intr_reach_counter == reg2hw.watermark.q & reg2hw.watermark.q != 32'h0;


endmodule : i2s

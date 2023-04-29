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

  logic [SampleWidth-1:0] data_rx;
  logic data_rx_valid;
  logic data_rx_ready;
  logic data_rx_overflow;

  logic event_i2s_event;

  logic [CounterWidth-1:0] sample_width;
  assign sample_width = {reg2hw.control.data_width.q, 3'h7};


  // I2s RX -> Bus
  assign data_rx_ready = reg2hw.rxdata.re; // bus read signal
  assign hw2reg.rxdata.d = data_rx;

  // DMA signal
  assign i2s_rx_valid_o = data_rx_valid;

  // STATUS signal
  assign hw2reg.status.rx_data_ready.d = data_rx_valid;
  assign hw2reg.status.rx_overflow.d = data_rx_overflow;


  // Register logic
  i2s_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) i2s_reg_top_i (
      .clk_i,
      .rst_ni,
      .reg_req_i,
      .reg_rsp_o,
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
      .en_i(reg2hw.control.en.q),
      .en_ws_i(reg2hw.control.en_ws.q),
      .en_rx_left_i(reg2hw.control.en_rx.q[0]),
      .en_rx_right_i(reg2hw.control.en_rx.q[1]),

      .i2s_sck_o   (i2s_sck_o),
      .i2s_sck_oe_o(i2s_sck_oe_o),
      .i2s_sck_i   (i2s_sck_i),
      .i2s_ws_o    (i2s_ws_o),
      .i2s_ws_oe_o (i2s_ws_oe_o),
      .i2s_ws_i    (i2s_ws_i),
      .i2s_sd_o    (i2s_sd_o),
      .i2s_sd_oe_o (i2s_sd_oe_o),
      .i2s_sd_i    (i2s_sd_i),

      .cfg_clock_div_i(reg2hw.clkdividx.q),
      .cfg_sample_width_i(sample_width),

      .data_rx_o(data_rx),
      .data_rx_valid_o(data_rx_valid),
      .data_rx_ready_i(data_rx_ready),
      .data_rx_overflow_o(data_rx_overflow)
  );


  // watermark counter
  // count bus reads and trigger interrupt 
  event_counter #(
      .WIDTH(WatermarkSize)
  ) watermark_counter (
      .clk_i,
      .rst_ni,
      .clear_i(reg2hw.control.reset_watermark.q),  // synchronous clear
      .en_i(data_rx_ready & data_rx_valid & (|reg2hw.watermark.q) && reg2hw.control.en_watermark.q),    // enable the counter
      .limit_i(reg2hw.watermark.q),
      .q_o(hw2reg.waterlevel.d),
      .overflow_o(event_i2s_event)
  );

  assign intr_i2s_event_o = event_i2s_event & reg2hw.control.intr_en.q;


endmodule : i2s

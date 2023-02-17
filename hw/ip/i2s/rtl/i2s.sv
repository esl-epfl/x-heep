// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Tim Frey <tim.frey@epfl.ch>, EPFL, STI-SEL
// Date: 07.02.2023
// Description: I2s peripheral

module i2s #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic,
    parameter int unsigned FIFO_DEPTH = 256,
    localparam int unsigned FIFO_ADDR_WIDTH = $clog2(FIFO_DEPTH)
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
    input  logic i2s_sd_i
);

  import i2s_reg_pkg::*;
  localparam SampleWidth = (1 << BytePerSampleWidth) * 8;
  localparam CounterWidth = BytePerSampleWidth + 3;

  // Interface signals
  i2s_reg2hw_t reg2hw;
  i2s_hw2reg_t hw2reg;


  logic sck;

  // RX Window Interface signals
  reg_req_t    rx_win_h2d;
  reg_rsp_t    rx_win_d2h;


  logic rx_fifo_ready;
  logic [SampleWidth-1:0] rx_fifo_data_in;
  logic rx_fifo_data_in_valid;

  logic [SampleWidth-1:0] rx_fifo_data_out;
  logic rx_fifo_data_out_valid;

  logic rx_fifo_err;

  logic [CounterWidth-1:0] sample_width;
  assign sample_width = {reg2hw.bytepersample.q, 3'h7};

  // FIFO -> RX WINDOW
  assign rx_win_d2h.ready = rx_win_h2d.valid && rx_win_h2d.addr[BlockAw-1:0] == i2s_reg_pkg::I2S_RXDATA_OFFSET && !rx_win_h2d.write;
  assign rx_win_d2h.rdata = rx_fifo_data_out;
  assign rx_win_d2h.error = !rx_fifo_data_out_valid;


  // RX FIFO
  cdc_fifo_gray #(
      .WIDTH(SampleWidth),
      .LOG_DEPTH(FIFO_ADDR_WIDTH)
  ) rx_fifo_i (
      .src_clk_i  (clk_i),
      .src_rst_ni (rst_ni),
      .src_ready_o(rx_fifo_ready),
      .src_data_i (rx_fifo_data_in),
      .src_valid_i(rx_fifo_data_in_valid),

      .dst_rst_ni (rst_ni),
      .dst_clk_i  (sck),
      .dst_data_o (rx_fifo_data_out),
      .dst_valid_o(rx_fifo_data_out_valid),
      .dst_ready_i(rx_win_d2h.ready)
  );


  // STATUS 
  // assign hw2reg.status.fill_level.d = rx_fifo_usage;
  // assign hw2reg.status.fill_level.de = 1'b1;
  // assign hw2reg.status.full.d = rx_fifo_full;
  // assign hw2reg.status.full.de = 1'b1;
  assign hw2reg.status.empty.d = !rx_fifo_data_out_valid;
  assign hw2reg.status.empty.de = 1'b1;

  assign hw2reg.status.overflow.de = rx_fifo_err | reg2hw.control.q;
  assign hw2reg.status.overflow.d = rx_fifo_err & !reg2hw.control.q;


  // reset control bits immediatelly 
  // assign hw2reg.control.clear_fifo.de = reg2hw.control.clear_fifo.q;
  // assign hw2reg.control.clear_fifo.d = 1'b0;

  assign hw2reg.control.de = reg2hw.control.q;
  assign hw2reg.control.d = 1'b0;



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
      .clk_i (clk_i),
      .rst_ni(rst_ni),
      .en_i  (reg2hw.cfg.en),

      .i2s_sck_o   (i2s_sck_o),
      .i2s_sck_oe_o(i2s_sck_oe_o),
      .i2s_sck_i   (i2s_sck_i),
      .i2s_ws_o    (i2s_ws_o),
      .i2s_ws_oe_o (i2s_ws_oe_o),
      .i2s_ws_i    (i2s_ws_i),
      .i2s_sd_o    (i2s_sd_o),
      .i2s_sd_oe_o (i2s_sd_oe_o),
      .i2s_sd_i    (i2s_sd_i),

      .sck_o(sck),

      .cfg_clk_ws_en_i(reg2hw.cfg.gen_clk_ws.q),
      .cfg_lsb_first_i(reg2hw.cfg.lsb_first.q),
      .cfg_clock_div_i(reg2hw.clkdividx.q),
      .cfg_sample_width_i(sample_width),

      .fifo_rx_data_o(rx_fifo_data_in),
      .fifo_rx_data_valid_o(rx_fifo_data_in_valid),
      .fifo_rx_data_ready_i(rx_fifo_ready),
      .fifo_rx_err_o(rx_fifo_err)
  );

endmodule : i2s

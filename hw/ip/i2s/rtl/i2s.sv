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
    input  logic i2s_sd_i
);

  import i2s_reg_pkg::*;


  // Interface signals
  i2s_reg2hw_t reg2hw;
  i2s_hw2reg_t hw2reg;


  // RX Window Interface signals
  reg_req_t    rx_win_h2d;
  reg_rsp_t    rx_win_d2h;


  integer      count;


  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (~rst_ni) begin
    end else begin
      if (reg2hw.inputdata.qe) begin
        hw2reg.outputdata.d  <= reg2hw.inputdata.q;
        hw2reg.outputdata.de <= 1'b1;
      end else begin
        hw2reg.outputdata.de <= 1'b0;
      end
    end
  end

  assign rx_win_d2h.ready = rx_win_h2d.valid && rx_win_h2d.addr[BlockAw-1:0] == i2s_reg_pkg::I2S_RXDATA_OFFSET && !rx_win_h2d.write;
  assign rx_win_d2h.rdata = count;
  assign rx_win_d2h.error = 1'b0;

  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (~rst_ni) begin
      count <= 0;
    end else begin
      if (rx_win_d2h.ready) count <= count + 1;
    end
  end

  assign i2s_sck_o = 1'b0;
  assign i2s_sck_oe_o = 1'b1;
  assign i2s_ws_o = 1'b0;
  assign i2s_ws_oe_o = 1'b1;
  assign i2s_sd_o = 1'b0;
  assign i2s_sd_oe_o = 1'b0;

  logic sck;
  logic ws;

  assign sck = i2s_sck_oe_o ? i2s_sck_o : i2s_sck_i;
  assign ws  = i2s_ws_oe_o ? i2s_ws_o : i2s_ws_i;

  logic [31:0] data;
  logic [31:0] storage;

  logic store;
  int index;
  logic right_ch;

  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (~rst_ni) begin
      index <= 0;
      right_ch <= 1'b0;
    end else begin
      if (sck == 1'b0) begin
        if (index < 32) begin
          data[32-index] <= i2s_sd_i;
        end
        if (ws != right_ch) begin
          index <= 0;
          right_ch <= ws;
          store <= 1;
        end
        if (store) begin
          storage <= data;
          store   <= 0;
        end
      end
    end
  end

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


endmodule : i2s

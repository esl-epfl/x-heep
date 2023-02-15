// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Tim Frey <tim.frey@epfl.ch>, EPFL, STI-SEL
// Date: 13.02.2023
// Description: I2s peripheral

module i2s_rx_channel #(
    parameter  int unsigned SampleWidth,
    localparam int unsigned CounterWidth = $clog2(SampleWidth + 1)
) (
    input logic sck_i,
    input logic rst_ni,
    input logic en_i,
    input logic ws_i,
    input logic sd_i,

    // config
    input logic cfg_lsb_first_i,
    input logic [CounterWidth-1:0] cfg_sample_width_i,

    // FIFO
    output logic [SampleWidth-1:0] fifo_rx_data_o,
    output logic                   fifo_rx_data_valid_o,
    input  logic                   fifo_rx_data_ready_i,
    output logic                   fifo_rx_err_o
);


  logic                    r_ws_old;
  logic                    s_ws_edge;

  logic [ SampleWidth-1:0] r_shiftreg_ch0;
  logic [ SampleWidth-1:0] s_shiftreg_ch0;
  logic [ SampleWidth-1:0] r_shiftreg_ch0_shadow;

  logic [CounterWidth-1:0] r_count_bit;

  logic                    s_word_done;

  logic                    r_started;
  logic                    r_started_dly;

  logic                    r_ch0_valid;

  assign s_ws_edge = ws_i ^ r_ws_old;

  assign s_word_done = r_count_bit == cfg_sample_width_i;

  assign fifo_rx_data_o = r_ch0_valid ? r_shiftreg_ch0_shadow : {(SampleWidth) {1'b0}};
  assign fifo_rx_data_valid_o = r_ch0_valid;
  assign fifo_rx_err_o = r_ch0_valid & ~fifo_rx_data_ready_i & s_word_done;

  always_comb begin : proc_shiftreg
    s_shiftreg_ch0 = r_shiftreg_ch0;
    if (cfg_lsb_first_i) begin
      s_shiftreg_ch0 = {1'b0, r_shiftreg_ch0[SampleWidth-1:1]};
      s_shiftreg_ch0[cfg_sample_width_i-1] = sd_i;
    end else begin
      s_shiftreg_ch0 = {r_shiftreg_ch0[SampleWidth-2:0], sd_i};
    end
  end

  always_ff @(posedge sck_i, negedge rst_ni) begin
    if (rst_ni == 1'b0) begin
      r_shiftreg_ch0 <= 'h0;
      r_shiftreg_ch0_shadow <= 'h0;
      r_ch0_valid <= 1'b0;
    end else begin
      if (r_started_dly) begin
        r_shiftreg_ch0 <= s_shiftreg_ch0;
        if (s_word_done) begin
          r_shiftreg_ch0_shadow <= r_shiftreg_ch0;
          r_ch0_valid <= 1'b1;
        end
      end
      if (r_ch0_valid) begin
        if (fifo_rx_data_ready_i) begin
          r_ch0_valid <= 1'b0;
        end
      end
    end
  end

  always_ff @(posedge sck_i, negedge rst_ni) begin
    if (rst_ni == 1'b0) begin
      r_count_bit <= 'h0;
    end else begin
      if (r_started_dly) begin
        if (s_word_done) r_count_bit <= 'h0;
        else r_count_bit <= r_count_bit + 1;
      end
    end
  end


  always_ff @(posedge sck_i, negedge rst_ni) begin
    if (rst_ni == 1'b0) begin
      r_ws_old      <= 'h0;
      r_started     <= 'h0;
      r_started_dly <= 'h0;
    end else begin
      r_ws_old <= ws_i;
      r_started_dly <= r_started;
      if (s_ws_edge) begin
        if (en_i) r_started <= 1'b1;
        else r_started <= 1'b0;
      end
    end
  end


endmodule : i2s_rx_channel

// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Tim Frey <tim.frey@epfl.ch>, EPFL, STI-SEL
// Date: 13.02.2023
// Description: I2s peripheral

// Adapted from github.com/pulp-platform/udma_i2s/blob/master/rtl/i2s_rx_channel.sv 
// by Antonio Pullini (pullinia@iis.ee.ethz.ch)

module i2s_rx_channel #(
    parameter  int unsigned SampleWidth,
    localparam int unsigned CounterWidth = $clog2(SampleWidth)
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


  logic r_ws_old;
  logic s_ws_edge;

  logic [SampleWidth-1:0] r_shiftreg;
  logic [SampleWidth-1:0] s_shiftreg;
  logic [SampleWidth-1:0] r_shadow;

  logic [CounterWidth-1:0] r_count_bit;

  logic s_word_done;

  logic r_started;
  logic r_started_dly;  // "delayed" by one extra cycle as the ws edge comes one early 

  logic r_valid;

  assign s_ws_edge = ws_i ^ r_ws_old;

  assign s_word_done = r_count_bit == cfg_sample_width_i;

  assign fifo_rx_data_o = r_valid ? r_shadow : {(SampleWidth) {1'b0}};
  assign fifo_rx_data_valid_o = r_valid;
  assign fifo_rx_err_o = r_valid & ~fifo_rx_data_ready_i & s_word_done;

  always_comb begin : proc_shiftreg
    s_shiftreg = r_shiftreg;
    if (cfg_lsb_first_i) begin
      s_shiftreg = {1'b0, r_shiftreg[SampleWidth-1:1]};
      s_shiftreg[cfg_sample_width_i] = sd_i;
    end else begin
      s_shiftreg = {r_shiftreg[SampleWidth-2:0], sd_i};
    end
  end

  always_ff @(posedge sck_i, negedge rst_ni) begin
    if (rst_ni == 1'b0) begin
      r_shiftreg <= 'h0;
      r_shadow <= 'h0;
      r_valid <= 1'b0;
    end else begin
      if (r_started_dly) begin
        r_shiftreg <= s_shiftreg;
        if (s_word_done) begin
          r_shadow <= r_shiftreg;
          r_valid  <= 1'b1;
        end
      end
      if (r_valid) begin
        if (fifo_rx_data_ready_i) begin
          r_valid <= 1'b0;
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

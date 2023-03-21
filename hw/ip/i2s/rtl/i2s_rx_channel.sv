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
    output logic [SampleWidth-1:0] data_o,
    output logic                   data_valid_o,
    input  logic                   data_ready_i
);


  logic r_ws_old;
  logic s_ws_edge;

  logic [SampleWidth-1:0] r_shiftreg;
  logic [SampleWidth-1:0] s_shiftreg;
  logic [SampleWidth-1:0] r_shadow;

  logic [CounterWidth-1:0] r_count_bit;

  logic word_done;
  logic width_overflow;

  logic r_started;

  logic r_valid;

  assign s_ws_edge = ws_i ^ r_ws_old;


  assign data_o = r_shadow;
  assign data_valid_o = r_valid;

  always_comb begin : proc_shiftreg
    s_shiftreg = r_shiftreg;
    if (cfg_lsb_first_i) begin
      s_shiftreg = {1'b0, r_shiftreg[SampleWidth-1:1]};
      s_shiftreg[cfg_sample_width_i] = sd_i;
    end else begin
      if (word_done) begin
        s_shiftreg = 'h0;
      end
      if (!width_overflow) begin
        s_shiftreg[cfg_sample_width_i-r_count_bit] = sd_i;
      end
    end
  end

  always_ff @(posedge sck_i, negedge rst_ni) begin
    if (rst_ni == 1'b0) begin
      r_shiftreg <= 'h0;
      r_shadow <= 'h0;
      r_valid <= 1'b0;
    end else begin
      if (r_started) begin
        r_shiftreg <= s_shiftreg;
        if (word_done) begin
          r_shadow <= r_shiftreg;
          r_valid  <= 1'b1;
        end
      end
      if (r_valid) begin
        if (data_ready_i) begin
          r_valid <= 1'b0;
        end
      end
    end
  end

  always_ff @(posedge sck_i, negedge rst_ni) begin
    if (~rst_ni) begin
      word_done <= 1'b0;
    end else begin
      word_done <= r_started & s_ws_edge;
    end
  end

  always_ff @(posedge sck_i, negedge rst_ni) begin
    if (rst_ni == 1'b0) begin
      r_count_bit <= 'h0;
      width_overflow <= 1'b0;
    end else begin
      if (r_started) begin
        if (s_ws_edge) begin
          r_count_bit <= 'h0;
          width_overflow <= 1'b0;
        end else if (r_count_bit < cfg_sample_width_i) begin
          r_count_bit <= r_count_bit + 1;
        end else begin
          width_overflow <= 1'b1;
        end
      end
    end
  end


  always_ff @(posedge sck_i, negedge rst_ni) begin
    if (rst_ni == 1'b0) begin
      r_ws_old  <= 'h0;
      r_started <= 'h0;
    end else begin
      r_ws_old <= ws_i;
      if (s_ws_edge) begin
        if (en_i) r_started <= 1'b1;
        else r_started <= 1'b0;
      end
    end
  end


endmodule : i2s_rx_channel

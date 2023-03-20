// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Tim Frey <tim.frey@epfl.ch>, EPFL, STI-SEL
// Date: 13.02.2023
// Description: I2s peripheral

// Adapted from github.com/pulp-platform/udma_i2s/blob/master/rtl/i2s_ws_gen.sv 
// by Antonio Pullini (pullinia@iis.ee.ethz.ch)

module i2s_ws_gen #(
    parameter SampleWidth,
    localparam int unsigned CounterWidth = $clog2(SampleWidth)
) (
    input logic sck_i,
    input logic rst_ni,
    input logic en_i,

    output logic ws_o,

    input logic [CounterWidth-1:0] cfg_sample_width_i
);

  logic [CounterWidth-1:0] r_counter;

  always_ff @(posedge sck_i, negedge rst_ni) begin
    if (rst_ni == 1'b0) begin
      r_counter <= 'h0;
    end else begin
      if (en_i) begin
        if (r_counter == cfg_sample_width_i) r_counter <= 'h0;
        else r_counter <= r_counter + 1;
      end
    end
  end

  //Generate the internal WS signal
  always_ff @(negedge sck_i, negedge rst_ni) begin
    if (rst_ni == 1'b0) begin
      ws_o <= 1'b0;
    end else begin
      if (en_i) begin
        if (r_counter == cfg_sample_width_i) ws_o <= ~ws_o;
      end
    end
  end


endmodule : i2s_ws_gen

// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Tim Frey <tim.frey@epfl.ch>, EPFL, STI-SEL
// Date: 13.02.2023
// Description: I2s peripheral

module i2s_clk_gen #(
    parameter ClkDivSize
) (
    input logic clk_i,
    input logic rst_ni,
    input logic en_i,

    output logic sck_o,
    input logic [ClkDivSize-1:0] cfg_clock_div_i
);


  logic [ClkDivSize-1:0] r_counter;
  logic                  r_clk;
  logic [ClkDivSize-1:0] r_sampled_config;
  logic                  r_clock_en;

  //Generate the internal clock signal
  always_ff @(posedge clk_i, negedge rst_ni) begin
    if (rst_ni == 1'b0) begin
      r_counter        <= 'h0;
      r_sampled_config <= 'h0;
      r_clk            <= 1'b0;
      r_clock_en       <= 1'b0;
    end else begin
      if (en_i && !r_clock_en) begin
        r_clock_en       <= 1'b1;
        r_sampled_config <= cfg_clock_div_i;
      end else if (!en_i) begin
        if (!r_clk) begin
          r_counter  <= 'h0;
          r_clock_en <= 1'b0;
        end else begin
          if (r_counter == r_sampled_config) begin
            r_sampled_config <= cfg_clock_div_i;
            r_counter        <= 'h0;
            r_clk            <= 'h0;
          end else begin
            r_counter <= r_counter + 1;
          end
        end
      end else begin
        if (r_counter == r_sampled_config) begin
          r_counter        <= 'h0;
          r_sampled_config <= cfg_clock_div_i;
          r_clk            <= ~r_clk;
        end else begin
          r_counter <= r_counter + 1;
        end
      end
    end
  end


  assign sck_o = r_clk;




endmodule : i2s_clk_gen

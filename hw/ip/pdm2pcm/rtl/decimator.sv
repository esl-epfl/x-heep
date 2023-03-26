// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Pierre Guillod <pierre.guillod@epfl.ch>, EPFL, STI-SEL
// Date: 14.12.2022
// Description: Decimator component

module decimator #(
    // Width of counting-related signals
    parameter COUNTER_WIDTH = 4
) (
    // Clock input
    input logic clk_i,
    // Reset input
    input logic rst_i,
    // Clear input
    input logic clr_i,

    // Periodicity of the decimation
    input logic [COUNTER_WIDTH-1:0] par_decimation_index,
    // When HIGH, enables `r_sample_nr` incrementation each `clk_i` RISING.
    input logic en_i,
    // HIGH when `reg_counter` == `par_decimation_index` (may last several clock cycles)
    output logic en_o
);

  // Counter register
  logic [COUNTER_WIDTH-1:0] reg_counter;

  // Output condition
  assign en_o = en_i & (reg_counter == par_decimation_index);

  // Counter transition logic & FFs
  always_ff @(posedge clk_i or negedge rst_i) begin
    if (~rst_i) begin
      reg_counter <= 0;
    end else begin
      if (clr_i) begin
        reg_counter <= 0;
      end else if (en_i) begin
        if (reg_counter == par_decimation_index) reg_counter <= 0;
        else reg_counter <= reg_counter + 1;
      end
    end
  end

endmodule


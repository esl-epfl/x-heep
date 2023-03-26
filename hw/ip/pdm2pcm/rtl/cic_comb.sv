// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Pierre Guillod <pierre.guillod@epfl.ch>, EPFL, STI-SEL
// Date: 14.12.2022
// Description: Comb component of the CIC filter

module cic_comb #(
    // Width of the datapath
    parameter WIDTH = 4
) (
    // Clock input
    input logic clk_i,
    // Reset input
    input logic rstn_i,
    // Enable input
    input logic en_i,
    // Clear input
    input logic clr_i,

    // Data input
    input  logic [WIDTH-1:0] data_i,
    // Data output
    output logic [WIDTH-1:0] data_o
);

  // Register for the previous signal data
  logic [WIDTH-1:0] r_previousdata;
  // Register to buffer output data
  logic [WIDTH-1:0] r_data;
  // Auxiliary signal
  logic [WIDTH-1:0] s_sum;

  // Comb equation
  assign s_sum  = data_i - r_previousdata;

  assign data_o = r_data;

  // Memory points and buffer transition logic & FFs
  always_ff @(posedge clk_i or negedge rstn_i) begin
    if (~rstn_i) begin
      r_previousdata <= 'h0;
      r_data         <= 'h0;
    end else begin
      if (clr_i) begin
        r_previousdata <= 'h0;
        r_data         <= 'h0;
      end else if (en_i) begin
        r_data <= s_sum;
        r_previousdata <= data_i;
      end
    end
  end

endmodule


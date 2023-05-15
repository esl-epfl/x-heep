// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Pierre Guillod <pierre.guillod@epfl.ch>, EPFL, STI-SEL
// Date: 14.12.2022
// Description: Integrator component of the CIC filter

module cic_integrator #(
    // Width of the datapath
    parameter WIDTH = 18
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

  // Register with accumulator state
  logic [WIDTH-1:0] r_accumulator;
  // Auxiliary signal
  logic [WIDTH-1:0] s_sum;

  // Integrator equation
  assign s_sum  = r_accumulator + data_i;

  assign data_o = r_accumulator;

  // Memory point transition logic & FFs
  always_ff @(posedge clk_i or negedge rstn_i) begin
    if (~rstn_i) begin
      r_accumulator <= 'h0;
    end else begin
      if (clr_i) begin
        r_accumulator <= 'h0;
      end else if (en_i) begin
        r_accumulator <= s_sum;
      end
    end
  end

endmodule


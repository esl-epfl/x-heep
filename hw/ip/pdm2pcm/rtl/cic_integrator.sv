// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// Authors: Pierre Guillod <pierre.guillod@epfl.ch>, EPFL, STI-SEL
//          Jérémie Moullet<jeremie.moullet@epfl.ch>,EPFL, STI-SEL  
//
// Date: 06.2025
//
// Description: Single-stage integrator for CIC decimation.
//              
// This module accumulates input samples over time:
//    data_o = ∑(data_i)
// Acts as a low-pass filter in the CIC structure.
//
// Parameters:
//   - WIDTH: Bit-width of the data path.
//
// Ports:
//   - clk_i, rstn_i : Clock and active-low reset.
//   - en_i, clr_i   : Enable and synchronous clear.
//   - data_i        : Input sample.
//   - data_o        : Accumulated output sample.
//
// Behavior:
//   - On en_i: adds input to internal state.
//   - On clr_i or reset: clears accumulator.

module cic_integrator #(
    // Width of the datapath
    parameter integer WIDTH = 18
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


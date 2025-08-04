// Copyright 2025 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// Authors: Pierre Guillod <pierre.guillod@epfl.ch>, EPFL, STI-SEL
//          Jérémie Moullet<jeremie.moullet@eofl.ch>,EPFL, STI-SEL
//
// Date: 06.2025
//
// Description: Single-stage comb filter for CIC decimation.
//              Computes difference: y[n] = x[n] - x[n-D]
//
// Parameters:
//   - WIDTH          : Bit-width of the datapath.
//   - DELAYCOMBWIDTH : Bit-width of the programmable delay index.
//
// Ports:
//   - clk_i, rstn_i : Clock and active-low reset.
//   - en_i, clr_i   : Enable and synchronous clear.
//   - par_delay     : Configurable delay D.
//   - data_i        : Input sample.
//   - data_o        : Output sample.
//
// Notes:
//   - Internally maintains a shift register of delayed samples.
//   - If par_delay = 0, output default to input.


module cic_comb #(
    // Width of the datapath
    parameter integer WIDTH = 18,
    // Width of the delay parameter
    parameter integer DELAYCOMBWIDTH = 5
) (
    // Clock input
    input logic clk_i,
    // Reset input
    input logic rstn_i,
    // Enable input
    input logic en_i,
    // Clear input
    input logic clr_i,

    // Value of delay parameter
    input logic [DELAYCOMBWIDTH-1:0] par_delay,

    // Data input
    input  logic [WIDTH-1:0] data_i,
    // Data output
    output logic [WIDTH-1:0] data_o
);

  // Parameter determined at compile time
  localparam integer MAX_DELAY = 2 ** DELAYCOMBWIDTH - 1;

  // Register for the previous signal data
  logic [WIDTH-1:0] r_previousdata[MAX_DELAY:0];

  // Register to buffer output data
  logic [WIDTH-1:0] r_data;

  // Auxiliary signal
  logic [WIDTH-1:0] s_sum;

  // Comb equation
  assign s_sum  = (par_delay == 0) ? data_i : data_i - r_previousdata[par_delay-1];

  assign data_o = r_data;

  // Memory points and buffer transition logic & FFs
  always_ff @(posedge clk_i or negedge rstn_i) begin
    if (~rstn_i) begin
      for (int i = 0; i < MAX_DELAY; i++) begin
        r_previousdata[i] <= 'h0;
      end
      r_data <= 'h0;

    end else begin
      if (clr_i) begin
        for (int i = 0; i < MAX_DELAY; i++) begin
          r_previousdata[i] <= 'h0;
        end
        r_data <= 'h0;
      end else if (en_i) begin
        r_data <= s_sum;
        r_previousdata[0] <= data_i;
        for (int j = 0; j < MAX_DELAY; j++) begin
          r_previousdata[j+1] <= r_previousdata[j];
        end
      end
    end
  end

endmodule


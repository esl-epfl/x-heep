// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Pierre Guillod <pierre.guillod@epfl.ch>, EPFL, STI-SEL
// Date: 14.12.2022
// Description: FIR filter with symmetric coefficients

module fir #(
    // Width of the datapath
    parameter WIDTH = 18,
    // Width of the filter coefficients
    parameter COEFSWIDTH = 18,
    // Number of stages (= order = total number of coefficients - 1)
    parameter STAGES = 26,
    // Total number of coefficients
    localparam TOTCOEFS = STAGES + 1,
    // Number of free coefficients
    localparam NUMCOEFS = int'($ceil(TOTCOEFS / 2.0))
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
    output logic [WIDTH-1:0] data_o,

    // Free coefficients array (will be mapped to `coeffs`)
    input logic [COEFSWIDTH-1:0] freecoeffs[0:NUMCOEFS-1]
);

  // Filter impulse response coefficients array
  logic [COEFSWIDTH-1:0] coeffs[0:STAGES];

  // Registers to store the signal taps
  logic [WIDTH-1:0] memory_points[0:STAGES];
  // Auxiliary signals to make the sum parametrizable
  logic [WIDTH-1:0] partial_sums[0:STAGES];

  // freecoeffs to coeffs mapping
  genvar k;
  generate
    for (k = 0; k < TOTCOEFS; k = k + 1) begin : coeffs_mapping
      // Remove $pow function to avoid errors in Synopsys DC
      // assign coeffs[k] = freecoeffs[int'($floor($sqrt($pow((TOTCOEFS-1)/2.0-k, 2))))];
      assign coeffs[k] = freecoeffs[int'($floor($sqrt(((TOTCOEFS-1)/2.0-k)*((TOTCOEFS-1)/2.0-k))))];
    end
  endgenerate


  // `memory_points[0]` is not a register
  assign memory_points[0] = data_i;
  assign partial_sums[0] = coeffs[0] * memory_points[0];
  assign data_o = partial_sums[STAGES];

  // Memory points transition logic & FFs
  genvar i;
  generate
    for (i = 0; i < STAGES; i = i + 1) begin : memory_points_ff

      always_ff @(posedge clk_i or negedge rstn_i) begin
        if (~rstn_i) begin
          memory_points[i+1] <= 'h0;
        end else begin
          if (clr_i) begin
            memory_points[i+1] <= 'h0;
          end else if (en_i) begin
            memory_points[i+1] <= memory_points[i];
          end
        end
      end
    end

  endgenerate

  // Parametized sum (for a filter impulse response with variable length)
  genvar j;
  generate
    for (j = 0; j < STAGES; j = j + 1) begin : summation
      logic [WIDTH-1:0] operand_a;
      logic [WIDTH-1:0] operand_b;
      logic [2*WIDTH:0] product;

      assign operand_a = coeffs[j+1];
      assign operand_b = memory_points[j+1];
      assign product = signed'(operand_a) * signed'(operand_b);

      assign partial_sums[j+1] = signed'(partial_sums[j]) + signed'(product[2*WIDTH-1-2:WIDTH-2]);
    end
  endgenerate

endmodule


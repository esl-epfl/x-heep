// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Pierre Guillod <pierre.guillod@epfl.ch>, EPFL, STI-SEL
// Date: 14.12.2022
// Description: Integrators instances of the CIC filter

module cic_integrators #(
    // Number of integrators
    parameter STAGES = 4,
    // Width of the datapath
    parameter WIDTH  = 18
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

  // Auxiliary array to pass data from one instance to the next one
  logic [WIDTH-1:0] integrator_data[0:STAGES];
  // First element is the input
  assign integrator_data[0] = data_i;
  // Last element is the output
  assign data_o = integrator_data[STAGES];

  // Stages instantiation
  genvar i;
  generate
    for (i = 0; i < STAGES; i = i + 1) begin : cic_stages

      cic_integrator #(WIDTH) cic_integrator_inst (
          .clk_i (clk_i),
          .rstn_i(rstn_i),
          .clr_i (clr_i),
          .en_i  (en_i),
          .data_i(integrator_data[i]),
          .data_o(integrator_data[i+1])
      );

    end
  endgenerate

endmodule


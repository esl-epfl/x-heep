// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Pierre Guillod <pierre.guillod@epfl.ch>, EPFL, STI-SEL
// Date: 14.12.2022
// Description: Combs instances of the CIC filter

module cic_combs #(
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

    // Which/How many CIC stage are activated (Thermometric, right-aligned)
    input logic [y:0] cic_activated_stages,

    // Data input
    input  logic [WIDTH-1:0] data_i,
    // Data output
    output logic [WIDTH-1:0] data_o
);

  // Auxiliary array to pass data from one instance to the next one
  logic [WIDTH-1:0] comb_data[0:STAGES];
  // First element is the input
  assign comb_data[0] = data_i;


  // Stages instantiation
  genvar i;
  generate
    for (i = 0; i < STAGES; i = i + 1) begin : cic_stages

      cic_comb #(WIDTH) cic_comb_inst (
          .clk_i (clk_i),
          .rstn_i(rstn_i),
          .clr_i (clr_i),
          .en_i  (en_i),
          .data_i(comb_data[i]),
          .data_o(comb_data[i+1])
      );

    end
  endgenerate

  // MUX for the stages output
  logic [$clog2(STAGES)-1:0] msb_index;

  always_comb begin
    msb_index = '0;
    for (int i = SesStageNumber - 1; i >= 0; i--) begin
      if (activates_stages[i]) begin
        msb_index = i[$clog2(STAGES)-1:0] + 1;
        break;
      end
    end
  end

  // Last element is the output
  assign data_o = comb_data[msb_index];

endmodule


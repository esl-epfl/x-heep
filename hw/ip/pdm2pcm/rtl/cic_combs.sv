// Copyright 2025 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Authors: Pierre Guillod <pierre.guillod@epfl.ch>, EPFL, STI-SEL
//          Jérémie Moullet<jeremie.moullet@eofl.ch>,EPFL, STI-SEL
//
// Date: 06.2025
//
// Description: Cascaded comb stages of a CIC decimation filter.
//              Each stage computes a delayed difference.
//
// Parameters:
//   - MAX_STAGE_CIC  : Total number of comb stages.
//   - WIDTH          : Bit-width of the datapath.
//   - DELAYCOMBWIDTH : Bit-width of the programmable delay index.
//
// Ports:
//   - clk_i, rstn_i : Clock and active-low reset.
//   - en_i, clr_i   : Enable and synchronous clear.
//   - par_cic_activated_stages : Right-aligned bitmask enabling selected stages.
//   - par_delay_combs : Common delay D used for all comb stages.
//   - data_i         : Input sample.
//   - data_o         : Output from the last active stage.
//
// Notes:
//   - Internally instantiates STAGES `cic_comb` modules.
//   - A MUX selects the output of the last active stage.
//   - If no stages are active, output defaults to input.

module cic_combs #(
    // Number of integrators
    parameter integer MAX_STAGE_CIC = 4,
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

    // Which/How many CIC stage are activated (Thermometric, right-aligned)
    input logic [ MAX_STAGE_CIC-1:0] par_cic_activated_stages,
    // Value of delay parameter
    input logic [DELAYCOMBWIDTH-1:0] par_delay_combs,

    // Data input
    input  logic [WIDTH-1:0] data_i,
    // Data output
    output logic [WIDTH-1:0] data_o
);

  // Auxiliary array to pass data from one instance to the next one
  logic [WIDTH-1:0] comb_data[0:MAX_STAGE_CIC];
  // First element is the input
  assign comb_data[0] = data_i;


  // Stages instantiation
  genvar i;
  generate
    for (i = 0; i < MAX_STAGE_CIC; i = i + 1) begin : cic_stages
      cic_comb #(WIDTH, DELAYCOMBWIDTH) cic_comb_inst (
          .clk_i(clk_i),
          .rstn_i(rstn_i),
          .clr_i(clr_i),
          .en_i(par_cic_activated_stages[i] & en_i),
          .par_delay(par_delay_combs),
          .data_i(comb_data[i]),
          .data_o(comb_data[i+1])
      );
    end
  endgenerate

  // MUX for the stages output
  logic [$clog2(MAX_STAGE_CIC)-1:0] msb_index;

  always_comb begin
    msb_index = '0;
    for (int k = MAX_STAGE_CIC - 1; k >= 0; k--) begin
      if (par_cic_activated_stages[k]) begin
        msb_index = k[$clog2(MAX_STAGE_CIC)-1:0] + 1;
        break;
      end
    end
  end

  // Last element is the output
  assign data_o = comb_data[msb_index];

endmodule


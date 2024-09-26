// Copyright 2023 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Davide Schiavone <davide.schiavone@epfl.ch>

// Dummy Empy Module just to let Verilator Compile

module configurable_delay #(
  parameter int unsigned NUM_STEPS, // The desired number of delay taps. Must be
  // a power of 2. Don't use very large values
  // here, otherwise strategy to just let STA
  // (with the right SDC) do the job for us
  // will not work.
  localparam DELAY_SEL_WIDTH = $clog2(NUM_STEPS)
) (
  input  logic       clk_i,
  /* verilator lint_off UNUSED */
  input  logic       enable_i,
  /* verilator lint_off UNUSED */
  input  logic [DELAY_SEL_WIDTH-1:0] delay_i,
  output logic clk_o
);
  assign clk_o = clk_i;


endmodule

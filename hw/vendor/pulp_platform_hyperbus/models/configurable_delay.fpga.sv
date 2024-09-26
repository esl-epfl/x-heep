// Copyright 2023 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Thomas Benz <tbenz@iis.ee.ethz.ch>
// Paul Scheffler <paulsc@iis.ee.ethz.ch>
//
// Based on work of:
// Fabian Schuiki <fschuiki@iis.ee.ethz.ch>
// Florian Zaruba <zarubaf@iis.ee.ethz.ch>

`timescale 1ps/1ps

(* no_ungroup *)
(* no_boundary_optimization *)
module configurable_delay #(
  parameter int unsigned NUM_STEPS, // The desired number of delay taps. Must be
                                   // a power of 2. Don't use very large values
                                   // here, otherwise strategy to just let STA
                                   // (with the right SDC) do the job for us
                                   // will not work.
  localparam DELAY_SEL_WIDTH = $clog2(NUM_STEPS)
) (
  input logic                       clk_i,
  input logic                       enable_i,
  input logic [DELAY_SEL_WIDTH-1:0] delay_i,
  output logic                      clk_o
);

  IBUF #
    (
     .IBUF_LOW_PWR ("FALSE")
     ) u_ibufg_sys_clk_o
      (
       .I  (clk_i),
       .O  (clk_o)
       );

endmodule

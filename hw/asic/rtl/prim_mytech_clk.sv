// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module mytech_clk_gating (
   input  logic clk_i,
   input  logic en_i,
   input  logic test_en_i,
   output logic clk_o
);


  /*
    add here your standard cell
  */


endmodule

module mytech_clk_inverter (
  input  logic clk_i,
  output logic clk_o
);

  /*
    add here your standard cell
  */

endmodule


module mytech_clk_mux2 (
  input  logic clk0_i,
  input  logic clk1_i,
  input  logic clk_sel_i,
  output logic clk_o
);

  /*
    add here your standard cell
  */

endmodule

module cluster_clock_inverter(
  input  logic clk_i,
  output logic clk_o
);

  mytech_clk_inverter clk_inv_i (
    .*
  );

endmodule

module pulp_clock_mux2 (
  input  logic clk0_i,
  input  logic clk1_i,
  input  logic clk_sel_i,
  output logic clk_o
);

  mytech_clk_mux2 clk_mux2_i (
    .*
  );

endmodule

module cv32e40p_clock_gate (
   input  logic clk_i,
   input  logic en_i,
   input  logic scan_cg_en_i,
   output logic clk_o
);

  mytech_clk_gating clk_gate_i (
    .clk_i,
    .en_i,
    .test_en_i(scan_cg_en_i),
    .clk_o
  );

endmodule
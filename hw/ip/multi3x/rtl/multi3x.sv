// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Ziyue Feng
// Date 15.08.2024

module multi3x #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic
) (
    input logic clk_i,
    input logic rst_ni,

    // Register interface
    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o
);

  import multi3x_reg_pkg::*;

  // Interface signals
  multi3x_reg2hw_t reg2hw;
  multi3x_hw2reg_t hw2reg;

  assign hw2reg.dataout.d  = reg2hw.datain.q * 3;
  assign hw2reg.dataout.de = 1;

  multi3x_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) multi3x_reg_top_i (
      .clk_i,
      .rst_ni,
      .reg_req_i,
      .reg_rsp_o,
      .reg2hw,
      .hw2reg,
      .devmode_i(1'b1)
  );

endmodule

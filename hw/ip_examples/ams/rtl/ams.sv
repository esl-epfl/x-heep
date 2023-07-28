// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

module ams #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic
) (
    input logic clk_i,
    input logic rst_ni,
    input reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o
);

  import ams_reg_pkg::*;

  ams_reg2hw_t reg2hw;
  ams_hw2reg_t hw2reg;

  assign hw2reg.get.de = 1;

  ams_adc_1b ams_adc_1b_i (
      .sel(reg2hw.sel.q),
      .out(hw2reg.get.d)
  );

  ams_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) ams_reg_top_i (
      .clk_i,
      .rst_ni,
      .reg_req_i,
      .reg_rsp_o,
      .reg2hw,
      .hw2reg,
      .devmode_i(1'b1)
  );

endmodule : ams


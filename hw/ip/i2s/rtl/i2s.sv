// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Tim Frey <tim.frey@epfl.ch>, EPFL, STI-SEL
// Date: 07.02.2023
// Description: I2s peripheral

module i2s #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic
) (
    input logic clk_i,
    input logic rst_ni,

    // Register interface
    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o

    // I2s interface
    // input  logic i2s_i,
    // output logic i2s_clk_o
);

  import i2s_reg_pkg::*;


  // Interface signals
  i2s_reg2hw_t reg2hw;
  i2s_hw2reg_t hw2reg;

  integer count;


  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (~rst_ni) begin
      count <= 0;
    end else begin
      if (reg2hw.inputdata.qe) begin
        hw2reg.outputdata.d <= reg2hw.inputdata.q;
        hw2reg.outputdata.de <= 1'b1;
        count <= count + 1;
      end else begin
        hw2reg.outputdata.de <= 1'b0;
      end
    end
  end

  i2s_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) i2s_reg_top_i (
      .clk_i,
      .rst_ni,
      .reg_req_i,
      .reg_rsp_o,
      .reg2hw,
      .hw2reg,
      .devmode_i(1'b1)
  );

endmodule : i2s

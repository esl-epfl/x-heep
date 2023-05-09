// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

`include "common_cells/assertions.svh"

module fast_intr_ctrl #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic
) (
    input logic clk_i,
    input logic rst_ni,

    // Bus Interface
    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    input  logic [14:0] fast_intr_i,
    output logic [14:0] fast_intr_o
);

  import fast_intr_ctrl_reg_pkg::*;

  fast_intr_ctrl_reg2hw_t reg2hw;
  fast_intr_ctrl_hw2reg_t hw2reg;

  logic [14:0] fast_intr_pending_de;
  logic [14:0] fast_intr_clear_de;

  fast_intr_ctrl_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) fast_intr_ctrl_reg_top_i (
      .clk_i,
      .rst_ni,
      .reg_req_i,
      .reg_rsp_o,
      .reg2hw,
      .hw2reg,
      .devmode_i(1'b1)
  );

  for (genvar i = 0; i < 15; i++) begin : gen_fast_interrupt

    always_comb begin
      if (reg2hw.fast_intr_clear.q[i]) begin
        fast_intr_pending_de[i] = 1'b1;
        hw2reg.fast_intr_pending.d[i] = 1'b0;
        fast_intr_clear_de[i] = 1'b1;
        hw2reg.fast_intr_clear.d[i] = 1'b0;
      end else begin
        if (fast_intr_i[i]) begin
          fast_intr_pending_de[i] = 1'b1;
          hw2reg.fast_intr_pending.d[i] = reg2hw.fast_intr_enable.q[i];
          fast_intr_clear_de[i] = 1'b0;
          hw2reg.fast_intr_clear.d[i] = 1'b0;
        end else begin
          fast_intr_pending_de[i] = 1'b0;
          hw2reg.fast_intr_pending.d[i] = 1'b0;
          fast_intr_clear_de[i] = 1'b0;
          hw2reg.fast_intr_clear.d[i] = 1'b0;
        end
      end
    end

  end

  assign fast_intr_o = reg2hw.fast_intr_pending.q;
  assign hw2reg.fast_intr_pending.de = |fast_intr_pending_de;
  assign hw2reg.fast_intr_clear.de = |fast_intr_clear_de;

endmodule : fast_intr_ctrl

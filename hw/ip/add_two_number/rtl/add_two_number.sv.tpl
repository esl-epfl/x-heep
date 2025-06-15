// Copyright 2025 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Francesco Poluzzi

module add_two_number #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic
) (
    input logic clk_i,
    input logic rst_ni,
    input reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o
);

  import add_two_number_reg_pkg::*;

  add_two_number_reg2hw_t reg2hw;
  add_two_number_hw2reg_t hw2reg;

  always_ff @(posedge clk_i or negedge rst_ni) begin

    if (!rst_ni) begin
      hw2reg.add_two_number_result.d  <= '0;
      hw2reg.add_two_number_result.de <= 1'b0;
      hw2reg.add_two_number_ctrl.d    <= 1'b0;
      hw2reg.add_two_number_ctrl.de   <= 1'b0;
    end else begin

      hw2reg.add_two_number_result.de <= 1'b0;
      hw2reg.add_two_number_ctrl.de   <= 1'b0;

      if (reg2hw.add_two_number_ctrl.q == 1'b1) begin
        hw2reg.add_two_number_result.d  <= reg2hw.add_two_number_operand0.q
                                        + reg2hw.add_two_number_operand1.q;
        hw2reg.add_two_number_result.de <= 1'b1;
        hw2reg.add_two_number_ctrl.d <= 1'b0;
        hw2reg.add_two_number_ctrl.de <= 1'b1;
      end
    end
  end

  add_two_number_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t),
      .AW       (4)
  ) u_regs (
      .clk_i,
      .rst_ni,
      .reg_req_i,
      .reg_rsp_o,
      .reg2hw,
      .hw2reg,
      .devmode_i(1'b1)
  );

endmodule

/*
 * Copyright 2024 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *
 * Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *                         <tommaso.terzano@gmail.com>
 *  
 * Info: Generic pipe register
 */

module pipe_reg #(
    parameter WIDTH = 8
) (
    input logic clk_i,
    input logic rst_ni,
    input logic [WIDTH-1:0] data_in,
    output logic [WIDTH-1:0] data_out
);

  logic [WIDTH-1:0] reg_data;

  always_ff @(posedge clk_i, negedge rst_ni) begin
    if (!rst_ni) begin
      reg_data <= '0;
    end else begin
      reg_data <= data_in;
    end
  end

  assign data_out = reg_data;
endmodule

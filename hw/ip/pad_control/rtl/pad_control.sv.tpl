// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module pad_control #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic,
    parameter NUM_PAD = 1
) (
    input logic clk_i,
    input logic rst_ni,

    // Bus Interface
    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    output logic [NUM_PAD-1:0][7:0] pad_attributes_o,
    output logic [NUM_PAD-1:0][3:0] pad_muxes_o

);

  import core_v_mini_mcu_pkg::*;

  import pad_control_reg_pkg::*;

  pad_control_reg2hw_t reg2hw;

% for pad in total_pad_list:
  assign pad_attributes_o[${pad.localparam}] = reg2hw.pad_attribute_${pad.name}.q;
% endfor


% for pad in pad_muxed_list:
  assign pad_muxes_o[${pad.localparam}] = reg2hw.pad_mux_${pad.name}.q;
% endfor

endmodule : pad_control

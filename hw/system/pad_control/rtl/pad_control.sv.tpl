// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module pad_control #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic,
% if not (total_pad_muxed > 0 or pads_attributes != None):
    /* verilator lint_off UNUSED */
% endif
    parameter NUM_PAD = 1
) (

% if not (total_pad_muxed > 0 or pads_attributes != None):
    /* verilator lint_off UNUSED */
% endif
    input logic clk_i,
% if not (total_pad_muxed > 0 or pads_attributes != None):
    /* verilator lint_off UNUSED */
% endif
    input logic rst_ni,

    // Bus Interface
% if not (total_pad_muxed > 0 or pads_attributes != None):
    /* verilator lint_off UNUSED */
% endif
    input  reg_req_t reg_req_i,
% if not (total_pad_muxed > 0 or pads_attributes != None):
    /* verilator lint_off UNDRIVEN */
% endif
    output reg_rsp_t reg_rsp_o
% if total_pad_muxed > 0 or pads_attributes != None:
      ,
% endif
% if pads_attributes != None:
    output logic [NUM_PAD-1:0][${pads_attributes['bits']}] pad_attributes_o
% if total_pad_muxed > 0:
      ,
% endif
% endif
% if total_pad_muxed > 0:
    output logic [NUM_PAD-1:0][${max_total_pad_mux_bitlengh-1}:0] pad_muxes_o
% endif
);

% if total_pad_muxed > 0 or pads_attributes != None:

  import core_v_mini_mcu_pkg::*;

  import pad_control_reg_pkg::*;

  pad_control_reg2hw_t reg2hw;

  pad_control_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) pad_control_reg_top_i (
      .clk_i,
      .rst_ni,
      .reg_req_i,
      .reg_rsp_o,
      .reg2hw,
      .devmode_i(1'b1)
  );
% endif

% if pads_attributes != None:
% for pad in total_pad_list:
  assign pad_attributes_o[${pad.localparam}] = reg2hw.pad_attribute_${pad.name.lower()}.q;
% endfor
% endif


% for pad in pad_muxed_list:
  assign pad_muxes_o[${pad.localparam}] = $unsigned(reg2hw.pad_mux_${pad.name.lower()}.q);
% endfor

endmodule : pad_control

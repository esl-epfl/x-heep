// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module pad_control #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic,
% if not xheep.get_pad_manager().get_mk_ctrl():
    /* verilator lint_off UNUSED */
% endif
    parameter NUM_PAD = 1
) (

% if not xheep.get_pad_manager().get_mk_ctrl():
    /* verilator lint_off UNUSED */
% endif
    input logic clk_i,
% if not xheep.get_pad_manager().get_mk_ctrl():
    /* verilator lint_off UNUSED */
% endif
    input logic rst_ni,

    // Bus Interface
% if not xheep.get_pad_manager().get_mk_ctrl():
    /* verilator lint_off UNUSED */
% endif
    input  reg_req_t reg_req_i,
% if not xheep.get_pad_manager().get_mk_ctrl():
    /* verilator lint_off UNDRIVEN */
% endif
    output reg_rsp_t reg_rsp_o
% if xheep.get_pad_manager().get_mk_ctrl():
      ,
% endif
% if xheep.get_pad_manager().get_attr_bits() != 0:
    output logic [NUM_PAD-1:0][${xheep.get_pad_manager().get_attr_bits()}-1:0] pad_attributes_o
% if xheep.get_pad_manager().get_muxed_pad_num() > 0:
      ,
% endif
% endif
% if xheep.get_pad_manager().get_muxed_pad_num() > 0:
    output logic [NUM_PAD-1:0][${xheep.get_pad_manager().get_max_mux_bitlengh()-1}:0] pad_muxes_o
% endif
);

% if xheep.get_pad_manager().get_mk_ctrl():

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

% if xheep.get_pad_manager().get_attr_bits != 0:
% for pad in xheep.get_pad_manager().iterate_pad_index():
  assign pad_attributes_o[${pad}] = reg2hw.pad_attribute_${pad.lower()}.q;
% endfor
% endif


% for pad in xheep.get_pad_manager().iterate_muxed_pad_index():
  assign pad_muxes_o[${pad}] = $unsigned(reg2hw.pad_mux_${pad.lower()}.q);
% endfor

endmodule : pad_control

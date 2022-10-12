// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module x_heep_system
  import obi_pkg::*;
  import reg_pkg::*;
#(
    parameter PULP_XPULP = 0,
    parameter FPU = 0,
    parameter PULP_ZFINX = 0,
    parameter EXT_XBAR_NMASTER = 0
) (

% for pad in pad_list:
${pad.x_heep_system_interface}
% endfor

    input  obi_req_t  [EXT_XBAR_NMASTER-1:0] ext_xbar_master_req_i,
    output obi_resp_t [EXT_XBAR_NMASTER-1:0] ext_xbar_master_resp_o,

    output obi_req_t  ext_xbar_slave_req_o,
    input  obi_resp_t ext_xbar_slave_resp_i,

    output reg_req_t ext_peripheral_slave_req_o,
    input  reg_rsp_t ext_peripheral_slave_resp_i,

    input logic [core_v_mini_mcu_pkg::NEXT_INT-1:0] intr_vector_ext_i,

    output logic [31:0] exit_value_o

);

  import core_v_mini_mcu_pkg::*;

  // PAD controller
  reg_req_t pad_req;
  reg_rsp_t pad_resp;
  logic [core_v_mini_mcu_pkg::NUM_PAD-1:0][15:0] pad_attributes;

  //input, output pins from core_v_mini_mcu
% for pad in pad_list:
${pad.internal_signals}
% endfor

  core_v_mini_mcu #(
    .PULP_XPULP(PULP_XPULP),
    .FPU(FPU),
    .PULP_ZFINX(PULP_ZFINX),
    .EXT_XBAR_NMASTER(EXT_XBAR_NMASTER)
  ) core_v_mini_mcu_i (

% for pad in pad_list:
${pad.core_v_mini_mcu_bonding}
% endfor

    //External PADs
    .pad_req_o(pad_reg),
    .pad_resp_i(pad_resp),

    .exit_value_o,

    .ext_xbar_master_req_i,
    .ext_xbar_master_resp_o,

    .ext_xbar_slave_req_o,
    .ext_xbar_slave_resp_i,

    .ext_peripheral_slave_req_o,
    .ext_peripheral_slave_resp_i,

    .intr_vector_ext_i

  );

  pad_ring pad_ring_i (
% for pad in pad_list:
${pad.pad_ring_bonding_bonding}
% endfor
    .pad_attributes_i(pad_attributes)
  );



  pad_attribute #(
      .reg_req_t(reg_pkg::reg_req_t),
      .reg_rsp_t(reg_pkg::reg_rsp_t),
      .NUM_PAD  (core_v_mini_mcu_pkg::NUM_PAD)
  ) pad_attribute_i (
      .clk_i,
      .rst_ni,
      .reg_req_i(pad_reg),
      .reg_rsp_o(pad_resp),
      .pad_attributes_o(pad_attributes)
  );

endmodule  // x_heep_system

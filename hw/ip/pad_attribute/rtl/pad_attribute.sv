// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module pad_attribute #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic,
    parameter NUM_PAD = 1
) (
    input logic clk_i,
    input logic rst_ni,

    // Bus Interface
    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    output logic [NUM_PAD-1:0][15:0] pad_attributes_o

);

  import core_v_mini_mcu_pkg::*;

  logic [NUM_PAD_PORT_SEL_WIDTH-1:0] pad_select;

  reg_pkg::reg_req_t [NUM_PAD-1:0] pad_attribute_reg_req;
  reg_pkg::reg_rsp_t [NUM_PAD-1:0] pad_attribute_reg_rsp;
  reg_pkg::reg_req_t [NUM_PAD-1:0] pad_attribute_reg_req_addr_fix;


  pad_attribute_reg_pkg::pad_attribute_reg2hw_t [NUM_PAD-1:0] reg2hw;

  addr_decode #(
      .NoIndices(NUM_PAD),
      .NoRules(NUM_PAD),
      .addr_t(logic [31:0]),
      .rule_t(addr_map_rule_pkg::addr_map_rule_t)
  ) i_addr_decode_soc_regbus_periph_xbar (
      .addr_i(reg_req_i.addr),
      .addr_map_i(core_v_mini_mcu_pkg::PAD_ADDR_RULES),
      .idx_o(pad_select),
      .dec_valid_o(),
      .dec_error_o(),
      .en_default_idx_i(1'b0),
      .default_idx_i('0)
  );

  reg_demux #(
      .NoPorts(NUM_PAD),
      .req_t  (reg_req_t),
      .rsp_t  (reg_rsp_t)
  ) reg_demux_i (
      .clk_i,
      .rst_ni,
      .in_select_i(pad_select),
      .in_req_i(reg_req_i),
      .in_rsp_o(reg_rsp_o),
      .out_req_o(pad_attribute_reg_req),
      .out_rsp_i(pad_attribute_reg_rsp)
  );

  genvar i;
  generate

    for (i = 0; i < NUM_PAD; i++) begin
      always_comb begin
        pad_attribute_reg_req_addr_fix[i] = pad_attribute_reg_req[i];
        pad_attribute_reg_req_addr_fix[i].addr = '0;
      end
      pad_attribute_reg_top #(
          .reg_req_t(reg_req_t),
          .reg_rsp_t(reg_rsp_t)
      ) pad_attribute_reg_top_i (
          .clk_i,
          .rst_ni,
          .reg_req_i(pad_attribute_reg_req_addr_fix[i]),
          .reg_rsp_o(pad_attribute_reg_rsp[i]),
          // To HW
          .reg2hw(reg2hw[i]),
          .devmode_i(1'b1)
      );
      assign pad_attributes_o[i] = reg2hw[i].attribute.q;
    end
  endgenerate


endmodule : pad_attribute

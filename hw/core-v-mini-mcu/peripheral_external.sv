// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module peripheral_external
  import obi_pkg::*;
(
    input logic clk_i,
    input logic rst_ni,

    input  obi_req_t  slave_req_i,
    output obi_resp_t slave_resp_o,

    output obi_req_t  master_req_o,
    input  obi_resp_t master_resp_i
);

  import testharness_pkg::*;
  import reg_pkg::*;
  import tlul_pkg::*;

  reg_pkg::reg_req_t peripheral_req;
  reg_pkg::reg_rsp_t peripheral_rsp;

  reg_pkg::reg_req_t [testharness_pkg::EXT_NPERIPHERALS-1:0] peripheral_slv_req;
  reg_pkg::reg_rsp_t [testharness_pkg::EXT_NPERIPHERALS-1:0] peripheral_slv_rsp;

  tlul_pkg::tl_h2d_t uart_tl_h2d;
  tlul_pkg::tl_d2h_t uart_tl_d2h;

  //Address Decoder
  logic [EXT_PERIPHERALS_PORT_SEL_WIDTH-1:0] peripheral_select;

  periph_to_reg #(
      .req_t(reg_pkg::reg_req_t),
      .rsp_t(reg_pkg::reg_rsp_t),
      .IW(1)
  ) periph_to_reg_i (
      .clk_i,
      .rst_ni,
      .req_i(slave_req_i.req),
      .add_i(slave_req_i.addr),
      .wen_i(~slave_req_i.we),
      .wdata_i(slave_req_i.wdata),
      .be_i(slave_req_i.be),
      .id_i('0),
      .gnt_o(slave_resp_o.gnt),
      .r_rdata_o(slave_resp_o.rdata),
      .r_opc_o(),
      .r_id_o(),
      .r_valid_o(slave_resp_o.rvalid),
      .reg_req_o(peripheral_req),
      .reg_rsp_i(peripheral_rsp)
  );

  addr_decode #(
      .NoIndices(testharness_pkg::EXT_NPERIPHERALS),
      .NoRules(testharness_pkg::EXT_NPERIPHERALS),
      .addr_t(logic [31:0]),
      .rule_t(addr_map_rule_pkg::addr_map_rule_t)
  ) i_addr_decode_soc_regbus_periph_xbar (
      .addr_i(peripheral_req.addr),
      .addr_map_i(testharness_pkg::EXT_XBAR_ADDR_RULES),
      .idx_o(peripheral_select),
      .dec_valid_o(),
      .dec_error_o(),
      .en_default_idx_i(1'b0),
      .default_idx_i('0)
  );

  reg_demux #(
      .NoPorts(testharness_pkg::EXT_NPERIPHERALS),
      .req_t  (reg_pkg::reg_req_t),
      .rsp_t  (reg_pkg::reg_rsp_t)
  ) reg_demux_i (
      .clk_i,
      .rst_ni,
      .in_select_i(peripheral_select),
      .in_req_i(peripheral_req),
      .in_rsp_o(peripheral_rsp),
      .out_req_o(peripheral_slv_req),
      .out_rsp_i(peripheral_slv_rsp)
  );

  // Example peripheral with master port to access memory
  memcopy_periph #(
      .reg_req_t (reg_pkg::reg_req_t),
      .reg_rsp_t (reg_pkg::reg_rsp_t),
      .obi_req_t (obi_pkg::obi_req_t),
      .obi_resp_t(obi_pkg::obi_resp_t)
  ) memcopy_periph_i (
      .clk_i,
      .rst_ni,
      .reg_req_i(peripheral_slv_req[testharness_pkg::MEMCOPY_CTRL_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[testharness_pkg::MEMCOPY_CTRL_IDX]),
      .master_req_o(master_req_o),
      .master_resp_i(master_resp_i)
  );

  // Add you peripheral here

endmodule : peripheral_external

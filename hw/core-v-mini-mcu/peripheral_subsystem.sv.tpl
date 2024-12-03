// Copyright(// Copyright) 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

<%
  domain = next(xheep.iter_peripheral_domains_normal())
%>

module peripheral_subsystem
  import obi_pkg::*;
  import reg_pkg::*;
#(
    //do not touch these parameters
    parameter NEXT_INT_RND         = core_v_mini_mcu_pkg::NEXT_INT == 0 ? 1 : core_v_mini_mcu_pkg::NEXT_INT
) (
    input logic clk_i,
    input logic rst_ni,

    // Clock-gating signal
    input logic clk_gate_en_ni,

    input  obi_req_t  slave_req_i,
    output obi_resp_t slave_resp_o,

    ${xheep.get_rh().get_node_ports(domain.get_node()).strip(",")}
);

  import core_v_mini_mcu_pkg::*;
  import tlul_pkg::*;
  import rv_plic_reg_pkg::*;

  reg_pkg::reg_req_t peripheral_req;
  reg_pkg::reg_rsp_t peripheral_rsp;

  reg_pkg::reg_req_t [core_v_mini_mcu_pkg::PERIPHERAL_COUNT-1:0] peripheral_slv_req;
  reg_pkg::reg_rsp_t [core_v_mini_mcu_pkg::PERIPHERAL_COUNT-1:0] peripheral_slv_rsp;


% for periph in domain.iter_peripherals():
% for sig in periph.make_local_signals():
  ${sig.t} ${sig.name};
% endfor
% endfor

 ${xheep.get_rh().get_node_local_signals(domain.get_node())}


  //Address Decoder
  logic [PERIPHERAL_PORT_SEL_WIDTH-1:0] peripheral_select;

  obi_pkg::obi_req_t slave_fifo_req_sel;
  obi_pkg::obi_resp_t slave_fifo_resp_sel;

  // Clock-gating
  logic clk_cg;
  tc_clk_gating clk_gating_cell (
      .clk_i,
      .en_i(clk_gate_en_ni),
      .test_en_i(1'b0),
      .clk_o(clk_cg)
  );


`ifdef REMOVE_OBI_FIFO

  assign slave_fifo_req_sel = slave_req_i;
  assign slave_resp_o       = slave_fifo_resp_sel;

`else

  obi_pkg::obi_req_t slave_fifoin_req;
  obi_pkg::obi_resp_t slave_fifoin_resp;

  obi_pkg::obi_req_t slave_fifoout_req;
  obi_pkg::obi_resp_t slave_fifoout_resp;

  obi_fifo obi_fifo_i (
      .clk_i(clk_cg),
      .rst_ni,
      .producer_req_i (slave_fifoin_req),
      .producer_resp_o(slave_fifoin_resp),
      .consumer_req_o (slave_fifoout_req),
      .consumer_resp_i(slave_fifoout_resp)
  );

  assign slave_fifo_req_sel = slave_fifoout_req;
  assign slave_fifoout_resp = slave_fifo_resp_sel;
  assign slave_fifoin_req   = slave_req_i;
  assign slave_resp_o       = slave_fifoin_resp;

`endif

  periph_to_reg #(
      .req_t(reg_pkg::reg_req_t),
      .rsp_t(reg_pkg::reg_rsp_t),
      .IW(1)
  ) periph_to_reg_i (
      .clk_i(clk_cg),
      .rst_ni,
      .req_i(slave_fifo_req_sel.req),
      .add_i(slave_fifo_req_sel.addr),
      .wen_i(~slave_fifo_req_sel.we),
      .wdata_i(slave_fifo_req_sel.wdata),
      .be_i(slave_fifo_req_sel.be),
      .id_i('0),
      .gnt_o(slave_fifo_resp_sel.gnt),
      .r_rdata_o(slave_fifo_resp_sel.rdata),
      .r_opc_o(),
      .r_id_o(),
      .r_valid_o(slave_fifo_resp_sel.rvalid),
      .reg_req_o(peripheral_req),
      .reg_rsp_i(peripheral_rsp)
  );

  addr_decode #(
      .NoIndices(core_v_mini_mcu_pkg::PERIPHERAL_COUNT),
      .NoRules(core_v_mini_mcu_pkg::PERIPHERAL_COUNT),
      .addr_t(logic [31:0]),
      .rule_t(addr_map_rule_pkg::addr_map_rule_t)
  ) i_addr_decode_soc_regbus_periph_xbar (
      .addr_i(peripheral_req.addr),
      .addr_map_i(core_v_mini_mcu_pkg::PERIPHERAL_ADDR_RULES),
      .idx_o(peripheral_select),
      .dec_valid_o(),
      .dec_error_o(),
      .en_default_idx_i(1'b0),
      .default_idx_i('0)
  );

  reg_demux #(
      .NoPorts(core_v_mini_mcu_pkg::PERIPHERAL_COUNT),
      .req_t  (reg_pkg::reg_req_t),
      .rsp_t  (reg_pkg::reg_rsp_t)
  ) reg_demux_i (
      .clk_i(clk_cg),
      .rst_ni,
      .in_select_i(peripheral_select),
      .in_req_i(peripheral_req),
      .in_rsp_o(peripheral_rsp),
      .out_req_o(peripheral_slv_req),
      .out_rsp_i(peripheral_slv_rsp)
  );


% for periph in domain.iter_peripherals():
${periph.make_instantiation(xheep.get_rh())}
% endfor

endmodule : peripheral_subsystem

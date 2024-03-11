// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module x_heep_system
  import obi_pkg::*;
  import reg_pkg::*;
#(
    parameter COREV_PULP = 0,
    parameter FPU = 0,
    parameter ZFINX = 0,
    parameter EXT_XBAR_NMASTER = 0,
    parameter X_EXT = 0,  // eXtension interface in cv32e40x
    //do not touch these parameters
    parameter EXT_XBAR_NMASTER_RND = EXT_XBAR_NMASTER == 0 ? 1 : EXT_XBAR_NMASTER,
    parameter EXT_DOMAINS_RND = core_v_mini_mcu_pkg::EXTERNAL_DOMAINS == 0 ? 1 : core_v_mini_mcu_pkg::EXTERNAL_DOMAINS,
    parameter NEXT_INT_RND = core_v_mini_mcu_pkg::NEXT_INT == 0 ? 1 : core_v_mini_mcu_pkg::NEXT_INT

) (
    input logic [NEXT_INT_RND-1:0] intr_vector_ext_i,

    input  obi_req_t  [EXT_XBAR_NMASTER_RND-1:0] ext_xbar_master_req_i,
    output obi_resp_t [EXT_XBAR_NMASTER_RND-1:0] ext_xbar_master_resp_o,

    // External slave ports
    output obi_req_t  ext_core_instr_req_o,
    input  obi_resp_t ext_core_instr_resp_i,
    output obi_req_t  ext_core_data_req_o,
    input  obi_resp_t ext_core_data_resp_i,
    output obi_req_t  ext_debug_master_req_o,
    input  obi_resp_t ext_debug_master_resp_i,
    output obi_req_t  ext_dma_read_ch0_req_o,
    input  obi_resp_t ext_dma_read_ch0_resp_i,
    output obi_req_t  ext_dma_write_ch0_req_o,
    input  obi_resp_t ext_dma_write_ch0_resp_i,
    output obi_req_t  ext_dma_addr_ch0_req_o,
    input  obi_resp_t ext_dma_addr_ch0_resp_i,

    output reg_req_t ext_peripheral_slave_req_o,
    input  reg_rsp_t ext_peripheral_slave_resp_i,

    output logic [EXT_DOMAINS_RND-1:0] external_subsystem_powergate_switch_no,
    input  logic [EXT_DOMAINS_RND-1:0] external_subsystem_powergate_switch_ack_ni,
    output logic [EXT_DOMAINS_RND-1:0] external_subsystem_powergate_iso_no,
    output logic [EXT_DOMAINS_RND-1:0] external_subsystem_rst_no,
    output logic [EXT_DOMAINS_RND-1:0] external_ram_banks_set_retentive_no,
    output logic [EXT_DOMAINS_RND-1:0] external_subsystem_clkgate_en_no,

    output logic [31:0] exit_value_o,

    input logic ext_dma_slot_tx_i,
    input logic ext_dma_slot_rx_i,

    // eXtension interface
    if_xif.cpu_compressed xif_compressed_if,
    if_xif.cpu_issue      xif_issue_if,
    if_xif.cpu_commit     xif_commit_if,
    if_xif.cpu_mem        xif_mem_if,
    if_xif.cpu_mem_result xif_mem_result_if,
    if_xif.cpu_result     xif_result_if,

    ${xheep.get_pad_manager().make_root_io_ports(internal=True).strip(",")}
);

  import core_v_mini_mcu_pkg::*;

  ${xheep.get_rh().get_node_local_signals(xheep.get_rh().get_root_node())}

  // PM signals
  logic cpu_subsystem_powergate_switch_n;
  logic cpu_subsystem_powergate_switch_ack_n;
  logic peripheral_subsystem_powergate_switch_n;
  logic peripheral_subsystem_powergate_switch_ack_n;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_switch_n;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_switch_ack_n;

  // PAD controller
  reg_req_t pad_req;
  reg_rsp_t pad_resp;
% if xheep.get_pad_manager().get_attr_bits() != 0:
  logic [core_v_mini_mcu_pkg::NUM_PAD-1:0][${xheep.get_pad_manager().get_attr_bits()}-1:0] pad_attributes;
% endif
 % if xheep.get_pad_manager().get_muxed_pad_num() > 0:
  logic [core_v_mini_mcu_pkg::NUM_PAD-1:0][${xheep.get_pad_manager().get_max_mux_bitlengh()-1}:0] pad_muxes;
% endif

  logic rst_ngen;

  //input, output pins from core_v_mini_mcu

  core_v_mini_mcu #(
    .COREV_PULP(COREV_PULP),
    .FPU(FPU),
    .ZFINX(ZFINX),
    .EXT_XBAR_NMASTER(EXT_XBAR_NMASTER),
    .X_EXT(X_EXT)
  ) core_v_mini_mcu_i (
    .clk_i(${xheep.get_rh().use_source_as_sv("clk", xheep.get_rh().get_root_node())}),
    .rst_ni(rst_ngen),
    .intr_vector_ext_i,
    .xif_compressed_if,
    .xif_issue_if,
    .xif_commit_if,
    .xif_mem_if,
    .xif_mem_result_if,
    .xif_result_if,
    .pad_req_o(pad_req),
    .pad_resp_i(pad_resp),
    .ext_xbar_master_req_i,
    .ext_xbar_master_resp_o,
    .ext_core_instr_req_o,
    .ext_core_instr_resp_i,
    .ext_core_data_req_o,
    .ext_core_data_resp_i,
    .ext_debug_master_req_o,
    .ext_debug_master_resp_i,
    .ext_dma_read_ch0_req_o,
    .ext_dma_read_ch0_resp_i,
    .ext_dma_write_ch0_req_o,
    .ext_dma_write_ch0_resp_i,
    .ext_dma_addr_ch0_req_o,
    .ext_dma_addr_ch0_resp_i,
    .ext_peripheral_slave_req_o,
    .ext_peripheral_slave_resp_i,
    .cpu_subsystem_powergate_switch_no(cpu_subsystem_powergate_switch_n),
    .cpu_subsystem_powergate_switch_ack_ni(cpu_subsystem_powergate_switch_ack_n),
    .peripheral_subsystem_powergate_switch_no(peripheral_subsystem_powergate_switch_n),
    .peripheral_subsystem_powergate_switch_ack_ni(peripheral_subsystem_powergate_switch_ack_n),
    .memory_subsystem_banks_powergate_switch_no(memory_subsystem_banks_powergate_switch_n),
    .memory_subsystem_banks_powergate_switch_ack_ni(memory_subsystem_banks_powergate_switch_ack_n),
    .external_subsystem_powergate_switch_no,
    .external_subsystem_powergate_switch_ack_ni,
    .external_subsystem_powergate_iso_no,
    .external_subsystem_rst_no,
    .external_ram_banks_set_retentive_no,
    .external_subsystem_clkgate_en_no,
    .exit_value_o,
    ${xheep.get_rh().get_instantiation_signals(xheep.get_mcu_node()).strip(",")}
  );

  pad_ring pad_ring_i (
    ${xheep.get_rh().get_instantiation_signals(xheep.get_pad_manager().get_pad_ring_node())}
    ${xheep.get_pad_manager().make_root_io_ports_use()}
% if xheep.get_pad_manager().get_attr_bits() != 0:
    .pad_attributes_i(pad_attributes)
% else:
    .pad_attributes_i('0)
% endif
  );

  pad_control #(
      .reg_req_t(reg_pkg::reg_req_t),
      .reg_rsp_t(reg_pkg::reg_rsp_t),
      .NUM_PAD  (core_v_mini_mcu_pkg::NUM_PAD)
  ) pad_control_i (
      .clk_i(${xheep.get_rh().use_source_as_sv("clk", xheep.get_rh().get_root_node())}),
      .rst_ni(rst_ngen),
      .reg_req_i(pad_req),
      .reg_rsp_o(pad_resp)
% if xheep.get_pad_manager().get_mk_ctrl():
      ,
% endif
% if xheep.get_pad_manager().get_attr_bits() != 0:
      .pad_attributes_o(pad_attributes)
% if xheep.get_pad_manager().get_muxed_pad_num() > 0:
      ,
% endif
% endif
% if xheep.get_pad_manager().get_muxed_pad_num() > 0:
      .pad_muxes_o(pad_muxes)
% endif
  );

${xheep.get_pad_manager().make_muxers(xheep.get_rh())}

  rstgen rstgen_i (
    .clk_i(${xheep.get_rh().use_source_as_sv("clk", xheep.get_rh().get_root_node())}),
    .rst_ni(${xheep.get_rh().use_source_as_sv("rst_n", xheep.get_rh().get_root_node())}),
    .test_mode_i(1'b0),
    .rst_no(rst_ngen),
    .init_no()
  );

% for i in range(xheep.get_ext_intr() -1, -1, -1):
assign ${xheep.get_rh().use_source_as_sv(f"ext_intr_{i}", xheep.get_rh().get_root_node())} = intr_vector_ext_i[${i}];
% endfor

assign ${xheep.get_rh().use_source_as_sv("dma_ext_rx", xheep.get_rh().get_root_node())} = ext_dma_slot_rx_i;
assign ${xheep.get_rh().use_source_as_sv("dma_ext_tx", xheep.get_rh().get_root_node())} = ext_dma_slot_tx_i;

endmodule  // x_heep_system

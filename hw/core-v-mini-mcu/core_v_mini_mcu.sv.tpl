// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module core_v_mini_mcu
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

    input logic rst_ni,
    input logic clk_i,


    // eXtension interface
    if_xif.cpu_compressed xif_compressed_if,
    if_xif.cpu_issue      xif_issue_if,
    if_xif.cpu_commit     xif_commit_if,
    if_xif.cpu_mem        xif_mem_if,
    if_xif.cpu_mem_result xif_mem_result_if,
    if_xif.cpu_result     xif_result_if,

    output reg_req_t pad_req_o,
    input  reg_rsp_t pad_resp_i,

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

    input logic [NEXT_INT_RND-1:0] intr_vector_ext_i,

    output logic cpu_subsystem_powergate_switch_no,
    input  logic cpu_subsystem_powergate_switch_ack_ni,
    output logic peripheral_subsystem_powergate_switch_no,
    input  logic peripheral_subsystem_powergate_switch_ack_ni,
    output logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_switch_no,
    input  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_switch_ack_ni,
    output logic [EXT_DOMAINS_RND-1:0] external_subsystem_powergate_switch_no,
    input  logic [EXT_DOMAINS_RND-1:0] external_subsystem_powergate_switch_ack_ni,
    output logic [EXT_DOMAINS_RND-1:0] external_subsystem_powergate_iso_no,
    output logic [EXT_DOMAINS_RND-1:0] external_subsystem_rst_no,
    output logic [EXT_DOMAINS_RND-1:0] external_ram_banks_set_retentive_no,
    output logic [EXT_DOMAINS_RND-1:0] external_subsystem_clkgate_en_no,

    output logic [31:0] exit_value_o,

    ${xheep.get_rh().get_node_ports(xheep.get_mcu_node()).strip(",")}
);

  import core_v_mini_mcu_pkg::*;
  import cv32e40p_apu_core_pkg::*;

  localparam NUM_BYTES = core_v_mini_mcu_pkg::MEM_SIZE;
  localparam DM_HALTADDRESS = core_v_mini_mcu_pkg::DEBUG_START_ADDRESS + 32'h00000800; //debug rom code (section .text in linker) starts at 0x800

  localparam JTAG_IDCODE = 32'h10001c05;
  localparam BOOT_ADDR = core_v_mini_mcu_pkg::BOOTROM_START_ADDRESS;
  localparam NUM_MHPMCOUNTERS = 1;

  // Log top level parameter values
`ifndef SYNTHESIS
  initial begin
    $display("[X-HEEP]: NUM_BYTES = %dKB", NUM_BYTES / 1024);
  end
`endif

  // masters signals
  obi_req_t core_instr_req;
  obi_resp_t core_instr_resp;
  obi_req_t core_data_req;
  obi_resp_t core_data_resp;
  obi_req_t debug_master_req;
  obi_resp_t debug_master_resp;
  obi_req_t dma_read_ch0_req;
  obi_resp_t dma_read_ch0_resp;
  obi_req_t dma_write_ch0_req;
  obi_resp_t dma_write_ch0_resp;
  obi_req_t dma_addr_ch0_req;
  obi_resp_t dma_addr_ch0_resp;

  // ram signals
  obi_req_t [core_v_mini_mcu_pkg::NUM_BANKS-1:0] ram_slave_req;
  obi_resp_t [core_v_mini_mcu_pkg::NUM_BANKS-1:0] ram_slave_resp;

  // debug signals
  obi_req_t debug_slave_req;
  obi_resp_t debug_slave_resp;

  // peripherals signals
  obi_req_t ao_peripheral_slave_req;
  obi_resp_t ao_peripheral_slave_resp;
  obi_req_t peripheral_slave_req;
  obi_resp_t peripheral_slave_resp;

  // signals to debug unit
  logic debug_core_req;
  logic debug_reset_n;

  // core
  logic core_sleep;

  // irq signals
  logic irq_ack;
  logic [4:0] irq_id_out;
  logic [14:0] irq_fast;

  // Memory Map SPI Region
  obi_req_t flash_mem_slave_req;
  obi_resp_t flash_mem_slave_resp;

  // interrupt array
  logic [31:0] intr;
  logic [14:0] fast_intr;

  // Power manager
  logic cpu_subsystem_powergate_iso_n;
  logic cpu_subsystem_rst_n;
  logic peripheral_subsystem_powergate_iso_n;
  logic peripheral_subsystem_rst_n;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_iso_n;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_set_retentive_n;

   // Clock gating signals
   logic peripheral_subsystem_clkgate_en_n;
   logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0]memory_subsystem_clkgate_en_n;


  // Auto generated
  ${xheep.get_rh().get_node_local_signals(xheep.get_mcu_node())}

  assign intr = {
    1'b0,
    irq_fast,
    4'b0,
    ${xheep.get_rh().use_target_as_sv("irq_external", xheep.get_mcu_node())},
    3'b0,
    ${xheep.get_rh().use_target_as_sv("rv_timer_0_direct_irq", xheep.get_mcu_node())},
    3'b0,
    ${xheep.get_rh().use_target_as_sv("irq_software", xheep.get_mcu_node())},
    3'b0
  };

  assign fast_intr = ${xheep.get_rh().use_target_as_sv_array_multi("fast_irq_target", xheep.get_mcu_node(), 15)};


  cpu_subsystem #(
      .BOOT_ADDR(BOOT_ADDR),
      .COREV_PULP(COREV_PULP),
      .FPU(FPU),
      .ZFINX(ZFINX),
      .NUM_MHPMCOUNTERS(NUM_MHPMCOUNTERS),
      .DM_HALTADDRESS(DM_HALTADDRESS),
      .X_EXT(X_EXT)
  ) cpu_subsystem_i (
      // Clock and Reset
      .clk_i,
      .rst_ni(cpu_subsystem_rst_n && debug_reset_n),
      .core_instr_req_o(core_instr_req),
      .core_instr_resp_i(core_instr_resp),
      .core_data_req_o(core_data_req),
      .core_data_resp_i(core_data_resp),
      .xif_compressed_if,
      .xif_issue_if,
      .xif_commit_if,
      .xif_mem_if,
      .xif_mem_result_if,
      .xif_result_if,
      .irq_i(intr),
      .irq_ack_o(irq_ack),
      .irq_id_o(irq_id_out),
      .debug_req_i(debug_core_req),
      .core_sleep_o(core_sleep)
  );

  debug_subsystem #(
      .JTAG_IDCODE(JTAG_IDCODE)
  ) debug_subsystem_i (
      .clk_i,
      .rst_ni,
      .jtag_tck_i(${xheep.get_rh().use_source_as_sv("jtag_tck", xheep.get_mcu_node())}),
      .jtag_tms_i(${xheep.get_rh().use_source_as_sv("jtag_tms", xheep.get_mcu_node())}),
      .jtag_trst_ni(${xheep.get_rh().use_source_as_sv("jtag_trst_n", xheep.get_mcu_node())}),
      .jtag_tdi_i(${xheep.get_rh().use_source_as_sv("jtag_tdi", xheep.get_mcu_node())}),
      .jtag_tdo_o(${xheep.get_rh().use_source_as_sv("jtag_tdo", xheep.get_mcu_node())}),
      .debug_core_req_o(debug_core_req),
      .debug_ndmreset_no(debug_reset_n),
      .debug_slave_req_i(debug_slave_req),
      .debug_slave_resp_o(debug_slave_resp),
      .debug_master_req_o(debug_master_req),
      .debug_master_resp_i(debug_master_resp)
  );

  system_bus #(
      .NUM_BANKS(core_v_mini_mcu_pkg::NUM_BANKS),
      .EXT_XBAR_NMASTER(EXT_XBAR_NMASTER)
  ) system_bus_i (
      .clk_i,
      .rst_ni(rst_ni && debug_reset_n),
      .core_instr_req_i(core_instr_req),
      .core_instr_resp_o(core_instr_resp),
      .core_data_req_i(core_data_req),
      .core_data_resp_o(core_data_resp),
      .debug_master_req_i(debug_master_req),
      .debug_master_resp_o(debug_master_resp),
      .dma_read_ch0_req_i(dma_read_ch0_req),
      .dma_read_ch0_resp_o(dma_read_ch0_resp),
      .dma_write_ch0_req_i(dma_write_ch0_req),
      .dma_write_ch0_resp_o(dma_write_ch0_resp),
      .dma_addr_ch0_req_i(dma_addr_ch0_req),
      .dma_addr_ch0_resp_o(dma_addr_ch0_resp),
      .ext_xbar_master_req_i(ext_xbar_master_req_i),
      .ext_xbar_master_resp_o(ext_xbar_master_resp_o),
      .ram_req_o(ram_slave_req),
      .ram_resp_i(ram_slave_resp),
      .debug_slave_req_o(debug_slave_req),
      .debug_slave_resp_i(debug_slave_resp),
      .ao_peripheral_slave_req_o(ao_peripheral_slave_req),
      .ao_peripheral_slave_resp_i(ao_peripheral_slave_resp),
      .peripheral_slave_req_o(peripheral_slave_req),
      .peripheral_slave_resp_i(peripheral_slave_resp),
      .flash_mem_slave_req_o(flash_mem_slave_req),
      .flash_mem_slave_resp_i(flash_mem_slave_resp),
      .ext_core_instr_req_o(ext_core_instr_req_o),
      .ext_core_instr_resp_i(ext_core_instr_resp_i),
      .ext_core_data_req_o(ext_core_data_req_o),
      .ext_core_data_resp_i(ext_core_data_resp_i),
      .ext_debug_master_req_o(ext_debug_master_req_o),
      .ext_debug_master_resp_i(ext_debug_master_resp_i),
      .ext_dma_read_ch0_req_o(ext_dma_read_ch0_req_o),
      .ext_dma_read_ch0_resp_i(ext_dma_read_ch0_resp_i),
      .ext_dma_write_ch0_req_o(ext_dma_write_ch0_req_o),
      .ext_dma_write_ch0_resp_i(ext_dma_write_ch0_resp_i),
      .ext_dma_addr_ch0_req_o(ext_dma_addr_ch0_req_o),
      .ext_dma_addr_ch0_resp_i(ext_dma_addr_ch0_resp_i)
  );

  memory_subsystem #(
      .NUM_BANKS(core_v_mini_mcu_pkg::NUM_BANKS)
  ) memory_subsystem_i (
      .clk_i,
      .rst_ni(rst_ni && debug_reset_n),
      .clk_gate_en_ni(memory_subsystem_clkgate_en_n),
      .ram_req_i(ram_slave_req),
      .ram_resp_o(ram_slave_resp),
      .set_retentive_ni(memory_subsystem_banks_set_retentive_n)
  );

  ao_peripheral_subsystem ao_peripheral_subsystem_i (
      .clk_i,
      .rst_ni(rst_ni && debug_reset_n),
      .slave_req_i(ao_peripheral_slave_req),
      .slave_resp_o(ao_peripheral_slave_resp),
      .exit_value_o,
      .spimemio_req_i(flash_mem_slave_req),
      .spimemio_resp_o(flash_mem_slave_resp),
      .intr_i(intr),
      .intr_vector_ext_i,
      .core_sleep_i(core_sleep),
      .cpu_subsystem_powergate_switch_no,
      .cpu_subsystem_powergate_switch_ack_ni,
      .cpu_subsystem_powergate_iso_no(cpu_subsystem_powergate_iso_n),
      .cpu_subsystem_rst_no(cpu_subsystem_rst_n),
      .peripheral_subsystem_powergate_switch_no,
      .peripheral_subsystem_powergate_switch_ack_ni,
      .peripheral_subsystem_powergate_iso_no(peripheral_subsystem_powergate_iso_n),
      .peripheral_subsystem_rst_no(peripheral_subsystem_rst_n),
      .memory_subsystem_banks_powergate_switch_no,
      .memory_subsystem_banks_powergate_switch_ack_ni,
      .memory_subsystem_banks_powergate_iso_no(memory_subsystem_banks_powergate_iso_n),
      .memory_subsystem_banks_set_retentive_no(memory_subsystem_banks_set_retentive_n),
      .external_subsystem_powergate_switch_no,
      .external_subsystem_powergate_switch_ack_ni,
      .external_subsystem_powergate_iso_no,
      .external_subsystem_rst_no,
      .external_ram_banks_set_retentive_no,
      .peripheral_subsystem_clkgate_en_no(peripheral_subsystem_clkgate_en_n),
      .memory_subsystem_clkgate_en_no(memory_subsystem_clkgate_en_n),
      .external_subsystem_clkgate_en_no,
      .dma_read_ch0_req_o(dma_read_ch0_req),
      .dma_read_ch0_resp_i(dma_read_ch0_resp),
      .dma_write_ch0_req_o(dma_write_ch0_req),
      .dma_write_ch0_resp_i(dma_write_ch0_resp),
      .dma_addr_ch0_req_o(dma_addr_ch0_req),
      .dma_addr_ch0_resp_i(dma_addr_ch0_resp),
      .pad_req_o,
      .pad_resp_i,
      .fast_intr_i(fast_intr),
      .fast_intr_o(irq_fast),
      .ext_peripheral_slave_req_o,
      .ext_peripheral_slave_resp_i,
      ${xheep.get_rh().get_instantiation_signals(xheep.get_ao_node()).strip(",")}
  );
<%
  domain = next(xheep.iter_peripheral_domains_normal())
%>
  peripheral_subsystem peripheral_subsystem_i (
      .clk_i,
      .rst_ni(peripheral_subsystem_rst_n && debug_reset_n),
      .clk_gate_en_ni(peripheral_subsystem_clkgate_en_n),
      .slave_req_i(peripheral_slave_req),
      .slave_resp_o(peripheral_slave_resp),
      ${xheep.get_rh().get_instantiation_signals(domain.get_node()).strip(",")}
  );

endmodule  // core_v_mini_mcu

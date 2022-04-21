// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module core_v_mini_mcu #(
    parameter PULP_XPULP = 0,
    parameter FPU        = 0,
    parameter PULP_ZFINX = 0
) (
    input logic clk_i,
    input logic rst_ni,

    input  logic jtag_tck_i,
    input  logic jtag_tms_i,
    input  logic jtag_trst_ni,
    input  logic jtag_tdi_i,
    output logic jtag_tdo_o,

    input  logic uart_rx_i,
    output logic uart_tx_o,

    input  logic        fetch_enable_i,
    output logic [31:0] exit_value_o,
    output logic        exit_valid_o
);

  import core_v_mini_mcu_pkg::*;
  import obi_pkg::*;
  import cv32e40p_apu_core_pkg::*;

  localparam NUM_BYTES = core_v_mini_mcu_pkg::MEM_SIZE;
  localparam DM_HALTADDRESS = core_v_mini_mcu_pkg::DEBUG_START_ADDRESS + 32'h00000800; //debug rom code (section .text in linker) starts at 0x800

  localparam JTAG_IDCODE = 32'h10001c05;
  localparam BOOT_ADDR = 'h180;
  localparam NUM_MHPMCOUNTERS = 1;


  // signals connecting core to memory

  obi_req_t         core_instr_req;
  obi_resp_t        core_instr_resp;
  obi_req_t         core_data_req;
  obi_resp_t        core_data_resp;
  obi_req_t         debug_master_req;
  obi_resp_t        debug_master_resp;

  obi_req_t         ram0_slave_req;
  obi_resp_t        ram0_slave_resp;
  obi_req_t         ram1_slave_req;
  obi_resp_t        ram1_slave_resp;
  obi_req_t         debug_slave_req;
  obi_resp_t        debug_slave_resp;
  obi_req_t         peripheral_slave_req;
  obi_resp_t        peripheral_slave_resp;
  obi_req_t         slow_ram_slave_req;
  obi_resp_t        slow_ram_slave_resp;

  // signals to debug unit
  logic             debug_core_req;

  // irq signals
  logic             irq_ack;
  logic      [ 4:0] irq_id_out;
  logic             irq_software;
  logic             irq_timer;
  logic             irq_external;
  logic      [15:0] irq_fast;


  cpu_subsystem #(
      .BOOT_ADDR       (BOOT_ADDR),
      .PULP_XPULP      (PULP_XPULP),
      .FPU             (FPU),
      .PULP_ZFINX      (PULP_ZFINX),
      .NUM_MHPMCOUNTERS(NUM_MHPMCOUNTERS),
      .DM_HALTADDRESS  (DM_HALTADDRESS)
  ) cpu_subsystem_i (
      // Clock and Reset
      .clk_i,
      .rst_ni,
      .core_instr_req_o(core_instr_req),
      .core_instr_resp_i(core_instr_resp),
      .core_data_req_o(core_data_req),
      .core_data_resp_i(core_data_resp),
      .irq_i({irq_fast, 4'b0, irq_external, 3'b0, irq_timer, 3'b0, irq_software, 3'b0}),
      .irq_ack_o(irq_ack),
      .irq_id_o(irq_id_out),
      .debug_req_i(debug_core_req),
      .fetch_enable_i(fetch_enable_i)
  );


  debug_subsystem #(
      .JTAG_IDCODE(JTAG_IDCODE)
  ) debug_subsystem_i (
      .clk_i (clk_i),
      .rst_ni(rst_ni),

      .jtag_tck_i  (jtag_tck_i),
      .jtag_tms_i  (jtag_tms_i),
      .jtag_trst_ni(jtag_trst_ni),
      .jtag_tdi_i  (jtag_tdi_i),
      .jtag_tdo_o  (jtag_tdo_o),

      .debug_core_req_o(debug_core_req),

      .debug_slave_req_i  (debug_slave_req),
      .debug_slave_resp_o (debug_slave_resp),
      .debug_master_req_o (debug_master_req),
      .debug_master_resp_i(debug_master_resp)

  );

  system_bus system_bus_i (
      .clk_i (clk_i),
      .rst_ni(rst_ni),

      .core_instr_req_i   (core_instr_req),
      .core_instr_resp_o  (core_instr_resp),
      .core_data_req_i    (core_data_req),
      .core_data_resp_o   (core_data_resp),
      .debug_master_req_i (debug_master_req),
      .debug_master_resp_o(debug_master_resp),

      .ram0_req_o             (ram0_slave_req),
      .ram0_resp_i            (ram0_slave_resp),
      .ram1_req_o             (ram1_slave_req),
      .ram1_resp_i            (ram1_slave_resp),
      .debug_slave_req_o      (debug_slave_req),
      .debug_slave_resp_i     (debug_slave_resp),
      .peripheral_slave_req_o (peripheral_slave_req),
      .peripheral_slave_resp_i(peripheral_slave_resp),
      .slow_ram_req_o         (slow_ram_slave_req),
      .slow_ram_resp_i        (slow_ram_slave_resp)

  );

  memory_subsystem #(
      .NUM_BYTES(NUM_BYTES)
  ) memory_subsystem_i (
      .clk_i,
      .rst_ni,
      .ram0_req_i (ram0_slave_req),
      .ram0_resp_o(ram0_slave_resp),
      .ram1_req_i (ram1_slave_req),
      .ram1_resp_o(ram1_slave_resp)
  );

  peripheral_subsystem peripheral_subsystem_i (
      .clk_i,
      .rst_ni,

      .slave_req_i (peripheral_slave_req),
      .slave_resp_o(peripheral_slave_resp),

      .exit_valid_o(exit_valid_o),
      .exit_value_o(exit_value_o),

      .uart_rx_i(uart_rx_i),
      .uart_tx_o(uart_tx_o),
      .uart_tx_en_o(),
      .uart_intr_tx_watermark_o() ,
      .uart_intr_rx_watermark_o() ,
      .uart_intr_tx_empty_o()  ,
      .uart_intr_rx_overflow_o()  ,
      .uart_intr_rx_frame_err_o() ,
      .uart_intr_rx_break_err_o() ,
      .uart_intr_rx_timeout_o()   ,
      .uart_intr_rx_parity_err_o()
  );

  slow_memory #(
      .NumWords (128),
      .DataWidth(32'd32)
  ) slow_ram_i (
      .clk_i  (clk_i),
      .rst_ni (rst_ni),
      .req_i  (slow_ram_slave_req.req),
      .we_i   (slow_ram_slave_req.we),
      .addr_i (slow_ram_slave_req.addr[8:2]),
      .wdata_i(slow_ram_slave_req.wdata),
      .be_i   (slow_ram_slave_req.be),
      // output ports
      .gnt_o(slow_ram_slave_resp.gnt),
      .rdata_o(slow_ram_slave_resp.rdata),
      .rvalid_o(slow_ram_slave_resp.rvalid)
  );

  assign irq_software = '0;
  assign irq_timer    = '0;
  assign irq_external = '0;
  assign irq_fast     = '0;


endmodule  // core_v_mini_mcu

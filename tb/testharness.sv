// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module testharness #(
    parameter PULP_XPULP = 0,
    parameter FPU        = 0,
    parameter PULP_ZFINX = 0,
    parameter JTAG_DPI   = 0
) (
    input logic clk_i,
    input logic rst_ni,

    input  logic        boot_select_i,
    input  logic        jtag_tck_i,
    input  logic        jtag_tms_i,
    input  logic        jtag_trst_ni,
    input  logic        jtag_tdi_i,
    output logic        jtag_tdo_o,
    input  logic        fetch_enable_i,
    output logic [31:0] exit_value_o,
    output logic        exit_valid_o
);

  `include "tb_util.svh"

  import obi_pkg::*;
  import reg_pkg::*;
  import testharness_pkg::*;

  logic uart_rx;
  logic uart_tx;
  logic sim_jtag_enable = (JTAG_DPI == 1);
  logic sim_jtag_tck;
  logic sim_jtag_tms;
  logic sim_jtag_trst;
  logic sim_jtag_tdi;
  logic sim_jtag_tdo;
  logic sim_jtag_trstn;

  // External xbar master/slave and peripheral ports
  obi_req_t [testharness_pkg::EXT_XBAR_NMASTER-1:0] master_req;
  obi_resp_t [testharness_pkg::EXT_XBAR_NMASTER-1:0] master_resp;
  obi_req_t slave_req;
  obi_resp_t slave_resp;
  reg_req_t periph_slave_req;
  reg_rsp_t periph_slave_resp;
  // External peripheral example port
  reg_req_t memcopy_periph_req;
  reg_rsp_t memcopy_periph_rsp;
  // External xbar slave example port
  obi_req_t slow_ram_slave_req;
  obi_resp_t slow_ram_slave_resp;


  logic flash_csb;
  logic flash_clk;

  logic flash_io0_oe;
  logic flash_io1_oe;
  logic flash_io2_oe;
  logic flash_io3_oe;

  logic flash_io0_do;
  logic flash_io1_do;
  logic flash_io2_do;
  logic flash_io3_do;

  logic flash_io0_di;
  logic flash_io1_di;
  logic flash_io2_di;
  logic flash_io3_di;

  wire flash_io0_io;
  wire flash_io1_io;
  wire flash_io2_io;
  wire flash_io3_io;

  //TODO: fix with PAD
  assign flash_io0_io = (flash_io0_oe) ? flash_io0_do : 1'bz;
  assign flash_io1_io = (flash_io1_oe) ? flash_io1_do : 1'bz;
  assign flash_io2_io = (flash_io2_oe) ? flash_io2_do : 1'bz;
  assign flash_io3_io = (flash_io3_oe) ? flash_io3_do : 1'bz;

  assign flash_io0_di = flash_io0_io;
  assign flash_io1_di = flash_io1_io;
  assign flash_io2_di = flash_io2_io;
  assign flash_io3_di = flash_io3_io;


  core_v_mini_mcu #(
      .PULP_XPULP      (PULP_XPULP),
      .FPU             (FPU),
      .PULP_ZFINX      (PULP_ZFINX),
      .EXT_XBAR_NMASTER(testharness_pkg::EXT_XBAR_NMASTER)
  ) core_v_mini_mcu_i (
      .clk_i,
      .rst_ni,

      .boot_select_i,
      
      .jtag_tck_i  (sim_jtag_tck),
      .jtag_tms_i  (sim_jtag_tms),
      .jtag_trst_ni(sim_jtag_trstn),
      .jtag_tdi_i  (sim_jtag_tdi),
      .jtag_tdo_o  (sim_jtag_tdo),

      .ext_xbar_master_req_i(master_req),
      .ext_xbar_master_resp_o(master_resp),
      .ext_xbar_slave_req_o(slave_req),
      .ext_xbar_slave_resp_i(slave_resp),
      .ext_peripheral_slave_req_o(periph_slave_req),
      .ext_peripheral_slave_resp_i(periph_slave_resp),

      .uart_rx_i(uart_rx),
      .uart_tx_o(uart_tx),

      .flash_csb_o(flash_csb),
      .flash_clk_o(flash_clk),

      .flash_io0_oe_o(flash_io0_oe),
      .flash_io1_oe_o(flash_io1_oe),
      .flash_io2_oe_o(flash_io2_oe),
      .flash_io3_oe_o(flash_io3_oe),

      .flash_io0_do_o(flash_io0_do),
      .flash_io1_do_o(flash_io1_do),
      .flash_io2_do_o(flash_io2_do),
      .flash_io3_do_o(flash_io3_do),

      .flash_io0_di_i(flash_io0_di),
      .flash_io1_di_i(flash_io1_di),
      .flash_io2_di_i(flash_io2_di),
      .flash_io3_di_i(flash_io3_di),

      .fetch_enable_i,
      .exit_value_o,
      .exit_valid_o
  );

  uartdpi #(
      .BAUD('d7200),
      // Frequency shouldn't matter since we are sending with the same clock.
      .FREQ('d125_000),
      .NAME("uart0")
  ) i_uart0 (
      .clk_i,
      .rst_ni,
      .tx_o(uart_rx),
      .rx_i(uart_tx)
  );

  // jtag calls from dpi
  SimJTAG #(
      .TICK_DELAY(1),
      .PORT      (4567)
  ) i_sim_jtag (
      .clock          (clk_i),
      .reset          (~rst_ni),
      .enable         (sim_jtag_enable),
      .init_done      (rst_ni),
      .jtag_TCK       (sim_jtag_tck),
      .jtag_TMS       (sim_jtag_tms),
      .jtag_TDI       (sim_jtag_tdi),
      .jtag_TRSTn     (sim_jtag_trstn),
      .jtag_TDO_data  (sim_jtag_tdo),
      .jtag_TDO_driven(1'b1),
      .exit           ()
  );

  assign slow_ram_slave_req = slave_req;
  assign slave_resp = slow_ram_slave_resp;

  assign memcopy_periph_req = periph_slave_req;
  assign periph_slave_resp = memcopy_periph_rsp;

`ifdef MODELSIM
  spiflash flash_1 (
      .csb(flash_csb),
      .clk(flash_clk),
      .io0(flash_io0_io),  // MOSI
      .io1(flash_io1_io),  // MISO
      .io2(flash_io2_io),
      .io3(flash_io3_io)
  );
`endif

`ifdef USE_EXTERNAL_DEVICE_EXAMPLE
  // External xbar slave memory example
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

  // External peripheral example with master port to access memory
  memcopy_periph #(
      .reg_req_t (reg_pkg::reg_req_t),
      .reg_rsp_t (reg_pkg::reg_rsp_t),
      .obi_req_t (obi_pkg::obi_req_t),
      .obi_resp_t(obi_pkg::obi_resp_t)
  ) memcopy_periph_i (
      .clk_i,
      .rst_ni,
      .reg_req_i(memcopy_periph_req),
      .reg_rsp_o(memcopy_periph_rsp),
      .master_req_o(master_req[testharness_pkg::EXT_MASTER0_IDX]),
      .master_resp_i(master_resp[testharness_pkg::EXT_MASTER0_IDX])
  );
`else
  assign slow_ram_slave_resp.gnt = '0;
  assign slow_ram_slave_resp.rdata = '0;
  assign slow_ram_slave_resp.rvalid = '0;

  assign memcopy_periph_rsp.error = '0;
  assign memcopy_periph_rsp.ready = '0;
  assign memcopy_periph_rsp.rdata = '0;

  assign master_req[testharness_pkg::EXT_MASTER0_IDX].req = '0;
  assign master_req[testharness_pkg::EXT_MASTER0_IDX].we = '0;
  assign master_req[testharness_pkg::EXT_MASTER0_IDX].be = '0;
  assign master_req[testharness_pkg::EXT_MASTER0_IDX].addr = '0;
  assign master_req[testharness_pkg::EXT_MASTER0_IDX].wdata = '0;
`endif

endmodule  // testharness

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

  logic uart_rx;
  logic uart_tx;
  logic sim_jtag_enable = (JTAG_DPI == 1);
  logic sim_jtag_tck;
  logic sim_jtag_tms;
  logic sim_jtag_trst;
  logic sim_jtag_tdi;
  logic sim_jtag_tdo;
  logic sim_jtag_trstn;

  core_v_mini_mcu #(
      .PULP_XPULP(PULP_XPULP),
      .FPU       (FPU),
      .PULP_ZFINX(PULP_ZFINX)
  ) core_v_mini_mcu_i (
      .clk_i,
      .rst_ni,

      .jtag_tck_i  (sim_jtag_tck),
      .jtag_tms_i  (sim_jtag_tms),
      .jtag_trst_ni(sim_jtag_trstn),
      .jtag_tdi_i  (sim_jtag_tdi),
      .jtag_tdo_o  (sim_jtag_tdo),

      .uart_rx_i(uart_rx),
      .uart_tx_o(uart_tx),

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

endmodule  // testharness

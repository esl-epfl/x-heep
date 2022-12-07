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

    input logic [core_v_mini_mcu_pkg::NEXT_INT-1:0] intr_vector_ext_i,

    input  obi_req_t  [EXT_XBAR_NMASTER-1:0] ext_xbar_master_req_i,
    output obi_resp_t [EXT_XBAR_NMASTER-1:0] ext_xbar_master_resp_o,

    output obi_req_t  ext_xbar_slave_req_o,
    input  obi_resp_t ext_xbar_slave_resp_i,

    output reg_req_t ext_peripheral_slave_req_o,
    input  reg_rsp_t ext_peripheral_slave_resp_i,

    output logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_subsystem_powergate_switch_o,
    input  logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_subsystem_powergate_switch_ack_i,
    output logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_subsystem_powergate_iso_o,
    output logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_subsystem_rst_no,
    output logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_ram_banks_set_retentive_o,

    output logic [31:0] exit_value_o,

    inout logic clk_i,
    inout logic rst_ni,
    inout logic boot_select_i,
    inout logic execute_from_flash_i,
    inout logic jtag_tck_i,
    inout logic jtag_tms_i,
    inout logic jtag_trst_ni,
    inout logic jtag_tdi_i,
    inout logic jtag_tdo_o,
    inout logic uart_rx_i,
    inout logic uart_tx_o,
    inout logic exit_valid_o,
    inout logic gpio_0_io,
    inout logic gpio_1_io,
    inout logic gpio_2_io,
    inout logic gpio_3_io,
    inout logic gpio_4_io,
    inout logic gpio_5_io,
    inout logic gpio_6_io,
    inout logic gpio_7_io,
    inout logic gpio_8_io,
    inout logic gpio_9_io,
    inout logic gpio_10_io,
    inout logic gpio_11_io,
    inout logic gpio_12_io,
    inout logic gpio_13_io,
    inout logic gpio_14_io,
    inout logic gpio_15_io,
    inout logic gpio_16_io,
    inout logic gpio_17_io,
    inout logic gpio_18_io,
    inout logic gpio_19_io,
    inout logic gpio_20_io,
    inout logic gpio_21_io,
    inout logic gpio_22_io,
    inout logic gpio_23_io,
    inout logic gpio_24_io,
    inout logic gpio_25_io,
    inout logic gpio_26_io,
    inout logic gpio_27_io,
    inout logic gpio_28_io,
    inout logic gpio_29_io,
    inout logic spi_flash_sck_io,
    inout logic spi_flash_cs_0_io,
    inout logic spi_flash_cs_1_io,
    inout logic spi_flash_sd_0_io,
    inout logic spi_flash_sd_1_io,
    inout logic spi_flash_sd_2_io,
    inout logic spi_flash_sd_3_io,
    inout logic spi_sck_io,
    inout logic spi_cs_0_io,
    inout logic spi_cs_1_io,
    inout logic spi_sd_0_io,
    inout logic spi_sd_1_io,
    inout logic spi_sd_2_io,
    inout logic spi_sd_3_io,
    inout logic i2c_scl_io,
    inout logic i2c_sda_io
);

  import core_v_mini_mcu_pkg::*;

  // PM signals
  logic cpu_subsystem_powergate_switch;
  logic cpu_subsystem_powergate_switch_ack;
  logic cpu_subsystem_powergate_iso;
  logic cpu_subsystem_rst_n;
  logic peripheral_subsystem_powergate_switch;
  logic peripheral_subsystem_powergate_switch_ack;
  logic peripheral_subsystem_powergate_iso;
  logic peripheral_subsystem_rst_n;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_switch;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_switch_ack;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_iso;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_set_retentive;

  // PAD controller
  reg_req_t pad_req;
  reg_rsp_t pad_resp;
  logic [core_v_mini_mcu_pkg::NUM_PAD-1:0][7:0] pad_attributes;
  logic [core_v_mini_mcu_pkg::NUM_PAD-1:0][3:0] pad_muxes;

  logic rst_ngen;

  //input, output pins from core_v_mini_mcu
  logic clk_in_x, clk_out_x, clk_oe_x;

  logic rst_nin_x, rst_nout_x, rst_noe_x;

  logic boot_select_in_x, boot_select_out_x, boot_select_oe_x;

  logic execute_from_flash_in_x, execute_from_flash_out_x, execute_from_flash_oe_x;

  logic jtag_tck_in_x, jtag_tck_out_x, jtag_tck_oe_x;

  logic jtag_tms_in_x, jtag_tms_out_x, jtag_tms_oe_x;

  logic jtag_trst_nin_x, jtag_trst_nout_x, jtag_trst_noe_x;

  logic jtag_tdi_in_x, jtag_tdi_out_x, jtag_tdi_oe_x;

  logic jtag_tdo_in_x, jtag_tdo_out_x, jtag_tdo_oe_x;

  logic uart_rx_in_x, uart_rx_out_x, uart_rx_oe_x;

  logic uart_tx_in_x, uart_tx_out_x, uart_tx_oe_x;

  logic exit_valid_in_x, exit_valid_out_x, exit_valid_oe_x;

  logic gpio_0_in_x, gpio_0_out_x, gpio_0_oe_x;

  logic gpio_1_in_x, gpio_1_out_x, gpio_1_oe_x;

  logic gpio_2_in_x, gpio_2_out_x, gpio_2_oe_x;

  logic gpio_3_in_x, gpio_3_out_x, gpio_3_oe_x;

  logic gpio_4_in_x, gpio_4_out_x, gpio_4_oe_x;

  logic gpio_5_in_x, gpio_5_out_x, gpio_5_oe_x;

  logic gpio_6_in_x, gpio_6_out_x, gpio_6_oe_x;

  logic gpio_7_in_x, gpio_7_out_x, gpio_7_oe_x;

  logic gpio_8_in_x, gpio_8_out_x, gpio_8_oe_x;

  logic gpio_9_in_x, gpio_9_out_x, gpio_9_oe_x;

  logic gpio_10_in_x, gpio_10_out_x, gpio_10_oe_x;

  logic gpio_11_in_x, gpio_11_out_x, gpio_11_oe_x;

  logic gpio_12_in_x, gpio_12_out_x, gpio_12_oe_x;

  logic gpio_13_in_x, gpio_13_out_x, gpio_13_oe_x;

  logic gpio_14_in_x, gpio_14_out_x, gpio_14_oe_x;

  logic gpio_15_in_x, gpio_15_out_x, gpio_15_oe_x;

  logic gpio_16_in_x, gpio_16_out_x, gpio_16_oe_x;

  logic gpio_17_in_x, gpio_17_out_x, gpio_17_oe_x;

  logic gpio_18_in_x, gpio_18_out_x, gpio_18_oe_x;

  logic gpio_19_in_x, gpio_19_out_x, gpio_19_oe_x;

  logic gpio_20_in_x, gpio_20_out_x, gpio_20_oe_x;

  logic gpio_21_in_x, gpio_21_out_x, gpio_21_oe_x;

  logic gpio_22_in_x, gpio_22_out_x, gpio_22_oe_x;

  logic gpio_23_in_x, gpio_23_out_x, gpio_23_oe_x;

  logic gpio_24_in_x, gpio_24_out_x, gpio_24_oe_x;

  logic gpio_25_in_x, gpio_25_out_x, gpio_25_oe_x;

  logic gpio_26_in_x, gpio_26_out_x, gpio_26_oe_x;

  logic gpio_27_in_x, gpio_27_out_x, gpio_27_oe_x;

  logic gpio_28_in_x, gpio_28_out_x, gpio_28_oe_x;

  logic gpio_29_in_x, gpio_29_out_x, gpio_29_oe_x;

  logic spi_flash_sck_in_x, spi_flash_sck_out_x, spi_flash_sck_oe_x;

  logic spi_flash_cs_0_in_x, spi_flash_cs_0_out_x, spi_flash_cs_0_oe_x;

  logic spi_flash_cs_1_in_x, spi_flash_cs_1_out_x, spi_flash_cs_1_oe_x;

  logic spi_flash_sd_0_in_x, spi_flash_sd_0_out_x, spi_flash_sd_0_oe_x;

  logic spi_flash_sd_1_in_x, spi_flash_sd_1_out_x, spi_flash_sd_1_oe_x;

  logic spi_flash_sd_2_in_x, spi_flash_sd_2_out_x, spi_flash_sd_2_oe_x;

  logic spi_flash_sd_3_in_x, spi_flash_sd_3_out_x, spi_flash_sd_3_oe_x;

  logic spi_sck_in_x, spi_sck_out_x, spi_sck_oe_x;

  logic spi_cs_0_in_x, spi_cs_0_out_x, spi_cs_0_oe_x;

  logic spi_cs_1_in_x, spi_cs_1_out_x, spi_cs_1_oe_x;

  logic spi_sd_0_in_x, spi_sd_0_out_x, spi_sd_0_oe_x;

  logic spi_sd_1_in_x, spi_sd_1_out_x, spi_sd_1_oe_x;

  logic spi_sd_2_in_x, spi_sd_2_out_x, spi_sd_2_oe_x;

  logic spi_sd_3_in_x, spi_sd_3_out_x, spi_sd_3_oe_x;

  logic i2c_scl_in_x, i2c_scl_out_x, i2c_scl_oe_x;
  logic gpio_31_in_x, gpio_31_out_x, gpio_31_oe_x;
  logic i2c_scl_in_x_muxed, i2c_scl_out_x_muxed, i2c_scl_oe_x_muxed;

  logic i2c_sda_in_x, i2c_sda_out_x, i2c_sda_oe_x;
  logic gpio_30_in_x, gpio_30_out_x, gpio_30_oe_x;
  logic i2c_sda_in_x_muxed, i2c_sda_out_x_muxed, i2c_sda_oe_x_muxed;


  core_v_mini_mcu #(
      .PULP_XPULP(PULP_XPULP),
      .FPU(FPU),
      .PULP_ZFINX(PULP_ZFINX),
      .EXT_XBAR_NMASTER(EXT_XBAR_NMASTER)
  ) core_v_mini_mcu_i (

      .rst_ni(rst_ngen),
      .clk_i (clk_in_x),


      .boot_select_i(boot_select_in_x),

      .execute_from_flash_i(execute_from_flash_in_x),

      .jtag_tck_i(jtag_tck_in_x),

      .jtag_tms_i(jtag_tms_in_x),

      .jtag_trst_ni(jtag_trst_nin_x),

      .jtag_tdi_i(jtag_tdi_in_x),

      .jtag_tdo_o(jtag_tdo_out_x),

      .uart_rx_i(uart_rx_in_x),

      .uart_tx_o(uart_tx_out_x),

      .exit_valid_o(exit_valid_out_x),

      .gpio_0_i(gpio_0_in_x),
      .gpio_0_o(gpio_0_out_x),
      .gpio_0_oe_o(gpio_0_oe_x),

      .gpio_1_i(gpio_1_in_x),
      .gpio_1_o(gpio_1_out_x),
      .gpio_1_oe_o(gpio_1_oe_x),

      .gpio_2_i(gpio_2_in_x),
      .gpio_2_o(gpio_2_out_x),
      .gpio_2_oe_o(gpio_2_oe_x),

      .gpio_3_i(gpio_3_in_x),
      .gpio_3_o(gpio_3_out_x),
      .gpio_3_oe_o(gpio_3_oe_x),

      .gpio_4_i(gpio_4_in_x),
      .gpio_4_o(gpio_4_out_x),
      .gpio_4_oe_o(gpio_4_oe_x),

      .gpio_5_i(gpio_5_in_x),
      .gpio_5_o(gpio_5_out_x),
      .gpio_5_oe_o(gpio_5_oe_x),

      .gpio_6_i(gpio_6_in_x),
      .gpio_6_o(gpio_6_out_x),
      .gpio_6_oe_o(gpio_6_oe_x),

      .gpio_7_i(gpio_7_in_x),
      .gpio_7_o(gpio_7_out_x),
      .gpio_7_oe_o(gpio_7_oe_x),

      .gpio_8_i(gpio_8_in_x),
      .gpio_8_o(gpio_8_out_x),
      .gpio_8_oe_o(gpio_8_oe_x),

      .gpio_9_i(gpio_9_in_x),
      .gpio_9_o(gpio_9_out_x),
      .gpio_9_oe_o(gpio_9_oe_x),

      .gpio_10_i(gpio_10_in_x),
      .gpio_10_o(gpio_10_out_x),
      .gpio_10_oe_o(gpio_10_oe_x),

      .gpio_11_i(gpio_11_in_x),
      .gpio_11_o(gpio_11_out_x),
      .gpio_11_oe_o(gpio_11_oe_x),

      .gpio_12_i(gpio_12_in_x),
      .gpio_12_o(gpio_12_out_x),
      .gpio_12_oe_o(gpio_12_oe_x),

      .gpio_13_i(gpio_13_in_x),
      .gpio_13_o(gpio_13_out_x),
      .gpio_13_oe_o(gpio_13_oe_x),

      .gpio_14_i(gpio_14_in_x),
      .gpio_14_o(gpio_14_out_x),
      .gpio_14_oe_o(gpio_14_oe_x),

      .gpio_15_i(gpio_15_in_x),
      .gpio_15_o(gpio_15_out_x),
      .gpio_15_oe_o(gpio_15_oe_x),

      .gpio_16_i(gpio_16_in_x),
      .gpio_16_o(gpio_16_out_x),
      .gpio_16_oe_o(gpio_16_oe_x),

      .gpio_17_i(gpio_17_in_x),
      .gpio_17_o(gpio_17_out_x),
      .gpio_17_oe_o(gpio_17_oe_x),

      .gpio_18_i(gpio_18_in_x),
      .gpio_18_o(gpio_18_out_x),
      .gpio_18_oe_o(gpio_18_oe_x),

      .gpio_19_i(gpio_19_in_x),
      .gpio_19_o(gpio_19_out_x),
      .gpio_19_oe_o(gpio_19_oe_x),

      .gpio_20_i(gpio_20_in_x),
      .gpio_20_o(gpio_20_out_x),
      .gpio_20_oe_o(gpio_20_oe_x),

      .gpio_21_i(gpio_21_in_x),
      .gpio_21_o(gpio_21_out_x),
      .gpio_21_oe_o(gpio_21_oe_x),

      .gpio_22_i(gpio_22_in_x),
      .gpio_22_o(gpio_22_out_x),
      .gpio_22_oe_o(gpio_22_oe_x),

      .gpio_23_i(gpio_23_in_x),
      .gpio_23_o(gpio_23_out_x),
      .gpio_23_oe_o(gpio_23_oe_x),

      .gpio_24_i(gpio_24_in_x),
      .gpio_24_o(gpio_24_out_x),
      .gpio_24_oe_o(gpio_24_oe_x),

      .gpio_25_i(gpio_25_in_x),
      .gpio_25_o(gpio_25_out_x),
      .gpio_25_oe_o(gpio_25_oe_x),

      .gpio_26_i(gpio_26_in_x),
      .gpio_26_o(gpio_26_out_x),
      .gpio_26_oe_o(gpio_26_oe_x),

      .gpio_27_i(gpio_27_in_x),
      .gpio_27_o(gpio_27_out_x),
      .gpio_27_oe_o(gpio_27_oe_x),

      .gpio_28_i(gpio_28_in_x),
      .gpio_28_o(gpio_28_out_x),
      .gpio_28_oe_o(gpio_28_oe_x),

      .gpio_29_i(gpio_29_in_x),
      .gpio_29_o(gpio_29_out_x),
      .gpio_29_oe_o(gpio_29_oe_x),

      .spi_flash_sck_i(spi_flash_sck_in_x),
      .spi_flash_sck_o(spi_flash_sck_out_x),
      .spi_flash_sck_oe_o(spi_flash_sck_oe_x),

      .spi_flash_cs_0_i(spi_flash_cs_0_in_x),
      .spi_flash_cs_0_o(spi_flash_cs_0_out_x),
      .spi_flash_cs_0_oe_o(spi_flash_cs_0_oe_x),

      .spi_flash_cs_1_i(spi_flash_cs_1_in_x),
      .spi_flash_cs_1_o(spi_flash_cs_1_out_x),
      .spi_flash_cs_1_oe_o(spi_flash_cs_1_oe_x),

      .spi_flash_sd_0_i(spi_flash_sd_0_in_x),
      .spi_flash_sd_0_o(spi_flash_sd_0_out_x),
      .spi_flash_sd_0_oe_o(spi_flash_sd_0_oe_x),

      .spi_flash_sd_1_i(spi_flash_sd_1_in_x),
      .spi_flash_sd_1_o(spi_flash_sd_1_out_x),
      .spi_flash_sd_1_oe_o(spi_flash_sd_1_oe_x),

      .spi_flash_sd_2_i(spi_flash_sd_2_in_x),
      .spi_flash_sd_2_o(spi_flash_sd_2_out_x),
      .spi_flash_sd_2_oe_o(spi_flash_sd_2_oe_x),

      .spi_flash_sd_3_i(spi_flash_sd_3_in_x),
      .spi_flash_sd_3_o(spi_flash_sd_3_out_x),
      .spi_flash_sd_3_oe_o(spi_flash_sd_3_oe_x),

      .spi_sck_i(spi_sck_in_x),
      .spi_sck_o(spi_sck_out_x),
      .spi_sck_oe_o(spi_sck_oe_x),

      .spi_cs_0_i(spi_cs_0_in_x),
      .spi_cs_0_o(spi_cs_0_out_x),
      .spi_cs_0_oe_o(spi_cs_0_oe_x),

      .spi_cs_1_i(spi_cs_1_in_x),
      .spi_cs_1_o(spi_cs_1_out_x),
      .spi_cs_1_oe_o(spi_cs_1_oe_x),

      .spi_sd_0_i(spi_sd_0_in_x),
      .spi_sd_0_o(spi_sd_0_out_x),
      .spi_sd_0_oe_o(spi_sd_0_oe_x),

      .spi_sd_1_i(spi_sd_1_in_x),
      .spi_sd_1_o(spi_sd_1_out_x),
      .spi_sd_1_oe_o(spi_sd_1_oe_x),

      .spi_sd_2_i(spi_sd_2_in_x),
      .spi_sd_2_o(spi_sd_2_out_x),
      .spi_sd_2_oe_o(spi_sd_2_oe_x),

      .spi_sd_3_i(spi_sd_3_in_x),
      .spi_sd_3_o(spi_sd_3_out_x),
      .spi_sd_3_oe_o(spi_sd_3_oe_x),

      .i2c_scl_i(i2c_scl_in_x),
      .i2c_scl_o(i2c_scl_out_x),
      .i2c_scl_oe_o(i2c_scl_oe_x),
      .gpio_31_i(gpio_31_in_x),
      .gpio_31_o(gpio_31_out_x),
      .gpio_31_oe_o(gpio_31_oe_x),

      .i2c_sda_i(i2c_sda_in_x),
      .i2c_sda_o(i2c_sda_out_x),
      .i2c_sda_oe_o(i2c_sda_oe_x),
      .gpio_30_i(gpio_30_in_x),
      .gpio_30_o(gpio_30_out_x),
      .gpio_30_oe_o(gpio_30_oe_x),

      .intr_vector_ext_i,
      .pad_req_o(pad_req),
      .pad_resp_i(pad_resp),
      .ext_xbar_master_req_i,
      .ext_xbar_master_resp_o,
      .ext_xbar_slave_req_o,
      .ext_xbar_slave_resp_i,
      .ext_peripheral_slave_req_o,
      .ext_peripheral_slave_resp_i,
      .cpu_subsystem_powergate_switch_o(cpu_subsystem_powergate_switch),
      .cpu_subsystem_powergate_switch_ack_i(cpu_subsystem_powergate_switch_ack),
      .peripheral_subsystem_powergate_switch_o(peripheral_subsystem_powergate_switch),
      .peripheral_subsystem_powergate_switch_ack_i(peripheral_subsystem_powergate_switch_ack),
      .memory_subsystem_banks_powergate_switch_o(memory_subsystem_banks_powergate_switch),
      .memory_subsystem_banks_powergate_switch_ack_i(memory_subsystem_banks_powergate_switch_ack),
      .external_subsystem_powergate_switch_o,
      .external_subsystem_powergate_switch_ack_i,
      .external_subsystem_powergate_iso_o,
      .external_subsystem_rst_no,
      .external_ram_banks_set_retentive_o,
      .exit_value_o
  );


  import pkg_padframe::port_signals_soc2pad_t;
  import pkg_padframe::port_signals_pad2soc_t;

  import pkg_padframe::static_connection_signals_soc2pad_t;
  import pkg_padframe::static_connection_signals_pad2soc_t;

  port_signals_soc2pad_t port_signals_soc2pad;
  port_signals_pad2soc_t port_signals_pad2soc;

  static_connection_signals_soc2pad_t static_connection_signals_soc2pad;
  static_connection_signals_pad2soc_t static_connection_signals_pad2soc;
  assign static_connection_signals_soc2pad.xheep.exit_valid_i = exit_valid_out_x;

  assign static_connection_signals_soc2pad.xheep.jtag_tdo_i = jtag_tdo_out_x;

  assign static_connection_signals_soc2pad.xheep.uart_tx_i = uart_tx_out_x;

  assign static_connection_signals_soc2pad.xheep.spi_sck_i = spi_sck_out_x;
  assign static_connection_signals_soc2pad.xheep.spi_cs_00_i = spi_cs_0_out_x;
  assign static_connection_signals_soc2pad.xheep.spi_cs_01_i = spi_cs_1_out_x;
  assign static_connection_signals_soc2pad.xheep.spi_cs_00_oe_i = spi_cs_0_oe_x;
  assign static_connection_signals_soc2pad.xheep.spi_cs_01_oe_i = spi_cs_1_oe_x;
  assign static_connection_signals_soc2pad.xheep.spi_sd_00_i = spi_sd_0_out_x;
  assign static_connection_signals_soc2pad.xheep.spi_sd_01_i = spi_sd_1_out_x;
  assign static_connection_signals_soc2pad.xheep.spi_sd_02_i = spi_sd_2_out_x;
  assign static_connection_signals_soc2pad.xheep.spi_sd_03_i = spi_sd_3_out_x;
  assign static_connection_signals_soc2pad.xheep.spi_sd_00_oe_i = spi_sd_0_oe_x;
  assign static_connection_signals_soc2pad.xheep.spi_sd_01_oe_i = spi_sd_1_oe_x;
  assign static_connection_signals_soc2pad.xheep.spi_sd_02_oe_i = spi_sd_2_oe_x;
  assign static_connection_signals_soc2pad.xheep.spi_sd_03_oe_i = spi_sd_3_oe_x;

  assign static_connection_signals_soc2pad.xheep.spi_flash_sck_i = spi_flash_sck_out_x;
  assign static_connection_signals_soc2pad.xheep.spi_flash_cs_00_i = spi_flash_cs_0_out_x;
  assign static_connection_signals_soc2pad.xheep.spi_flash_cs_01_i = spi_flash_cs_1_out_x;
  assign static_connection_signals_soc2pad.xheep.spi_flash_cs_00_oe_i = spi_flash_cs_0_oe_x;
  assign static_connection_signals_soc2pad.xheep.spi_flash_cs_01_oe_i = spi_flash_cs_1_oe_x;
  assign static_connection_signals_soc2pad.xheep.spi_flash_sd_00_i = spi_flash_sd_0_out_x;
  assign static_connection_signals_soc2pad.xheep.spi_flash_sd_01_i = spi_flash_sd_1_out_x;
  assign static_connection_signals_soc2pad.xheep.spi_flash_sd_02_i = spi_flash_sd_2_out_x;
  assign static_connection_signals_soc2pad.xheep.spi_flash_sd_03_i = spi_flash_sd_3_out_x;
  assign static_connection_signals_soc2pad.xheep.spi_flash_sd_00_oe_i = spi_flash_sd_0_oe_x;
  assign static_connection_signals_soc2pad.xheep.spi_flash_sd_01_oe_i = spi_flash_sd_1_oe_x;
  assign static_connection_signals_soc2pad.xheep.spi_flash_sd_02_oe_i = spi_flash_sd_2_oe_x;
  assign static_connection_signals_soc2pad.xheep.spi_flash_sd_03_oe_i = spi_flash_sd_3_oe_x;

  assign static_connection_signals_soc2pad.xheep.gpio_00_i = gpio_0_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_01_i = gpio_1_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_02_i = gpio_2_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_03_i = gpio_3_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_04_i = gpio_4_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_05_i = gpio_5_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_06_i = gpio_6_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_07_i = gpio_7_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_08_i = gpio_8_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_09_i = gpio_9_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_10_i = gpio_10_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_11_i = gpio_11_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_12_i = gpio_12_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_13_i = gpio_13_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_14_i = gpio_14_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_15_i = gpio_15_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_16_i = gpio_16_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_17_i = gpio_17_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_18_i = gpio_18_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_19_i = gpio_19_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_20_i = gpio_20_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_21_i = gpio_21_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_22_i = gpio_22_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_23_i = gpio_23_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_24_i = gpio_24_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_25_i = gpio_25_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_26_i = gpio_26_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_27_i = gpio_27_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_28_i = gpio_28_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_29_i = gpio_29_out_x;
  assign static_connection_signals_soc2pad.xheep.gpio_00_oe_i = gpio_0_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_01_oe_i = gpio_1_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_02_oe_i = gpio_2_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_03_oe_i = gpio_3_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_04_oe_i = gpio_4_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_05_oe_i = gpio_5_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_06_oe_i = gpio_6_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_07_oe_i = gpio_7_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_08_oe_i = gpio_8_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_09_oe_i = gpio_9_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_10_oe_i = gpio_10_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_11_oe_i = gpio_11_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_12_oe_i = gpio_12_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_13_oe_i = gpio_13_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_14_oe_i = gpio_14_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_15_oe_i = gpio_15_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_16_oe_i = gpio_16_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_17_oe_i = gpio_17_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_18_oe_i = gpio_18_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_19_oe_i = gpio_19_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_20_oe_i = gpio_20_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_21_oe_i = gpio_21_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_22_oe_i = gpio_22_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_23_oe_i = gpio_23_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_24_oe_i = gpio_24_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_25_oe_i = gpio_25_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_26_oe_i = gpio_26_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_27_oe_i = gpio_27_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_28_oe_i = gpio_28_oe_x;
  assign static_connection_signals_soc2pad.xheep.gpio_29_oe_i = gpio_29_oe_x;


  assign clk_in_x = static_connection_signals_pad2soc.xheep.clk_o;
  assign rst_nin_x = static_connection_signals_pad2soc.xheep.rst_o;
  assign boot_select_in_x = static_connection_signals_pad2soc.xheep.boot_select_o;
  assign execute_from_flash_in_x = static_connection_signals_pad2soc.xheep.execute_from_flash_o;
  assign uart_rx_in_x = static_connection_signals_pad2soc.xheep.uart_rx_o;

  assign jtag_tck_in_x = static_connection_signals_pad2soc.xheep.jtag_tck_o;
  assign jtag_tdi_in_x = static_connection_signals_pad2soc.xheep.jtag_tdi_o;
  assign jtag_tms_in_x = static_connection_signals_pad2soc.xheep.jtag_tms_o;
  assign jtag_trst_in_x = static_connection_signals_pad2soc.xheep.jtag_trst_o;
  assign spi_sck_in_x = static_connection_signals_pad2soc.xheep.spi_sck_o;
  assign spi_cs_0_in_x = static_connection_signals_pad2soc.xheep.spi_cs_00_o;
  assign spi_cs_1_in_x = static_connection_signals_pad2soc.xheep.spi_cs_01_o;
  assign spi_sd_0_in_x = static_connection_signals_pad2soc.xheep.spi_sd_00_o;
  assign spi_sd_1_in_x = static_connection_signals_pad2soc.xheep.spi_sd_01_o;
  assign spi_sd_2_in_x = static_connection_signals_pad2soc.xheep.spi_sd_02_o;
  assign spi_sd_3_in_x = static_connection_signals_pad2soc.xheep.spi_sd_03_o;

  assign spi_flash_sck_in_x = static_connection_signals_pad2soc.xheep.spi_flash_sck_o;
  assign spi_flash_cs_0_in_x = static_connection_signals_pad2soc.xheep.spi_flash_cs_00_o;
  assign spi_flash_cs_1_in_x = static_connection_signals_pad2soc.xheep.spi_flash_cs_01_o;
  assign spi_flash_sd_0_in_x = static_connection_signals_pad2soc.xheep.spi_flash_sd_00_o;
  assign spi_flash_sd_1_in_x = static_connection_signals_pad2soc.xheep.spi_flash_sd_01_o;
  assign spi_flash_sd_2_in_x = static_connection_signals_pad2soc.xheep.spi_flash_sd_02_o;
  assign spi_flash_sd_3_in_x = static_connection_signals_pad2soc.xheep.spi_flash_sd_03_o;

  assign gpio_0_in_x = static_connection_signals_pad2soc.xheep.gpio_00_o;
  assign gpio_1_in_x = static_connection_signals_pad2soc.xheep.gpio_01_o;
  assign gpio_2_in_x = static_connection_signals_pad2soc.xheep.gpio_02_o;
  assign gpio_3_in_x = static_connection_signals_pad2soc.xheep.gpio_03_o;
  assign gpio_4_in_x = static_connection_signals_pad2soc.xheep.gpio_04_o;
  assign gpio_5_in_x = static_connection_signals_pad2soc.xheep.gpio_05_o;
  assign gpio_6_in_x = static_connection_signals_pad2soc.xheep.gpio_06_o;
  assign gpio_7_in_x = static_connection_signals_pad2soc.xheep.gpio_07_o;
  assign gpio_8_in_x = static_connection_signals_pad2soc.xheep.gpio_08_o;
  assign gpio_9_in_x = static_connection_signals_pad2soc.xheep.gpio_09_o;
  assign gpio_10_in_x = static_connection_signals_pad2soc.xheep.gpio_10_o;
  assign gpio_11_in_x = static_connection_signals_pad2soc.xheep.gpio_11_o;
  assign gpio_12_in_x = static_connection_signals_pad2soc.xheep.gpio_12_o;
  assign gpio_13_in_x = static_connection_signals_pad2soc.xheep.gpio_13_o;
  assign gpio_14_in_x = static_connection_signals_pad2soc.xheep.gpio_14_o;
  assign gpio_15_in_x = static_connection_signals_pad2soc.xheep.gpio_15_o;
  assign gpio_16_in_x = static_connection_signals_pad2soc.xheep.gpio_16_o;
  assign gpio_17_in_x = static_connection_signals_pad2soc.xheep.gpio_17_o;
  assign gpio_18_in_x = static_connection_signals_pad2soc.xheep.gpio_18_o;
  assign gpio_19_in_x = static_connection_signals_pad2soc.xheep.gpio_19_o;
  assign gpio_20_in_x = static_connection_signals_pad2soc.xheep.gpio_20_o;
  assign gpio_21_in_x = static_connection_signals_pad2soc.xheep.gpio_21_o;
  assign gpio_22_in_x = static_connection_signals_pad2soc.xheep.gpio_22_o;
  assign gpio_23_in_x = static_connection_signals_pad2soc.xheep.gpio_23_o;
  assign gpio_24_in_x = static_connection_signals_pad2soc.xheep.gpio_24_o;
  assign gpio_25_in_x = static_connection_signals_pad2soc.xheep.gpio_25_o;
  assign gpio_26_in_x = static_connection_signals_pad2soc.xheep.gpio_26_o;
  assign gpio_27_in_x = static_connection_signals_pad2soc.xheep.gpio_27_o;
  assign gpio_28_in_x = static_connection_signals_pad2soc.xheep.gpio_28_o;
  assign gpio_29_in_x = static_connection_signals_pad2soc.xheep.gpio_29_o;

  assign port_signals_soc2pad.xheep.i2c.i2c_scl_i = i2c_scl_out_x;
  assign port_signals_soc2pad.xheep.i2c.i2c_sda_i = i2c_sda_out_x;
  assign port_signals_soc2pad.xheep.i2c.i2c_scl_oe_i = i2c_scl_oe_x;
  assign port_signals_soc2pad.xheep.i2c.i2c_sda_oe_i = i2c_sda_oe_x;
  assign port_signals_soc2pad.xheep.gpio.gpio_30_i = gpio_30_out_x;
  assign port_signals_soc2pad.xheep.gpio.gpio_31_i = gpio_31_out_x;
  assign port_signals_soc2pad.xheep.gpio.gpio_30_oe_i = gpio_30_oe_x;
  assign port_signals_soc2pad.xheep.gpio.gpio_31_oe_i = gpio_31_oe_x;

  assign i2c_scl_in_x = port_signals_pad2soc.xheep.i2c.i2c_scl_o;
  assign i2c_sda_in_x = port_signals_pad2soc.xheep.i2c.i2c_sda_o;
  assign gpio_30_in_x = port_signals_pad2soc.xheep.gpio.gpio_30_o;
  assign gpio_31_in_x = port_signals_pad2soc.xheep.gpio.gpio_31_o;

  assign clk_out_x = 1'b0;
  assign clk_oe_x = 1'b0;
  assign rst_nout_x = 1'b0;
  assign rst_noe_x = 1'b0;
  assign boot_select_out_x = 1'b0;
  assign boot_select_oe_x = 1'b0;
  assign execute_from_flash_out_x = 1'b0;
  assign execute_from_flash_oe_x = 1'b0;
  assign jtag_tck_out_x = 1'b0;
  assign jtag_tck_oe_x = 1'b0;
  assign jtag_tms_out_x = 1'b0;
  assign jtag_tms_oe_x = 1'b0;
  assign jtag_trst_nout_x = 1'b0;
  assign jtag_trst_noe_x = 1'b0;
  assign jtag_tdi_out_x = 1'b0;
  assign jtag_tdi_oe_x = 1'b0;
  assign jtag_tdo_oe_x = 1'b1;
  assign uart_rx_out_x = 1'b0;
  assign uart_rx_oe_x = 1'b0;
  assign uart_tx_oe_x = 1'b1;
  assign exit_valid_oe_x = 1'b1;

  padframe
   #(
      .req_t(reg_pkg::reg_req_t),
      .resp_t(reg_pkg::reg_rsp_t)
    ) xpadframe (
      .clk_i                               (clk_in_x),
      .rst_ni                              (rst_nin_x),
      .static_connection_signals_pad2soc   (static_connection_signals_pad2soc),
      .static_connection_signals_soc2pad   (static_connection_signals_soc2pad),
      .port_signals_pad2soc                (port_signals_pad2soc),
      .port_signals_soc2pad                (port_signals_soc2pad),
      .pad_xheep_pad_clk_pad               (clk_i),
      .pad_xheep_pad_rst_pad               (rst_ni),
      .pad_xheep_pad_boot_select_pad       (boot_select_i),
      .pad_xheep_pad_execute_from_flash_pad(execute_from_flash_i),
      .pad_xheep_pad_jtag_tck_pad          (jtag_tck_i),
      .pad_xheep_pad_jtag_tms_pad          (jtag_tms_i),
      .pad_xheep_pad_jtag_trst_pad         (jtag_trst_i),
      .pad_xheep_pad_jtag_tdi_pad          (jtag_tdi_i),
      .pad_xheep_pad_jtag_tdo_pad          (jtag_tdo_o),
      .pad_xheep_pad_exit_valid_pad        (exit_valid_o),
      .pad_xheep_pad_uart_rx_pad           (uart_rx_i),
      .pad_xheep_pad_uart_tx_pad           (uart_tx_o),
      .pad_xheep_pad_gpio_00_i_pad         (gpio_0_io),
      .pad_xheep_pad_gpio_01_i_pad         (gpio_1_io),
      .pad_xheep_pad_gpio_02_i_pad         (gpio_2_io),
      .pad_xheep_pad_gpio_03_i_pad         (gpio_3_io),
      .pad_xheep_pad_gpio_04_i_pad         (gpio_4_io),
      .pad_xheep_pad_gpio_05_i_pad         (gpio_5_io),
      .pad_xheep_pad_gpio_06_i_pad         (gpio_6_io),
      .pad_xheep_pad_gpio_07_i_pad         (gpio_7_io),
      .pad_xheep_pad_gpio_08_i_pad         (gpio_8_io),
      .pad_xheep_pad_gpio_09_i_pad         (gpio_9_io),
      .pad_xheep_pad_gpio_10_i_pad         (gpio_10_io),
      .pad_xheep_pad_gpio_11_i_pad         (gpio_11_io),
      .pad_xheep_pad_gpio_12_i_pad         (gpio_12_io),
      .pad_xheep_pad_gpio_13_i_pad         (gpio_13_io),
      .pad_xheep_pad_gpio_14_i_pad         (gpio_14_io),
      .pad_xheep_pad_gpio_15_i_pad         (gpio_15_io),
      .pad_xheep_pad_gpio_16_i_pad         (gpio_16_io),
      .pad_xheep_pad_gpio_17_i_pad         (gpio_17_io),
      .pad_xheep_pad_gpio_18_i_pad         (gpio_18_io),
      .pad_xheep_pad_gpio_19_i_pad         (gpio_19_io),
      .pad_xheep_pad_gpio_20_i_pad         (gpio_20_io),
      .pad_xheep_pad_gpio_21_i_pad         (gpio_21_io),
      .pad_xheep_pad_gpio_22_i_pad         (gpio_22_io),
      .pad_xheep_pad_gpio_23_i_pad         (gpio_23_io),
      .pad_xheep_pad_gpio_24_i_pad         (gpio_24_io),
      .pad_xheep_pad_gpio_25_i_pad         (gpio_25_io),
      .pad_xheep_pad_gpio_26_i_pad         (gpio_26_io),
      .pad_xheep_pad_gpio_27_i_pad         (gpio_27_io),
      .pad_xheep_pad_gpio_28_i_pad         (gpio_28_io),
      .pad_xheep_pad_gpio_29_i_pad         (gpio_29_io),
      .pad_xheep_pad_spi_flash_sck_pad     (spi_flash_sck_io),
      .pad_xheep_pad_spi_flash_cs_00_pad   (spi_flash_cs_0_io),
      .pad_xheep_pad_spi_flash_cs_01_pad   (spi_flash_cs_1_io),   
      .pad_xheep_pad_spi_flash_sd_00_pad   (spi_flash_sd_0_io),
      .pad_xheep_pad_spi_flash_sd_01_pad   (spi_flash_sd_1_io),
      .pad_xheep_pad_spi_flash_sd_02_pad   (spi_flash_sd_2_io),
      .pad_xheep_pad_spi_flash_sd_03_pad   (spi_flash_sd_3_io),
      .pad_xheep_pad_spi_sck_pad           (spi_sck_io),
      .pad_xheep_pad_spi_cs_00_pad         (spi_cs_0_io),
      .pad_xheep_pad_spi_cs_01_pad         (spi_cs_1_io),
      .pad_xheep_pad_spi_sd_00_pad         (spi_sd_0_io),
      .pad_xheep_pad_spi_sd_01_pad         (spi_sd_1_io),
      .pad_xheep_pad_spi_sd_02_pad         (spi_sd_2_io),
      .pad_xheep_pad_spi_sd_03_pad         (spi_sd_3_io),
      .pad_xheep_pad_io_30_pad             (i2c_sda_io),
      .pad_xheep_pad_io_31_pad             (i2c_scl_io),
      .config_req_i                        (pad_req),
      .config_rsp_o                        (pad_resp)
    );

  // always_comb begin
  //   i2c_scl_in_x = 1'b0;
  //   gpio_31_in_x = 1'b0;
  //   unique case (pad_muxes[core_v_mini_mcu_pkg::PAD_I2C_SCL])
  //     0: begin
  //       i2c_scl_out_x_muxed = i2c_scl_out_x;
  //       i2c_scl_oe_x_muxed = i2c_scl_oe_x;
  //       i2c_scl_in_x = i2c_scl_in_x_muxed;
  //     end
  //     1: begin
  //       i2c_scl_out_x_muxed = gpio_31_out_x;
  //       i2c_scl_oe_x_muxed = gpio_31_oe_x;
  //       gpio_31_in_x = i2c_scl_in_x_muxed;
  //     end
  //     default: begin
  //       i2c_scl_out_x_muxed = i2c_scl_out_x;
  //       i2c_scl_oe_x_muxed = i2c_scl_oe_x;
  //       i2c_scl_in_x = i2c_scl_in_x_muxed;
  //     end
  //   endcase
  // end
  // always_comb begin
  //   i2c_sda_in_x = 1'b0;
  //   gpio_30_in_x = 1'b0;
  //   unique case (pad_muxes[core_v_mini_mcu_pkg::PAD_I2C_SDA])
  //     0: begin
  //       i2c_sda_out_x_muxed = i2c_sda_out_x;
  //       i2c_sda_oe_x_muxed = i2c_sda_oe_x;
  //       i2c_sda_in_x = i2c_sda_in_x_muxed;
  //     end
  //     1: begin
  //       i2c_sda_out_x_muxed = gpio_30_out_x;
  //       i2c_sda_oe_x_muxed = gpio_30_oe_x;
  //       gpio_30_in_x = i2c_sda_in_x_muxed;
  //     end
  //     default: begin
  //       i2c_sda_out_x_muxed = i2c_sda_out_x;
  //       i2c_sda_oe_x_muxed = i2c_sda_oe_x;
  //       i2c_sda_in_x = i2c_sda_in_x_muxed;
  //     end
  //   endcase
  // end


  // pad_control #(
  //     .reg_req_t(reg_pkg::reg_req_t),
  //     .reg_rsp_t(reg_pkg::reg_rsp_t),
  //     .NUM_PAD  (core_v_mini_mcu_pkg::NUM_PAD)
  // ) pad_control_i (
  //     .clk_i(clk_in_x),
  //     .rst_ni(rst_ngen),
  //     .reg_req_i(pad_req),
  //     .reg_rsp_o(pad_resp),
  //     .pad_attributes_o(pad_attributes),
  //     .pad_muxes_o(pad_muxes)
  // );

  rstgen rstgen_i (
      .clk_i(clk_in_x),
      .rst_ni(rst_nin_x),
      .test_mode_i(1'b0),
      .rst_no(rst_ngen),
      .init_no()
  );


endmodule  // x_heep_system

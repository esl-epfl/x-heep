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
    inout logic gpio_30_io,
    inout logic gpio_31_io,
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
    inout logic i2c_sda_io,

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
  logic clk_in_x;
  logic rst_nin_x;
  logic boot_select_in_x;
  logic execute_from_flash_in_x;
  logic jtag_tck_in_x;
  logic jtag_tms_in_x;
  logic jtag_trst_nin_x;
  logic jtag_tdi_in_x;
  logic jtag_tdo_out_x;
  logic uart_rx_in_x;
  logic uart_tx_out_x;
  logic exit_valid_out_x;
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
  logic gpio_30_in_x, gpio_30_out_x, gpio_30_oe_x;
  logic gpio_31_in_x, gpio_31_out_x, gpio_31_oe_x;
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
  logic i2c_sda_in_x, i2c_sda_out_x, i2c_sda_oe_x;

  core_v_mini_mcu #(
      .PULP_XPULP(PULP_XPULP),
      .FPU(FPU),
      .PULP_ZFINX(PULP_ZFINX),
      .EXT_XBAR_NMASTER(EXT_XBAR_NMASTER)
  ) core_v_mini_mcu_i (

      .clk_i(clk_in_x),
      .rst_ni(rst_nin_x),
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
      .gpio_30_i(gpio_30_in_x),
      .gpio_30_o(gpio_30_out_x),
      .gpio_30_oe_o(gpio_30_oe_x),
      .gpio_31_i(gpio_31_in_x),
      .gpio_31_o(gpio_31_out_x),
      .gpio_31_oe_o(gpio_31_oe_x),
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
      .i2c_sda_i(i2c_sda_in_x),
      .i2c_sda_o(i2c_sda_out_x),
      .i2c_sda_oe_o(i2c_sda_oe_x),

      //External PADs
      .pad_req_o (pad_reg),
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
      .clk_io(clk_i),
      .clk_o(clk_in_x),
      .rst_nio(rst_ni),
      .rst_no(rst_nin_x),
      .boot_select_io(boot_select_i),
      .boot_select_o(boot_select_in_x),
      .execute_from_flash_io(execute_from_flash_i),
      .execute_from_flash_o(execute_from_flash_in_x),
      .jtag_tck_io(jtag_tck_i),
      .jtag_tck_o(jtag_tck_in_x),
      .jtag_tms_io(jtag_tms_i),
      .jtag_tms_o(jtag_tms_in_x),
      .jtag_trst_nio(jtag_trst_ni),
      .jtag_trst_no(jtag_trst_nin_x),
      .jtag_tdi_io(jtag_tdi_i),
      .jtag_tdi_o(jtag_tdi_in_x),
      .jtag_tdo_io(jtag_tdo_o),
      .jtag_tdo_i(jtag_tdo_out_x),
      .uart_rx_io(uart_rx_i),
      .uart_rx_o(uart_rx_in_x),
      .uart_tx_io(uart_tx_o),
      .uart_tx_i(uart_tx_out_x),
      .exit_valid_io(exit_valid_o),
      .exit_valid_i(exit_valid_out_x),
      .gpio_0_io(gpio_0_io),
      .gpio_0_o(gpio_0_in_x),
      .gpio_0_i(gpio_0_out_x),
      .gpio_0_oe_i(gpio_0_oe_x),
      .gpio_1_io(gpio_1_io),
      .gpio_1_o(gpio_1_in_x),
      .gpio_1_i(gpio_1_out_x),
      .gpio_1_oe_i(gpio_1_oe_x),
      .gpio_2_io(gpio_2_io),
      .gpio_2_o(gpio_2_in_x),
      .gpio_2_i(gpio_2_out_x),
      .gpio_2_oe_i(gpio_2_oe_x),
      .gpio_3_io(gpio_3_io),
      .gpio_3_o(gpio_3_in_x),
      .gpio_3_i(gpio_3_out_x),
      .gpio_3_oe_i(gpio_3_oe_x),
      .gpio_4_io(gpio_4_io),
      .gpio_4_o(gpio_4_in_x),
      .gpio_4_i(gpio_4_out_x),
      .gpio_4_oe_i(gpio_4_oe_x),
      .gpio_5_io(gpio_5_io),
      .gpio_5_o(gpio_5_in_x),
      .gpio_5_i(gpio_5_out_x),
      .gpio_5_oe_i(gpio_5_oe_x),
      .gpio_6_io(gpio_6_io),
      .gpio_6_o(gpio_6_in_x),
      .gpio_6_i(gpio_6_out_x),
      .gpio_6_oe_i(gpio_6_oe_x),
      .gpio_7_io(gpio_7_io),
      .gpio_7_o(gpio_7_in_x),
      .gpio_7_i(gpio_7_out_x),
      .gpio_7_oe_i(gpio_7_oe_x),
      .gpio_8_io(gpio_8_io),
      .gpio_8_o(gpio_8_in_x),
      .gpio_8_i(gpio_8_out_x),
      .gpio_8_oe_i(gpio_8_oe_x),
      .gpio_9_io(gpio_9_io),
      .gpio_9_o(gpio_9_in_x),
      .gpio_9_i(gpio_9_out_x),
      .gpio_9_oe_i(gpio_9_oe_x),
      .gpio_10_io(gpio_10_io),
      .gpio_10_o(gpio_10_in_x),
      .gpio_10_i(gpio_10_out_x),
      .gpio_10_oe_i(gpio_10_oe_x),
      .gpio_11_io(gpio_11_io),
      .gpio_11_o(gpio_11_in_x),
      .gpio_11_i(gpio_11_out_x),
      .gpio_11_oe_i(gpio_11_oe_x),
      .gpio_12_io(gpio_12_io),
      .gpio_12_o(gpio_12_in_x),
      .gpio_12_i(gpio_12_out_x),
      .gpio_12_oe_i(gpio_12_oe_x),
      .gpio_13_io(gpio_13_io),
      .gpio_13_o(gpio_13_in_x),
      .gpio_13_i(gpio_13_out_x),
      .gpio_13_oe_i(gpio_13_oe_x),
      .gpio_14_io(gpio_14_io),
      .gpio_14_o(gpio_14_in_x),
      .gpio_14_i(gpio_14_out_x),
      .gpio_14_oe_i(gpio_14_oe_x),
      .gpio_15_io(gpio_15_io),
      .gpio_15_o(gpio_15_in_x),
      .gpio_15_i(gpio_15_out_x),
      .gpio_15_oe_i(gpio_15_oe_x),
      .gpio_16_io(gpio_16_io),
      .gpio_16_o(gpio_16_in_x),
      .gpio_16_i(gpio_16_out_x),
      .gpio_16_oe_i(gpio_16_oe_x),
      .gpio_17_io(gpio_17_io),
      .gpio_17_o(gpio_17_in_x),
      .gpio_17_i(gpio_17_out_x),
      .gpio_17_oe_i(gpio_17_oe_x),
      .gpio_18_io(gpio_18_io),
      .gpio_18_o(gpio_18_in_x),
      .gpio_18_i(gpio_18_out_x),
      .gpio_18_oe_i(gpio_18_oe_x),
      .gpio_19_io(gpio_19_io),
      .gpio_19_o(gpio_19_in_x),
      .gpio_19_i(gpio_19_out_x),
      .gpio_19_oe_i(gpio_19_oe_x),
      .gpio_20_io(gpio_20_io),
      .gpio_20_o(gpio_20_in_x),
      .gpio_20_i(gpio_20_out_x),
      .gpio_20_oe_i(gpio_20_oe_x),
      .gpio_21_io(gpio_21_io),
      .gpio_21_o(gpio_21_in_x),
      .gpio_21_i(gpio_21_out_x),
      .gpio_21_oe_i(gpio_21_oe_x),
      .gpio_22_io(gpio_22_io),
      .gpio_22_o(gpio_22_in_x),
      .gpio_22_i(gpio_22_out_x),
      .gpio_22_oe_i(gpio_22_oe_x),
      .gpio_23_io(gpio_23_io),
      .gpio_23_o(gpio_23_in_x),
      .gpio_23_i(gpio_23_out_x),
      .gpio_23_oe_i(gpio_23_oe_x),
      .gpio_24_io(gpio_24_io),
      .gpio_24_o(gpio_24_in_x),
      .gpio_24_i(gpio_24_out_x),
      .gpio_24_oe_i(gpio_24_oe_x),
      .gpio_25_io(gpio_25_io),
      .gpio_25_o(gpio_25_in_x),
      .gpio_25_i(gpio_25_out_x),
      .gpio_25_oe_i(gpio_25_oe_x),
      .gpio_26_io(gpio_26_io),
      .gpio_26_o(gpio_26_in_x),
      .gpio_26_i(gpio_26_out_x),
      .gpio_26_oe_i(gpio_26_oe_x),
      .gpio_27_io(gpio_27_io),
      .gpio_27_o(gpio_27_in_x),
      .gpio_27_i(gpio_27_out_x),
      .gpio_27_oe_i(gpio_27_oe_x),
      .gpio_28_io(gpio_28_io),
      .gpio_28_o(gpio_28_in_x),
      .gpio_28_i(gpio_28_out_x),
      .gpio_28_oe_i(gpio_28_oe_x),
      .gpio_29_io(gpio_29_io),
      .gpio_29_o(gpio_29_in_x),
      .gpio_29_i(gpio_29_out_x),
      .gpio_29_oe_i(gpio_29_oe_x),
      .gpio_30_io(gpio_30_io),
      .gpio_30_o(gpio_30_in_x),
      .gpio_30_i(gpio_30_out_x),
      .gpio_30_oe_i(gpio_30_oe_x),
      .gpio_31_io(gpio_31_io),
      .gpio_31_o(gpio_31_in_x),
      .gpio_31_i(gpio_31_out_x),
      .gpio_31_oe_i(gpio_31_oe_x),
      .spi_flash_sck_io(spi_flash_sck_io),
      .spi_flash_sck_o(spi_flash_sck_in_x),
      .spi_flash_sck_i(spi_flash_sck_out_x),
      .spi_flash_sck_oe_i(spi_flash_sck_oe_x),
      .spi_flash_cs_0_io(spi_flash_cs_0_io),
      .spi_flash_cs_0_o(spi_flash_cs_0_in_x),
      .spi_flash_cs_0_i(spi_flash_cs_0_out_x),
      .spi_flash_cs_0_oe_i(spi_flash_cs_0_oe_x),
      .spi_flash_cs_1_io(spi_flash_cs_1_io),
      .spi_flash_cs_1_o(spi_flash_cs_1_in_x),
      .spi_flash_cs_1_i(spi_flash_cs_1_out_x),
      .spi_flash_cs_1_oe_i(spi_flash_cs_1_oe_x),
      .spi_flash_sd_0_io(spi_flash_sd_0_io),
      .spi_flash_sd_0_o(spi_flash_sd_0_in_x),
      .spi_flash_sd_0_i(spi_flash_sd_0_out_x),
      .spi_flash_sd_0_oe_i(spi_flash_sd_0_oe_x),
      .spi_flash_sd_1_io(spi_flash_sd_1_io),
      .spi_flash_sd_1_o(spi_flash_sd_1_in_x),
      .spi_flash_sd_1_i(spi_flash_sd_1_out_x),
      .spi_flash_sd_1_oe_i(spi_flash_sd_1_oe_x),
      .spi_flash_sd_2_io(spi_flash_sd_2_io),
      .spi_flash_sd_2_o(spi_flash_sd_2_in_x),
      .spi_flash_sd_2_i(spi_flash_sd_2_out_x),
      .spi_flash_sd_2_oe_i(spi_flash_sd_2_oe_x),
      .spi_flash_sd_3_io(spi_flash_sd_3_io),
      .spi_flash_sd_3_o(spi_flash_sd_3_in_x),
      .spi_flash_sd_3_i(spi_flash_sd_3_out_x),
      .spi_flash_sd_3_oe_i(spi_flash_sd_3_oe_x),
      .spi_sck_io(spi_sck_io),
      .spi_sck_o(spi_sck_in_x),
      .spi_sck_i(spi_sck_out_x),
      .spi_sck_oe_i(spi_sck_oe_x),
      .spi_cs_0_io(spi_cs_0_io),
      .spi_cs_0_o(spi_cs_0_in_x),
      .spi_cs_0_i(spi_cs_0_out_x),
      .spi_cs_0_oe_i(spi_cs_0_oe_x),
      .spi_cs_1_io(spi_cs_1_io),
      .spi_cs_1_o(spi_cs_1_in_x),
      .spi_cs_1_i(spi_cs_1_out_x),
      .spi_cs_1_oe_i(spi_cs_1_oe_x),
      .spi_sd_0_io(spi_sd_0_io),
      .spi_sd_0_o(spi_sd_0_in_x),
      .spi_sd_0_i(spi_sd_0_out_x),
      .spi_sd_0_oe_i(spi_sd_0_oe_x),
      .spi_sd_1_io(spi_sd_1_io),
      .spi_sd_1_o(spi_sd_1_in_x),
      .spi_sd_1_i(spi_sd_1_out_x),
      .spi_sd_1_oe_i(spi_sd_1_oe_x),
      .spi_sd_2_io(spi_sd_2_io),
      .spi_sd_2_o(spi_sd_2_in_x),
      .spi_sd_2_i(spi_sd_2_out_x),
      .spi_sd_2_oe_i(spi_sd_2_oe_x),
      .spi_sd_3_io(spi_sd_3_io),
      .spi_sd_3_o(spi_sd_3_in_x),
      .spi_sd_3_i(spi_sd_3_out_x),
      .spi_sd_3_oe_i(spi_sd_3_oe_x),
      .i2c_scl_io(i2c_scl_io),
      .i2c_scl_o(i2c_scl_in_x),
      .i2c_scl_i(i2c_scl_out_x),
      .i2c_scl_oe_i(i2c_scl_oe_x),
      .i2c_sda_io(i2c_sda_io),
      .i2c_sda_o(i2c_sda_in_x),
      .i2c_sda_i(i2c_sda_out_x),
      .i2c_sda_oe_i(i2c_sda_oe_x),
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

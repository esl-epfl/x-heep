// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module femu
  import obi_pkg::*;
  import reg_pkg::*;
#(
) (
    inout logic clk_in,
    inout logic rst_i,

    //visibility signals
    output logic rst_led,
    output logic clk_led,
    output logic clk_out,

    inout logic boot_select_i,
    inout logic execute_from_flash_i,

    inout logic [29:0] gpio_io,

    output logic exit_value_o,
    inout  logic exit_valid_o,

    inout logic [3:0] spi_sd_io,
    inout logic spi_csb_o,
    inout logic spi_sck_o,

    inout logic spi2_sd_0_io,
    inout logic spi2_sd_1_io,
    inout logic spi2_sd_2_io,
    inout logic spi2_sd_3_io,
    inout logic [1:0] spi2_csb_o,
    inout logic spi2_sck_o,

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
  logic clk_in_x,clk_out_x,clk_oe_x;

  logic rst_nin_x,rst_nout_x,rst_noe_x;

  logic boot_select_in_x,boot_select_out_x,boot_select_oe_x;

  logic execute_from_flash_in_x,execute_from_flash_out_x,execute_from_flash_oe_x;

  logic jtag_tck_in_x,jtag_tck_out_x,jtag_tck_oe_x;

  logic jtag_tms_in_x,jtag_tms_out_x,jtag_tms_oe_x;

  logic jtag_trst_nin_x,jtag_trst_nout_x,jtag_trst_noe_x;

  logic jtag_tdi_in_x,jtag_tdi_out_x,jtag_tdi_oe_x;

  logic jtag_tdo_in_x,jtag_tdo_out_x,jtag_tdo_oe_x;

  logic uart_rx_in_x,uart_rx_out_x,uart_rx_oe_x;

  logic uart_tx_in_x,uart_tx_out_x,uart_tx_oe_x;

  logic exit_valid_in_x,exit_valid_out_x,exit_valid_oe_x;

  logic gpio_0_in_x,gpio_0_out_x,gpio_0_oe_x;

  logic gpio_1_in_x,gpio_1_out_x,gpio_1_oe_x;

  logic gpio_2_in_x,gpio_2_out_x,gpio_2_oe_x;

  logic gpio_3_in_x,gpio_3_out_x,gpio_3_oe_x;

  logic gpio_4_in_x,gpio_4_out_x,gpio_4_oe_x;

  logic gpio_5_in_x,gpio_5_out_x,gpio_5_oe_x;

  logic gpio_6_in_x,gpio_6_out_x,gpio_6_oe_x;

  logic gpio_7_in_x,gpio_7_out_x,gpio_7_oe_x;

  logic gpio_8_in_x,gpio_8_out_x,gpio_8_oe_x;

  logic gpio_9_in_x,gpio_9_out_x,gpio_9_oe_x;

  logic gpio_10_in_x,gpio_10_out_x,gpio_10_oe_x;

  logic gpio_11_in_x,gpio_11_out_x,gpio_11_oe_x;

  logic gpio_12_in_x,gpio_12_out_x,gpio_12_oe_x;

  logic gpio_13_in_x,gpio_13_out_x,gpio_13_oe_x;

  logic gpio_14_in_x,gpio_14_out_x,gpio_14_oe_x;

  logic gpio_15_in_x,gpio_15_out_x,gpio_15_oe_x;

  logic gpio_16_in_x,gpio_16_out_x,gpio_16_oe_x;

  logic gpio_17_in_x,gpio_17_out_x,gpio_17_oe_x;

  logic gpio_18_in_x,gpio_18_out_x,gpio_18_oe_x;

  logic gpio_19_in_x,gpio_19_out_x,gpio_19_oe_x;

  logic gpio_20_in_x,gpio_20_out_x,gpio_20_oe_x;

  logic gpio_21_in_x,gpio_21_out_x,gpio_21_oe_x;

  logic gpio_22_in_x,gpio_22_out_x,gpio_22_oe_x;

  logic spi_flash_sck_in_x,spi_flash_sck_out_x,spi_flash_sck_oe_x;

  logic spi_flash_cs_0_in_x,spi_flash_cs_0_out_x,spi_flash_cs_0_oe_x;

  logic spi_flash_cs_1_in_x,spi_flash_cs_1_out_x,spi_flash_cs_1_oe_x;

  logic spi_flash_sd_0_in_x,spi_flash_sd_0_out_x,spi_flash_sd_0_oe_x;

  logic spi_flash_sd_1_in_x,spi_flash_sd_1_out_x,spi_flash_sd_1_oe_x;

  logic spi_flash_sd_2_in_x,spi_flash_sd_2_out_x,spi_flash_sd_2_oe_x;

  logic spi_flash_sd_3_in_x,spi_flash_sd_3_out_x,spi_flash_sd_3_oe_x;

  logic spi_sck_in_x,spi_sck_out_x,spi_sck_oe_x;

  logic spi_cs_0_in_x,spi_cs_0_out_x,spi_cs_0_oe_x;

  logic spi_cs_1_in_x,spi_cs_1_out_x,spi_cs_1_oe_x;

  logic spi_sd_0_in_x,spi_sd_0_out_x,spi_sd_0_oe_x;

  logic spi_sd_1_in_x,spi_sd_1_out_x,spi_sd_1_oe_x;

  logic spi_sd_2_in_x,spi_sd_2_out_x,spi_sd_2_oe_x;

  logic spi_sd_3_in_x,spi_sd_3_out_x,spi_sd_3_oe_x;

  logic spi2_cs_0_in_x,spi2_cs_0_out_x,spi2_cs_0_oe_x;
  logic gpio_23_in_x,gpio_23_out_x,gpio_23_oe_x;
  logic spi2_cs_0_in_x_muxed,spi2_cs_0_out_x_muxed,spi2_cs_0_oe_x_muxed;

  logic spi2_cs_1_in_x,spi2_cs_1_out_x,spi2_cs_1_oe_x;
  logic gpio_24_in_x,gpio_24_out_x,gpio_24_oe_x;
  logic spi2_cs_1_in_x_muxed,spi2_cs_1_out_x_muxed,spi2_cs_1_oe_x_muxed;

  logic spi2_sck_in_x,spi2_sck_out_x,spi2_sck_oe_x;
  logic gpio_25_in_x,gpio_25_out_x,gpio_25_oe_x;
  logic spi2_sck_in_x_muxed,spi2_sck_out_x_muxed,spi2_sck_oe_x_muxed;

  logic spi2_sd_0_in_x,spi2_sd_0_out_x,spi2_sd_0_oe_x;
  logic gpio_26_in_x,gpio_26_out_x,gpio_26_oe_x;
  logic spi2_sd_0_in_x_muxed,spi2_sd_0_out_x_muxed,spi2_sd_0_oe_x_muxed;

  logic spi2_sd_1_in_x,spi2_sd_1_out_x,spi2_sd_1_oe_x;
  logic gpio_27_in_x,gpio_27_out_x,gpio_27_oe_x;
  logic spi2_sd_1_in_x_muxed,spi2_sd_1_out_x_muxed,spi2_sd_1_oe_x_muxed;

  logic spi2_sd_2_in_x,spi2_sd_2_out_x,spi2_sd_2_oe_x;
  logic gpio_28_in_x,gpio_28_out_x,gpio_28_oe_x;
  logic spi2_sd_2_in_x_muxed,spi2_sd_2_out_x_muxed,spi2_sd_2_oe_x_muxed;

  logic spi2_sd_3_in_x,spi2_sd_3_out_x,spi2_sd_3_oe_x;
  logic gpio_29_in_x,gpio_29_out_x,gpio_29_oe_x;
  logic spi2_sd_3_in_x_muxed,spi2_sd_3_out_x_muxed,spi2_sd_3_oe_x_muxed;

  logic i2c_scl_in_x,i2c_scl_out_x,i2c_scl_oe_x;
  logic gpio_31_in_x,gpio_31_out_x,gpio_31_oe_x;
  logic i2c_scl_in_x_muxed,i2c_scl_out_x_muxed,i2c_scl_oe_x_muxed;

  logic i2c_sda_in_x,i2c_sda_out_x,i2c_sda_oe_x;
  logic gpio_30_in_x,gpio_30_out_x,gpio_30_oe_x;
  logic i2c_sda_in_x_muxed,i2c_sda_out_x_muxed,i2c_sda_oe_x_muxed;


  wire                               clk_gen;
  logic [                      31:0] exit_value;
  wire                               rst_n;
  logic [CLK_LED_COUNT_LENGTH - 1:0] clk_count;

  // low active reset
  assign rst_n   = !rst_i;

  // reset LED for debugging
  assign rst_led = rst_n;

  // counter to blink an LED
  assign clk_led = clk_count[CLK_LED_COUNT_LENGTH-1];

  always_ff @(posedge clk_gen or negedge rst_n) begin : clk_count_process
    if (!rst_n) begin
      clk_count <= '0;
    end else begin
      clk_count <= clk_count + 1;
    end
  end

  // clock output for debugging
  assign clk_out = clk_gen;

  xilinx_clk_wizard_wrapper xilinx_clk_wizard_wrapper_i (
      .clk_125MHz(clk_i),
      .clk_out1_0(clk_gen)
  );

  logic clk_i;
  assign clk_i = clk_gen;

  core_v_mini_mcu #(
    .PULP_XPULP(PULP_XPULP),
    .FPU(FPU),
    .PULP_ZFINX(PULP_ZFINX),
    .EXT_XBAR_NMASTER(EXT_XBAR_NMASTER)
  ) core_v_mini_mcu_i (

    .rst_ni(rst_ngen),
    .clk_i(clk_in_x),


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

    .spi2_cs_0_i(spi2_cs_0_in_x),
    .spi2_cs_0_o(spi2_cs_0_out_x),
    .spi2_cs_0_oe_o(spi2_cs_0_oe_x),
    .gpio_23_i(gpio_23_in_x),
    .gpio_23_o(gpio_23_out_x),
    .gpio_23_oe_o(gpio_23_oe_x),

    .spi2_cs_1_i(spi2_cs_1_in_x),
    .spi2_cs_1_o(spi2_cs_1_out_x),
    .spi2_cs_1_oe_o(spi2_cs_1_oe_x),
    .gpio_24_i(gpio_24_in_x),
    .gpio_24_o(gpio_24_out_x),
    .gpio_24_oe_o(gpio_24_oe_x),

    .spi2_sck_i(spi2_sck_in_x),
    .spi2_sck_o(spi2_sck_out_x),
    .spi2_sck_oe_o(spi2_sck_oe_x),
    .gpio_25_i(gpio_25_in_x),
    .gpio_25_o(gpio_25_out_x),
    .gpio_25_oe_o(gpio_25_oe_x),

    .spi2_sd_0_i(spi2_sd_0_in_x),
    .spi2_sd_0_o(spi2_sd_0_out_x),
    .spi2_sd_0_oe_o(spi2_sd_0_oe_x),
    .gpio_26_i(gpio_26_in_x),
    .gpio_26_o(gpio_26_out_x),
    .gpio_26_oe_o(gpio_26_oe_x),

    .spi2_sd_1_i(spi2_sd_1_in_x),
    .spi2_sd_1_o(spi2_sd_1_out_x),
    .spi2_sd_1_oe_o(spi2_sd_1_oe_x),
    .gpio_27_i(gpio_27_in_x),
    .gpio_27_o(gpio_27_out_x),
    .gpio_27_oe_o(gpio_27_oe_x),

    .spi2_sd_2_i(spi2_sd_2_in_x),
    .spi2_sd_2_o(spi2_sd_2_out_x),
    .spi2_sd_2_oe_o(spi2_sd_2_oe_x),
    .gpio_28_i(gpio_28_in_x),
    .gpio_28_o(gpio_28_out_x),
    .gpio_28_oe_o(gpio_28_oe_x),

    .spi2_sd_3_i(spi2_sd_3_in_x),
    .spi2_sd_3_o(spi2_sd_3_out_x),
    .spi2_sd_3_oe_o(spi2_sd_3_oe_x),
    .gpio_29_i(gpio_29_in_x),
    .gpio_29_o(gpio_29_out_x),
    .gpio_29_oe_o(gpio_29_oe_x),

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

    .intr_vector_ext_i('0),
    .ext_xbar_master_req_i('0),
    .ext_xbar_master_resp_o(),
    .ext_xbar_slave_req_o(),
    .ext_xbar_slave_resp_i('0),
    .ext_peripheral_slave_req_o(),
    .ext_peripheral_slave_resp_i('0),
    .external_subsystem_powergate_switch_o(),
    .external_subsystem_powergate_switch_ack_i(),
    .external_subsystem_powergate_iso_o(),
    .external_subsystem_rst_no(),
    .external_ram_banks_set_retentive_o(),
    .exit_value_o(exit_value),
    .pad_req_o(pad_req),
    .pad_resp_i(pad_resp),
    .cpu_subsystem_powergate_switch_o(cpu_subsystem_powergate_switch),
    .cpu_subsystem_powergate_switch_ack_i(cpu_subsystem_powergate_switch_ack),
    .peripheral_subsystem_powergate_switch_o(peripheral_subsystem_powergate_switch),
    .peripheral_subsystem_powergate_switch_ack_i(peripheral_subsystem_powergate_switch_ack),
    .memory_subsystem_banks_powergate_switch_o(memory_subsystem_banks_powergate_switch),
    .memory_subsystem_banks_powergate_switch_ack_i(memory_subsystem_banks_powergate_switch_ack),
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
    .spi2_cs_0_io(spi2_cs_0_io),
    .spi2_cs_0_o(spi2_cs_0_in_x_muxed),
    .spi2_cs_0_i(spi2_cs_0_out_x_muxed),
    .spi2_cs_0_oe_i(spi2_cs_0_oe_x_muxed),
    .spi2_cs_1_io(spi2_cs_1_io),
    .spi2_cs_1_o(spi2_cs_1_in_x_muxed),
    .spi2_cs_1_i(spi2_cs_1_out_x_muxed),
    .spi2_cs_1_oe_i(spi2_cs_1_oe_x_muxed),
    .spi2_sck_io(spi2_sck_io),
    .spi2_sck_o(spi2_sck_in_x_muxed),
    .spi2_sck_i(spi2_sck_out_x_muxed),
    .spi2_sck_oe_i(spi2_sck_oe_x_muxed),
    .spi2_sd_0_io(spi2_sd_0_io),
    .spi2_sd_0_o(spi2_sd_0_in_x_muxed),
    .spi2_sd_0_i(spi2_sd_0_out_x_muxed),
    .spi2_sd_0_oe_i(spi2_sd_0_oe_x_muxed),
    .spi2_sd_1_io(spi2_sd_1_io),
    .spi2_sd_1_o(spi2_sd_1_in_x_muxed),
    .spi2_sd_1_i(spi2_sd_1_out_x_muxed),
    .spi2_sd_1_oe_i(spi2_sd_1_oe_x_muxed),
    .spi2_sd_2_io(spi2_sd_2_io),
    .spi2_sd_2_o(spi2_sd_2_in_x_muxed),
    .spi2_sd_2_i(spi2_sd_2_out_x_muxed),
    .spi2_sd_2_oe_i(spi2_sd_2_oe_x_muxed),
    .spi2_sd_3_io(spi2_sd_3_io),
    .spi2_sd_3_o(spi2_sd_3_in_x_muxed),
    .spi2_sd_3_i(spi2_sd_3_out_x_muxed),
    .spi2_sd_3_oe_i(spi2_sd_3_oe_x_muxed),
    .i2c_scl_io(i2c_scl_io),
    .i2c_scl_o(i2c_scl_in_x_muxed),
    .i2c_scl_i(i2c_scl_out_x_muxed),
    .i2c_scl_oe_i(i2c_scl_oe_x_muxed),
    .i2c_sda_io(i2c_sda_io),
    .i2c_sda_o(i2c_sda_in_x_muxed),
    .i2c_sda_i(i2c_sda_out_x_muxed),
    .i2c_sda_oe_i(i2c_sda_oe_x_muxed),
    .pad_attributes_i(pad_attributes)
  );

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


  always_comb
  begin
   spi2_cs_0_in_x=1'b0;
   gpio_23_in_x=1'b0;
   unique case(pad_muxes[core_v_mini_mcu_pkg::PAD_SPI2_CS_0])
    0: begin
      spi2_cs_0_out_x_muxed = spi2_cs_0_out_x;
      spi2_cs_0_oe_x_muxed = spi2_cs_0_oe_x;
      spi2_cs_0_in_x = spi2_cs_0_in_x_muxed;
    end
    1: begin
      spi2_cs_0_out_x_muxed = gpio_23_out_x;
      spi2_cs_0_oe_x_muxed = gpio_23_oe_x;
      gpio_23_in_x = spi2_cs_0_in_x_muxed;
    end
    default: begin
      spi2_cs_0_out_x_muxed = spi2_cs_0_out_x;
      spi2_cs_0_oe_x_muxed = spi2_cs_0_oe_x;
      spi2_cs_0_in_x = spi2_cs_0_in_x_muxed;
    end
   endcase
  end
  always_comb
  begin
   spi2_cs_1_in_x=1'b0;
   gpio_24_in_x=1'b0;
   unique case(pad_muxes[core_v_mini_mcu_pkg::PAD_SPI2_CS_1])
    0: begin
      spi2_cs_1_out_x_muxed = spi2_cs_1_out_x;
      spi2_cs_1_oe_x_muxed = spi2_cs_1_oe_x;
      spi2_cs_1_in_x = spi2_cs_1_in_x_muxed;
    end
    1: begin
      spi2_cs_1_out_x_muxed = gpio_24_out_x;
      spi2_cs_1_oe_x_muxed = gpio_24_oe_x;
      gpio_24_in_x = spi2_cs_1_in_x_muxed;
    end
    default: begin
      spi2_cs_1_out_x_muxed = spi2_cs_1_out_x;
      spi2_cs_1_oe_x_muxed = spi2_cs_1_oe_x;
      spi2_cs_1_in_x = spi2_cs_1_in_x_muxed;
    end
   endcase
  end
  always_comb
  begin
   spi2_sck_in_x=1'b0;
   gpio_25_in_x=1'b0;
   unique case(pad_muxes[core_v_mini_mcu_pkg::PAD_SPI2_SCK])
    0: begin
      spi2_sck_out_x_muxed = spi2_sck_out_x;
      spi2_sck_oe_x_muxed = spi2_sck_oe_x;
      spi2_sck_in_x = spi2_sck_in_x_muxed;
    end
    1: begin
      spi2_sck_out_x_muxed = gpio_25_out_x;
      spi2_sck_oe_x_muxed = gpio_25_oe_x;
      gpio_25_in_x = spi2_sck_in_x_muxed;
    end
    default: begin
      spi2_sck_out_x_muxed = spi2_sck_out_x;
      spi2_sck_oe_x_muxed = spi2_sck_oe_x;
      spi2_sck_in_x = spi2_sck_in_x_muxed;
    end
   endcase
  end
  always_comb
  begin
   spi2_sd_0_in_x=1'b0;
   gpio_26_in_x=1'b0;
   unique case(pad_muxes[core_v_mini_mcu_pkg::PAD_SPI2_SD_0])
    0: begin
      spi2_sd_0_out_x_muxed = spi2_sd_0_out_x;
      spi2_sd_0_oe_x_muxed = spi2_sd_0_oe_x;
      spi2_sd_0_in_x = spi2_sd_0_in_x_muxed;
    end
    1: begin
      spi2_sd_0_out_x_muxed = gpio_26_out_x;
      spi2_sd_0_oe_x_muxed = gpio_26_oe_x;
      gpio_26_in_x = spi2_sd_0_in_x_muxed;
    end
    default: begin
      spi2_sd_0_out_x_muxed = spi2_sd_0_out_x;
      spi2_sd_0_oe_x_muxed = spi2_sd_0_oe_x;
      spi2_sd_0_in_x = spi2_sd_0_in_x_muxed;
    end
   endcase
  end
  always_comb
  begin
   spi2_sd_1_in_x=1'b0;
   gpio_27_in_x=1'b0;
   unique case(pad_muxes[core_v_mini_mcu_pkg::PAD_SPI2_SD_1])
    0: begin
      spi2_sd_1_out_x_muxed = spi2_sd_1_out_x;
      spi2_sd_1_oe_x_muxed = spi2_sd_1_oe_x;
      spi2_sd_1_in_x = spi2_sd_1_in_x_muxed;
    end
    1: begin
      spi2_sd_1_out_x_muxed = gpio_27_out_x;
      spi2_sd_1_oe_x_muxed = gpio_27_oe_x;
      gpio_27_in_x = spi2_sd_1_in_x_muxed;
    end
    default: begin
      spi2_sd_1_out_x_muxed = spi2_sd_1_out_x;
      spi2_sd_1_oe_x_muxed = spi2_sd_1_oe_x;
      spi2_sd_1_in_x = spi2_sd_1_in_x_muxed;
    end
   endcase
  end
  always_comb
  begin
   spi2_sd_2_in_x=1'b0;
   gpio_28_in_x=1'b0;
   unique case(pad_muxes[core_v_mini_mcu_pkg::PAD_SPI2_SD_2])
    0: begin
      spi2_sd_2_out_x_muxed = spi2_sd_2_out_x;
      spi2_sd_2_oe_x_muxed = spi2_sd_2_oe_x;
      spi2_sd_2_in_x = spi2_sd_2_in_x_muxed;
    end
    1: begin
      spi2_sd_2_out_x_muxed = gpio_28_out_x;
      spi2_sd_2_oe_x_muxed = gpio_28_oe_x;
      gpio_28_in_x = spi2_sd_2_in_x_muxed;
    end
    default: begin
      spi2_sd_2_out_x_muxed = spi2_sd_2_out_x;
      spi2_sd_2_oe_x_muxed = spi2_sd_2_oe_x;
      spi2_sd_2_in_x = spi2_sd_2_in_x_muxed;
    end
   endcase
  end
  always_comb
  begin
   spi2_sd_3_in_x=1'b0;
   gpio_29_in_x=1'b0;
   unique case(pad_muxes[core_v_mini_mcu_pkg::PAD_SPI2_SD_3])
    0: begin
      spi2_sd_3_out_x_muxed = spi2_sd_3_out_x;
      spi2_sd_3_oe_x_muxed = spi2_sd_3_oe_x;
      spi2_sd_3_in_x = spi2_sd_3_in_x_muxed;
    end
    1: begin
      spi2_sd_3_out_x_muxed = gpio_29_out_x;
      spi2_sd_3_oe_x_muxed = gpio_29_oe_x;
      gpio_29_in_x = spi2_sd_3_in_x_muxed;
    end
    default: begin
      spi2_sd_3_out_x_muxed = spi2_sd_3_out_x;
      spi2_sd_3_oe_x_muxed = spi2_sd_3_oe_x;
      spi2_sd_3_in_x = spi2_sd_3_in_x_muxed;
    end
   endcase
  end
  always_comb
  begin
   i2c_scl_in_x=1'b0;
   gpio_31_in_x=1'b0;
   unique case(pad_muxes[core_v_mini_mcu_pkg::PAD_I2C_SCL])
    0: begin
      i2c_scl_out_x_muxed = i2c_scl_out_x;
      i2c_scl_oe_x_muxed = i2c_scl_oe_x;
      i2c_scl_in_x = i2c_scl_in_x_muxed;
    end
    1: begin
      i2c_scl_out_x_muxed = gpio_31_out_x;
      i2c_scl_oe_x_muxed = gpio_31_oe_x;
      gpio_31_in_x = i2c_scl_in_x_muxed;
    end
    default: begin
      i2c_scl_out_x_muxed = i2c_scl_out_x;
      i2c_scl_oe_x_muxed = i2c_scl_oe_x;
      i2c_scl_in_x = i2c_scl_in_x_muxed;
    end
   endcase
  end
  always_comb
  begin
   i2c_sda_in_x=1'b0;
   gpio_30_in_x=1'b0;
   unique case(pad_muxes[core_v_mini_mcu_pkg::PAD_I2C_SDA])
    0: begin
      i2c_sda_out_x_muxed = i2c_sda_out_x;
      i2c_sda_oe_x_muxed = i2c_sda_oe_x;
      i2c_sda_in_x = i2c_sda_in_x_muxed;
    end
    1: begin
      i2c_sda_out_x_muxed = gpio_30_out_x;
      i2c_sda_oe_x_muxed = gpio_30_oe_x;
      gpio_30_in_x = i2c_sda_in_x_muxed;
    end
    default: begin
      i2c_sda_out_x_muxed = i2c_sda_out_x;
      i2c_sda_oe_x_muxed = i2c_sda_oe_x;
      i2c_sda_in_x = i2c_sda_in_x_muxed;
    end
   endcase
  end


  pad_control #(
      .reg_req_t(reg_pkg::reg_req_t),
      .reg_rsp_t(reg_pkg::reg_rsp_t),
      .NUM_PAD  (core_v_mini_mcu_pkg::NUM_PAD)
  ) pad_control_i (
      .clk_i(clk_in_x),
      .rst_ni(rst_ngen),
      .reg_req_i(pad_req),
      .reg_rsp_o(pad_resp),
      .pad_attributes_o(pad_attributes),
      .pad_muxes_o(pad_muxes)
  );

  rstgen rstgen_i (
    .clk_i(clk_in_x),
    .rst_ni(rst_n),
    .test_mode_i(1'b0),
    .rst_no(rst_ngen),
    .init_no()
  );


  assign exit_value_o = exit_value[0];
endmodule  // x_heep_system

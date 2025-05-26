// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module pad_ring (
    inout wire clk_io,
    output logic clk_o,
    inout wire rst_nio,
    output logic rst_no,
    inout wire boot_select_io,
    output logic boot_select_o,
    inout wire execute_from_flash_io,
    output logic execute_from_flash_o,
    inout wire jtag_tck_io,
    output logic jtag_tck_o,
    inout wire jtag_tms_io,
    output logic jtag_tms_o,
    inout wire jtag_trst_nio,
    output logic jtag_trst_no,
    inout wire jtag_tdi_io,
    output logic jtag_tdi_o,
    inout wire jtag_tdo_io,
    input logic jtag_tdo_i,
    inout wire uart_rx_io,
    output logic uart_rx_o,
    inout wire uart_tx_io,
    input logic uart_tx_i,
    inout wire exit_valid_io,
    input logic exit_valid_i,
    inout wire gpio_0_io,
    input logic gpio_0_i,
    output logic gpio_0_o,
    input logic gpio_0_oe_i,
    inout wire gpio_1_io,
    input logic gpio_1_i,
    output logic gpio_1_o,
    input logic gpio_1_oe_i,
    inout wire gpio_2_io,
    input logic gpio_2_i,
    output logic gpio_2_o,
    input logic gpio_2_oe_i,
    inout wire gpio_3_io,
    input logic gpio_3_i,
    output logic gpio_3_o,
    input logic gpio_3_oe_i,
    inout wire gpio_4_io,
    input logic gpio_4_i,
    output logic gpio_4_o,
    input logic gpio_4_oe_i,
    inout wire gpio_5_io,
    input logic gpio_5_i,
    output logic gpio_5_o,
    input logic gpio_5_oe_i,
    inout wire gpio_6_io,
    input logic gpio_6_i,
    output logic gpio_6_o,
    input logic gpio_6_oe_i,
    inout wire gpio_7_io,
    input logic gpio_7_i,
    output logic gpio_7_o,
    input logic gpio_7_oe_i,
    inout wire gpio_8_io,
    input logic gpio_8_i,
    output logic gpio_8_o,
    input logic gpio_8_oe_i,
    inout wire gpio_9_io,
    input logic gpio_9_i,
    output logic gpio_9_o,
    input logic gpio_9_oe_i,
    inout wire gpio_10_io,
    input logic gpio_10_i,
    output logic gpio_10_o,
    input logic gpio_10_oe_i,
    inout wire gpio_11_io,
    input logic gpio_11_i,
    output logic gpio_11_o,
    input logic gpio_11_oe_i,
    inout wire gpio_12_io,
    input logic gpio_12_i,
    output logic gpio_12_o,
    input logic gpio_12_oe_i,
    inout wire gpio_13_io,
    input logic gpio_13_i,
    output logic gpio_13_o,
    input logic gpio_13_oe_i,
    inout wire spi_flash_sck_io,
    input logic spi_flash_sck_i,
    output logic spi_flash_sck_o,
    input logic spi_flash_sck_oe_i,
    inout wire spi_flash_cs_0_io,
    input logic spi_flash_cs_0_i,
    output logic spi_flash_cs_0_o,
    input logic spi_flash_cs_0_oe_i,
    inout wire spi_flash_cs_1_io,
    input logic spi_flash_cs_1_i,
    output logic spi_flash_cs_1_o,
    input logic spi_flash_cs_1_oe_i,
    inout wire spi_flash_sd_0_io,
    input logic spi_flash_sd_0_i,
    output logic spi_flash_sd_0_o,
    input logic spi_flash_sd_0_oe_i,
    inout wire spi_flash_sd_1_io,
    input logic spi_flash_sd_1_i,
    output logic spi_flash_sd_1_o,
    input logic spi_flash_sd_1_oe_i,
    inout wire spi_flash_sd_2_io,
    input logic spi_flash_sd_2_i,
    output logic spi_flash_sd_2_o,
    input logic spi_flash_sd_2_oe_i,
    inout wire spi_flash_sd_3_io,
    input logic spi_flash_sd_3_i,
    output logic spi_flash_sd_3_o,
    input logic spi_flash_sd_3_oe_i,
    inout wire spi_sck_io,
    input logic spi_sck_i,
    output logic spi_sck_o,
    input logic spi_sck_oe_i,
    inout wire spi_cs_0_io,
    input logic spi_cs_0_i,
    output logic spi_cs_0_o,
    input logic spi_cs_0_oe_i,
    inout wire spi_cs_1_io,
    input logic spi_cs_1_i,
    output logic spi_cs_1_o,
    input logic spi_cs_1_oe_i,
    inout wire spi_sd_0_io,
    input logic spi_sd_0_i,
    output logic spi_sd_0_o,
    input logic spi_sd_0_oe_i,
    inout wire spi_sd_1_io,
    input logic spi_sd_1_i,
    output logic spi_sd_1_o,
    input logic spi_sd_1_oe_i,
    inout wire spi_sd_2_io,
    input logic spi_sd_2_i,
    output logic spi_sd_2_o,
    input logic spi_sd_2_oe_i,
    inout wire spi_sd_3_io,
    input logic spi_sd_3_i,
    output logic spi_sd_3_o,
    input logic spi_sd_3_oe_i,
    inout wire spi_slave_sck_io,
    input logic spi_slave_sck_i,
    output logic spi_slave_sck_o,
    input logic spi_slave_sck_oe_i,
    inout wire spi_slave_cs_io,
    input logic spi_slave_cs_i,
    output logic spi_slave_cs_o,
    input logic spi_slave_cs_oe_i,
    inout wire spi_slave_miso_io,
    input logic spi_slave_miso_i,
    output logic spi_slave_miso_o,
    input logic spi_slave_miso_oe_i,
    inout wire spi_slave_mosi_io,
    input logic spi_slave_mosi_i,
    output logic spi_slave_mosi_o,
    input logic spi_slave_mosi_oe_i,
    inout wire pdm2pcm_pdm_io,
    input logic pdm2pcm_pdm_i,
    output logic pdm2pcm_pdm_o,
    input logic pdm2pcm_pdm_oe_i,
    inout wire pdm2pcm_clk_io,
    input logic pdm2pcm_clk_i,
    output logic pdm2pcm_clk_o,
    input logic pdm2pcm_clk_oe_i,
    inout wire i2s_sck_io,
    input logic i2s_sck_i,
    output logic i2s_sck_o,
    input logic i2s_sck_oe_i,
    inout wire i2s_ws_io,
    input logic i2s_ws_i,
    output logic i2s_ws_o,
    input logic i2s_ws_oe_i,
    inout wire i2s_sd_io,
    input logic i2s_sd_i,
    output logic i2s_sd_o,
    input logic i2s_sd_oe_i,
    inout wire spi2_cs_0_io,
    input logic spi2_cs_0_i,
    output logic spi2_cs_0_o,
    input logic spi2_cs_0_oe_i,
    inout wire spi2_cs_1_io,
    input logic spi2_cs_1_i,
    output logic spi2_cs_1_o,
    input logic spi2_cs_1_oe_i,
    inout wire spi2_sck_io,
    input logic spi2_sck_i,
    output logic spi2_sck_o,
    input logic spi2_sck_oe_i,
    inout wire spi2_sd_0_io,
    input logic spi2_sd_0_i,
    output logic spi2_sd_0_o,
    input logic spi2_sd_0_oe_i,
    inout wire spi2_sd_1_io,
    input logic spi2_sd_1_i,
    output logic spi2_sd_1_o,
    input logic spi2_sd_1_oe_i,
    inout wire spi2_sd_2_io,
    input logic spi2_sd_2_i,
    output logic spi2_sd_2_o,
    input logic spi2_sd_2_oe_i,
    inout wire spi2_sd_3_io,
    input logic spi2_sd_3_i,
    output logic spi2_sd_3_o,
    input logic spi2_sd_3_oe_i,
    inout wire i2c_scl_io,
    input logic i2c_scl_i,
    output logic i2c_scl_o,
    input logic i2c_scl_oe_i,
    inout wire i2c_sda_io,
    input logic i2c_sda_i,
    output logic i2c_sda_o,
    input logic i2c_sda_oe_i,


    // here just for simplicity
    /* verilator lint_off UNUSED */
    input logic [core_v_mini_mcu_pkg::NUM_PAD-1:0][0:0] pad_attributes_i

);

pad_cell_input #(.PADATTR(0)) pad_clk_i (
   .pad_in_i(1'b0),
   .pad_oe_i(1'b0),
   .pad_out_o(clk_o),
   .pad_io(clk_io),
   .pad_attributes_i('0));


pad_cell_input #(.PADATTR(0)) pad_rst_i (
   .pad_in_i(1'b0),
   .pad_oe_i(1'b0),
   .pad_out_o(rst_no),
   .pad_io(rst_nio),
   .pad_attributes_i('0));


pad_cell_input #(.PADATTR(0)) pad_boot_select_i (
   .pad_in_i(1'b0),
   .pad_oe_i(1'b0),
   .pad_out_o(boot_select_o),
   .pad_io(boot_select_io),
   .pad_attributes_i('0));


pad_cell_input #(.PADATTR(0)) pad_execute_from_flash_i (
   .pad_in_i(1'b0),
   .pad_oe_i(1'b0),
   .pad_out_o(execute_from_flash_o),
   .pad_io(execute_from_flash_io),
   .pad_attributes_i('0));


pad_cell_input #(.PADATTR(0)) pad_jtag_tck_i (
   .pad_in_i(1'b0),
   .pad_oe_i(1'b0),
   .pad_out_o(jtag_tck_o),
   .pad_io(jtag_tck_io),
   .pad_attributes_i('0));


pad_cell_input #(.PADATTR(0)) pad_jtag_tms_i (
   .pad_in_i(1'b0),
   .pad_oe_i(1'b0),
   .pad_out_o(jtag_tms_o),
   .pad_io(jtag_tms_io),
   .pad_attributes_i('0));


pad_cell_input #(.PADATTR(0)) pad_jtag_trst_i (
   .pad_in_i(1'b0),
   .pad_oe_i(1'b0),
   .pad_out_o(jtag_trst_no),
   .pad_io(jtag_trst_nio),
   .pad_attributes_i('0));


pad_cell_input #(.PADATTR(0)) pad_jtag_tdi_i (
   .pad_in_i(1'b0),
   .pad_oe_i(1'b0),
   .pad_out_o(jtag_tdi_o),
   .pad_io(jtag_tdi_io),
   .pad_attributes_i('0));


pad_cell_output #(.PADATTR(0)) pad_jtag_tdo_i (
   .pad_in_i(jtag_tdo_i),
   .pad_oe_i(1'b1),
   .pad_out_o(),
   .pad_io(jtag_tdo_io),
   .pad_attributes_i('0));


pad_cell_input #(.PADATTR(0)) pad_uart_rx_i (
   .pad_in_i(1'b0),
   .pad_oe_i(1'b0),
   .pad_out_o(uart_rx_o),
   .pad_io(uart_rx_io),
   .pad_attributes_i('0));


pad_cell_output #(.PADATTR(0)) pad_uart_tx_i (
   .pad_in_i(uart_tx_i),
   .pad_oe_i(1'b1),
   .pad_out_o(),
   .pad_io(uart_tx_io),
   .pad_attributes_i('0));


pad_cell_output #(.PADATTR(0)) pad_exit_valid_i (
   .pad_in_i(exit_valid_i),
   .pad_oe_i(1'b1),
   .pad_out_o(),
   .pad_io(exit_valid_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_gpio_0_i (
   .pad_in_i(gpio_0_i),
   .pad_oe_i(gpio_0_oe_i),
   .pad_out_o(gpio_0_o),
   .pad_io(gpio_0_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_gpio_1_i (
   .pad_in_i(gpio_1_i),
   .pad_oe_i(gpio_1_oe_i),
   .pad_out_o(gpio_1_o),
   .pad_io(gpio_1_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_gpio_2_i (
   .pad_in_i(gpio_2_i),
   .pad_oe_i(gpio_2_oe_i),
   .pad_out_o(gpio_2_o),
   .pad_io(gpio_2_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_gpio_3_i (
   .pad_in_i(gpio_3_i),
   .pad_oe_i(gpio_3_oe_i),
   .pad_out_o(gpio_3_o),
   .pad_io(gpio_3_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_gpio_4_i (
   .pad_in_i(gpio_4_i),
   .pad_oe_i(gpio_4_oe_i),
   .pad_out_o(gpio_4_o),
   .pad_io(gpio_4_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_gpio_5_i (
   .pad_in_i(gpio_5_i),
   .pad_oe_i(gpio_5_oe_i),
   .pad_out_o(gpio_5_o),
   .pad_io(gpio_5_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_gpio_6_i (
   .pad_in_i(gpio_6_i),
   .pad_oe_i(gpio_6_oe_i),
   .pad_out_o(gpio_6_o),
   .pad_io(gpio_6_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_gpio_7_i (
   .pad_in_i(gpio_7_i),
   .pad_oe_i(gpio_7_oe_i),
   .pad_out_o(gpio_7_o),
   .pad_io(gpio_7_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_gpio_8_i (
   .pad_in_i(gpio_8_i),
   .pad_oe_i(gpio_8_oe_i),
   .pad_out_o(gpio_8_o),
   .pad_io(gpio_8_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_gpio_9_i (
   .pad_in_i(gpio_9_i),
   .pad_oe_i(gpio_9_oe_i),
   .pad_out_o(gpio_9_o),
   .pad_io(gpio_9_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_gpio_10_i (
   .pad_in_i(gpio_10_i),
   .pad_oe_i(gpio_10_oe_i),
   .pad_out_o(gpio_10_o),
   .pad_io(gpio_10_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_gpio_11_i (
   .pad_in_i(gpio_11_i),
   .pad_oe_i(gpio_11_oe_i),
   .pad_out_o(gpio_11_o),
   .pad_io(gpio_11_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_gpio_12_i (
   .pad_in_i(gpio_12_i),
   .pad_oe_i(gpio_12_oe_i),
   .pad_out_o(gpio_12_o),
   .pad_io(gpio_12_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_gpio_13_i (
   .pad_in_i(gpio_13_i),
   .pad_oe_i(gpio_13_oe_i),
   .pad_out_o(gpio_13_o),
   .pad_io(gpio_13_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi_flash_sck_i (
   .pad_in_i(spi_flash_sck_i),
   .pad_oe_i(spi_flash_sck_oe_i),
   .pad_out_o(spi_flash_sck_o),
   .pad_io(spi_flash_sck_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi_flash_cs_0_i (
   .pad_in_i(spi_flash_cs_0_i),
   .pad_oe_i(spi_flash_cs_0_oe_i),
   .pad_out_o(spi_flash_cs_0_o),
   .pad_io(spi_flash_cs_0_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi_flash_cs_1_i (
   .pad_in_i(spi_flash_cs_1_i),
   .pad_oe_i(spi_flash_cs_1_oe_i),
   .pad_out_o(spi_flash_cs_1_o),
   .pad_io(spi_flash_cs_1_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi_flash_sd_0_i (
   .pad_in_i(spi_flash_sd_0_i),
   .pad_oe_i(spi_flash_sd_0_oe_i),
   .pad_out_o(spi_flash_sd_0_o),
   .pad_io(spi_flash_sd_0_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi_flash_sd_1_i (
   .pad_in_i(spi_flash_sd_1_i),
   .pad_oe_i(spi_flash_sd_1_oe_i),
   .pad_out_o(spi_flash_sd_1_o),
   .pad_io(spi_flash_sd_1_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi_flash_sd_2_i (
   .pad_in_i(spi_flash_sd_2_i),
   .pad_oe_i(spi_flash_sd_2_oe_i),
   .pad_out_o(spi_flash_sd_2_o),
   .pad_io(spi_flash_sd_2_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi_flash_sd_3_i (
   .pad_in_i(spi_flash_sd_3_i),
   .pad_oe_i(spi_flash_sd_3_oe_i),
   .pad_out_o(spi_flash_sd_3_o),
   .pad_io(spi_flash_sd_3_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi_sck_i (
   .pad_in_i(spi_sck_i),
   .pad_oe_i(spi_sck_oe_i),
   .pad_out_o(spi_sck_o),
   .pad_io(spi_sck_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi_cs_0_i (
   .pad_in_i(spi_cs_0_i),
   .pad_oe_i(spi_cs_0_oe_i),
   .pad_out_o(spi_cs_0_o),
   .pad_io(spi_cs_0_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi_cs_1_i (
   .pad_in_i(spi_cs_1_i),
   .pad_oe_i(spi_cs_1_oe_i),
   .pad_out_o(spi_cs_1_o),
   .pad_io(spi_cs_1_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi_sd_0_i (
   .pad_in_i(spi_sd_0_i),
   .pad_oe_i(spi_sd_0_oe_i),
   .pad_out_o(spi_sd_0_o),
   .pad_io(spi_sd_0_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi_sd_1_i (
   .pad_in_i(spi_sd_1_i),
   .pad_oe_i(spi_sd_1_oe_i),
   .pad_out_o(spi_sd_1_o),
   .pad_io(spi_sd_1_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi_sd_2_i (
   .pad_in_i(spi_sd_2_i),
   .pad_oe_i(spi_sd_2_oe_i),
   .pad_out_o(spi_sd_2_o),
   .pad_io(spi_sd_2_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi_sd_3_i (
   .pad_in_i(spi_sd_3_i),
   .pad_oe_i(spi_sd_3_oe_i),
   .pad_out_o(spi_sd_3_o),
   .pad_io(spi_sd_3_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi_slave_sck_i (
   .pad_in_i(spi_slave_sck_i),
   .pad_oe_i(spi_slave_sck_oe_i),
   .pad_out_o(spi_slave_sck_o),
   .pad_io(spi_slave_sck_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi_slave_cs_i (
   .pad_in_i(spi_slave_cs_i),
   .pad_oe_i(spi_slave_cs_oe_i),
   .pad_out_o(spi_slave_cs_o),
   .pad_io(spi_slave_cs_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi_slave_miso_i (
   .pad_in_i(spi_slave_miso_i),
   .pad_oe_i(spi_slave_miso_oe_i),
   .pad_out_o(spi_slave_miso_o),
   .pad_io(spi_slave_miso_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi_slave_mosi_i (
   .pad_in_i(spi_slave_mosi_i),
   .pad_oe_i(spi_slave_mosi_oe_i),
   .pad_out_o(spi_slave_mosi_o),
   .pad_io(spi_slave_mosi_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_pdm2pcm_pdm_i (
   .pad_in_i(pdm2pcm_pdm_i),
   .pad_oe_i(pdm2pcm_pdm_oe_i),
   .pad_out_o(pdm2pcm_pdm_o),
   .pad_io(pdm2pcm_pdm_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_pdm2pcm_clk_i (
   .pad_in_i(pdm2pcm_clk_i),
   .pad_oe_i(pdm2pcm_clk_oe_i),
   .pad_out_o(pdm2pcm_clk_o),
   .pad_io(pdm2pcm_clk_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_i2s_sck_i (
   .pad_in_i(i2s_sck_i),
   .pad_oe_i(i2s_sck_oe_i),
   .pad_out_o(i2s_sck_o),
   .pad_io(i2s_sck_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_i2s_ws_i (
   .pad_in_i(i2s_ws_i),
   .pad_oe_i(i2s_ws_oe_i),
   .pad_out_o(i2s_ws_o),
   .pad_io(i2s_ws_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_i2s_sd_i (
   .pad_in_i(i2s_sd_i),
   .pad_oe_i(i2s_sd_oe_i),
   .pad_out_o(i2s_sd_o),
   .pad_io(i2s_sd_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi2_cs_0_i (
   .pad_in_i(spi2_cs_0_i),
   .pad_oe_i(spi2_cs_0_oe_i),
   .pad_out_o(spi2_cs_0_o),
   .pad_io(spi2_cs_0_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi2_cs_1_i (
   .pad_in_i(spi2_cs_1_i),
   .pad_oe_i(spi2_cs_1_oe_i),
   .pad_out_o(spi2_cs_1_o),
   .pad_io(spi2_cs_1_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi2_sck_i (
   .pad_in_i(spi2_sck_i),
   .pad_oe_i(spi2_sck_oe_i),
   .pad_out_o(spi2_sck_o),
   .pad_io(spi2_sck_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi2_sd_0_i (
   .pad_in_i(spi2_sd_0_i),
   .pad_oe_i(spi2_sd_0_oe_i),
   .pad_out_o(spi2_sd_0_o),
   .pad_io(spi2_sd_0_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi2_sd_1_i (
   .pad_in_i(spi2_sd_1_i),
   .pad_oe_i(spi2_sd_1_oe_i),
   .pad_out_o(spi2_sd_1_o),
   .pad_io(spi2_sd_1_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi2_sd_2_i (
   .pad_in_i(spi2_sd_2_i),
   .pad_oe_i(spi2_sd_2_oe_i),
   .pad_out_o(spi2_sd_2_o),
   .pad_io(spi2_sd_2_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_spi2_sd_3_i (
   .pad_in_i(spi2_sd_3_i),
   .pad_oe_i(spi2_sd_3_oe_i),
   .pad_out_o(spi2_sd_3_o),
   .pad_io(spi2_sd_3_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_i2c_scl_i (
   .pad_in_i(i2c_scl_i),
   .pad_oe_i(i2c_scl_oe_i),
   .pad_out_o(i2c_scl_o),
   .pad_io(i2c_scl_io),
   .pad_attributes_i('0));


pad_cell_inout #(.PADATTR(0)) pad_i2c_sda_i (
   .pad_in_i(i2c_sda_i),
   .pad_oe_i(i2c_sda_oe_i),
   .pad_out_o(i2c_sda_o),
   .pad_io(i2c_sda_io),
   .pad_attributes_i('0));




endmodule  // pad_ring

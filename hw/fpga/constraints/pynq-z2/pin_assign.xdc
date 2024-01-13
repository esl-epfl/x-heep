# Copyright 2022 EPFL
# Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
# SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

set_property -dict {PACKAGE_PIN H16 IOSTANDARD LVCMOS33} [get_ports clk_i]
set_property -dict {PACKAGE_PIN L19 IOSTANDARD LVCMOS33} [get_ports rst_i]
set_property -dict {PACKAGE_PIN M14 IOSTANDARD LVCMOS33} [get_ports rst_led]
set_property -dict {PACKAGE_PIN N16 IOSTANDARD LVCMOS33} [get_ports clk_led]
set_property -dict {PACKAGE_PIN W9 IOSTANDARD LVCMOS33} [get_ports clk_out]
set_property -dict {PACKAGE_PIN R14 IOSTANDARD LVCMOS33} [get_ports exit_valid_o]
set_property -dict {PACKAGE_PIN P14 IOSTANDARD LVCMOS33} [get_ports exit_value_o]
set_property -dict {PACKAGE_PIN M19 IOSTANDARD LVCMOS33} [get_ports execute_from_flash_i]
set_property -dict {PACKAGE_PIN M20 IOSTANDARD LVCMOS33} [get_ports boot_select_i]

## Pmoda
## RPi GPIO 7-0 are shared with pmoda_rpi_gpio_tri_io[7:0]

# QSPI
# Q0 / MOSI
# Q1 / MISO
# Q2 / nWP
# Q3 / nHLD

set_property -dict {PACKAGE_PIN U18 IOSTANDARD LVCMOS33} [get_ports spi_flash_csb_o]
set_property -dict {PACKAGE_PIN Y18 IOSTANDARD LVCMOS33} [get_ports spi_flash_sck_o]
set_property -dict {PACKAGE_PIN U19 IOSTANDARD LVCMOS33} [get_ports {spi_flash_sd_io[0]}]
set_property -dict {PACKAGE_PIN Y19 IOSTANDARD LVCMOS33} [get_ports {spi_flash_sd_io[1]}]
set_property -dict {PACKAGE_PIN W18 IOSTANDARD LVCMOS33} [get_ports {spi_flash_sd_io[2]}]
set_property -dict {PACKAGE_PIN Y16 IOSTANDARD LVCMOS33} [get_ports {spi_flash_sd_io[3]}]
set_property -dict {PACKAGE_PIN W19 IOSTANDARD LVCMOS33} [get_ports jtag_trst_ni]

set_property -dict {PACKAGE_PIN F16 IOSTANDARD LVCMOS33} [get_ports spi_csb_o]
set_property -dict {PACKAGE_PIN H15 IOSTANDARD LVCMOS33} [get_ports spi_sck_o]
set_property -dict {PACKAGE_PIN T12 IOSTANDARD LVCMOS33} [get_ports {spi_sd_io[0]}]
set_property -dict {PACKAGE_PIN W15 IOSTANDARD LVCMOS33} [get_ports {spi_sd_io[1]}]
set_property -dict {PACKAGE_PIN P18 IOSTANDARD LVCMOS33} [get_ports {spi_sd_io[2]}]
set_property -dict {PACKAGE_PIN N17 IOSTANDARD LVCMOS33} [get_ports {spi_sd_io[3]}]

## Pmodb
set_property -dict {PACKAGE_PIN W14 IOSTANDARD LVCMOS33} [get_ports uart_tx_o]
set_property -dict {PACKAGE_PIN V16 IOSTANDARD LVCMOS33} [get_ports uart_rx_i]
set_property -dict {PACKAGE_PIN Y14 IOSTANDARD LVCMOS33} [get_ports jtag_tdi_i]
set_property -dict {PACKAGE_PIN V12 IOSTANDARD LVCMOS33} [get_ports jtag_tdo_o]
set_property -dict {PACKAGE_PIN T11 IOSTANDARD LVCMOS33} [get_ports jtag_tms_i]
set_property -dict {PACKAGE_PIN W16 IOSTANDARD LVCMOS33} [get_ports jtag_tck_i]

set_property -dict {PACKAGE_PIN T14 IOSTANDARD LVCMOS33} [get_ports {gpio_io[0]}]
set_property -dict {PACKAGE_PIN Y8 IOSTANDARD LVCMOS33} [get_ports {gpio_io[1]}]
set_property -dict {PACKAGE_PIN W8 IOSTANDARD LVCMOS33} [get_ports {gpio_io[2]}]
set_property -dict {PACKAGE_PIN Y7 IOSTANDARD LVCMOS33} [get_ports {gpio_io[3]}]
set_property -dict {PACKAGE_PIN Y6 IOSTANDARD LVCMOS33} [get_ports {gpio_io[4]}]
set_property -dict {PACKAGE_PIN U12 IOSTANDARD LVCMOS33} [get_ports {gpio_io[5]}]
set_property -dict {PACKAGE_PIN W10 IOSTANDARD LVCMOS33} [get_ports {gpio_io[6]}]
set_property -dict {PACKAGE_PIN V10 IOSTANDARD LVCMOS33} [get_ports {gpio_io[7]}]
set_property -dict {PACKAGE_PIN V8 IOSTANDARD LVCMOS33} [get_ports {gpio_io[8]}]
set_property -dict {PACKAGE_PIN U8 IOSTANDARD LVCMOS33} [get_ports {gpio_io[9]}]
set_property -dict {PACKAGE_PIN V7 IOSTANDARD LVCMOS33} [get_ports {gpio_io[10]}]
set_property -dict {PACKAGE_PIN U7 IOSTANDARD LVCMOS33} [get_ports {gpio_io[11]}]
set_property -dict {PACKAGE_PIN V6 IOSTANDARD LVCMOS33} [get_ports {gpio_io[12]}]
set_property -dict {PACKAGE_PIN U13 IOSTANDARD LVCMOS33} [get_ports {gpio_io[13]}]
set_property -dict {PACKAGE_PIN V13 IOSTANDARD LVCMOS33} [get_ports {gpio_io[14]}]
set_property -dict {PACKAGE_PIN Y9 IOSTANDARD LVCMOS33} [get_ports {pdm2pcm_clk_io}]
set_property -dict {PACKAGE_PIN A20 IOSTANDARD LVCMOS33} [get_ports {pdm2pcm_pdm_io}]
set_property -dict {PACKAGE_PIN B19 IOSTANDARD LVCMOS33} [get_ports {i2s_sck_io}]
set_property -dict {PACKAGE_PIN B20 IOSTANDARD LVCMOS33} [get_ports {i2s_ws_io}]
set_property -dict {PACKAGE_PIN P15 IOSTANDARD LVCMOS33} [get_ports {i2s_sd_io}]

## Tri-color LD5 for TARGET_PYNQ_Z2
set_property -dict {PACKAGE_PIN M15 IOSTANDARD LVCMOS33} [get_ports {gpio_io[15]}]
set_property -dict {PACKAGE_PIN G14 IOSTANDARD LVCMOS33} [get_ports {gpio_io[16]}]
set_property -dict {PACKAGE_PIN L14 IOSTANDARD LVCMOS33} [get_ports {gpio_io[17]}]

set_property -dict {PACKAGE_PIN W6 IOSTANDARD LVCMOS33} [get_ports {spi2_csb_o[0]}]
set_property -dict {PACKAGE_PIN T15 IOSTANDARD LVCMOS33} [get_ports {spi2_csb_o[1]}]
set_property -dict {PACKAGE_PIN C20 IOSTANDARD LVCMOS33} [get_ports {spi2_sck_o}]
set_property -dict {PACKAGE_PIN V17 IOSTANDARD LVCMOS33} [get_ports {spi2_sd_io[0]}]
set_property -dict {PACKAGE_PIN V18 IOSTANDARD LVCMOS33} [get_ports {spi2_sd_io[1]}]
set_property -dict {PACKAGE_PIN T16 IOSTANDARD LVCMOS33} [get_ports {spi2_sd_io[2]}]
set_property -dict {PACKAGE_PIN R17 IOSTANDARD LVCMOS33} [get_ports {spi2_sd_io[3]}]
set_property -dict {PACKAGE_PIN W13 IOSTANDARD LVCMOS33} [get_ports {i2c_scl_io}]
set_property -dict {PACKAGE_PIN T10 IOSTANDARD LVCMOS33} [get_ports {i2c_sda_io}]

set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets jtag_tck_i_IBUF]


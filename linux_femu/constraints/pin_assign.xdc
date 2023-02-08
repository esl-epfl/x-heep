# Copyright 2022 EPFL
# Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
# SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

set_property -dict {PACKAGE_PIN H16 IOSTANDARD LVCMOS33} [get_ports clk_in]
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

set_property -dict {PACKAGE_PIN F16 IOSTANDARD LVCMOS33} [get_ports spi_csb_io]
set_property -dict {PACKAGE_PIN H15 IOSTANDARD LVCMOS33} [get_ports spi_sck_io]
set_property -dict {PACKAGE_PIN T12 IOSTANDARD LVCMOS33} [get_ports {spi_sd_0_io}]
set_property -dict {PACKAGE_PIN W15 IOSTANDARD LVCMOS33} [get_ports {spi_sd_1_io}]
set_property -dict {PACKAGE_PIN P18 IOSTANDARD LVCMOS33} [get_ports {spi_sd_2_io}]
set_property -dict {PACKAGE_PIN N17 IOSTANDARD LVCMOS33} [get_ports {spi_sd_3_io}]

## Pmodb

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
set_property -dict {PACKAGE_PIN Y9 IOSTANDARD LVCMOS33} [get_ports {gpio_io[15]}]
set_property -dict {PACKAGE_PIN A20 IOSTANDARD LVCMOS33} [get_ports {gpio_io[16]}]
set_property -dict {PACKAGE_PIN B19 IOSTANDARD LVCMOS33} [get_ports {gpio_io[17]}]
set_property -dict {PACKAGE_PIN B20 IOSTANDARD LVCMOS33} [get_ports {gpio_io[18]}]
set_property -dict {PACKAGE_PIN P15 IOSTANDARD LVCMOS33} [get_ports {gpio_io[19]}]
set_property -dict {PACKAGE_PIN F20 IOSTANDARD LVCMOS33} [get_ports {gpio_io[20]}]
set_property -dict {PACKAGE_PIN F19 IOSTANDARD LVCMOS33} [get_ports {gpio_io[21]}]
set_property -dict {PACKAGE_PIN P16 IOSTANDARD LVCMOS33} [get_ports {gpio_io[22]}]
set_property -dict {PACKAGE_PIN W6 IOSTANDARD LVCMOS33} [get_ports {spi2_csb_io[0]}]
set_property -dict {PACKAGE_PIN T15 IOSTANDARD LVCMOS33} [get_ports {spi2_csb_io[1]}]
set_property -dict {PACKAGE_PIN C20 IOSTANDARD LVCMOS33} [get_ports {spi2_sck_io}]
set_property -dict {PACKAGE_PIN V17 IOSTANDARD LVCMOS33} [get_ports {spi2_sd_0_io}]
set_property -dict {PACKAGE_PIN V18 IOSTANDARD LVCMOS33} [get_ports {spi2_sd_1_io}]
set_property -dict {PACKAGE_PIN T16 IOSTANDARD LVCMOS33} [get_ports {spi2_sd_2_io}]
set_property -dict {PACKAGE_PIN R17 IOSTANDARD LVCMOS33} [get_ports {spi2_sd_3_io}]
#set_property -dict {PACKAGE_PIN W6 IOSTANDARD LVCMOS33} [get_ports {gpio_io[24]}]
#set_property -dict {PACKAGE_PIN C20 IOSTANDARD LVCMOS33} [get_ports {gpio_io[25]}]
#set_property -dict {PACKAGE_PIN V17 IOSTANDARD LVCMOS33} [get_ports {gpio_io[26]}]
#set_property -dict {PACKAGE_PIN V18 IOSTANDARD LVCMOS33} [get_ports {gpio_io[27]}]
#set_property -dict {PACKAGE_PIN T16 IOSTANDARD LVCMOS33} [get_ports {gpio_io[28]}]
#set_property -dict {PACKAGE_PIN R17 IOSTANDARD LVCMOS33} [get_ports {gpio_io[29]}]
#set_property -dict {PACKAGE_PIN V15 IOSTANDARD LVCMOS33} [get_ports {gpio_io[30]}]
#set_property -dict {PACKAGE_PIN D19 IOSTANDARD LVCMOS33} [get_ports {gpio_io[31]}]

set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets jtag_tck_i_IBUF]

set_property PACKAGE_PIN W13 [get_ports i2c_scl_io]
set_property PACKAGE_PIN T10 [get_ports i2c_sda_io]
set_property IOSTANDARD LVCMOS33 [get_ports i2c_scl_io]
set_property IOSTANDARD LVCMOS33 [get_ports i2c_sda_io]

# Copyright 2022 EPFL
# Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
# SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#set_property PACKAGE_PIN E7  [get_ports clk_i] 
set_property PACKAGE_PIN H16  [get_ports clk_i]
set_property PACKAGE_PIN L19 [get_ports rst_i]
set_property PACKAGE_PIN M14 [get_ports rst_led]
set_property PACKAGE_PIN N16 [get_ports clk_led]
set_property PACKAGE_PIN W9  [get_ports clk_out]
set_property PACKAGE_PIN R14 [get_ports exit_valid_o]
set_property PACKAGE_PIN P14 [get_ports exit_value_o]
set_property PACKAGE_PIN M19 [get_ports fetch_enable_i]
set_property PACKAGE_PIN M20 [get_ports boot_select_i]
set_property PACKAGE_PIN B5  [get_ports phy_rst]
#set_property PACKAGE_PIN T10 [get_ports jtag_tck_i]
#set_property PACKAGE_PIN T11 [get_ports jtag_tdo_o]
#set_property PACKAGE_PIN Y14 [get_ports jtag_tdi_i]
#set_property PACKAGE_PIN W14 [get_ports jtag_tms_i]
#set_property PACKAGE_PIN V16 [get_ports jtag_trst_ni]
#set_property PACKAGE_PIN Y18  [get_ports uart_tx_o]
#set_property PACKAGE_PIN Y19  [get_ports uart_rx_i]

## Pmoda
## RPi GPIO 7-0 are shared with pmoda_rpi_gpio_tri_io[7:0]

# QSPI
# Q0 / MOSI
# Q1 / MISO
# Q2 / nWP
# Q3 / nHLD

set_property -dict {PACKAGE_PIN U18 IOSTANDARD LVCMOS33} [get_ports {spi_flash_cs}]
set_property -dict {PACKAGE_PIN Y18 IOSTANDARD LVCMOS33} [get_ports {spi_flash_clk}]
set_property -dict {PACKAGE_PIN U19 IOSTANDARD LVCMOS33} [get_ports {spi_flash_dio[0]}]
set_property -dict {PACKAGE_PIN Y19 IOSTANDARD LVCMOS33} [get_ports {spi_flash_dio[1]}]
set_property -dict {PACKAGE_PIN W18 IOSTANDARD LVCMOS33} [get_ports {spi_flash_dio[2]}]
set_property -dict {PACKAGE_PIN Y16 IOSTANDARD LVCMOS33} [get_ports {spi_flash_dio[3]}]
set_property -dict {PACKAGE_PIN Y17 IOSTANDARD LVCMOS33} [get_ports {crst}]
set_property -dict {PACKAGE_PIN W19 IOSTANDARD LVCMOS33} [get_ports {jtag_trst_ni}]


## Pmodb
set_property -dict {PACKAGE_PIN W14 IOSTANDARD LVCMOS33} [get_ports uart_rx_i]
set_property -dict {PACKAGE_PIN V16 IOSTANDARD LVCMOS33} [get_ports uart_tx_o]
set_property -dict {PACKAGE_PIN Y14 IOSTANDARD LVCMOS33} [get_ports jtag_tdi_i]
set_property -dict {PACKAGE_PIN V12 IOSTANDARD LVCMOS33} [get_ports jtag_tdo_o]
set_property -dict {PACKAGE_PIN T11 IOSTANDARD LVCMOS33} [get_ports jtag_tms_i]
set_property -dict {PACKAGE_PIN W16 IOSTANDARD LVCMOS33} [get_ports jtag_tck_i]
#set_property -dict {PACKAGE_PIN T10 IOSTANDARD LVCMOS33} [get_ports {spi_flash_dio[0]}]
#set_property -dict {PACKAGE_PIN W13 IOSTANDARD LVCMOS33} [get_ports {spi_flash_dio[1]}]



set_property IOSTANDARD LVCMOS33 [get_ports clk_i]
set_property IOSTANDARD LVCMOS33 [get_ports clk_out]
set_property IOSTANDARD LVCMOS33 [get_ports boot_select_i]
set_property IOSTANDARD LVCMOS33 [get_ports rst_i]
set_property IOSTANDARD LVCMOS33 [get_ports rst_led]
set_property IOSTANDARD LVCMOS33 [get_ports clk_led]
set_property IOSTANDARD LVCMOS33 [get_ports exit_valid_o]
set_property IOSTANDARD LVCMOS33 [get_ports exit_value_o]
set_property IOSTANDARD LVCMOS33 [get_ports fetch_enable_i]
set_property IOSTANDARD LVCMOS33 [get_ports jtag_tck_i]
set_property IOSTANDARD LVCMOS33 [get_ports jtag_tdo_o]
set_property IOSTANDARD LVCMOS33 [get_ports jtag_tdi_i]
set_property IOSTANDARD LVCMOS33 [get_ports jtag_tms_i]
set_property IOSTANDARD LVCMOS33 [get_ports jtag_trst_ni]
set_property IOSTANDARD LVCMOS33 [get_ports uart_tx_o]
set_property IOSTANDARD LVCMOS33 [get_ports uart_rx_i]

set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets jtag_tck_i_IBUF]

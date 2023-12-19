# Copyright 2022 EPFL
# Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
# SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

set_property PACKAGE_PIN E23      [get_ports "clk_i_N"] ;# Bank  28 VCCO - VCC1V8   - IO_L13N_T2L_N1_GC_QBC_28
set_property IOSTANDARD  LVDS     [get_ports "clk_i_N"] ;# Bank  28 VCCO - VCC1V8   - IO_L13N_T2L_N1_GC_QBC_28
set_property PACKAGE_PIN F23      [get_ports "clk_i_P"] ;# Bank  28 VCCO - VCC1V8   - IO_L13P_T2L_N0_GC_QBC_28
set_property IOSTANDARD  LVDS     [get_ports "clk_i_P"] ;# Bank  28 VCCO - VCC1V8   - IO_L13P_T2L_N0_GC_QBC_28
set_property PACKAGE_PIN M11      [get_ports rst_i]
set_property IOSTANDARD LVCMOS33  [get_ports rst_i]

set_property -dict {PACKAGE_PIN D5 IOSTANDARD LVCMOS33} [get_ports rst_led]
set_property -dict {PACKAGE_PIN D6 IOSTANDARD LVCMOS33} [get_ports clk_led]
set_property -dict {PACKAGE_PIN C30 IOSTANDARD LVCMOS33}  [get_ports clk_out]
set_property -dict {PACKAGE_PIN A5 IOSTANDARD LVCMOS33} [get_ports exit_valid_o]
set_property -dict {PACKAGE_PIN B5 IOSTANDARD LVCMOS33} [get_ports exit_value_o]
set_property -dict {PACKAGE_PIN E4 IOSTANDARD LVCMOS33} [get_ports execute_from_flash_i]
set_property -dict {PACKAGE_PIN D4 IOSTANDARD LVCMOS33} [get_ports boot_select_i]

# Pmod
set_property -dict {PACKAGE_PIN G8 IOSTANDARD LVCMOS33} [get_ports uart_tx_o]
set_property -dict {PACKAGE_PIN G6 IOSTANDARD LVCMOS33} [get_ports uart_rx_i]
set_property -dict {PACKAGE_PIN H6 IOSTANDARD LVCMOS33} [get_ports jtag_tck_i]
set_property -dict {PACKAGE_PIN H8 IOSTANDARD LVCMOS33} [get_ports jtag_tdi_i]
set_property -dict {PACKAGE_PIN J6 IOSTANDARD LVCMOS33} [get_ports jtag_tdo_o]
set_property -dict {PACKAGE_PIN G7 IOSTANDARD LVCMOS33} [get_ports jtag_tms_i]

set_property -dict {PACKAGE_PIN L10 IOSTANDARD LVCMOS33} [get_ports spi_flash_csb_o]
set_property -dict {PACKAGE_PIN J9 IOSTANDARD LVCMOS33} [get_ports spi_flash_sck_o]
set_property -dict {PACKAGE_PIN M10 IOSTANDARD LVCMOS33} [get_ports {spi_flash_sd_io[0]}]
set_property -dict {PACKAGE_PIN K9 IOSTANDARD LVCMOS33} [get_ports {spi_flash_sd_io[1]}]
set_property -dict {PACKAGE_PIN M8 IOSTANDARD LVCMOS33} [get_ports {spi_flash_sd_io[2]}]
set_property -dict {PACKAGE_PIN K8 IOSTANDARD LVCMOS33} [get_ports {spi_flash_sd_io[3]}]
# Copyright 2022 EPFL
# Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
# SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

# Clock signal
set_property -dict {PACKAGE_PIN E23 IOSTANDARD LVDS} [get_ports "clk_i_N"] ;# Bank 28 VCCO - VCC1V8 - IO_L13N_T2L_N1_GC_QBC_28
set_property -dict {PACKAGE_PIN F23 IOSTANDARD LVDS} [get_ports "clk_i_P"] ;# Bank 28 VCCO - VCC1V8 - IO_L13P_T2L_N0_GC_QBC_28
create_clock -add -name sys_clk_pin -period 10.00 -waveform {0 5} [get_ports {clk_i}];
set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets jtag_tck_i_IBUF]

set_property -dict {PACKAGE_PIN M11 IOSTANDARD LVCMOS33} [get_ports rst_i]

# LEDs
set_property -dict {PACKAGE_PIN D5 IOSTANDARD LVCMOS33} [get_ports rst_led]
set_property -dict {PACKAGE_PIN C30 IOSTANDARD LVCMOS33} [get_ports clk_out]
set_property -dict {PACKAGE_PIN D6 IOSTANDARD LVCMOS33} [get_ports clk_led]
set_property -dict {PACKAGE_PIN A5 IOSTANDARD LVCMOS33} [get_ports exit_valid_o]
set_property -dict {PACKAGE_PIN B5 IOSTANDARD LVCMOS33} [get_ports exit_value_o]
set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets rst_led_OBUF]
set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets clk_out_OBUF]
set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets clk_led_OBUF]

#Switches
set_property -dict {PACKAGE_PIN E4 IOSTANDARD LVCMOS33} [get_ports execute_from_flash_i]
set_property -dict {PACKAGE_PIN D4 IOSTANDARD LVCMOS33} [get_ports boot_select_i]

# FLASH
# QSPI
# Q0 / MOSI
# Q1 / MISO
# Q2 / nWP
# Q3 / nHLD
set_property -dict {PACKAGE_PIN L10 IOSTANDARD LVCMOS33} [get_ports spi_flash_csb_o] # Pmod1[4]
set_property -dict {PACKAGE_PIN J9 IOSTANDARD LVCMOS33} [get_ports spi_flash_sck_o] # Pmod1[0]
set_property -dict {PACKAGE_PIN M10 IOSTANDARD LVCMOS33} [get_ports {spi_flash_sd_io[0]}] # Pmod1[5]
set_property -dict {PACKAGE_PIN K9 IOSTANDARD LVCMOS33} [get_ports {spi_flash_sd_io[1]}] # Pmod1[1]
set_property -dict {PACKAGE_PIN M8 IOSTANDARD LVCMOS33} [get_ports {spi_flash_sd_io[2]}] # Pmod1[6]
set_property -dict {PACKAGE_PIN K8 IOSTANDARD LVCMOS33} [get_ports {spi_flash_sd_io[3]}] # Pmod1[2]

# UART
set_property -dict {PACKAGE_PIN G8 IOSTANDARD LVCMOS33} [get_ports uart_tx_o] # Pmod0[0]
set_property -dict {PACKAGE_PIN G6 IOSTANDARD LVCMOS33} [get_ports uart_rx_i] # Pmod0[4]

# JTAG
set_property -dict {PACKAGE_PIN H8 IOSTANDARD LVCMOS33} [get_ports jtag_tdi_i] # Pmod0[1]
set_property -dict {PACKAGE_PIN J6 IOSTANDARD LVCMOS33} [get_ports jtag_tdo_o] # Pmod0[6]
set_property -dict {PACKAGE_PIN G7 IOSTANDARD LVCMOS33} [get_ports jtag_tms_i] # Pmod0[2]
set_property -dict {PACKAGE_PIN H6 IOSTANDARD LVCMOS33} [get_ports jtag_tck_i] # Pmod0[5]
set_property -dict {PACKAGE_PIN M9 IOSTANDARD LVCMOS33} [get_ports jtag_trst_ni] # Pmod1[7]

# I2C
set_property -dict {PACKAGE_PIN J7 IOSTANDARD LVCMOS33} [get_ports {i2c_scl_io}] # Pmod0[7]
set_property -dict {PACKAGE_PIN H7 IOSTANDARD LVCMOS33} [get_ports {i2c_sda_io}] # Pmod0[3]

###############################
## The following pins are sent to the FMC connector, using the LA pins as single-ended
# The bank only supports up to 1.8 V !!!

# SPI SD
set_property -dict {PACKAGE_PIN H19 IOSTANDARD LVCMOS18} [get_ports spi_csb_o] # fmc_lpc_la06_p
set_property -dict {PACKAGE_PIN G19 IOSTANDARD LVCMOS18} [get_ports spi_sck_o] # fmc_lpc_la06_n
set_property -dict {PACKAGE_PIN L15 IOSTANDARD LVCMOS18} [get_ports {spi_sd_io[0]}] # fmc_lpc_la10_p
set_property -dict {PACKAGE_PIN K15 IOSTANDARD LVCMOS18} [get_ports {spi_sd_io[1]}] # fmc_lpc_la10_n
set_property -dict {PACKAGE_PIN C13 IOSTANDARD LVCMOS18} [get_ports {spi_sd_io[2]}] # fmc_lpc_la14_p
set_property -dict {PACKAGE_PIN C12 IOSTANDARD LVCMOS18} [get_ports {spi_sd_io[3]}] # fmc_lpc_la14_n

# GPIOs
set_property -dict {PACKAGE_PIN A8 IOSTANDARD LVCMOS18} [get_ports {gpio_io[0]}] # fmc_lpc_la27_p
set_property -dict {PACKAGE_PIN A7 IOSTANDARD LVCMOS18} [get_ports {gpio_io[1]}] # fmc_lpc_la27_n
set_property -dict {PACKAGE_PIN K17 IOSTANDARD LVCMOS18} [get_ports {gpio_io[2]}] # fmc_lpc_la05_p
set_property -dict {PACKAGE_PIN J17 IOSTANDARD LVCMOS18} [get_ports {gpio_io[3]}] # fmc_lpc_la05_n
set_property -dict {PACKAGE_PIN H16 IOSTANDARD LVCMOS18} [get_ports {gpio_io[4]}] # fmc_lpc_la09_p
set_property -dict {PACKAGE_PIN G16 IOSTANDARD LVCMOS18} [get_ports {gpio_io[5]}] # fmc_lpc_la09_n
set_property -dict {PACKAGE_PIN G15 IOSTANDARD LVCMOS18} [get_ports {gpio_io[6]}] # fmc_lpc_la13_p
set_property -dict {PACKAGE_PIN H15 IOSTANDARD LVCMOS18} [get_ports {gpio_io[7]}] # fmc_lpc_la13_n
set_property -dict {PACKAGE_PIN B11 IOSTANDARD LVCMOS18} [get_ports {gpio_io[8]}] # fmc_lpc_la23_p
set_property -dict {PACKAGE_PIN A11 IOSTANDARD LVCMOS18} [get_ports {gpio_io[9]}] # fmc_lpc_la23_n
set_property -dict {PACKAGE_PIN B9 IOSTANDARD LVCMOS18} [get_ports {gpio_io[10]}] # fmc_lpc_la26_p
set_property -dict {PACKAGE_PIN B8 IOSTANDARD LVCMOS18} [get_ports {gpio_io[11]}] # fmc_lpc_la26_n
set_property -dict {PACKAGE_PIN K19 IOSTANDARD LVCMOS18} [get_ports {gpio_io[12]}] # fmc_lpc_la03_p
set_property -dict {PACKAGE_PIN K18 IOSTANDARD LVCMOS18} [get_ports {gpio_io[13]}] # fmc_lpc_la03_n
set_property -dict {PACKAGE_PIN E18 IOSTANDARD LVCMOS18} [get_ports {gpio_io[14]}] # fmc_lpc_la08_p

# PDM2PCM
set_property -dict {PACKAGE_PIN G18 IOSTANDARD LVCMOS18} [get_ports {pdm2pcm_clk_io}] # fmc_lpc_la12_p
set_property -dict {PACKAGE_PIN F18 IOSTANDARD LVCMOS18} [get_ports {pdm2pcm_pdm_io}] # fmc_lpc_la12_n

# I2S
set_property -dict {PACKAGE_PIN D17 IOSTANDARD LVCMOS18} [get_ports {i2s_sck_io}] # fmc_lpc_la16_p
set_property -dict {PACKAGE_PIN C17 IOSTANDARD LVCMOS18} [get_ports {i2s_ws_io}] # fmc_lpc_la16_n
set_property -dict {PACKAGE_PIN F12 IOSTANDARD LVCMOS18} [get_ports {i2s_sd_io}] # fmc_lpc_la20_p

# SPI2
set_property -dict {PACKAGE_PIN H13 IOSTANDARD LVCMOS18} [get_ports {spi2_csb_o[0]}] # fmc_lpc_la22_p
set_property -dict {PACKAGE_PIN H12 IOSTANDARD LVCMOS18} [get_ports {spi2_csb_o[1]}] # fmc_lpc_la22_n
set_property -dict {PACKAGE_PIN C7 IOSTANDARD LVCMOS18} [get_ports {spi2_sck_o}] # fmc_lpc_la25_p
set_property -dict {PACKAGE_PIN C6 IOSTANDARD LVCMOS18} [get_ports {spi2_sd_io[0]}] # fmc_lpc_la25_n
set_property -dict {PACKAGE_PIN K10 IOSTANDARD LVCMOS18} [get_ports {spi2_sd_io[1]}] # fmc_lpc_la29_p
set_property -dict {PACKAGE_PIN J10 IOSTANDARD LVCMOS18} [get_ports {spi2_sd_io[2]}] # fmc_lpc_la29_n
set_property -dict {PACKAGE_PIN F7 IOSTANDARD LVCMOS18} [get_ports {spi2_sd_io[3]}] # fmc_lpc_la31_p

# Tri-color LEDs for TARGET_PYNQ_Z2
set_property -dict {PACKAGE_PIN C9 IOSTANDARD LVCMOS18} [get_ports {gpio_io[15]}] # fmc_lpc_lafmc_lpc_la33_p
set_property -dict {PACKAGE_PIN C8 IOSTANDARD LVCMOS18} [get_ports {gpio_io[16]}] # fmc_lpc_la33_n
set_property -dict {PACKAGE_PIN L20 IOSTANDARD LVCMOS18} [get_ports {gpio_io[17]}] # fmc_lpc_la02_p


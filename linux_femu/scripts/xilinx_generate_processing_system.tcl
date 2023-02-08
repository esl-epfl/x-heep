# Copyright 2022 EPFL
# Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
# SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
# Define design macros

# Select board
set_property board_part tul.com.tw:pynq-z2:part0:1.0 [current_project]

# Create block design
create_bd_design "processing_system"

# Add Zynq Processing System
create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:5.5 processing_system7_0
apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 -config {make_external "FIXED_IO, DDR" apply_board_preset "1" Master "Disable" Slave "Disable" }  [get_bd_cells processing_system7_0]
set_property -dict [list CONFIG.PCW_FPGA0_PERIPHERAL_FREQMHZ {20} CONFIG.PCW_USE_S_AXI_HP0 {1} CONFIG.PCW_QSPI_GRP_SINGLE_SS_ENABLE {1} CONFIG.PCW_ENET0_PERIPHERAL_ENABLE {0} CONFIG.PCW_SD0_PERIPHERAL_ENABLE {0} CONFIG.PCW_UART0_PERIPHERAL_ENABLE {0} CONFIG.PCW_UART1_PERIPHERAL_ENABLE {1} CONFIG.PCW_UART1_UART1_IO {EMIO} CONFIG.PCW_USB0_PERIPHERAL_ENABLE {0} CONFIG.PCW_GPIO_MIO_GPIO_ENABLE {0} CONFIG.PCW_GPIO_EMIO_GPIO_ENABLE {1} CONFIG.PCW_GPIO_EMIO_GPIO_IO {5}] [get_bd_cells processing_system7_0]

# Add AXI Interconnect
create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_interconnect_0
set_property -dict [list CONFIG.NUM_SI {2} CONFIG.NUM_MI {2}] [get_bd_cells axi_interconnect_0]

# Add Constant
create_bd_cell -type ip -vlnv xilinx.com:ip:xlconstant:1.1 xlconstant_0
set_property -dict [list CONFIG.CONST_WIDTH {2} CONFIG.CONST_VAL {0b11}] [get_bd_cells xlconstant_0]

# Add Concatenation
create_bd_cell -type ip -vlnv xilinx.com:ip:xlconcat:2.1 xlconcat_0
set_property -dict [list CONFIG.IN0_WIDTH.VALUE_SRC USER CONFIG.IN1_WIDTH.VALUE_SRC USER CONFIG.IN2_WIDTH.VALUE_SRC USER] [get_bd_cells xlconcat_0]
set_property -dict [list CONFIG.NUM_PORTS {3} CONFIG.IN0_WIDTH {2} CONFIG.IN2_WIDTH {2}] [get_bd_cells xlconcat_0]

# Add Slices
create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 xlslice_0
create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 xlslice_1
create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 xlslice_2
create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 xlslice_3
set_property -dict [list CONFIG.DIN_TO {3} CONFIG.DIN_FROM {3} CONFIG.DIN_WIDTH {5} CONFIG.DOUT_WIDTH {1}] [get_bd_cells xlslice_0]
set_property -dict [list CONFIG.DIN_TO {4} CONFIG.DIN_FROM {4} CONFIG.DIN_WIDTH {5} CONFIG.DOUT_WIDTH {1}] [get_bd_cells xlslice_1]
set_property -dict [list CONFIG.DIN_TO {1} CONFIG.DIN_FROM {1} CONFIG.DIN_WIDTH {5} CONFIG.DOUT_WIDTH {1}] [get_bd_cells xlslice_2]
set_property -dict [list CONFIG.DIN_TO {0} CONFIG.DIN_FROM {0} CONFIG.DIN_WIDTH {5} CONFIG.DOUT_WIDTH {1}] [get_bd_cells xlslice_3]

# Create port AXI_HP
make_bd_intf_pins_external [get_bd_intf_pins axi_interconnect_0/S00_AXI]
set_property name AXI_HP [get_bd_intf_ports S00_AXI_0]
set_property -dict [list CONFIG.FREQ_HZ {20000000}] [get_bd_intf_ports AXI_HP]

# Create port M_AXI
make_bd_intf_pins_external  [get_bd_intf_pins axi_interconnect_0/M01_AXI]
set_property name M_AXI [get_bd_intf_ports M01_AXI_0]
set_property -dict [list CONFIG.FREQ_HZ {20000000}] [get_bd_intf_ports M_AXI]

# Connect AXI Interconnect and Zynq Processing System
connect_bd_intf_net -boundary_type upper [get_bd_intf_pins axi_interconnect_0/M00_AXI] [get_bd_intf_pins processing_system7_0/S_AXI_HP0]
connect_bd_intf_net [get_bd_intf_pins processing_system7_0/M_AXI_GP0] -boundary_type upper [get_bd_intf_pins axi_interconnect_0/S01_AXI]
apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (20 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}} [get_bd_pins axi_interconnect_0/ACLK]
apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (20 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}} [get_bd_pins axi_interconnect_0/M00_ACLK]
apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (20 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}} [get_bd_pins axi_interconnect_0/M01_ACLK]
apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (20 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}} [get_bd_pins axi_interconnect_0/S00_ACLK]
apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (20 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}} [get_bd_pins axi_interconnect_0/S01_ACLK]

# Create port AXI_HP_ACLK
create_bd_port -dir O -type clk AXI_HP_ACLK

# Create port AXI_HP_ARESETN
create_bd_port -dir O -type rst AXI_HP_ARESETN

# Connect AXI_HP_ACLK and AXI_HP_ARESETN
connect_bd_net [get_bd_ports AXI_HP_ACLK] [get_bd_pins processing_system7_0/FCLK_CLK0]
connect_bd_net [get_bd_ports AXI_HP_ARESETN] [get_bd_pins rst_ps7_0_20M/peripheral_aresetn]

# Create port gpio_jtag_tdo_o
make_bd_pins_external [get_bd_pins xlconcat_0/In1]
set_property name gpio_jtag_tdo_o [get_bd_ports In1_0]

# Connect Constant and Concatenation
connect_bd_net [get_bd_pins xlconstant_0/dout] [get_bd_pins xlconcat_0/In0]
connect_bd_net [get_bd_pins xlconcat_0/In2] [get_bd_pins xlconstant_0/dout]
connect_bd_net [get_bd_pins xlconcat_0/dout] [get_bd_pins processing_system7_0/GPIO_I]

# Create port gpio_jtag_tdi_i
make_bd_pins_external [get_bd_pins xlslice_0/Dout]
set_property name gpio_jtag_tdi_i [get_bd_ports Dout_0]

# Create port gpio_jtag_tck_i
make_bd_pins_external [get_bd_pins xlslice_1/Dout]
set_property name gpio_jtag_tck_i [get_bd_ports Dout_0]

# Create port gpio_jtag_tms_i
make_bd_pins_external [get_bd_pins xlslice_2/Dout]
set_property name gpio_jtag_tms_i [get_bd_ports Dout_0]

# Create port gpio_jtag_trst_ni
make_bd_pins_external [get_bd_pins xlslice_3/Dout]
set_property name gpio_jtag_trst_ni [get_bd_ports Dout_0]

# Connect Slices
connect_bd_net [get_bd_pins xlslice_0/Din] [get_bd_pins processing_system7_0/GPIO_O]
connect_bd_net [get_bd_pins xlslice_1/Din] [get_bd_pins processing_system7_0/GPIO_O]
connect_bd_net [get_bd_pins xlslice_2/Din] [get_bd_pins processing_system7_0/GPIO_O]
connect_bd_net [get_bd_pins xlslice_3/Din] [get_bd_pins processing_system7_0/GPIO_O]

# Create port UART
make_bd_intf_pins_external [get_bd_intf_pins processing_system7_0/UART_1]
set_property name UART [get_bd_intf_ports UART_1_0]

# Assign addresses
assign_bd_address

# Validate design
validate_bd_design

# Save design
save_bd_design

# Close design
close_bd_design [get_bd_designs processing_system]

# Make wrapper
set wrapper_path [ make_wrapper -fileset sources_1 -files [ get_files -norecurse processing_system.bd ] -top ]
add_files -norecurse -fileset sources_1 $wrapper_path

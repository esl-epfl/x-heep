create_bd_design "processing_system"
create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:5.5 processing_system7_0
set_property -dict [list CONFIG.PCW_USE_M_AXI_GP0 {1} CONFIG.PCW_GPIO_EMIO_GPIO_ENABLE {1} CONFIG.PCW_GPIO_EMIO_GPIO_IO {5}] [get_bd_cells processing_system7_0]
apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 -config {make_external "FIXED_IO, DDR" Master "Disable" Slave "Disable" }  [get_bd_cells processing_system7_0]
create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_interconnect_0
apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (50 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_0/ACLK]
set_property -dict [list CONFIG.NUM_MI {1}] [get_bd_cells axi_interconnect_0]
make_bd_intf_pins_external  [get_bd_intf_pins axi_interconnect_0/S00_AXI]
set_property -dict [list CONFIG.PCW_USE_S_AXI_HP0 {1}] [get_bd_cells processing_system7_0]
connect_bd_intf_net -boundary_type upper [get_bd_intf_pins axi_interconnect_0/M00_AXI] [get_bd_intf_pins processing_system7_0/S_AXI_HP0]
apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (50 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_0/M00_ACLK]
connect_bd_net [get_bd_pins axi_interconnect_0/S00_ACLK] [get_bd_pins processing_system7_0/FCLK_CLK0]
connect_bd_net [get_bd_pins axi_interconnect_0/S00_ARESETN] [get_bd_pins rst_ps7_0_50M/peripheral_aresetn]
set_property -dict [list CONFIG.PCW_FPGA0_PERIPHERAL_FREQMHZ {20}] [get_bd_cells processing_system7_0]
set_property name AXI_HP [get_bd_intf_ports S00_AXI_0]
#set_property name AXI_HP_ACLK [get_bd_ports S00_ACLK_0]
#set_property name AXI_HP_ARESETN [get_bd_ports S00_ARESETN_0]
create_bd_port -dir O -type clk AXI_HP_ACLK
connect_bd_net [get_bd_ports AXI_HP_ACLK] [get_bd_pins processing_system7_0/FCLK_CLK0]
set_property -dict [list CONFIG.FREQ_HZ {20000000}] [get_bd_ports AXI_HP_ACLK]
set_property CONFIG.ASSOCIATED_BUSIF {AXI_HP} [get_bd_ports /AXI_HP_ACLK]
create_bd_port -dir O -type rst AXI_HP_ARESETN
connect_bd_net [get_bd_ports AXI_HP_ARESETN] [get_bd_pins rst_ps7_0_50M/peripheral_aresetn]



# Adding master port and slave port to interconnect. Master port exported to be attached to address_hiijacker IP. Slave port attached to M_GP0 of PS.
set_property -dict [list CONFIG.NUM_SI {2} CONFIG.NUM_MI {2} CONFIG.NUM_MI {2}] [get_bd_cells axi_interconnect_0]
connect_bd_net [get_bd_pins processing_system7_0/M_AXI_GP0_ACLK] [get_bd_pins processing_system7_0/FCLK_CLK0]
connect_bd_net [get_bd_pins axi_interconnect_0/S01_ACLK] [get_bd_pins processing_system7_0/FCLK_CLK0]
connect_bd_net [get_bd_pins axi_interconnect_0/M01_ACLK] [get_bd_pins processing_system7_0/FCLK_CLK0]
connect_bd_net [get_bd_pins axi_interconnect_0/S01_ARESETN] [get_bd_pins rst_ps7_0_50M/peripheral_aresetn]
connect_bd_net [get_bd_pins axi_interconnect_0/M01_ARESETN] [get_bd_pins rst_ps7_0_50M/peripheral_aresetn]
connect_bd_intf_net -boundary_type upper [get_bd_intf_pins axi_interconnect_0/S01_AXI] [get_bd_intf_pins processing_system7_0/M_AXI_GP0]
make_bd_intf_pins_external  [get_bd_intf_pins axi_interconnect_0/M01_AXI]
set_property CONFIG.ASSOCIATED_BUSIF {AXI_HP:M01_AXI_0} [get_bd_ports /AXI_HP_ACLK]
set_property name M_AXI [get_bd_intf_ports M01_AXI_0]
#assign_bd_address -target_address_space /AXI_HP [get_bd_addr_segs M_AXI/Reg] -force
assign_bd_address -target_address_space /AXI_HP [get_bd_addr_segs processing_system7_0/S_AXI_HP0/HP0_DDR_LOWOCM] -force
assign_bd_address -target_address_space /processing_system7_0/Data [get_bd_addr_segs M_AXI/Reg] -force
set_property offset 0x40000000 [get_bd_addr_segs {processing_system7_0/Data/SEG_M_AXI_Reg}]



# SYSTEM ILA TO DEBUG THE AXI_SPI_SLAVE

create_bd_cell -type ip -vlnv xilinx.com:ip:system_ila:1.1 system_ila_0
#connect_bd_intf_net [get_bd_intf_pins system_ila_0/SLOT_0_AXI] -boundary_type upper [get_bd_intf_pins axi_interconnect_0/M00_AXI]
connect_bd_intf_net [get_bd_intf_pins system_ila_0/SLOT_0_AXI] -boundary_type upper [get_bd_intf_pins axi_interconnect_0/S00_AXI]
set_property -dict [list CONFIG.C_DATA_DEPTH {2048}] [get_bd_cells system_ila_0]
apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (50 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins system_ila_0/clk]
create_bd_port -dir I spi_test_cs
create_bd_port -dir I spi_test_clk
create_bd_port -dir I -from 3 -to 0 -type data spi_test_data
set_property -dict [list CONFIG.C_MON_TYPE {MIX}] [get_bd_cells system_ila_0]
create_bd_cell -type ip -vlnv xilinx.com:ip:xlconcat:2.1 xlconcat_1
connect_bd_net [get_bd_ports spi_test_cs] [get_bd_pins xlconcat_1/In0]
connect_bd_net [get_bd_ports spi_test_clk] [get_bd_pins xlconcat_1/In1]
set_property -dict [list CONFIG.NUM_PORTS {3}] [get_bd_cells xlconcat_1]
connect_bd_net [get_bd_ports spi_test_data] [get_bd_pins xlconcat_1/In2]
connect_bd_net [get_bd_pins xlconcat_1/dout] [get_bd_pins system_ila_0/probe0]
set_property -dict [list CONFIG.IN2_WIDTH.VALUE_SRC USER CONFIG.IN1_WIDTH.VALUE_SRC USER CONFIG.IN0_WIDTH.VALUE_SRC USER] [get_bd_cells xlconcat_1]
set_property -dict [list CONFIG.IN2_WIDTH {4}] [get_bd_cells xlconcat_1]
set_property -dict [list CONFIG.C_NUM_MONITOR_SLOTS {2}] [get_bd_cells system_ila_0]
connect_bd_intf_net [get_bd_intf_pins system_ila_0/SLOT_1_AXI] -boundary_type upper [get_bd_intf_pins axi_interconnect_0/M01_AXI]



create_bd_port -dir I gpio_jtag_tdo_o
create_bd_port -dir O gpio_jtag_tdi_i
create_bd_port -dir O gpio_jtag_tms_i
create_bd_port -dir O gpio_jtag_trst_ni
create_bd_port -dir O gpio_jtag_tck_i

create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 xlslice_0
create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 xlslice_1
create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 xlslice_2
create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 xlslice_3

set_property -dict [list CONFIG.DIN_TO {3} CONFIG.DIN_FROM {3} CONFIG.DIN_WIDTH {5} CONFIG.DOUT_WIDTH {1}] [get_bd_cells xlslice_0]
set_property -dict [list CONFIG.DIN_TO {4} CONFIG.DIN_FROM {4} CONFIG.DIN_WIDTH {5} CONFIG.DOUT_WIDTH {1}] [get_bd_cells xlslice_1]
set_property -dict [list CONFIG.DIN_TO {1} CONFIG.DIN_FROM {1} CONFIG.DIN_WIDTH {5} CONFIG.DOUT_WIDTH {1}] [get_bd_cells xlslice_2]
set_property -dict [list CONFIG.DIN_TO {0} CONFIG.DIN_FROM {0} CONFIG.DIN_WIDTH {5} CONFIG.DOUT_WIDTH {1}] [get_bd_cells xlslice_3]


connect_bd_net [get_bd_pins xlslice_0/Din] [get_bd_pins processing_system7_0/GPIO_O]
connect_bd_net [get_bd_pins xlslice_1/Din] [get_bd_pins processing_system7_0/GPIO_O]
connect_bd_net [get_bd_pins xlslice_2/Din] [get_bd_pins processing_system7_0/GPIO_O]
connect_bd_net [get_bd_pins xlslice_3/Din] [get_bd_pins processing_system7_0/GPIO_O]

connect_bd_net [get_bd_ports gpio_jtag_tdi_i] [get_bd_pins xlslice_0/Dout]
connect_bd_net [get_bd_ports gpio_jtag_tck_i] [get_bd_pins xlslice_1/Dout]
connect_bd_net [get_bd_ports gpio_jtag_tms_i] [get_bd_pins xlslice_2/Dout]
connect_bd_net [get_bd_ports gpio_jtag_trst_ni] [get_bd_pins xlslice_3/Dout]


create_bd_cell -type ip -vlnv xilinx.com:ip:xlconcat:2.1 xlconcat_0
create_bd_cell -type ip -vlnv xilinx.com:ip:xlconstant:1.1 xlconstant_0
set_property -dict [list CONFIG.CONST_WIDTH {2} CONFIG.CONST_VAL {0b11}] [get_bd_cells xlconstant_0]

set_property -dict [list CONFIG.IN0_WIDTH.VALUE_SRC USER CONFIG.IN1_WIDTH.VALUE_SRC USER CONFIG.IN2_WIDTH.VALUE_SRC USER] [get_bd_cells xlconcat_0]
set_property -dict [list CONFIG.NUM_PORTS {3} CONFIG.IN0_WIDTH {2} CONFIG.IN2_WIDTH {2}] [get_bd_cells xlconcat_0]
connect_bd_net [get_bd_pins xlconstant_0/dout] [get_bd_pins xlconcat_0/In2]
connect_bd_net [get_bd_pins xlconstant_0/dout] [get_bd_pins xlconcat_0/In0]
connect_bd_net [get_bd_pins xlconcat_0/dout] [get_bd_pins processing_system7_0/GPIO_I]
connect_bd_net [get_bd_ports gpio_jtag_tdo_o] [get_bd_pins xlconcat_0/In1]

# ILA
#create_bd_cell -type ip -vlnv xilinx.com:ip:ila:6.2 ila_0
#set_property -dict [list CONFIG.PCW_EN_CLK0_PORT {1}] [get_bd_cells processing_system7_0]
#connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins ila_0/clk]
#set_property -dict [list CONFIG.C_NUM_OF_PROBES {4} CONFIG.C_ENABLE_ILA_AXI_MON {false} CONFIG.C_MONITOR_TYPE {Native}] [get_bd_cells ila_0]
#connect_bd_net [get_bd_pins ila_0/probe0] [get_bd_pins xlslice_0/Dout]
#connect_bd_net [get_bd_pins ila_0/probe1] [get_bd_pins xlslice_1/Dout]
#connect_bd_net [get_bd_pins ila_0/probe2] [get_bd_pins xlslice_2/Dout]
#connect_bd_net [get_bd_pins ila_0/probe3] [get_bd_pins xlslice_3/Dout]

set_property -dict [list CONFIG.PCW_UART1_PERIPHERAL_ENABLE {1} CONFIG.PCW_UART1_UART1_IO {EMIO}] [get_bd_cells processing_system7_0]
#set_property -dict [list CONFIG.PCW_UART0_PERIPHERAL_ENABLE {1} CONFIG.PCW_UART0_GRP_FULL_ENABLE {0}] [get_bd_cells processing_system7_0]
make_bd_intf_pins_external  [get_bd_intf_pins processing_system7_0/UART_1]
set_property name UART [get_bd_intf_ports UART_1_0]

# ADD EXTERNAL HP_AXI PORT
#set_property -dict [list CONFIG.PCW_USE_S_AXI_HP0 {1} CONFIG.PCW_S_AXI_HP0_DATA_WIDTH {64}] [get_bd_cells processing_system7_0]
#make_bd_intf_pins_external  [get_bd_intf_pins processing_system7_0/S_AXI_HP0]
#set_property -dict [list CONFIG.PROTOCOL {AXI4}] [get_bd_intf_ports S_AXI_HP0_0]
#set_property name AXI_HP [get_bd_intf_ports S_AXI_HP0_0]
#make_bd_pins_external  [get_bd_pins processing_system7_0/S_AXI_HP0_ACLK]
#set_property name AXI_HP_ACLK [get_bd_ports S_AXI_HP0_ACLK_0]



#set_property -dict [list CONFIG.PCW_EN_CLK0_PORT {1}] [get_bd_cells processing_system7_0]
#set_property -dict [list CONFIG.PCW_FPGA0_PERIPHERAL_FREQMHZ {100}] [get_bd_cells processing_system7_0]


# ADD SYSTEM ILA FOR HP PORT
#create_bd_cell -type ip -vlnv xilinx.com:ip:system_ila:1.1 system_ila_0
#connect_bd_net [get_bd_ports AXI_HP_ACLK] [get_bd_pins processing_system7_0/FCLK_CLK0]
#connect_bd_net [get_bd_pins system_ila_0/clk] [get_bd_pins processing_system7_0/FCLK_CLK0]
#create_bd_cell -type ip -vlnv xilinx.com:ip:xlconstant:1.1 xlconstant_1
#connect_bd_net [get_bd_pins system_ila_0/resetn] [get_bd_pins xlconstant_1/dout]
#connect_bd_intf_net [get_bd_intf_pins system_ila_0/SLOT_0_AXI] [get_bd_intf_pins processing_system7_0/S_AXI_HP0]
#connect_debug_port dbg_hub/clk [get_bd_pins processing_system7_0/S_AXI_HP0_ACLK]


save_bd_design
close_bd_design "processing_system"

set wrapper_path [ make_wrapper -fileset sources_1 -files [ get_files -norecurse processing_system.bd ] -top ]
add_files -norecurse -fileset sources_1 $wrapper_path

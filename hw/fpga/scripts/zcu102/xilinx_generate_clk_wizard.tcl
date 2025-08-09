# Copyright 2022 EPFL
# Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
# SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
# Define design macros

set design_name xilinx_clk_wizard

# Create block design
create_bd_design $design_name

# Create instance and set properties
create_bd_cell -type ip -vlnv xilinx.com:ip:clk_wiz:6.0 clk_wiz_0

set_property -dict [list \
  CONFIG.PRIMITIVE {PLL} \
  CONFIG.CLK_IN1_BOARD_INTERFACE {user_si570_sysclk} \
  CONFIG.CLKOUT1_REQUESTED_OUT_FREQ {15} \
  CONFIG.PRIM_SOURCE {Differential_clock_capable_pin} \
  CONFIG.CLKIN1_JITTER_PS {33.330000000000005} \
  CONFIG.CLKOUT1_DRIVES {Buffer} \
  CONFIG.CLKOUT2_DRIVES {Buffer} \
  CONFIG.CLKOUT3_DRIVES {Buffer} \
  CONFIG.CLKOUT4_DRIVES {Buffer} \
  CONFIG.CLKOUT5_DRIVES {Buffer} \
  CONFIG.CLKOUT6_DRIVES {Buffer} \
  CONFIG.CLKOUT7_DRIVES {Buffer} \
  CONFIG.MMCM_DIVCLK_DIVIDE {2} \
  CONFIG.MMCM_BANDWIDTH {OPTIMIZED} \
  CONFIG.MMCM_CLKFBOUT_MULT_F {5} \
  CONFIG.MMCM_CLKIN1_PERIOD {3.333} \
  CONFIG.MMCM_CLKIN2_PERIOD {10.0} \
  CONFIG.MMCM_COMPENSATION {AUTO} \
  CONFIG.MMCM_CLKOUT0_DIVIDE_F {50} \
  CONFIG.CLKOUT1_JITTER {183.964} \
  CONFIG.CLKOUT1_PHASE_ERROR {107.936} \
  ] [get_bd_cells clk_wiz_0]


# Create ports
make_bd_pins_external [get_bd_cells clk_wiz_0]
make_bd_intf_pins_external [get_bd_cells clk_wiz_0]

# Save and close block design
save_bd_design
close_bd_design $design_name

# Create wrapper
set wrapper_path [ make_wrapper -fileset sources_1 -files [ get_files -norecurse xilinx_clk_wizard.bd ] -top ]
add_files -norecurse -fileset sources_1 $wrapper_path

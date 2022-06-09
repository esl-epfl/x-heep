
# Define design macros
set design_name                 xilinx_clk_wizard
set in_clk_freq_hz              125000000
set out_clk_freq_hz             20000000
set clkin1_jitter_ps            80.0
set clkout1_jitter              172.798
set clkout1_phase_error         96.948
set clkout1_requested_out_freq  20
set mmcm_clkfbout_mult_f        8.000
set mmcm_clkin1_period          8.000
set mmcm_clkout0_divide_f       50.000
set prim_in_freq                125.000
set use_locked                  false
set use_reset                   false

# Create block design
create_bd_design $design_name

# Create ports
set clk_125MHz [ create_bd_port -dir I -type clk -freq_hz $in_clk_freq_hz clk_125MHz ]
set clk_out1_0 [ create_bd_port -dir O -type clk clk_out1_0 ]
set_property -dict [ list CONFIG.FREQ_HZ $out_clk_freq_hz ] $clk_out1_0

# Create instance and set properties
set clk_wiz_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:clk_wiz:6.0 clk_wiz_0 ]
set_property -dict [ list \
 CONFIG.CLKIN1_JITTER_PS $clkin1_jitter_ps \
 CONFIG.CLKOUT1_JITTER $clkout1_jitter \
 CONFIG.CLKOUT1_PHASE_ERROR $clkout1_phase_error \
 CONFIG.CLKOUT1_REQUESTED_OUT_FREQ $clkout1_requested_out_freq \
 CONFIG.MMCM_CLKFBOUT_MULT_F $mmcm_clkfbout_mult_f \
 CONFIG.MMCM_CLKIN1_PERIOD $mmcm_clkin1_period \
 CONFIG.MMCM_CLKOUT0_DIVIDE_F $mmcm_clkout0_divide_f \
 CONFIG.PRIM_IN_FREQ $prim_in_freq \
 CONFIG.USE_LOCKED $use_locked \
 CONFIG.USE_RESET $use_reset \
] $clk_wiz_0

# Create port connections
connect_bd_net -net clk_in1_0_1 [get_bd_ports clk_125MHz] [get_bd_pins clk_wiz_0/clk_in1]
connect_bd_net -net clk_wiz_0_clk_out1 [get_bd_ports clk_out1_0] [get_bd_pins clk_wiz_0/clk_out1]

# Save and close block design
save_bd_design
close_bd_design $design_name

# create wrapper
set wrapper_path [make_wrapper -fileset sources_1 -files [ get_files -norecurse xilinx_clk_wizard.bd] -top]
add_files -norecurse -fileset sources_1 $wrapper_path

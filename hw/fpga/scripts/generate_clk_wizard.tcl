# Proc to create BD clk_wizard
proc cr_bd_clk_wizard { parentCell } {

  # CHANGE DESIGN NAME HERE
  set design_name clk_wizard

  common::send_gid_msg -ssname BD::TCL -id 2010 -severity "INFO" "Currently there is no design <$design_name> in project, so creating one..."

  create_bd_design $design_name

  set bCheckIPsPassed 1
  ##################################################################
  # CHECK IPs
  ##################################################################
  set bCheckIPs 1
  if { $bCheckIPs == 1 } {
     set list_check_ips "\
  xilinx.com:ip:clk_wiz:6.0\
  "

   set list_ips_missing ""
   common::send_gid_msg -ssname BD::TCL -id 2011 -severity "INFO" "Checking if the following IPs exist in the project's IP catalog: $list_check_ips ."

   foreach ip_vlnv $list_check_ips {
      set ip_obj [get_ipdefs -all $ip_vlnv]
      if { $ip_obj eq "" } {
         lappend list_ips_missing $ip_vlnv
      }
   }

   if { $list_ips_missing ne "" } {
      catch {common::send_gid_msg -ssname BD::TCL -id 2012 -severity "ERROR" "The following IPs are not found in the IP Catalog:\n  $list_ips_missing\n\nResolution: Please add the repository containing the IP(s) to the project." }
      set bCheckIPsPassed 0
   }

  }

  if { $bCheckIPsPassed != 1 } {
    common::send_gid_msg -ssname BD::TCL -id 2023 -severity "WARNING" "Will not continue with creation of design due to the error(s) above."
    return 3
  }

  variable script_folder

  if { $parentCell eq "" } {
     set parentCell [get_bd_cells /]
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2090 -severity "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2091 -severity "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj


  # Create interface ports

  # Create ports
  set clk_125MHz [ create_bd_port -dir I -type clk -freq_hz 125000000 clk_125MHz ]
  set clk_out1_0 [ create_bd_port -dir O -type clk clk_out1_0 ]
  set_property -dict [ list \
   CONFIG.FREQ_HZ {20000000} \
 ] $clk_out1_0

  # Create instance: clk_wiz_0, and set properties
  set clk_wiz_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:clk_wiz:6.0 clk_wiz_0 ]
  set_property -dict [ list \
   CONFIG.CLKIN1_JITTER_PS {80.0} \
   CONFIG.CLKOUT1_JITTER {172.798} \
   CONFIG.CLKOUT1_PHASE_ERROR {96.948} \
   CONFIG.CLKOUT1_REQUESTED_OUT_FREQ {20} \
   CONFIG.MMCM_CLKFBOUT_MULT_F {8.000} \
   CONFIG.MMCM_CLKIN1_PERIOD {8.000} \
   CONFIG.MMCM_CLKOUT0_DIVIDE_F {50.000} \
   CONFIG.PRIM_IN_FREQ {125.000} \
   CONFIG.USE_LOCKED {false} \
   CONFIG.USE_RESET {false} \
 ] $clk_wiz_0

  # Create port connections
  connect_bd_net -net clk_in1_0_1 [get_bd_ports clk_125MHz] [get_bd_pins clk_wiz_0/clk_in1]
  connect_bd_net -net clk_wiz_0_clk_out1 [get_bd_ports clk_out1_0] [get_bd_pins clk_wiz_0/clk_out1]

  # Create address segments


  # Restore current instance
  current_bd_instance $oldCurInst

  validate_bd_design
  save_bd_design
  close_bd_design $design_name
}
# End of cr_bd_clk_wizard()
cr_bd_clk_wizard ""
set_property REGISTERED_WITH_MANAGER "1" [get_files clk_wizard.bd ]
set_property SYNTH_CHECKPOINT_MODE "Hierarchical" [get_files clk_wizard.bd ]

#call make_wrapper to create wrapper files
if { [get_property IS_LOCKED [ get_files -norecurse clk_wizard.bd] ] == 1  } {
  import_files -fileset sources_1 [file normalize "${origin_dir}/tapeout/core-v-mini-mcu/build/openhwgroup.org_systems_core-v-mini-mcu_0/pynq-z2-vivado/openhwgroup.org_systems_core-v-mini-mcu_0.gen/sources_1/bd/clk_wizard/hdl/clk_wizard_wrapper.v" ]
} else {
  set wrapper_path [make_wrapper -fileset sources_1 -files [ get_files -norecurse clk_wizard.bd] -top]
  add_files -norecurse -fileset sources_1 $wrapper_path
}

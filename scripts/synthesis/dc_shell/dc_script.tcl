# Copyright 2022 OpenHW Group
# Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
# SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1


set SET_LIBS ${SCRIPT_DIR}/set_libs.tcl
set CONSTRAINTS ${SCRIPT_DIR}/set_constraints.tcl

remove_design -all

source ${SET_LIBS}

define_design_lib WORK -path ./work

source ${READ_SOURCES}.tcl

elaborate ${TOP_MODULE}
link

load_upf ../../../core-v-mini-mcu.dc.upf

write -f ddc -hierarchy -output ${REPORT_DIR}/precompiled.ddc

source ${CONSTRAINTS}

report_clocks > ${REPORT_DIR}/clocks.rpt
report_timing -loop -max_paths 10 > ${REPORT_DIR}/timing_loop.rpt


compile_ultra -no_autoungroup -no_boundary_optimization -timing -gate_clock

write -f ddc -hierarchy -output ${REPORT_DIR}/compiled.ddc

change_names -rules verilog -hier

write -format verilog -hier -o netlist.v

report_timing -nosplit > ${REPORT_DIR}/timing.rpt
report_area -hier -nosplit > ${REPORT_DIR}/area.rpt
report_resources -hierarchy > ${REPORT_DIR}/resources.rpt



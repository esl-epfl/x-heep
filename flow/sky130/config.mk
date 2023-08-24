export DESIGN_NICKNAME = core_v_mini_mcu
export DESIGN_NAME = core_v_mini_mcu
export PLATFORM    = sky130hd

export VERILOG_FILES = design.sv 

export ADDITIONAL_LIBS = $(wildcard ./lib/*.lib)

export SDC_FILE      = ./constraint.sdc
export ABC_AREA      = 1

export SYNTH_HIERARCHICAL = 1
export MAX_UNGROUP_SIZE = 100
# Adders degrade setup repair
export ADDER_MAP_FILE :=

export CORE_UTILIZATION = 20
export CORE_ASPECT_RATIO = 1
export CORE_MARGIN = 2

export PLACE_DENSITY_LB_ADDON = 0.2
export PLACE_DENSITY          = 0.2

export RTLMP_FLOW = True

export MACRO_PLACE_HALO = 70 70
export MACRO_PLACE_CHANNEL = 140 140

export FASTROUTE_TCL = $(PLATFORM_DIR)/fastroute.tcl



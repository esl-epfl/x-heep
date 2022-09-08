export PLATFORM    = sky130hd

export VERILOG_FILES = design.sv # $(shell cat "files.txt")

export ADDITIONAL_LIBS = $(wildcard ./lib/*.lib)

export SDC_FILE      = ./constraint.sdc
export ABC_AREA      = 1

export SYNTH_HIERARCHICAL = 1
export MAX_UNGROUP_SIZE = 100
# Adders degrade setup repair
export ADDER_MAP_FILE :=

export CORE_UTILIZATION = 40
export CORE_ASPECT_RATIO = 1
export CORE_MARGIN = 2

export PLACE_DENSITY_LB_ADDON = 0.2

export FASTROUTE_TCL = $(PLATFORM_DIR)/fastroute_base.tcl
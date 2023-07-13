export PLATFORM    = sky130hd

export VERILOG_FILES = design.sv # $(shell cat "files.txt")

export ADDITIONAL_LIBS = $(wildcard ./lib/*.lib)

export SDC_FILE      = ./constraint.sdc
export ABC_AREA      = 1

export SYNTH_HIERARCHICAL = 1
export MAX_UNGROUP_SIZE = 100
# Adders degrade setup repair
export ADDER_MAP_FILE :=

export CORE_UTILIZATION = 5
export CORE_ASPECT_RATIO = 1
export CORE_MARGIN = 2

export PLACE_DENSITY_LB_ADDON = 0.2

export FASTROUTE_TCL = $(PLATFORM_DIR)/fastroute_base.tcl

export ADDITIONAL_LEFS = /home/cerdogan/x-heep/hw/asic/sky130/sky130_sram_4kbyte_1rw_32x1024_8.lef
 
export ADDITIONAL_LIBS = /home/cerdogan/x-heep/hw/asic/sky130/sky130_sram_4kbyte_1rw_32x1024_8_TT_1p8V_25C.lib

export ADDITIONAL_GDS = /home/cerdogan/x-heep/hw/asic/sky130/sky130_sram_4kbyte_1rw_32x1024_8.gds


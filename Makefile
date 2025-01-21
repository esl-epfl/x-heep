# Copyright EPFL contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

MAKE                       = make

# Get the absolute path
mkfile_path := $(shell dirname "$(realpath $(firstword $(MAKEFILE_LIST)))")
$(info $$You are executing from: $(mkfile_path))

# Include the self-documenting tool
export FILE_FOR_HELP=$(mkfile_path)/Makefile

help:
	${mkfile_path}/util/MakefileHelp

# Setup to autogenerate python virtual environment
VENVDIR?=$(WORKDIR)/.venv
REQUIREMENTS_TXT?=$(wildcard python-requirements.txt)
include Makefile.venv

# FUSESOC and Python values (default)
ifndef CONDA_DEFAULT_ENV
$(info USING VENV)
FUSESOC = $(PWD)/$(VENV)/fusesoc
PYTHON  = $(PWD)/$(VENV)/python
else
$(info USING MINICONDA $(CONDA_DEFAULT_ENV))
FUSESOC := $(shell which fusesoc)
PYTHON  := $(shell which python)
endif

# Project options are based on the app to be build (default - hello_world)
PROJECT  ?= hello_world

LINK_FOLDER ?= $(mkfile_path)/sw/linker

# Linker options are 'on_chip' (default),'flash_load','flash_exec','freertos'
LINKER   ?= on_chip

# Target options are 'sim' (default) and 'pynq-z2' and 'nexys-a7-100t'
TARGET   	?= sim
MCU_CFG_PERIPHERALS  	?= mcu_cfg.hjson
X_HEEP_CFG  ?= configs/general.hjson
PAD_CFG  	?= pad_cfg.hjson
EXT_PAD_CFG ?=


# Compiler options are 'gcc' (default) and 'clang'
COMPILER ?= gcc

# Compiler prefix options are 'riscv32-unknown-' (default)
COMPILER_PREFIX ?= riscv32-unknown-

# Compiler flags to be passed (for both linking and compiling)
COMPILER_FLAGS ?=

# Arch options are any RISC-V ISA string supported by the CPU. Default 'rv32imc'
ARCH     ?= rv32imc

# Path relative from the location of sw/Makefile from which to fetch source files. The directory of that file is the default value.
SOURCE 	 ?= $(".")

# Simulation engines options are verilator (default) and questasim
SIMULATOR ?= verilator

# SIM_ARGS: Additional simulation arguments for run-app-verilator based on input parameters:
# - MAX_SIM_TIME: Maximum simulation time in clock cycles (unlimited if not provided)
SIM_ARGS += $(if $(MAX_SIM_TIME),+max_sim_time=$(MAX_SIM_TIME))

# Timeout for simulation, default 120
TIMEOUT ?= 120

# Flash read address for testing, in hexadecimal format 0x0000
FLASHREAD_ADDR ?= 0x0
FLASHREAD_FILE ?= $(mkfile_path)/flashcontent.hex
FLASHREAD_BYTES ?= 256


#binary to store in flash memory
FLASHWRITE_FILE ?= $(mkfile_path)/sw/build/main.hex

#max address in the hex file, used to program the flash
ifeq ($(wildcard $(FLASHWRITE_FILE)),)
	MAX_HEX_ADDRESS  := 0
	MAX_HEX_ADDRESS_DEC := 0
	BYTES_AFTER_MAX_HEX_ADDRESS := 0
	FLASHWRITE_BYTES := 0
else
	MAX_HEX_ADDRESS  := $(shell cat $(FLASHWRITE_FILE) | grep "@" | tail -1 | cut -c2-)
	MAX_HEX_ADDRESS_DEC := $(shell printf "%d" 0x$(MAX_HEX_ADDRESS))
	BYTES_AFTER_MAX_HEX_ADDRESS := $(shell tac $(FLASHWRITE_FILE) | awk 'BEGIN {count=0} /@/ {print count; exit} {count++}')
	FLASHWRITE_BYTES := $(shell echo $(MAX_HEX_ADDRESS_DEC) + $(BYTES_AFTER_MAX_HEX_ADDRESS)*16 | bc)
endif

# Export variables to sub-makefiles
export

## @section Conda
conda: environment.yml
	conda env create -f environment.yml

environment.yml: python-requirements.txt
	util/python-requirements2conda.sh

## @section Installation

## Generates mcu files core-v-mini-mcu files and build the design with fusesoc
## @param CPU=[cv32e20(default),cv32e40p,cv32e40x,cv32e40px]
## @param BUS=[onetoM(default),NtoM]
## @param MEMORY_BANKS=[2(default) to (16 - MEMORY_BANKS_IL)]
## @param MEMORY_BANKS_IL=[0(default),2,4,8]
mcu-gen:
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir hw/core-v-mini-mcu/include --cpu $(CPU) --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --external_domains $(EXTERNAL_DOMAINS) --external_pads $(EXT_PAD_CFG) --pkg-sv hw/core-v-mini-mcu/include/core_v_mini_mcu_pkg.sv.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir hw/core-v-mini-mcu/ --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --tpl-sv hw/core-v-mini-mcu/system_bus.sv.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir hw/core-v-mini-mcu/ --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --tpl-sv hw/core-v-mini-mcu/system_xbar.sv.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir hw/core-v-mini-mcu/ --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --tpl-sv hw/core-v-mini-mcu/memory_subsystem.sv.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir hw/core-v-mini-mcu/ --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --tpl-sv hw/core-v-mini-mcu/peripheral_subsystem.sv.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir hw/core-v-mini-mcu/  --cpu $(CPU) --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --tpl-sv hw/core-v-mini-mcu/cpu_subsystem.sv.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir tb/ --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --tpl-sv tb/tb_util.svh.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir hw/system/ --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --tpl-sv hw/system/pad_ring.sv.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir hw/core-v-mini-mcu/ --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --tpl-sv hw/core-v-mini-mcu/core_v_mini_mcu.sv.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir hw/system/ --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --tpl-sv hw/system/x_heep_system.sv.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir sw/device/lib/runtime --cpu $(CPU) --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --external_domains $(EXTERNAL_DOMAINS) --header-c sw/device/lib/runtime/core_v_mini_mcu.h.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir sw/device/lib/runtime --cpu $(CPU) --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --external_domains $(EXTERNAL_DOMAINS) --header-c sw/device/lib/runtime/core_v_mini_mcu_memory.h.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir $(LINK_FOLDER) --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --linker_script $(LINK_FOLDER)/link.ld.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir . --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --pkg-sv ./core-v-mini-mcu.upf.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir . --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --pkg-sv ./core-v-mini-mcu.dc.upf.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir hw/ip/power_manager/rtl --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --external_domains $(EXTERNAL_DOMAINS) --pkg-sv hw/ip/power_manager/data/power_manager.sv.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir hw/ip/power_manager/data --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --external_domains $(EXTERNAL_DOMAINS) --pkg-sv hw/ip/power_manager/data/power_manager.hjson.tpl
	bash -c "cd hw/ip/power_manager; source power_manager_gen.sh; cd ../../../"
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir sw/device/lib/drivers/power_manager --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --external_domains $(EXTERNAL_DOMAINS) --pkg-sv sw/device/lib/drivers/power_manager/data/power_manager.h.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir hw/system/pad_control/data --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --external_pads $(EXT_PAD_CFG) --pkg-sv hw/system/pad_control/data/pad_control.hjson.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir hw/system/pad_control/rtl --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --external_pads $(EXT_PAD_CFG) --pkg-sv hw/system/pad_control/rtl/pad_control.sv.tpl
	bash -c "cd hw/system/pad_control; source pad_control_gen.sh; cd ../../../"
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir $(LINK_FOLDER) --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --linker_script $(LINK_FOLDER)/link_flash_exec.ld.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir $(LINK_FOLDER) --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --linker_script $(LINK_FOLDER)/link_flash_load.ld.tpl
	$(PYTHON) ./util/structs_periph_gen.py
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir hw/fpga/ --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --tpl-sv hw/fpga/sram_wrapper.sv.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir hw/fpga/scripts/ --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --tpl-sv hw/fpga/scripts/generate_sram.tcl.tpl
	$(PYTHON) util/mcu_gen.py --config $(X_HEEP_CFG) --cfg_peripherals $(MCU_CFG_PERIPHERALS) --pads_cfg $(PAD_CFG) --outdir sw/device/lib/crt/ --bus $(BUS) --memorybanks $(MEMORY_BANKS) --memorybanks_il $(MEMORY_BANKS_IL) --tpl-sv sw/device/lib/crt/crt0.S.tpl
	$(MAKE) verible

## Display mcu_gen.py help
mcu-gen-help:
	$(PYTHON) util/mcu_gen.py -h

## Runs verible formating
verible:
	util/format-verible;

## @section APP FW Build

## Generates the build folder in sw using CMake to build (compile and linking)
## @param PROJECT=<folder_name_of_the_project_to_be_built>
## @param TARGET=sim(default),systemc,pynq-z2,nexys-a7-100t,zcu104
## @param LINKER=on_chip(default),flash_load,flash_exec
## @param COMPILER=gcc(default),clang
## @param COMPILER_PREFIX=riscv32-unknown-(default)
## @param ARCH=rv32imc(default),<any_RISC-V_ISA_string_supported_by_the_CPU>
app: clean-app
	@$(MAKE) -C sw PROJECT=$(PROJECT) TARGET=$(TARGET) LINKER=$(LINKER) LINK_FOLDER=$(LINK_FOLDER) COMPILER=$(COMPILER) COMPILER_PREFIX=$(COMPILER_PREFIX) COMPILER_FLAGS=$(COMPILER_FLAGS) ARCH=$(ARCH) SOURCE=$(SOURCE) \
	|| { \
	echo "\033[0;31mHmmm... seems like the compilation failed...\033[0m"; \
	echo "\033[0;31mIf you do not understand why, it is likely that you either:\033[0m"; \
	echo "\033[0;31m  a) offended the Leprechaun of Electronics\033[0m"; \
	echo "\033[0;31m  b) forgot to run make mcu-gen\033[0m"; \
	echo "\033[0;31mI would start by checking b) if I were you!\033[0m"; \
	exit 1; \
	}

## Just list the different application names available
app-list:
	@echo "Note: Applications outside the X-HEEP sw/applications directory will not be listed."
	tree sw/applications/

## Compile all the apps present in the repo
app-compile-all:
	bash util/test_all.sh nosim $(LINKER) $(COMPILER) $(TIMEOUT)

## @section Simulation

## Verilator simulation with C++
verilator-sim:
	$(FUSESOC) --cores-root . run --no-export --target=sim --tool=verilator $(FUSESOC_FLAGS) --build openhwgroup.org:systems:core-v-mini-mcu ${FUSESOC_PARAM} 2>&1 | tee buildsim.log

## Verilator simulation with SystemC
verilator-sim-sc:
	$(FUSESOC) --cores-root . run --no-export --target=sim_sc --tool=verilator $(FUSESOC_FLAGS) --build openhwgroup.org:systems:core-v-mini-mcu ${FUSESOC_PARAM} 2>&1 | tee buildsim.log

## Questasim simulation
questasim-sim:
	$(FUSESOC) --cores-root . run --no-export --target=sim --tool=modelsim $(FUSESOC_FLAGS) --build openhwgroup.org:systems:core-v-mini-mcu ${FUSESOC_PARAM} 2>&1 | tee buildsim.log

## Questasim simulation with HDL optimized compilation
questasim-sim-opt: questasim-sim
	$(MAKE) -C build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-modelsim opt

## Questasim simulation with HDL optimized compilation and UPF power domain description
## @param FUSESOC_PARAM="--USE_UPF"
questasim-sim-opt-upf: questasim-sim
	$(MAKE) -C build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-modelsim opt-upf

## VCS simulation
## @param CPU=cv32e20(default),cv32e40p,cv32e40x,cv32e40px
## @param BUS=onetoM(default),NtoM
vcs-sim:
	$(FUSESOC) --cores-root . run --no-export --target=sim --tool=vcs $(FUSESOC_FLAGS) --build openhwgroup.org:systems:core-v-mini-mcu ${FUSESOC_PARAM} 2>&1 | tee buildsim.log

## VCS-AMS simulation:
vcs-ams-sim:
	$(FUSESOC) --cores-root . run --no-export --target=sim --flag "ams_sim" --tool=vcs $(FUSESOC_FLAGS) --build openhwgroup.org:systems:core-v-mini-mcu ${FUSESOC_PARAM} 2>&1 | tee buildsim.log

## xcelium simulation
xcelium-sim:
	$(FUSESOC) --cores-root . run --no-export --target=sim --tool=xcelium $(FUSESOC_FLAGS) --build openhwgroup.org:systems:core-v-mini-mcu ${FUSESOC_PARAM} 2>&1 | tee buildsim.log

## Generates the build output for helloworld application
## Uses verilator to simulate the HW model and run the FW
## UART Dumping in uart0.log to show recollected results
run-helloworld: mcu-gen verilator-sim
	$(MAKE) -C sw PROJECT=hello_world TARGET=$(TARGET) LINKER=$(LINKER) COMPILER=$(COMPILER) COMPILER_PREFIX=$(COMPILER_PREFIX) ARCH=$(ARCH);
	cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator; \
	./Vtestharness +firmware=../../../sw/build/main.hex $(SIM_ARGS); \
	cat uart0.log; \
	echo '<end of uart0.log>'; \
	cd ../../..;

## Generates the build output for freertos blinky application
## Uses verilator to simulate the HW model and run the FW
## UART Dumping in uart0.log to show recollected results
run-blinkyfreertos: mcu-gen verilator-sim
	$(MAKE) -C sw PROJECT=example_freertos_blinky TARGET=$(TARGET) LINKER=$(LINKER) COMPILER=$(COMPILER) COMPILER_PREFIX=$(COMPILER_PREFIX) ARCH=$(ARCH);
	cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator; \
	./Vtestharness +firmware=../../../sw/build/main.hex $(SIM_ARGS); \
	cat uart0.log; \
	echo '<end of uart0.log>'; \
	cd ../../..;

## First builds the app and then uses verilator to simulate the HW model and run the FW
## UART Dumping in uart0.log to show recollected results
run-app-verilator: app
	cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator; \
	./Vtestharness +firmware=../../../sw/build/main.hex $(SIM_ARGS); \
	cat uart0.log; \
	echo '<end of uart0.log>'; \
	cd ../../..;

## Simulate all the apps present in the repo
app-simulate-all:
	bash util/test_all.sh $(LINKER) $(COMPILER) $(TIMEOUT) $(SIMULATOR)

## @section Vivado

## Builds (synthesis and implementation) the bitstream for the FPGA version using Vivado
## @param FPGA_BOARD=nexys-a7-100t,pynq-z2,zcu104
## @param FUSESOC_FLAGS=--flag=<flagname>
vivado-fpga:
	$(FUSESOC) --cores-root . run --no-export --target=$(FPGA_BOARD) $(FUSESOC_FLAGS) --build openhwgroup.org:systems:core-v-mini-mcu ${FUSESOC_PARAM} 2>&1 | tee buildvivado.log

vivado-fpga-nobuild:
	$(FUSESOC) --cores-root . run --no-export --target=$(FPGA_BOARD) $(FUSESOC_FLAGS) --setup openhwgroup.org:systems:core-v-mini-mcu ${FUSESOC_PARAM} 2>&1 | tee buildvivado.log

vivado-fpga-pgm:
	$(MAKE) -C build/openhwgroup.org_systems_core-v-mini-mcu_0/$(FPGA_BOARD)-vivado pgm

## @section ASIC
## Note that for this step you need to provide technology-dependent files (e.g., libs, constraints)
asic:
	$(FUSESOC) --cores-root . run --no-export --target=asic_synthesis $(FUSESOC_FLAGS) --setup openhwgroup.org:systems:core-v-mini-mcu ${FUSESOC_PARAM} 2>&1 | tee builddesigncompiler.log

openroad-sky130:
	git checkout hw/vendor/pulp_platform_common_cells/*
	sed -i 's/(\*[^\n]*\*)//g' hw/vendor/pulp_platform_common_cells/src/*.sv
	$(FUSESOC) --verbose --cores-root . run --target=asic_yosys_synthesis --flag=use_sky130 openhwgroup.org:systems:core-v-mini-mcu ${FUSESOC_PARAM} 2>&1 | tee buildopenroad.log
	git checkout hw/vendor/pulp_platform_common_cells/*


## @section Program, Execute, and Debug w/ EPFL_Programmer

## Read the id from the EPFL_Programmer flash
flash-readid:
	cd sw/vendor/yosyshq_icestorm/iceprog; make; \
	./iceprog -d i:0x0403:0x6011 -I B -t;

## Loads the obtained binary to the EPFL_Programmer flash
flash-prog:
	cd sw/vendor/yosyshq_icestorm/iceprog; make; \
	./iceprog -a $(FLASHWRITE_BYTES) -d i:0x0403:0x6011 -I B $(FLASHWRITE_FILE);

## Read the EPFL_Programmer flash
flash-read:
	cd sw/vendor/yosyshq_icestorm/iceprog; make; \
	./iceprog -d i:0x0403:0x6011 -I B -o $(shell printf "%d" $(FLASHREAD_ADDR)) -R $(FLASHREAD_BYTES) $(FLASHREAD_FILE);

## Erase the EPFL_Programmer flash
flash-erase:
	cd sw/vendor/yosyshq_icestorm/iceprog; make; \
	./iceprog -d i:0x0403:0x6011 -I B -b;

## Run openOCD w/ EPFL_Programmer
openOCD_epflp:
	xterm -e openocd -f ./tb/core-v-mini-mcu-pynq-z2-esl-programmer.cfg;

## Run openOCD w/ BSCAN of the Pynq-Z2 board
openOCD_bscan:
	xterm -e openocd -f ./tb/core-v-mini-mcu-pynq-z2-bscan.cfg;

## Start GDB
gdb_connect:
	$(MAKE) -C sw gdb_connect

## @section Cleaning commands

## Clean the CMake build folder
app-clean:
	if [ -f "sw/build/Makefile" ]; then\
		$(MAKE) -C sw/build clean;\
	else\
		$(MAKE) app-restore;\
	fi

## Removes the CMake build folder
app-restore:
	rm -rf sw/build

## Removes the HW build folder
clean-sim:
	@rm -rf build

## Does the same as app-restore
clean-app: app-restore

## Removes the CMake build folder and the HW build folder
clean-all: app-restore clean-sim

# Copyright EPFL contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

# Setup to autogenerate python virtual environment
VENVDIR?=$(WORKDIR)/.venv
REQUIREMENTS_TXT?=$(wildcard python-requirements.txt)
include Makefile.venv

FUSESOC = $(VENV)/fusesoc
PYTHON = $(VENV)/python

# Makefile to generates core-v-mini-mcu files and build the design with fusesoc

.PHONY: clean help

TARGET ?= sim
MCU_CFG ?= mcu_cfg.hjson
PAD_CFG ?= pad_cfg.hjson

linux-femu-gen: mcu-gen
	$(PYTHON) util/mcu_gen.py --cfg $(MCU_CFG) --pads_cfg $(PAD_CFG) --outdir linux_femu/rtl/ --tpl-sv linux_femu/rtl/linux_femu.sv.tpl
	$(MAKE) verible

# Generates mcu files
mcu-gen: |venv
	$(PYTHON) util/mcu_gen.py --cfg $(MCU_CFG) --pads_cfg $(PAD_CFG) --outdir hw/core-v-mini-mcu/include --cpu $(CPU) --bus $(BUS) --memorybanks $(MEMORY_BANKS) --external_domains $(EXTERNAL_DOMAINS) --pkg-sv hw/core-v-mini-mcu/include/core_v_mini_mcu_pkg.sv.tpl
	$(PYTHON) util/mcu_gen.py --cfg $(MCU_CFG) --pads_cfg $(PAD_CFG) --outdir hw/core-v-mini-mcu/ --memorybanks $(MEMORY_BANKS) --tpl-sv hw/core-v-mini-mcu/system_bus.sv.tpl
	$(PYTHON) util/mcu_gen.py --cfg $(MCU_CFG) --pads_cfg $(PAD_CFG) --outdir tb/ --memorybanks $(MEMORY_BANKS) --tpl-sv tb/tb_util.svh.tpl
	$(PYTHON) util/mcu_gen.py --cfg $(MCU_CFG) --pads_cfg $(PAD_CFG) --outdir hw/system/ --tpl-sv hw/system/pad_ring.sv.tpl
	$(PYTHON) util/mcu_gen.py --cfg $(MCU_CFG) --pads_cfg $(PAD_CFG) --outdir hw/core-v-mini-mcu/ --tpl-sv hw/core-v-mini-mcu/core_v_mini_mcu.sv.tpl
	$(PYTHON) util/mcu_gen.py --cfg $(MCU_CFG) --pads_cfg $(PAD_CFG) --outdir hw/system/ --tpl-sv hw/system/x_heep_system.sv.tpl
	$(PYTHON) util/mcu_gen.py --cfg $(MCU_CFG) --pads_cfg $(PAD_CFG) --outdir sw/device/lib/runtime --cpu $(CPU) --memorybanks $(MEMORY_BANKS) --header-c sw/device/lib/runtime/core_v_mini_mcu.h.tpl
	$(PYTHON) util/mcu_gen.py --cfg $(MCU_CFG) --pads_cfg $(PAD_CFG) --outdir sw/linker --memorybanks $(MEMORY_BANKS) --linker_script sw/linker/link.ld.tpl
	$(PYTHON) util/mcu_gen.py --cfg $(MCU_CFG) --pads_cfg $(PAD_CFG) --outdir . --memorybanks $(MEMORY_BANKS) --pkg-sv ./core-v-mini-mcu.upf.tpl
	$(PYTHON) util/mcu_gen.py --cfg $(MCU_CFG) --pads_cfg $(PAD_CFG) --outdir hw/ip/power_manager/rtl --memorybanks $(MEMORY_BANKS) --external_domains $(EXTERNAL_DOMAINS) --pkg-sv hw/ip/power_manager/data/power_manager.sv.tpl
	$(PYTHON) util/mcu_gen.py --cfg $(MCU_CFG) --pads_cfg $(PAD_CFG) --outdir hw/ip/power_manager/data --memorybanks $(MEMORY_BANKS) --external_domains $(EXTERNAL_DOMAINS) --pkg-sv hw/ip/power_manager/data/power_manager.hjson.tpl
	bash -c "cd hw/ip/power_manager; source power_manager_gen.sh; cd ../../../"
	$(PYTHON) util/mcu_gen.py --cfg mcu_cfg.hjson --pads_cfg $(PAD_CFG) --outdir sw/device/lib/drivers/power_manager --memorybanks $(MEMORY_BANKS) --external_domains $(EXTERNAL_DOMAINS) --pkg-sv sw/device/lib/drivers/power_manager/data/power_manager.h.tpl
	$(PYTHON) util/mcu_gen.py --cfg mcu_cfg.hjson --pads_cfg $(PAD_CFG) --outdir hw/system/pad_control/data --pkg-sv hw/system/pad_control/data/pad_control.hjson.tpl
	$(PYTHON) util/mcu_gen.py --cfg mcu_cfg.hjson --pads_cfg $(PAD_CFG) --outdir hw/system/pad_control/rtl --pkg-sv hw/system/pad_control/rtl/pad_control.sv.tpl
	bash -c "cd hw/system/pad_control; source pad_control_gen.sh; cd ../../../"
	$(PYTHON) util/mcu_gen.py --cfg mcu_cfg.hjson --pads_cfg $(PAD_CFG) --outdir sw/linker --memorybanks $(MEMORY_BANKS) --linker_script sw/linker/link_flash_exec.ld.tpl
	$(PYTHON) util/mcu_gen.py --cfg mcu_cfg.hjson --pads_cfg $(PAD_CFG) --outdir sw/linker --memorybanks $(MEMORY_BANKS) --linker_script sw/linker/link_flash_load.ld.tpl
	$(PYTHON) ./util/structs_periph_gen.py
	$(MAKE) verible

# Display mcu_gen.py help
mcu-gen-help: |venv
	$(PYTHON) util/mcu_gen.py -h

# Runs verible formating
verible:
	util/format-verible;

app-helloworld:
	$(MAKE) -C sw applications/hello_world/hello_world.hex  TARGET=$(TARGET)

app-matadd:
	$(MAKE) -C sw applications/matadd/matadd.hex TARGET=$(TARGET)

app-ext-periph:
	$(MAKE) -C sw applications/example_external_peripheral/example_external_peripheral.hex TARGET=$(TARGET)

app-gpio-cnt:
	$(MAKE) -C sw applications/example_gpio_cnt/example_gpio_cnt.hex TARGET=$(TARGET)

app-dma:
	$(MAKE) -C sw applications/dma_example/dma_example.hex TARGET=$(TARGET)

app-spi-host:
	$(MAKE) -C sw applications/spi_host_example/spi_host_example.hex TARGET=$(TARGET)

app-spi-host-dma:
	$(MAKE) -C sw applications/spi_host_dma_example/spi_host_dma_example.hex TARGET=$(TARGET)

app-spi-flash-write:
	$(MAKE) -C sw applications/spi_flash_write/spi_flash_write.hex TARGET=$(TARGET)

# Tools specific fusesoc call

# Simulation
verilator-sim: |venv
	$(FUSESOC) --cores-root . run --no-export --target=sim --tool=verilator $(FUSESOC_FLAGS) --setup --build openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildsim.log

questasim-sim: |venv
	$(FUSESOC) --cores-root . run --no-export --target=sim --tool=modelsim $(FUSESOC_FLAGS) --setup --build openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildsim.log

questasim-sim-opt: questasim-sim
	$(MAKE) -C build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-modelsim opt

questasim-sim-opt-upf: questasim-sim
	$(MAKE) -C build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-modelsim opt-upf

vcs-sim: |venv
	$(FUSESOC) --cores-root . run --no-export --target=sim --tool=vcs $(FUSESOC_FLAGS) --setup --build openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildsim.log

run-helloworld: mcu-gen verilator-sim app-helloworld |venv
	cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator; \
	./Vtestharness +firmware=../../../sw/applications/hello_world/hello_world.hex; \
	cat uart0.log; \
	cd ../../..;

# Emulation
vivado-fpga: |venv
	$(FUSESOC) --cores-root . run --no-export --target=$(FPGA_BOARD) $(FUSESOC_FLAGS) --setup --build openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildvivado.log

vivado-fpga-nobuild: |venv
	$(FUSESOC) --cores-root . run --no-export --target=$(FPGA_BOARD) $(FUSESOC_FLAGS) --setup openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildvivado.log

# ASIC
asic: |venv
	$(FUSESOC) --cores-root . run --no-export --target=asic_synthesis --setup --build openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildsim.log

help:
	@echo "SIMULATION BUILD TARGETS"
	@echo "Build for simulation :"
	@echo "\tmake [verilator,questasim,vcs]-sim"
	@echo "\tex: make verilator-sim"
	@echo "Change cpu and/or bus:"
	@echo "\tmake <toolname>-sim CPU=[cv32e20(default),cv32e40p] BUS=[onetoM(default),NtoM]"
	@echo "\tex: make verilator-sim CPU=cv32e40p BUS=NtoM)"
	@echo "Add fusesoc flags:"
	@echo "\tmake <toolname>-sim FUSESOC_FLAGS=\"--flag=<flagname0> --flag=<flagname1>\""
	@echo "\tex: make verilator-sim FUSESOC_FLAGS=\"--flag=use_external_device_example --flag=use_jtag_dpi\""
	@echo ""
	@echo "FPGA EMULATION BUILD TARGET"
	@echo "Build with vivado"
	@echo "\tmake vivado-fpga[-nobuild] FPGA_BOARD=[nexys-a7-100t,pynq-z2] FUSESOC_FLAGS=--flag=<flagname>"
	@echo "\tex: make vivado-fpga FPGA_BOARD=nexys-a7-100t FUSESOC_FLAGS=--flag=use_bscane_xilinx"
	@echo ""
	@echo "ASIC IMPLEMENTATION"
	@echo "You need to provide technology-dependent files (e.g., libs, constraints)"
	@echo "\tmake asic"
	@echo "SOFTWARE BUILD TARGETS"
	@echo "Build example applications:"
	@echo "\tmake app-[helloworld,matadd,ext-periph,gpio-cnt]"
	@echo "\tex: make app-helloworld"

clean: clean-app clean-sim

clean-sim:
	@rm -rf build

clean-app:
	$(MAKE) -C sw clean

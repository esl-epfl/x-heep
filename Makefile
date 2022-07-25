# Copyright EPFL contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0


# Makefile to generates core-v-mini-mcu files and build the design with fusesoc

.PHONY: clean help

# Generates mcu files
mcu-gen-sv:
	python util/mcu_gen.py --cfg mcu_cfg.hjson --outdir hw/core-v-mini-mcu/include --cpu $(CPU) --bus $(BUS) --pkg-sv hw/core-v-mini-mcu/include/core_v_mini_mcu_pkg.sv.tpl

mcu-gen-c:
	python util/mcu_gen.py --cfg mcu_cfg.hjson --outdir sw/device/lib/runtime --cpu $(CPU) --bus $(BUS) --pkg-sv sw/device/lib/runtime/core_v_mini_mcu.h.tpl

# Display mcu_gen.py help
mcu-gen-help:
	python util/mcu_gen.py -h

# Runs verible formating
verible:
	util/format-verible;

app-helloworld: mcu-gen-c
	$(MAKE) -C sw applications/hello_world/hello_world.hex

app-matadd: mcu-gen-c
	$(MAKE) -C sw applications/matadd/matadd.hex

app-ext-periph: mcu-gen-c
	$(MAKE) -C sw applications/example_external_peripheral/example_external_peripheral.hex

app-gpio-cnt: mcu-gen-c
	$(MAKE) -C sw applications/example_gpio_cnt/example_gpio_cnt.hex

# Tools specific fusesoc call
verilator-sim: mcu-gen-sv mcu-gen-c verible
	fusesoc --cores-root . run --no-export --target=sim --tool=verilator $(FUSESOC_FLAGS) --setup --build openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildsim.log

questasim-sim: mcu-gen-sv mcu-gen-c verible
	fusesoc --cores-root . run --no-export --target=sim --tool=modelsim $(FUSESOC_FLAGS) --setup --build openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildsim.log

questasim-sim-opt: mcu-gen-sv mcu-gen-c verible
	fusesoc --cores-root . run --no-export --target=sim_opt --tool=modelsim $(FUSESOC_FLAGS) --setup --build openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildsim.log

vcs-sim: mcu-gen-sv mcu-gen-c verible
	fusesoc --cores-root . run --no-export --target=sim --tool=vcs $(FUSESOC_FLAGS) --setup --build openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildsim.log

run-helloworld: verilator-sim app-helloworld
# run-helloworld:
	cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator; \
	./Vtestharness +firmware=../. ./../sw/applications/hello_world/hello_world.hex; \
	cat uart0.log; \
	cd ../../..;

help:
	@echo "SIMULATION BUILD TARGETS"
	@echo "Build for simulation :"
	@echo "\tmake [verilator,questasim,vcs]-sim"
	@echo "\tex: make verilator-sim"
	@echo "Change cpu and/or bus:"
	@echo "\tmake <toolname>-sim CPU=<cpu_type> BUS=<bus_type>"
	@echo "\tex: make verilator-sim CPU=cv32e40p BUS=NtoM)"
	@echo "Add fusesoc flags:"
	@echo "\tmake <toolname>-sim FUSESOC_FLAGS=\"--flag=<flagname0> --flag=<flagname1>\""
	@echo "\tex: make verilator-sim FUSESOC_FLAGS=\"--flag=use_external_device_example --flag=use_jtag_dpi\""
	@echo ""
	@echo "SOFTWARE BUILD TARGETS"
	@echo "Build example applications:"
	@echo "\tmake app-[helloworld,matadd,ext-periph,gpio-cnt]"
	@echo "\tex: make app-helloworld"

clean: clean-app clean-sim

clean-sim:
	@rm -rf build

clean-app:
	$(MAKE) -C sw clean

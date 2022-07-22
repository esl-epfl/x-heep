# Generated register defines for memcopy_periph
#
# Copyright information found in source file:
# Copyright EPFL contributors.
#
# Licensing information found in source file:
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

# Makefile to generates core-v-mini-mcu files and build the design with fusesoc

# Generates mcu files
mcu-gen-sv:
	python util/mcu_gen.py --cfg mcu_cfg.hjson --outdir hw/core-v-mini-mcu/include --cpu $(CPU) --bus $(BUS) --pkg-sv hw/core-v-mini-mcu/include/core_v_mini_mcu_pkg.sv.tpl;

mcu-gen-c:
	python util/mcu_gen.py --cfg mcu_cfg.hjson --outdir sw/device/lib/runtime --cpu $(CPU) --bus $(BUS) --pkg-sv sw/device/lib/runtime/core_v_mini_mcu.h.tpl;

# Display mcu_gen.py help
mcu-gen-help:
	python util/mcu_gen.py -h

# Runs verible formating
verible:
	util/format-verible;

helloworld: mcu-gen-c
	$(MAKE) -C sw applications/hello_world/hello_world.hex;

# Tools specific fusesoc call
verilator-sim: mcu-gen-sv mcu-gen-c verible
	fusesoc --cores-root . run --no-export --target=sim --tool=verilator --setup --build openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildsim.log;

questasim-sim: mcu-gen-sv mcu-gen-c verible
	fusesoc --cores-root . run --no-export --target=sim --tool=modelsim --setup --build openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildsim.log;

questasim-sim-opt: mcu-gen-sv mcu-gen-c verible
	fusesoc --cores-root . run --no-export --target=sim_opt --tool=modelsim --setup --build openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildsim.log;

vcs-sim: mcu-gen-sv mcu-gen-c verible
	fusesoc --cores-root . run --no-export --target=sim --tool=vcs --setup --build openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildsim.log

help:
	@echo "Build for simulation : make <toolname>-sim"
	@echo "Change cpu and/or bus: make <toolname>-sim CPU=cv32e40p BUS=NtoM"

clean:
	@rm -rf build

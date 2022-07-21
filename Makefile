# Generated register defines for memcopy_periph
#
# Copyright information found in source file:
# Copyright EPFL contributors.
#
# Licensing information found in source file:
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

mcu-gen-compile:
	python util/mcu_gen.py --cfg mcu_cfg.hjson --outdir hw/core-v-mini-mcu/include --pkg-sv hw/core-v-mini-mcu/include/core_v_mini_mcu_pkg.sv.tpl;
	util/format-verible;
	fusesoc --cores-root . run --no-export --target=sim --tool=verilator --setup --build openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildsim.log;

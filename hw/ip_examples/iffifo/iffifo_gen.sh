# Copyright EPFL contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

echo "Generating RTL"
${PYTHON} ../../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py -r -t rtl data/iffifo.hjson
echo "Generating SW"
${PYTHON} ../../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py --cdefines -o ../../../sw/device/lib/drivers/iffifo/iffifo_regs.h data/iffifo.hjson

echo  -e "\033[5mIn `iffifo_reg_top.sv`, replace `logic [0:0] reg_steer;` by `logic [1:0] reg_steer;`\033[0m"

# Copyright 2022 EPFL
# Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
# SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

# Author: Pierre Guillod <pierre.guillod@epfl.ch>, EPFL, STI-SEL
# Date: 19.02.2022

echo "Generating RTL"
./../../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py -r -t rtl data/pdm2pcm.hjson
echo "Generating SW"
./../../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py --cdefines -o ../../../sw/device/lib/drivers/pdm2pcm/pdm2pcm_regs.h data/pdm2pcm.hjson

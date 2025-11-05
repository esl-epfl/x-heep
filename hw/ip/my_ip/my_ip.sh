# Copyright EPFL and Politecnico di Torino contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

echo "Generating RTL"
${PYTHON} ../../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py -r -t rtl data/my_ip.hjson
echo "Generating SW"
${PYTHON} ../../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py --cdefines -o ../../../sw/device/lib/drivers/my_ip/my_ip.h data/my_ip.hjson

# Copyright EPFL contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

echo "Generating SW"
${PYTHON} ../../../../../hw/vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py --cdefines -o hyperbus_regs.h ../../../../../hw/vendor/pulp_platform_hyperbus/hyperbus_regs.hjson

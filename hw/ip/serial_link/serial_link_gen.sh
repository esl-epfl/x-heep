# Copyright EPFL contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

#echo "Generating RTL"
#${PYTHON} ../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py  -r -t . ../../vendor/pulp_platform_serial_link/src/regs/serial_link.hjson
#echo "Generating SW"
#${PYTHON} ../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py --cdefines -o ../../../../../sw/device/lib/drivers/serial_link/serial_link_regs.h ../../vendor/pulp_platform_serial_link/src/regs/serial_link.hjson

## SINGLECHANNEL
echo "Generating RTL"
${PYTHON} ../../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py  -r -t . ../../vendor/pulp_platform_serial_link/src/regs/serial_link_single_channel.hjson
echo "Generating SW"
${PYTHON} ../../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py --cdefines -o ../../../sw/device/lib/drivers/serial_link/serial_link_single_channel_regs.h ../../vendor/pulp_platform_serial_link/src/regs/serial_link_single_channel.hjson

# MULTICHANNEL
#echo "Generating RTL"
#${PYTHON} ../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py  -r -t . serial_link.hjson
#echo "Generating SW"
#${PYTHON} ../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py --cdefines -o ../../../../../sw/device/lib/drivers/serial_link/serial_link_regs.h serial_link.hjson

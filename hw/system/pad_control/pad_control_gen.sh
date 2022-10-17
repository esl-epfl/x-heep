
echo "Generating RTL"
../../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py -r -t rtl ./data/pad_control.hjson
echo "Generating SW"
mkdir -p ../../../sw/device/lib/drivers/pad_control
../../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py --cdefines -o ../../../sw/device/lib/drivers/pad_control/pad_control_regs.h ./data/pad_control.hjson

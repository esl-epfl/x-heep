
echo "Generating RTL"
${PYTHON} ../../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py -r -t rtl ./data/power_manager.hjson
echo "Generating SW"
${PYTHON} ../../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py --cdefines -o ../../../sw/device/lib/drivers/power_manager/power_manager_regs.h ./data/power_manager.hjson


echo "Generating RTL"
../../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py -r -t rtl data/soc_ctrl.hjson
echo "Generating SW"
../../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py --cdefines -o ../../../sw/device/lib/drivers/soc_ctrl/soc_ctrl_regs.h data/soc_ctrl.hjson

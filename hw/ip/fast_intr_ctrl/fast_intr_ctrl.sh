
echo "Generating RTL"
../../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py -r -t rtl ./data/fast_intr_ctrl.hjson
echo "Generating SW"
../../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py --cdefines -o ../../../sw/device/lib/drivers/fast_intr_ctrl/fast_intr_ctrl_regs.h ./data/fast_intr_ctrl.hjson

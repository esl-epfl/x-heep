# Generate OpenTitan IPs

These are ips cloned from OpenTitan.
Via the `regtool` application, OpenTitan generates `rtl`, `header` files and more.
Please referes to its documentation available at: https://docs.opentitan.org/doc/rm/register_tool/

e.g.

```
cd hw/vendor/lowrisc_opentitan/hw/ip/uart
../../../util/regtool.py -r -t rtl data/uart.hjson
../../../util/regtool.py --cdefines -o ../../../../../../sw/device/lib/drivers/uart/uart_regs.h  data/uart.hjson
```


## Modified Generator

OpenTitan generates peripherals that are compliant to the TUL bus as for example the `UART` used above.
However, in this project we also rely on https://github.com/pulp-platform/register_interface
to generate a different (lighter) bus interface for new made peripheral as the `ip` to control the SoC called `soc_ctrl`. The register_interface repository contains an OpenTitan
version that has been patched to cope with the register interface instead of the TUL one.

### Generate the soc_ctrl IP

From the core-v-mini-mcu root folder:

```
cd hw/ip/soc_ctrl
./../../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py -r -t rtl data/soc_ctrl.hjson
./../../vendor/pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py --cdefines -o ../../../sw/device/lib/drivers/soc_ctrl/soc_ctrl_regs.h data/soc_ctrl.hjson
```


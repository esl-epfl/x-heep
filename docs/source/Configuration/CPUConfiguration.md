# CPU Configuration

The currently available CPUs can be found in `hw/core-v-mini-mcu/include/core_v_mini_mcu_pkg.sv.tpl` and are:
- cv32e20
- cv32e40p
- cv32e40x
- cv32e40px

Furthermore, some CPUs admit extra configuration parameters. The cve2 CPU has the following:
- You can set the `RV32E` to `True` or `False` to enable or disable the RV32E extension.
- You can set the `RV32M` to any of the `rv32m_e` enumeral values to modify the RV32M extension configuration. You can find the possible values in the `hw/vendor/openhwgroup_cv32e20/rtl/cve2_pkg.sv` file.

The CPU can be configured in three different ways:

## Arguments in the make command

For example:

```bash
make mcu-gen CPU=cv32e40p
```

This method has the highest priority and will override any other CPU configuration specified elsewhere.
However, you cannot specify extra CPU parameters.

## Python configuration file

You can set the CPU configuration in the Python configuration file used to generate the MCU. For example:

```python
from x_heep_gen.cpu.cpu import CPU
...
xheep.set_cpu(CPU("cv32e40p"))
```

With the cv32e20 CPU, to enable the RV32E extension and set the RV32M extension to `RV32MSlow`:

```python
from x_heep_gen.cpu.cv32e20 import cv32e20
...
xheep.set_cpu(cv32e20(rv32e=True, rv32m="RV32MSlow"))
```

## HJSON configuration file

You can also specify the CPU configuration in the `.hjson` configuration file:

```
cpu_type: cv32e40p
```

With the cv32e20 CPU, to enable the RV32E extension and set the RV32M extension to `RV32MSlow`:

```
cpu_type: cv32e20

cve2_rv32e: True
cve2_rv32m: RV32MSlow
```

# CPU Configuration

The CPU can be configured with the arguments in the make command. For example:

```bash
make mcu-gen CPU=cv32e40p
```

The currently available CPUs can be found in `hw/core-v-mini-mcu/cpu_subsystem.sv.tpl` and are:
- cv32e20
- cv32e40p
- cv32e40x
- cv32e40px

You can also specify the CPU configuration in the `mcu_cfg.hjson` file. The default is:

```
cpu_type: cv32e20
```

Furthermore, some CPUs admit extra configuration parameters. The cve2 CPU has the following:
- You can set the `RV32E` to `1'b0` or `1'b1` to enable or disable the RV32E extension.
- You can set the `RV32M` to any of the `rv32m_e` enumeral values to modify the RV32M extension configuration. You can find the possible values in the `hw/vendor/openhwgroup_cv32e20/rtl/cve2_pkg.sv` file.

For example, to enable the RV32E extension and set the RV32M extension to `RV32MSlow`:

```
cpu_type: cv32e20

cve2_rv32e: 1'b1
cve2_rv32m: RV32MSlow
```
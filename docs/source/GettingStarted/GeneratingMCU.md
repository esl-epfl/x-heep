# Generating the microcontroller

X-HEEP uses a series of Python scripts to generate the desired configuration of the MCU and its related files using Mako `.tpl` templates. For example, `hw/core-v-mini-mcu/core_v_mini_mcu.sv.tpl` will generate `hw/core-v-mini-mcu/core_v_mini_mcu.sv`.

For the default configuration of X-HEEP, you simply have to run:

```
make mcu-gen
```

You can change the default values with some arguments. For example, you can change the CPU type (cv32e20), the default bus type (onetoM), the default continuous memory size (2 banks), or the default interleaved memory size (0 banks) with:

```
make mcu-gen CPU=cv32e40p BUS=NtoM MEMORY_BANKS=12 MEMORY_BANKS_IL=4
```

This generates X-HEEP with the cv32e40p core, a parallel bus, and 16 memory banks (12 continuous and 4 interleaved), 32KB each, for a total memory of 512KB.

This method is limited to 32KB banks. To configure the RAM banks with more flexibility, edit `configs/general.hjson` or provide your own.

```
make mcu-gen X_HEEP_CFG=configs/my_config.hjson
```

Both methods can work together as the make target overrides the hjson configuration.

For more information see the [Configuration](/Configuration/Configuration.rst) section.

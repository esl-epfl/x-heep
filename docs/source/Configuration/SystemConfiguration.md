# System Configuration

The X-HEEP system can be configured by three methods:
- Directly passing arguments to the make command.
- Using an hjson file.
- Using a Python script.

If an `hjson` file or `python` script is used for configuration, the following parameters can be overriden from the make command:

- `CPU`
- `BUS`
- `MEMORY_BANKS`
- `MEMORY_BANKS_IL`

## Make Command

The easiest way of changing the default values is with some arguments in the make command. For example, you can change the CPU type (cv32e20), the default bus type (onetoM), the default continuous memory size (2 banks), or the default interleaved memory size (0 banks) with:

```bash
make mcu-gen CPU=cv32e40p BUS=NtoM MEMORY_BANKS=12 MEMORY_BANKS_IL=4
```

This generates X-HEEP with the cv32e40p core, a parallel bus, and 16 memory banks (12 continuous and 4 interleaved), 32KB each, for a total memory of 512KB.

This method has certain limitations, such as the size of the memory banks, which are fixed at 32KB. For compatibility reasons `MEMORY_BANKS` does not create linker sections while `MEMORY_BANKS_IL` does create a linker section. You can find more details in the documented code of the Makefile and in the specific configuration sections of this chapter.

## Hjson Configuration File

X-HEEP can be configured in more detail using an hjson file. The default configurations and examples are located in the `configs` directory.

To run `mcu-gen` with a specific `hjson` configuration file, use the following command:

```{code} bash
make mcu-gen X_HEEP_CFG=configs/general.hjson
```

## Python Configuration

A more complex configuration can be done using a python script. The default configurations and examples are located in the `configs` directory.

The Python script should include a `config()` function that takes no parameters and returns a {py:class}`x_heep_gen.system.XHeep` instance.
The configuration is similar to the hjson one. The order in which sections are added is also the one used in hardware.

Since not all configurations are yet supported by the python modelling of X-HEEP, the hjson configuration file must also be provided with the missing configurations.
If using the Python config file, the hjson parameters that are supported by Python will be ignored except for the peripherals. Any peripheral not configured in Python will be added from the hjson config.

To run `mcu-gen` with a specific python configuration script, use the following command:

```{code} bash
make mcu-gen X_HEEP_CFG=configs/python_unsupported.hjson PYTHON_X_HEEP_CFG=configs/general.py
```

The `general.py` file is shown below as an example:

```{literalinclude} ../../../configs/general.py
:language: python
```

# System Configuration

The X-HEEP system can be configured by three methods:
- Directly passing arguments to the make command.
- Using an hjson file.
- Using a Python script.

## Make Command

The easiest way of changing the default values is with some arguments in the make command. For example, you can change the CPU type (cv32e20), the default bus type (onetoM), the default continuous memory size (2 banks), or the default interleaved memory size (0 banks) with:

```bash
make mcu-gen CPU=cv32e40p BUS=NtoM MEMORY_BANKS=12 MEMORY_BANKS_IL=4
```

This generates X-HEEP with the cv32e40p core, a parallel bus, and 16 memory banks (12 continuous and 4 interleaved), 32KB each, for a total memory of 512KB.

This method has certain limitations, such as the size of the memory banks, which are fixed at 32KB. You can find more details in the documented code of the Makefile.

## Hjson Configuration File

X-HEEP can be configured in more detail using an hjson file. The default configurations and examples are located in the `configs` directory.

To run `mcu-gen` with a specific `hjson` configuration file, use the following command:

```{code} bash
make mcu-gen X_HEEP_CFG=configs/name_of_file.hjson
```

The `general.hjson` file is shown below as an example:

```{literalinclude} ../../../configs/general.hjson
```

If an `hjson` file is used for configuration, the following parameters can be set in the make command to override the configuration:

- `BUS`
- `MEMORY_BANKS`
- `MEMORY_BANKS_IL` 

These will replace the configuration used in the hjson file. When one parameter is not provided the configuration files value is used.
The memory banks configured this way will only be 32KB.
For compatibility reasons `MEMORY_BANKS` does not create linker sections while `MEMORY_BANKS_IL` does create a linker section.

## Python Configuration

A more complex configuration can be done using a python script. The default configurations and examples are located in the `configs` directory.

The script should include a config function that returns a {py:class}`x_heep_gen.system.XHeep` instance.
The configuration is similar to the hjson one. The order in which sections are added is also the one used in hardware.
The script writer is responsible to call {py:meth}`x_heep_gen.system.XHeep.build` to make the system ready to be used, {py:meth}`x_heep_gen.system.XHeep.validate` to check for errors in the configuration, and to raise an error in case of failure.

To run `mcu-gen` with a specific python configuration script, use the following command:

```{code} bash
make mcu-gen X_HEEP_CFG=configs/name_of_file.py
```

The `example.py` file is shown below as an example:

```{literalinclude} ../../../configs/example.py
:language: python
```

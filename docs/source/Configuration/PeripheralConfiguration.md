# Peripheral Configuration

The default configuration of the peripherals can be found in either [mcu_cfg.hjson](https://github.com/esl-epfl/x-heep/blob/main/mcu_cfg.hjson) or [configs/example.py](https://github.com/esl-epfl/x-heep/blob/main/configs/example.py). A custom configuration can be created through python configuration.

## Overall structure

Peripherals are split between two domains :
 - Base peripheral domain ({py:class}`x_heep_gen.peripherals.base_peripherals.BasePeripheralDomain`)
 - User peripheral domain ({py:class}`x_heep_gen.peripherals.user_peripherals.UserPeripheralDomain`)

 Each domain contains a base adress and a length, representing the memory space they occupy.

Each domain has its corresponding peripherals : 
 - Base peripherals (Mandatory peripherals)
 - User peripherals (Optionnal peripherals)

Every peripheral has at least an offset and a length, which represent its position in the corresponding domain. Some peripherals have their own registers, and thus have a linked `hjson` configuration file. Each peripheral must be stored in the corresponding domain. Every peripheral is represented as a python class, more information about each peripheral can be found [here](https://x-heep.readthedocs.io/en/latest/Configuration/x_heep_gen/index.html)

## Adding a custom configuration

A toy example is shown in `configs/example.py` :
```{literalinclude} ../../../configs/example.py
:language: python
:lines: 39-59
```

First, both domains must be created. If a domain is not created, X-HEEP will be built with the default configuration from `mcu_cfg.hjson`. Base Peripheral domain starts from 0x20000000, User peripheral domain starts from 0x30000000, and each fills 1MB.

Each peripheral has its own class, that must be imported from `x_heep_gen.peripherals.base_peripherals.py`or `x_heep_gen.peripherals.user_peripherals.py`.

When creating a peripheral, the offset, length, and other peripheral dependent information can be passed to the constructor. If not, default values will be assigned. Concerning the offsets, they will not be assigned until {py:meth}`x_heep_gen.system.XHeep.build` is called. In the same domain, peripherals with specific offsets and peripherals without a specified offset can coexist.

{py:meth}`x_heep_gen.system.XHeep.build` computes automatically the non defined offsets. A greedy algorithm places peripherals on free memory spaces in the corresponding peripheral domain, from the peripheral that takes the most memory to the one that takes the less. If there is not enough space, an error is thrown.

To link a custom configuration file to a peripheral, the method {py:meth}`x_heep_gen.peripherals.abstractions.DataConfiguration.custom_configuration` must be called on the peripheral with the path to the custom configuration file.

When the peripheral is configured, it can be added to the corresponding domain with {py:meth}`x_heep_gen.peripherals.abstractions.PeripheralDomain.add_peripheral`. All changes made after this call to the peripheral will not be recorded.

Since all base peripherals are mandatory, there is a method to add all base peripherals that were not added previously : {py:meth}`x_heep_gen.peripherals.base_peripherals.add_missing_peripherals`. The missing base peripherals are added with a default configuration based on [mcu_cfg.hjson](https://github.com/esl-epfl/x-heep/blob/main/mcu_cfg.hjson), but with undefined offsets (they will be computed during {py:meth}`x_heep_gen.system.XHeep.build`).

When method {py:meth}`x_heep_gen.system.XHeep.validate` is called, it performs basic sanity checks (all configuration files must exist, no peripheral should overlap another one, peripherals shouldn't be outside the domain, ...).


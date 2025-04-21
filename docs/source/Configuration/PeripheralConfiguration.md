# Peripheral Configuration

The default configuration of the peripherals is in `mcu_cfg.hjson`. A custom configuration can be created through python configuration.

## Overall structure

There are two kinds of peripherals : 
 - Base peripherals (Mandatory peripherals)
 - User peripherals (Optionnal peripherals)

These peripherals must be stored in their corresponding domains ({py:class}`x_heep_gen.peripherals.base_peripherals.BasePeripheralDomain` or {py:class}`x_heep_gen.peripherals.user_peripherals.UserPeripheralDomain`). Each domain contains a base adress and a length, representing the memory space they occupy.

Every peripheral has at least an offset and a length, which represent its position in the corresponding domain. Some peripherals have their own registers, and thus have a linked `hjson` configuration file.

## Adding a custom configuration

A toy example is shown in `configs/example.py` :
```{literalinclude} ../../../configs/example.py
:language: python
:lines: 39-59
```

First, both domains must be created. If a domain is not created, X-HEEP will be built with the default configuration from `mcu_cfg.hjson`. Base Peripheral domain starts from 0x20000000, User peripheral domain starts from 0x30000000, and each fills 1MB.

Each peripheral has its own class, that must be imported from `x_heep_gen.peripherals.base_peripherals.py`or `x_heep_gen.peripherals.user_peripherals.py`.

When creating a peripheral, the offset, length, and other peripheral dependent information can be passed to the constructor. If not, default values will be assigned. Concerning the offset, it will not be assigned until {py:meth}`x_heep_gen.system.XHeep.build` is called (offsets will be computed based on the free space left in the domain). In the same domain, peripherals with specific offsets and peripherals without a specified offset can coexist.

To link a custom configuration file to a peripheral, the method {py:meth}`x_heep_gen.peripherals.abstractions.DataConfiguration.custom_configuration` must be called on the peripheral with the path to the custom configuration file.

When the peripheral is configured, it can be added to the corresponding domain with {py:meth}`x_heep_gen.peripherals.abstractions.PeripheralDomain.add_peripheral`. All changes made after this call to the peripheral will not be recorded.

Since all base peripherals are mandatory, there is a method to add all base peripherals that were not added previously : {py:meth}`x_heep_gen.peripherals.base_peripherals.add_missing_peripherals`. The missing base peripherals are added without any offset, and thus their offsets will be computed based on the free space left when {py:meth}`x_heep_gen.system.XHeep.build` is called.

When method {py:meth}`x_heep_gen.system.XHeep.validate` is called, it performs basic sanity checks (all configuration files must exist, no peripheral should overlap another one, peripherals shouldn't be outside the domain, ...).


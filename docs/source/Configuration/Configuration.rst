Configuration
=============

Project Configuration
---------------------
The project can be configured either by a hjson file or a python script.
The default configurations and examples are located in the `config` directory.

Hjson Configuration File
~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../../../configs/example_interleaved.hjson
    :language: js

Bus Type Configuration
^^^^^^^^^^^^^^^^^^^^^^
The bus type can either be configured to `NtoM` or `onetoM`.

Ram Bank Configuration
^^^^^^^^^^^^^^^^^^^^^^

The system can be configured with banks of different sizes. The sizes should be a power of two in kiB.
All banks are configured in one continuous address region.


To configure interleaved banks the number and the size of the banks have to be provided.
The following restrictions apply: All banks must have the same size and a power of two banks must be configured.


For continuous banks the default mode, only the `sizes` filed is required.
It can be either the size in kiB of a single bank,
a dictionary of the same format containing more banks, or a list of multiple entries.
If the `num` field is also provided the configuration in the `sizes` field is repeated `num` times.

.. code:: js

    ram_banks: {
        code: {sizes: 64} // configure just one bank
        
        data: {
            type: continuous // the default, can be omitted
            num: 2
            sizes: 32
        }
        
        alt_data: {sizes: [32, 32]} // the same as data but with a list

        more_complex: {
        // This also works recursively so we can easily have different sizes of banks
        // and in bigger numbers without listing them all one by one.
            sizes: [
                {
                    num: 4
                    sizes: 8
                },
                {
                    num: 16
                    sizes: 4
                },
            ]
        }

    }


Linker Section Configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The linker script sections can be configured in two ways.
Either a section can be matched with a group of ram banks
or it can be manually defined.

To automatically add a section the entry `auto_section: auto` should be added to the banks.
It will add a section that matches the banks exactly, it can only be used in the non recursive part.
The name of the section is set with the name of the group of banks.

When manually setting the the linker section the name and the start address have to be provided.
Additionally the end is either provided by it's address or by the size of the section.
If no end is provided the end is inferred by the start of the next linker section.
Or if no section follows, the end address of the last ram bank.

Both configuration types can be freely combined as long as no section overlap.
All sections will e sorted by the configuration system.

The first two sections should always be code and data.
The other name can be used in code with a `.xheep_` prefix, like in `example_matadd_interleaved`

.. code:: c

    int32_t __attribute__((section(".xheep_data_interleaved"))) m_c[16*16];

.. code:: js

    {
        ram_address: 0
        bus_type: "onetoM",
        ram_banks: {
            code_and_data: {
                num: 2
                sizes: [32]
            }
            i_am_a_section_name: {
                auto_section: auto
                sizes: 16
            }
        }

        linker_sections: 
        [
            {
                name: code
                start: 0

                // Alternatively the end tag can be used to provide the end.
                size: 0x00000C800
            },
            {
                name: data
                start: 0x00000C800
                // The end of this section will be at the beginning of the next.
                // In this example the next section is i_am_a_section_name
            }
        ]
    }




Python Configuration
~~~~~~~~~~~~~~~~~~~~

The same can be done by using a python script

.. literalinclude:: ../../../configs/example.py
    :language: python


The script should include a config function that return an :py:class:`x_heep_gen.system.XHeep` instance.
The configuration is simmilar to the hjson one. The order in which sections are added is also the one used in hardware.
The script writer is responsible to call :py:meth:`x_heep_gen.system.XHeep.build` and :py:meth:`x_heep_gen.system.XHeep.validate` and to raise an error in case of failure.
The first does make the system ready to be used and the second does check for errors in the configuration.



Select Configuration File
~~~~~~~~~~~~~~~~~~~~~~~~~

To configure the system call

.. code:: bash

    make mcu-gen X_HEEP_CFG=configs/name_of_file.hjson



Other configurations
~~~~~~~~~~~~~~~~~~~~

The pads are configured in `pad_cfg.hjson`.

One part of the configuration is in `mcu_cfg.hjson`.

Additionally if a `hjson` file is ussed for configuration the following parameters can be set to the make command to override the configuration:

- `BUS=NtoM,onetoM`
- `MEMORY_BANKS=integer`
- `MEMORY_BANKS_IL=integer` 

They will replace the configuration used in the configuration file.
When one parameter is not provided the configuration files value is used.
The memory banks configured this way will only be of size 32kiB.
For compatibility reasons `MEMORY_BANKS` does not create linker sections while `MEMORY_BANKS_IL` does create a linker section.


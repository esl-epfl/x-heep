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
All banks are configured in one continuous address region. Gap in memory will trigger exceptions in the configuration script.
All banks should also be aligned to their size.

The ram is configured per section. For each section the size and the number of banks can be configured.
The banks can also be configured to be interleaved.
Each section should be named. The first two sections should always be code and data.
The other name can be used in code with a `.xheep_` prefix, like in `example_matadd_interleaved`

.. code:: c

    int32_t __attribute__((section(".xheep_data_interleaved"))) m_c[16*16];


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


Python Configuration
~~~~~~~~~~~~~~~~~~~~

The same can be done by using a python script

.. literalinclude:: ../../../configs/example.py
    :language: python


The script should include a config function that return an :py:class:`x_heep_gen.system.XHeep` instance.
The configuration is simmilar to the hjson one. The order in which sections are added is also the one used in hardware.
The script writer is responsible to call :py:meth:`x_heep_gen.system.XHeep.validate` and to raise an error in case of failure.



Select Configuration File
~~~~~~~~~~~~~~~~~~~~~~~~~

To configure the system call

.. code:: bash

    make mcu-gen X_HEEP_CFG=configs/name_of_file.hjson



Other configurations
~~~~~~~~~~~~~~~~~~~~

The pads are configured in `pad_cfg.hjson`.

One part of the configuration is in `mcu_cfg.hjson`.
# Extending X-HEEP via the eXtension interface

X-HEEP is meant to be extended with your own custom IPs. For this purpose we support the [CORE-V-XIF](https://docs.openhwgroup.org/projects/openhw-group-core-v-xif/en/latest/intro.html) interface with the [cv32e40x](https://github.com/openhwgroup/cv32e40x) RISC-V CPU.

## X-HEEP based open-source examples

Here you can find a list of X-HEEP based open-source examples. If you want to include your project in this list, please open an issue with a link to your repository.
To get started you will need to integrate both your coprocessor and x-heep into a new system. 

## How to integrate your xif-compliant coprocessor into X-HEEP

- Create a new top-level repository and [vendorize](https://docs.opentitan.org/doc/ug/vendor_hw/) or add as git submodules both your CORE-V-XIF compliant coprocessor and x-heep.
- Copy the `x-heep/hw/system/x_heep_system.sv` as your new top-level module. Then modify it as needed to include your co-processor and connect it to the `core_v_mini_mcu` with the xif.
- In the x-heep root directory, remember to run `make mcu-gen CPU=cv32e40x`

To add this new top-level module to the simulation/synthesis flow you can extend the [FuseSoC](https://fusesoc.readthedocs.io/en/stable/user/index.html) support of x-heep.

- Create a new `tb/testharness.sv` modifying the one in `x-heep/tb/testharness.sv` by adding your new top-level module.
- Create a new `scripts/sim` directory adapting the corresponding ones in `x-heep/scripts/sim`.
- Create a new `<your_toplevel_name>.core` FuseSoC file following this example for Modelsim simulation and adapting it to your particular needs:
    ~~~
    CAPI=2:

    # Solderpad Hardware License, Version 2.1, see LICENSE for details.
    # SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

    name: "[YOUR_FUSESOC_IP_NAME]"
    description: "[YOUR_IP_DESCRIPTION]"

    filesets:
    files_rtl_generic:
        depend:
        - openhwgroup.org:systems:core-v-mini-mcu
        - x-heep:ip:pad_control
        - [YOUR_COPROCESSOR]
        files:
        - hw/[YOUR_TOPLEVEL_SV]
        - hw/vendor/esl-epfl_x-heep/hw/system/pad_ring.sv  
        file_type: systemVerilogSource
    
    tb:
        depend:
        - x-heep::tb-utils
        files:
        - tb/testharness.sv   # Your modified testharness
        - hw/vendor/esl-epfl_x-heep/tb/tb_top.sv
        file_type: systemVerilogSource

    # Scripts for hooks
    pre_build_uartdpi:
        files:
        - scripts/sim/compile_uart_dpi.sh  # Your modified scripts
        file_type: user

    pre_build_remote_bitbang:
        files:
        - scripts/sim/compile_remote_bitbang.sh  # Your modified scripts
        file_type: user

    pre_patch_modelsim_Makefile:
        files:
        - hw/vendor/esl-epfl_x-heep/scripts/sim/modelsim/patch_modelsim_Makefile.py
        file_type: user

    parameters:
    PULP_XPULP:
        datatype: int
        paramtype: vlogparam
        default: 0
    JTAG_DPI:
        datatype: int
        paramtype: vlogparam
        default: 0
    USE_EXTERNAL_DEVICE_EXAMPLE:
        datatype: bool
        paramtype: vlogdefine
        default: false
    USE_UPF:
        datatype: bool
        paramtype: vlogdefine
        default: false

    scripts:
    pre_build_remote_bitbang:
        cmd:
        - sh
        - ../../../scripts/sim/compile_remote_bitbang.sh
    pre_build_uartdpi:
        cmd:
        - sh
        - ../../../scripts/sim/compile_uart_dpi.sh
    pre_patch_modelsim_Makefile:
        cmd:
        - python
        - ../../../hw/vendor/esl-epfl_x-heep/scripts/sim/modelsim/patch_modelsim_Makefile.py

    targets:
    default: &default_target
        filesets:
        - files_rtl_generic

    sim:
        <<: *default_target
        default_tool: modelsim
        filesets_append:
        - tb
        - tool_modelsim? (pre_build_remote_bitbang)
        - tool_modelsim? (pre_build_uartdpi)
        - tool_modelsim? (pre_patch_modelsim_Makefile)
        toplevel:
        - tb_top
        hooks:
        pre_build:
            - tool_modelsim? (pre_build_uartdpi)
            - tool_modelsim? (pre_build_remote_bitbang)
            - tool_modelsim? (pre_patch_modelsim_Makefile) # this is required by Questa 2020 on
        parameters:
        - PULP_XPULP=0
        - use_jtag_dpi? (JTAG_DPI=1)
        - "!use_jtag_dpi? (JTAG_DPI=0)"
        - use_external_device_example? (USE_EXTERNAL_DEVICE_EXAMPLE=true)
        - use_upf? (USE_UPF=true)
        tools:
        modelsim:
            vlog_options:
            - -override_timescale 1ns/1ps
            - -suppress vlog-2583
            - -suppress vlog-2577
            - -pedanticerrors
            - -define MODELSIM
            vsim_options:
            - -sv_lib ../../../hw/vendor/esl-epfl_x-heep/hw/vendor/lowrisc_opentitan/hw/dv/dpi/uartdpi/uartdpi
            - -sv_lib ../../../hw/vendor/esl-epfl_x-heep/hw/vendor/pulp_platform_pulpissimo/rtl/tb/remote_bitbang/librbs
    ~~~

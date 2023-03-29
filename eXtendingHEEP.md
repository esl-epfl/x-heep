# Extending X-HEEP

`X-HEEP` is meant to be extended with your own custom IPs. `X-HEEP` itself posseses a hardware-software framework capable of working standalone. If you want to extend it, you will need to merge your hardware and software with `X-HEEP`'s.

For this purpose we support the [CORE-V-XIF](https://docs.openhwgroup.org/projects/openhw-group-core-v-xif/en/latest/intro.html) interface with the [cv32e40x](https://github.com/openhwgroup/cv32e40x) RISCV-CPU, and we expose master and slave ports to/from the bus.

Here you can find a list of `X-HEEP` based open-source examples. If you want to include your project in this list, please open an issue with a link to your repository.

* [CGRA-X-HEEP](https://github.com/esl-epfl/cgra_x_heep): A CGRA loosely coupled with X-HEEP.


## Vendorizing X-HEEP

In order to vendorize `X-HEEP` create inside your repository's base directory (`BASE`) a `hw/vendor` directory containing a file named `esl_epfl_x_heep.vendor.hjson`:

<details>
    <summary>Example of esl_epfl_x_heep.vendor.hjson</summary>

```
// Copyright EPFL
// Licensed under the Solderpad Hardware License v2.1, see LICENSE.txt for details
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

{
  name: "esl_epfl_x_heep",
  target_dir: "esl_epfl_x_heep",

  upstream: {
    url: "https://github.com/esl-epfl/x-heep.git",
    rev: "main",
  },

  patch_dir: "patches/esl_epfl_x_heep",

  exclude_from_upstream: [
    ".github",
    "ci",
  ]
}

```
</details>

The branch `main` can be replaced with an specific commit. This is the recommended operation to prevent accidental updates to an incompatible version. 

In a directory `BASE/util` add the [vendor python script](https://github.com/lowRISC/opentitan/blob/master/util/vendor.py). 

To vendorize or revendorize the X-HEEP repository inside a `BASE/hw/vendor/esl_epfl_x_heep/` folder run the following command from your `BASE`. 
```
util/vendor.py --update hw/vendor/esl_epfl_x_heep.vendor.hjson 
```


## Extending Hardware
To get started you will need to integrate both your coprocessor or accelerator and `X-HEEP` into a new system.

### Proposed repository folder structure
The following is an example repository folder structure. 
    
    BASE
    ├── hw
    │   ├── TOP
    │   │   └── top.sv
    │   └── vendor
    │       ├── your_copro
    │       ├── your_copro.vendor.hjson
    │       ├── esl_epfl_x_heep
    │       └── esl_epfl_x_heep.vendor.hjson
    ├── top.core
    ├── scripts
    │   └── sim
    │       ├── compile_remote_bitbang.sh
    │       └── compile_uart_dpi.sh
    ├── tb
    │   └── testharness.sv
    ├── util
    │   └── vendor.py
    ├── Makefile
    └── sw

To achieve this:

* Create a new top-level repository (`BASE`) and [vendorize](#vendorizing-x-heep) (or add as git submodules) both your `CORE-V-XIF/OBI` compliant coprocessor/accelerator and `X-HEEP`.
* Copy the `x-heep/hw/system/x_heep_system.sv` as your new top-level module. Then modify it as needed to include your co-processor and connect it to the `core_v_mini_mcu` with the `XIF`.
* Before [building software](#building-software) remember to run `make mcu-gen CPU=cv32e40x`.

To add this new top-level module to the simulation/synthesis flow you can extend the [FuseSoC](https://fusesoc.readthedocs.io/en/stable/user/index.html) support of `X-HEEP`.

- Create a new `tb/testharness.sv` modifying the one in `x-heep/tb/testharness.sv` by adding your new top-level module.
- Create a new `scripts/sim` directory adapting the corresponding ones in `x-heep/scripts/sim`.
- Create a new `<your_toplevel_name>.core` FuseSoC file following this example for Modelsim simulation and adapting it to your particular needs:

<details>
  <summary>Example of .core</summary>

```
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
```

</details>


## Building Software

If `X-HEEP` is vendorized into your project, you can still have software source files in any directory of your convenience and build them using `X-HEEP`'s compilation flow. 

### Proposed repository folder structure
The following is an example repository folder structure.

    BASE
    ├── sw
    │   ├── applications
    │   │   └── your_app
    │   │       ├── main.c
    │   │       ├── your_app.c
    │   │       ├── your_app.h
    │   │       └── ...
    │   ├── build -> ../hw/vendor/esl_epfl_x_heep/sw/build
    │   ├── device -> ../hw/vendor/esl_epfl_x_heep/sw/device
    │   ├── linker -> ../hw/vendor/esl_epfl_x_heep/sw/linker
    │   └── external
    │       ├── drivers
    │       │   └── your_copro
    │       │   	├── your_copro.c
    │       │   	├── your_copro.h
    │       │   	└── your_copro_defs.h -> ../../../../hw/vendor/your_copro/sw/your_copro_defs.h
    │       └── extensions
    │       	└── your_copro_x_heep.h
    ├── hw
    │   └── vendor
    │       ├── your_copro
    │       ├── esl_epfl_x_heep.vendor.hjson
    │       └── esl_epfl_x_heep
    │           ├── hw
    │           ├── sw
    │           │   ├── applications
    │           │   ├── build
    │           │   ├── device
    │           │   └── ...
    │           ├── Makefile
    │           ├── external.mk
    │           └── ...	
    ├── Makefile
    ├── util
    │   └── vendor.py
    └── ...
    
Where `BASE` is your repository's base directory, `esl_epfl_x_heep` is the vendorized `X-HEEP` repository and `your_app` is the name of the application you intend to build. 

### The /sw/ folder

The `BASE/sw/` folder must comply with `X-HEEP` repository structure and therefore include an `applications`, `build`, `device` and `linker` folder. 
It is not compulsory for it to be on the `BASE` directory, although this is the default structure that `X-HEEP`'s Makefiles will assume if no other path is specified through the `SOURCE` variable. 
Inside the `applications` folder different projects can be stored (still respecting the `name_of_project/main.c` structure). 
The `build`, `device` and `linker` should be linked with the vendorized folders inside `X-HEEP`.
In this example that is done from the `BASE` directory as follows:
```
ln -s ../hw/vendor/esl_epfl_x_heep/sw/build sw/build
ln -s ../hw/vendor/esl_epfl_x_heep/sw/device sw/device
ln -s ../hw/vendor/esl_epfl_x_heep/sw/linker sw/linker
```

### The /sw/applications folder
Inside the `sw/applications/` folder you may have different applications that can be built separately. Each application is a directory named after your application, containing one and only one `main.c` file which is built during the compilation process. The folder can contain other source or header files (of any name but `main.c`).  

### The /sw/external folder
In the `external` folder you can add whatever is necessary for software to work with your coprocessor/accelerator. This might include:

* Sources and header files
* Soft links to folders or files. 

The external folder or any of its subdirectories cannot contain neither a `device` nor a `applications` folder as it would collide with the respective folders inside `BASE/sw/`. It should also not contain a `main.c` file.  

### The BASE/Makefile
The `BASE/Makefile` is your own custom Makefile. You can use it as a bridge to access the Makefile from `X-HEEP`. 

To do so, it MUST include the `external.mk` AFTER all your custom rules. 


<details>
    <summary>Example of BASE/Makefile</summary>

```
MAKE     = make

test:
    @echo Nothing is executed from X-HEEP

app:
    @echo This is app being run from the x-heep repo
    $(MAKE) -f $(XHEEP_MAKE) $(MAKECMDGOALS) PROJECT=hello_world SOURCE=.

verilator-sim: 
    @echo You will not access verilator-sim

export HEEP_DIR = hw/vendor/esl_epfl_x_heep/
XHEEP_MAKE = $(HEEP_DIR)/external.mk
include $(XHEEP_MAKE)
```

* The `test` rule will not use the `X-HEEP` Makefile in any way.
* The `app` rule will perform actions before calling `X-HEEP` Makefile's `app` rule. In this case, the project and where the source files are to be extracted from is being specified. The `SOURCE=.` argument will set `X-HEEP`'s own `sw/` folder as the directory from which to fetch source files. This is an example of building inner sources from an external directory. 
* The `verilator-sim` rule will override the `X-HEEP` Makefile's one. 
* Any other target will be passed straight to `X-HEEP`'s Makefile. For example
```
make mcu-gen CPU=cv32e40x
```
</details>


### Different use cases
If you plan to vendorize `X-HEEP` in a different directory than the one proposed, just update in your `BASE/Makefile`:
```
export HEEP_DIR = <path_to_x_heep_relative_to_this_directory>
```

If you plan to store source files in a different location that the one proposed, just call `make` making the `SOURCE` path explicit. 
```
make app PROJECT=your_app SOURCE=<path_to_your_sw_relative_to_x_heep_sw>
```
Consider that inside this `sw` folder the same structure than the one proposed is required.  

#  eXtend X-HEEP

`X-HEEP` is meant to be extended with your own custom IPs. `X-HEEP` itself posseses a hardware-software framework capable of working standalone. If you want to extend it, you will need to merge your hardware and software with `X-HEEP`'s.

For this purpose we support the [CV-X-IF](https://docs.openhwgroup.org/projects/openhw-group-core-v-xif/en/latest/intro.html) interface with the [cv32e40x](https://github.com/openhwgroup/cv32e40x) or [cv32e40px](https://github.com/esl-epfl/cv32e40px) RISCV-CPU, and we expose master and slave ports to/from the bus.

> We recommend using the `cv32e40px` for pairing with your CV-X-IF compliant coprocessor. If you choose to use the `cv32e40x`, `X-HEEP` currently uses the revision [`0.9.0`](https://github.com/openhwgroup/cv32e40x/commit/f17028f2369373d9443e4636f2826218e8d54e0f). It is recommended to use the same revision in peripheral IPs to prevent conflicts during RTL compilation.

Here you can find a list of `X-HEEP` based open-source examples. If you want to include your project in this list, please open an issue with a link to your repository.

* [CGRA-X-HEEP](https://github.com/esl-epfl/cgra_x_heep): A CGRA loosely coupled with X-HEEP.
* [F-HEEP](https://github.com/davidmallasen/F-HEEP): System integrating [fpu_ss](https://github.com/pulp-platform/fpu_ss) into X-HEEP via the eXtension interface and cv32e40px.
* [KALIPSO](https://github.com/vlsi-lab/ntt_intt_kyber) and [KRONOS](https://github.com/vlsi-lab/keccak_integration/tree/keccak_xheep): Loosely-coupled, post-quantum cryptography accelerators for NTT/INTT and Keccak hash function integrated into X-HEEP.


In addition, the `X-HEEP` testbench has been extended with a `DMA`, dummy `PERIPHERALs` (including the `FLASH`), and a CORE-V-XIF compatible co-processor
implementing the `RV32F` RISC-V ISA. This has been done to help us maintaining and verifying the extension interface.

If you want to try the co-processor with a CORE-V-XIF compatible CPU as the `cv32e40px`, do as follow:

```
make mcu-gen CPU=cv32e40px
make verilator-sim FUSESOC_PARAM="--X_EXT=1"
make app PROJECT=example_matfadd ARCH=rv32imfc
./Vtestharness +firmware=../../../sw/build/main.hex
```

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

* Create a new top-level repository (`BASE`) and vendorize (or add as git submodules) both your `CORE-V-XIF/OBI` compliant coprocessor/accelerator and `X-HEEP`.
* Copy the `x-heep/hw/system/x_heep_system.sv` as your new top-level module. Then modify it as needed to include your co-processor and connect it to the `core_v_mini_mcu` with the `XIF`. The `XIF` SystemVerilog interface must be instantiated in the top-level module, where `X-HEEP` and your co-processor are connected. See the `X-HEEP` [testbench](./../../../tb/testharness.sv) as an example.
* Before building software remember to run `make mcu-gen CPU=cv32e40px`.

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
    COREV_PULP:
        datatype: int
        paramtype: vlogparam
        default: 0
    JTAG_DPI:
        datatype: int
        paramtype: vlogparam
        default: 0
    USE_UPF:
      datatype: bool
      paramtype: vlogdefine
      description: |
        Enables simulation with UPF with Modelsim/VCS

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
        - COREV_PULP
        - JTAG_DPI
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
    │       │   └── your_copro_x_heep.h
    │       └── lib
    │           └── crt
    │               └── external_crt0.S
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
* A `lib/crt/` directory with and `exteral_crt0.S` file (will be included inside `BASW/sw/device/lib/crt/crt0.S`).

The external folder or any of its subdirectories cannot contain neither a `device` nor an `applications` folder as it would collide with the respective folders inside `BASE/sw/`. It should also not contain a `main.c` file.

### The BASE/Makefile
The `BASE/Makefile` is your own custom Makefile. You can use it as a bridge to access the Makefile from `X-HEEP`.

To do so, it MUST include the `external.mk` AFTER all your custom rules.


<details>
    <summary>Example of BASE/Makefile</summary>

```Makefile
MAKE     = make
.PHONY: test
test:
    @echo Nothing is executed from X-HEEP, as test is not a target inside X-HEEP.

app:
    @echo This target will do something and then call the one inside X-HEEP.
    $(MAKE) -f $(XHEEP_MAKE) $(MAKECMDGOALS) PROJECT=hello_world SOURCE=.

verilator-sim:
    @echo You will not access the verilator-sim target from X-HEEP.

export HEEP_DIR = hw/vendor/esl_epfl_x_heep/
XHEEP_MAKE = $(HEEP_DIR)/external.mk
include $(XHEEP_MAKE)
```

* The `test` rule will not use the `X-HEEP` Makefile in any way. Make the target a prerequisite of `.PHONY` to prevent the X-HEEP Makefile from attempting to run a non-existent target.
* The `app` rule will perform actions before calling `X-HEEP` Makefile's `app` rule. In this case, the project and where the source files are to be extracted from is being specified. The `SOURCE=.` argument will set `X-HEEP`'s own `sw/` folder as the directory from which to fetch source files. This is an example of building inner sources from an external directory.
* The `verilator-sim` rule will override the `X-HEEP` Makefile's one.
* Any other target will be passed straight to `X-HEEP`'s Makefile. For example
```sh
make mcu-gen CPU=cv32e40px
```
</details>


### Excluding files from compilation
If you have files that need to be excluded from the gcc compilation flow, you can add them to a directory containing the keyword `exclude`, and/or rename the file to include the keyword `exclude`. 
In the following example, the files marked with ✅ will be compiled, and the ones marked with ❌ will not.  

    BASE
    ├── sw
    │   ├── applications
    │   │   └── your_app
    │   │       ├── ✅ main.c      
    │   │       ├── ✅ your_app.c
    │   │       ├──    your_app.h
    │   │       ├── ❌ my_kernel_exclude.c
    │   │       ├──    my_kernel.h
    │   │       └── exclude_files
    │   │           └── ❌ kernel_asm.S



### Makefile help
If you want that the commands `make` or `make help` show the help for your external Makefile, add the following lines before the first `include` directive or target.

<details>
    <summary>Addition to print the target's help</summary>

```Makefile
# HEEP_DIR might already be defined, you may want to move it to the top
export HEEP_DIR = hw/vendor/esl_epfl_x_heep/

# Get the path of this Makefile to pass to the Makefile help generator
MKFILE_PATH = $(shell dirname "$(realpath $(firstword $(MAKEFILE_LIST)))")
export FILE_FOR_HELP = $(MKFILE_PATH)/Makefile


## Call the help generator. Calling simply
## $ make
## or
## $ make help
## Will print the help of this project.
## With the parameter WHICH you can select to print
## either the help of X-HEEP (WHICH=xheep)
## or both this project's and X-HEEP's (WHICH=all)
help:
ifndef WHICH
	${HEEP_DIR}/util/MakefileHelp
else ifeq ($(filter $(WHICH),xheep x-heep),)
	${HEEP_DIR}/util/MakefileHelp
	$(MAKE) -C $(HEEP_DIR) help
else
	$(MAKE) -C $(HEEP_DIR) help
endif
```

</details>

> Remeber to add double hashes `##` on any comment you want printed on the help.
> Use `## @section SectionName` to divide the documentation in sections

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


## Inter-process communication using Verilator's DPI

The following [repository](https://github.com/specs-feup/x-heep) uses X-HEEP and the Verilator simulator to model a CPU-CGRA hybrid system. This architecture simulates the CPU integrated into the X-HEEP system, and an external Java process simulates the accelerator. Both components require a communication channel to exchange instructions and data. Using the existing infrastructure to to interact with an external OS process is not feasible at first sight, given that the X-HEEP ecosystem's pipeline encapsulates most of the simulation build and execution, with all modules supplied directly to Verilator. 

To circumvent this issue, this project uses [Direct Programming Interface (DPI)](https://verilator.org/guide/latest/connecting.html) calls (defined in `hw/ip_examples/cgraitf/cgraitfdpi.c`) to establish a connection and communicate with an external process through a Unix Domain Socket. This behavior mirrors the UART module (used as the skeleton code) that connects and outputs _printf_ information to the pseudo-terminal. These calls are embedded in a mock CGRA peripheral/interface, located in `hw/ip_examples/cgraitf/cgraitf.sv`. The module overrides reads and writes to the specified peripheral address, with the proper socket-based mechanism (_send_ or _recv_). The _simple_accelerator_ module could also be similarly customized to perform the same operations, using X-HEEP's  interfaces and memory access protocols. A given user program executed in the CPU (such as `sw/applications/cgra_itf/main.c`) must then select assignments to or from the address to trigger the appropriate action.
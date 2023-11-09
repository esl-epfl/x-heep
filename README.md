# Repository folder structure

    .
    ├── .github/workflows
    ├── ci/scripts
    ├── hw
    │   ├── asic
    │   ├── core-v-mini-mcu
    │   ├── fpga
    │   ├── ip
    │   ├── ip_examples
    │   ├── simulation
    │   └── vendor
    ├── scripts
    │   ├── sim
    │   └── synthesis
    ├── sw
    │   ├── applications
    │   ├── device/lib
    │   ├── linker
    │   └── vendor
    ├── tb
    ├── util
    └── README.md

<br />
<p align="left"><img src="logo/x-heep.png" width="250"></p>

`X-HEEP` (eXtendable Heterogeneous Energy-Efficient Platform) is a `RISC-V` microcontroller described in `SystemVerilog`
that can be configured to target small and tiny platforms as well as extended to support accelerators.
The cool thing about `X-HEEP` is that we provide a simple customizable MCU, so CPUs, common peripherals, memories, etc.
so that you can extend it with your own accelerator without modifying the MCU, but just instantiating it in your design.
By doing so, you inherit an IP capable of booting RTOS (such as `freeRTOS`) with the whole FW stack, including `HAL` drivers and `SDK`,
and you can focus on building your special HW supported by the microcontroller.

`X-HEEP` supports simulation with Verilator, Questasim, etc. Morever, FW can be built and linked by using `CMake` either with gcc or with clang. It can be implemented on FPGA, and it supports implementation in Silicon, which is its main (but not only) target. See below for more details.

The block diagram below shows the `X-HEEP` MCU

<p align="center"><img src="block_diagrams/core_v_mini_mcu.svg" width="650"></p>

# Self-documented Makefile

Note that under `util` folder, the file `generate-makefile-help` is employed to generate a self-documented helping output. In case of including any other target or command under the main `Makefile`, follow the same general and parameter descriptions as already provided for every target. Check the `help` output by doing `make` or `make help`. Moreover, **note that some of the parameters required for some of the targets are initiated with _default values_**

# Prerequisite

## 1. OS requirements

To use `X-HEEP`, first make sure you have the following apt packages, or install them as:

```bash
sudo apt install lcov libelf1 libelf-dev libftdi1-2 libftdi1-dev libncurses5 libssl-dev libudev-dev libusb-1.0-0 lsb-release texinfo autoconf cmake flex bison libexpat-dev gawk tree xterm python3-venv python3-dev
```

In general, have a look at the [Install required software](https://opentitan.org/guides/getting_started/index.html) section of the OpenTitan documentation.

It has been tested only on `Ubuntu 20`, and we know it does NOT WORK on `Ubuntu 22`.

## 2. Python


We rely on either (a) `miniconda`, or (b) `virtual environment` enviroment.

Choose between `2.a` or `2.b` to setup your enviroment.

### 2.a Miniconda

Install [Miniconda](https://docs.conda.io/en/latest/miniconda.html#linux-installers) python 3.8 version as described in the link,
and create the Conda enviroment:

```bash
make conda
```

You need to do it only the first time, then just activate the environment everytime you work with `X-HEEP` as

```bash
conda activate core-v-mini-mcu
```


### 2.b Virtual Environment

Install the python virtual environment just as:

```bash
make venv
```

You need to do it only the first time, then just activate the environment everytime you work with `X-HEEP` as

```bash
source .venv/bin/activate
```

## 3. Install the RISC-V Compiler:

```
git clone --branch 2022.01.17 --recursive https://github.com/riscv/riscv-gnu-toolchain
cd riscv-gnu-toolchain
./configure --prefix=/home/$USER/tools/riscv --with-abi=ilp32 --with-arch=rv32imc --with-cmodel=medlow
make
```

Then, set the `RISCV` env variable as:

```
export RISCV=/home/$USER/tools/riscv
```

Optionally you can also compile with clang/LLVM instead of gcc. For that you must install the clang compiler into the same `RISCV` path. The binaries of gcc and clang do not collide so you can have both residing in the same `RISCV` directory. For this you can set the `-DCMAKE_INSTALL_PREFIX` cmake variable to `$RISCV` when building LLVM. This can be accomplished by doing the following:

```
git clone https://github.com/llvm/llvm-project.git
cd llvm-project
git checkout llvmorg-14.0.0
mkdir build && cd build
cmake -G "Unix Makefiles" -DLLVM_ENABLE_PROJECTS=clang -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$RISCV -DLLVM_TARGETS_TO_BUILD="RISCV" ../llvm
cmake --build . --target install
```

## 4. Install Verilator:

```
export VERILATOR_VERSION=4.210

git clone https://github.com/verilator/verilator.git
cd verilator
git checkout v$VERILATOR_VERSION

autoconf
./configure --prefix=/home/$USER/tools/verilator/$VERILATOR_VERSION
make
make install
```
Then, set the `PATH` env variable to as:

```
export PATH=/home/$USER/tools/verilator/$VERILATOR_VERSION/bin:$PATH
```

In general, have a look at the [Install Verilator](https://opentitan.org/guides/getting_started/setup_verilator.html) section of the OpenTitan documentation.

If you want to see the vcd waveforms generated by the Verilator simulation, install GTKWAVE:

```
sudo apt install libcanberra-gtk-module libcanberra-gtk3-module
sudo apt-get install -y gtkwave
```

## Files are formatted with Verible

We use version v0.0-1824-ga3b5bedf

See: [Install Verible](https://opentitan.org/guides/getting_started/index.html#step-6a-install-verible-optional)

To format your RTL code type:

```
make verible
```
## Compilation Flow and Package Manager

We use [FuseSoC](https://github.com/olofk/fusesoc) for all the tools we use.

The `fusesoc` commands are inside the Makefile.

# Adding external IPs

This repository relies on [Vendor](https://opentitan.org/book/util/doc/vendor.html) to add new IPs. The `vendor.py` script in the [`./util`](./util/) folder implements what is describeb above, while [this](./ExternalDevices.md) file contains additional information on how to connect external devices to the system.

# Compiling with Makefile

You can compile the example applications and the platform using the Makefile. Type 'make help' or 'make' for more information. Moreover, please, check the different 'clean' commands to verify that you are using the corret one.

## Generate core-v-mini-mcu package

First, you have to generate the SystemVerilog package and C header file of the core-v-mini-mcu:

```
make mcu-gen
```

By default, `X-HEEP` deploys the [cv32e20](https://github.com/openhwgroup/cve2) RISC-V CPU.
Other supported CPUs are: the [cv32e40p](https://github.com/openhwgroup/cv32e40p), [cv32e40x](https://github.com/openhwgroup/cv32e40x), and the [cv32e40px](https://github.com/esl-epfl/cv32e40px).
The default bus type of `X-HEEP` is a single-master-at-a-time architecture, (called `onetoM`), but the cross-bar architecture is also supported by setting
the bus to `NtoM`. Also, the user can select the number of 32kB banks addressed in continuous mode and/or the interleaved mode.
By default, `X-HEEP` is generated with 2 continuous banks and 0 interleaved banks.

Below an example that changes the default configuration:

```
make mcu-gen CPU=cv32e40p BUS=NtoM MEMORY_BANKS=12 MEMORY_BANKS_IL=4
```

The last command generates x-heep with the cv32e40p core, with a parallel bus, and 16 memory banks (12 continuous and 4 interleaved),
each 32KB, for a total memory of 512KB.

If you are using `X-HEEP` just as a controller for your own system and you do not need any peripheral, you can use the `minimal` configuration file
when generating the MCU as:

```
make mcu-gen MCU_CFG=mcu_cfg_minimal.hjson
```

The `minimal` configuration is a work-in-progress, thus not all the APPs have been tested.


## Compiling Software

Don't forget to set the `RISCV` env variable to the compiler folder (without the `/bin` included).
To run 'hello world' application, just type 'make app'.

```
make app
```

To run any other application, please use the following command with appropiate parameters:

```
app PROJECT=<folder_name_of_the_project_to_be_built> TARGET=sim(default),pynq-z2 LINKER=on_chip(default),flash_load,flash_exec COMPILER=gcc(default),clang COMPILER_PREFIX=riscv32-unknown-(default) ARCH=rv32imc(default),<any RISC-V ISA string supported by the CPU>

Params:
- PROJECT (ex: <folder_name_of_the_project_to_be_built>, hello_world(default))
- TARGET (ex: sim(default),pynq-z2)
- LINKER (ex: on_chip(default),flash_load,flash_exec)
- COMPILER (ex: gcc(default),clang)
- COMPILER_PREFIX (ex: riscv32-unknown-(default))
- ARCH (ex: rv32imc(default),<any RISC-V ISA string supported by the CPU>)
```

For instance, to run 'hello world' app for the pynq-z2 FPGA targets, just run:

```
make app TARGET=pynq-z2
```

Or, if you use the OpenHW Group [GCC](https://www.embecosm.com/resources/tool-chain-downloads/#corev) compiler with CORE_PULP extensions, make sure to point the `RISCV` env variable to the OpenHW Group compiler, then just run:


```
make app COMPILER_PREFIX=riscv32-corev- ARCH=rv32imc_zicsr_zifencei_xcvhwlp1p0_xcvmem1p0_xcvmac1p0_xcvbi1p0_xcvalu1p0_xcvsimd1p0_xcvbitmanip1p0
```

This will create the executable file to be loaded into your target system (ASIC, FPGA, Simulation).
Remember that, `X-HEEP` is using CMake to compile and link. Thus, the generated files after having
compiled and linked are under `sw\build`

Alternatively, in case you are doing pure FW development and you are used to developing using Integrated Development Evironments (IDEs), please check [the IDE readme](./IDEs.md).

## FreeROTS based applications

'X-HEEP' supports 'FreeRTOS' based applications. Please see `sw\applications\blinky_freertos`.

After that, you can run the command to compile and link the FreeRTOS based application. Please also set 'LINKER' and 'TARGET' parameters if needed.

```
make app PROJECT=blinky_freertos
```

The main FreeRTOS configuration is allocated under `sw\freertos`, in `FreeRTOSConfig.h`. Please, change this file based on your application requirements.
Moreover, FreeRTOS is being fetch from 'https://github.com/FreeRTOS/FreeRTOS-Kernel.git' by CMake. Specifically, 'V10.5.1' is used. Finally, the fetch repository is located under `sw\build\_deps` after building.

## Simulating

This project supports simulation with Verilator, Synopsys VCS, and Siemens Questasim.
It relies on `fusesoc` to handle multiple EDA tools and parameters.
For example, if you want to set the `FPU` and `COREV_PULP` parameters of the `cv32e40p` CPU,
you need to add next to your compilation command `FUSESOC_PARAM="--COREV_PULP=1 --FPU=1"`
Below the different EDA examples commands.

### Compiling for Verilator

To simulate your application with Verilator, first compile the HDL:

```
make verilator-sim
```

then, go to your target system built folder

```
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator
```

and type to run your compiled software:

```
./Vtestharness +firmware=../../../sw/build/main.hex
```

or to execute all these three steps type:

```
make run-helloworld
```


### Compiling for VCS

To simulate your application with VCS, first compile the HDL:

```
make vcs-sim
```

then, go to your target system built folder

```
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-vcs
```

and type to run your compiled software:

```
./openhwgroup.org_systems_core-v-mini-mcu_0 +firmware=../../../sw/build/main.hex
```

Waveforms can be viewed with Verdi. Make sure you have the env variable `VERDI_HOME` set to your Verdi install folder, then run your compiled software as above, but with the `-gui` flag:

```
./openhwgroup.org_systems_core-v-mini-mcu_0 +firmware=../../../sw/build/main.hex -gui
```

An Analog / Mixed-Signal simulation of X-HEEP, combining both the RTL system verilog files for the digital part and a SPICE file connected through a `control.init` file for the analog / mixed-signal part, can be ran by typing

```
make vcs-ams-sim
```

then going to the target system built folder

```
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-vcs
```

and running the same executable as for the digital simulation. Note that with Verdi you can view both the digital and the analog waveforms.

Additional instructions on how to run an analog / mixed-signal simulation of X-HEEP can be found [here](AnalogMixedSignal.md). To try out the simulation, we provide an example SPICE netlist of an simple 1-bit ADC created by us and exported from [xschem](https://xschem.sourceforge.io/stefan/index.html) and which uses the PTM 65nm bulk CMOS model from [https://ptm.asu.edu](https://ptm.asu.edu/).

### Compiling for Questasim

To simulate your application with Questasim, first set the env variable `MODEL_TECH` to your Questasim bin folder, then compile the HDL:

```
make questasim-sim
```

then, go to your target system built folder

```
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-modelsim/
```

and type to run your compiled software:

```
make run PLUSARGS="c firmware=../../../sw/build/main.hex"
```

You can also use vopt for HDL optimized compilation:

```
make questasim-sim-opt
```

then go to

```
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim_opt-modelsim/
```
and

```
make run RUN_OPT=1 PLUSARGS="c firmware=../../../sw/build/main.hex"
```

You can also compile with the UPF power domain description as:

```
make questasim-sim-opt-upf FUSESOC_PARAM="--USE_UPF"
```

and then execute software as:

```
make run RUN_OPT=1 RUN_UPF=1 PLUSARGS="c firmware=../../../sw/build/main.hex"
```

Questasim version must be >= Questasim 2020.4

### UART DPI

To simulate the UART, we use the LowRISC OpenTitan [UART DPI](https://github.com/lowRISC/opentitan/tree/master/hw/dv/dpi/uartdpi).
Read how to interact with it in the Section "Running Software on a Verilator Simulation with Bazel" [here](https://opentitan.org/guides/getting_started/setup_verilator.html#running-software-on-a-verilator-simulation-with-bazel).
The output of the UART DPI module is printed in the `uart0.log` file in the simulation folder.

For example, to see the "hello world!" output of the Verilator simulation:

```
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator
./Vtestharness +firmware=../../../sw/build/main.hex
cat uart0.log
```

## Automatic testing

X-HEEP includes two tools to perform automatic tests over your modifications.

### Github CIs

Upon push, tests are run on Github runners, these include:
* The generated `.sv` files pushed are equal to those generated in the runner (the code does not depend on the modification of generated files)
* Vendor is up to date (the code does not depend on the modification of vendorized files)
* All applications can be built successfully using both gcc and clang

All test must be successful before PRs can be merged.

### Simulation script

Additionally, a `test_all.sh` script is provided. Apart from compiling all apps with both gcc and clang, it will simulate them and check the result.

The available parameters are:
* COMPILER: `gcc` (default) or `clang` (can provide more than one)
* SIMULATOR: `verilator` (default), `questasim` or disable simulation with `nosim` (only one, the last provided is used).
* LINKER: `on_chip`(default), `flash_load` or `flash_exec` (can provide more than one)
* TIMEOUT: Integer number of seconds (default 120)


#### Usage

##### Comands
You can use two different commands to compile or simulate all the existing APPs:
```
make app-compile-all
```
```
make app-simulate-all
```
Note that both commands allow the previous parameters to specify compiling or simulation options. E.g.:
```
make app-simulate-all LINKER=on_chip SIMULATOR=questasim COMPILER=clang TIMEOUT=150 
```

##### Manually
You can also **SOURCE** the script as
```bash
. util/test_all.sh on_chip questasim clang 150
```

*Pay special attention to the first period in the command!*
You will be killing simulations that take too long, if you **EXECUTE** (`./test_all.sh`) this action kills the script.

For both usages (commands or manual), the order of the arguments is irrelevant.

> Note: Be sure to commit all your changes before running the script!

* Applications that fail being built with gcc will not be simulated (skipped).
* Some applications are skipped by default for not being suitable for simulation.
* If a simulation takes too long (>timeout), it is killed.

* Upon starting, the script will modify the `mcu_cfg.hjson` file to include all peripherals (so the largest number of apps can be run), re-generates the mcu and re-builds the simulation model for the chosen tool.
These changes can be reverted at the end of the execution (default). If changes were not commited, accepting this operation will revert them!

The success of the script is not required for merging of a PR.

## Debug

Follow the [Debug](./Debug.md) guide to debug core-v-mini-mcu.

Alternatively, in case you are used to developing using Integrated Development Environments (IDEs), please check [the IDE readme](./IDEs.md).

## Execute From Flash

Follow the [ExecuteFromFlash](./ExecuteFromFlash.md) guide to exxecute code directly from the FLASH with modelsim, FPGA, or ASIC.

## Emulation on Xilinx FPGAs

This project offers two different X-HEEP implementetions on the Xilinx FPGAs, called Standalone-FEMU and Linux-FEMU.

### Standalone-FEMU (Standalone Fpga EMUlation)

In this version, the X-HEEP architecture is implemented on the programmable logic (PL) side of the FPGA, and its input/output are connected to the available headers on the FPGA board.

Make sure you have the FPGA board files installed in your Vivado.

For example, for the Xilinx Pynq-Z2 board, use the documentation provided at the following [link](https://pynq.readthedocs.io/en/v2.5/overlay_design_methodology/board_settings.html) to download and install them:

To build and program the bitstream for your FPGA with vivado, type:

```
make vivado-fpga FPGA_BOARD=pynq-z2
```

or add the flag `use_bscane_xilinx` to use the native Xilinx scanchain:

```
make vivado-fpga FPGA_BOARD=pynq-z2 FUSESOC_FLAGS=--flag=use_bscane_xilinx
```

Only Vivado 2021.2 has been tried.

To program the bitstream, open Vivado,

```
open --> Hardware Manager --> Open Target --> Autoconnect --> Program Device
```

and choose the file `openhwgroup.org_systems_core-v-mini-mcu_0.bit`

To run SW, follow the [Debug](./Debug.md) guide
to load the binaries with the HS2 cable over JTAG,
or follow the [ExecuteFromFlash](./ExecuteFromFlash.md)
guide if you have a FLASH attached to the FPGA.

Do not forget that the `pynq-z2` board requires you to have the ethernet cable attached to the board while running.

For example, if you want to run your application using flash_exec, do as follow:

compile your application, e.g. `make app PROJECT=example_matfadd TARGET=pynq-z2 ARCH=rv32imfc LINKER=flash_exec`

and then follow the [ExecuteFromFlash](./ExecuteFromFlash.md) to program the flash and set the boot buttons on the FPGA correctly.

To look at the output of your printf, run in another terminal:

`picocom -b 9600 -r -l --imap lfcrlf /dev/ttyUSB2`

Please be sure to use the right `ttyUSB` number (you can discover it with `dmesg --time-format iso | grep FTDI` for example).


### Linux-FEMU (Linux Fpga EMUlation)

In this version, the X-HEEP architecture is implemented on the programmable logic (PL) side of the FPGA and Linux is run on the ARM-based processing system (PS) side of the same chip.

Read the [following](./linux_femu/README.md) documentation to have more information about this implementation.


# ASIC Implementation

This project can be implemented using standard cells based ASIC flow.

## Synthesis with Synopsys Design Compiler

First, you need to provide technology-dependent implementations of some of the cells which require specific instantiation.

Then, please provide a set_libs.tcl and set_constraints.tcl scripts to set link and target libraries, and constraints as the clock.

To generate the `analyze` script for the synthesis scripts with DC, execute:

```
make asic
```

## OpenRoad support for SkyWater 130nm

We are working on supporting OpenRoad and SkyWater 130nm PDK, please refer to the
[OpenRoadFlow](./OpenRoadFlow.md) page. This is not ready yet, it has not been tested.

This relies on a fork of [edalize](https://github.com/davideschiavone/edalize) that contains templates for Design Compiler and OpenRoad.

## References

1. [Schiavone, Pasquale Davide, et al. "X-HEEP: An Open-Source, Configurable and Extendible RISC-V Microcontroller." 
Proceedings of the 20th ACM International Conference on Computing Frontiers. 2023.](https://dl.acm.org/doi/pdf/10.1145/3587135.3591431?casa_token=cAs3isVd0zkAAAAA:gmQBe3ip7X0Fz0hO8lSFbGN5-2fdu5vni1dxWWAIe9zCxQDW1PPerubUigOcl_an8HiZOhPuNrwzIw8)





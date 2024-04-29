# Compile with Makefile

You can compile the example applications and the platform using the Makefile. Type 'make help' or 'make' for more information. Moreover, please, check the different 'clean' commands to verify that you are using the corret one.

## Generate core-v-mini-mcu package

First, you have to generate the SystemVerilog package and C header file of the core-v-mini-mcu:

```
make mcu-gen
```

To change the default cpu type (i.e., cv32e20), the default bus type (i.e., onetoM),
the default continuous memory size (i.e., 2 continuous banks) or the default interleaved memory size (i.e., 0 interleaved banks):

```
make mcu-gen CPU=cv32e40p BUS=NtoM MEMORY_BANKS=12 MEMORY_BANKS_IL=4
```

The last command generates x-heep with the cv32e40p core, with a parallel bus, and 16 memory banks (12 continuous and 4 interleaved),
each 32KB, for a total memory of 512KB.
This method is limmited to 32KB banks.

To configure the ram banks with more flexibility, edit `configs/general.hjson` or provided your own one.
Both method work together the first one overrides the second.

```
make mcu-gen X_HEEP_CFG=configs/my_config.hjson
```

For more information see Configuration section.

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
make app COMPILER_PREFIX=riscv32-corev- ARCH=rv32imc_zicsr_zifencei_xcvhwlp_xcvmem_xcvmac_xcvbi_xcvalu_xcvsimd_xcvbitmanip
```

This will create the executable file to be loaded in your target system (ASIC, FPGA, Simulation).
Remember that, `X-HEEP` is using CMake to compile and link. Thus, the generated files after having
compiled and linked are under `sw\build`

## FreeROTS based applications

'X-HEEP' supports 'FreeRTOS' based applications. Please see `sw\applications\blinky_freertos`.

After that, you can run the command to compile and link the FreeRTOS based application. Please also set 'LINKER' and 'TARGET' parameters if needed.

```
make app PROJECT=blinky_freertos
```

The main FreeRTOS configuration is allocated under `sw\freertos`, in `FreeRTOSConfig.h`. Please, change this file based on your application requirements.
Moreover, FreeRTOS is being fetch from 'https://github.com/FreeRTOS/FreeRTOS-Kernel.git' by CMake. Specifically, 'V10.5.1' is used. Finally, the fetch repository is located under `sw\build\_deps` after building.


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
Minimal configuration of a core-v-mcu

## Prerequisite

1. [optional] If you wish to install the Conda enviroment with python 3.9:

```bash
$ conda update conda
$ conda env create -f environment.yml
```

Activate the environment with

```bash
$ source activate core-v-mini-mcu
```
2. Install the required Python tools:

```
pip3 install --user -r python-requirements.txt
```

Add '--root user_builds' to set your build foders for the pip packages
and add that folder to the `PATH` variable

## Adding external IPs

This repository relies on [vendor](https://docs.opentitan.org/doc/ug/vendor_hw/) to add new IPs.
In the ./util folder, the vendor.py scripts implements what is describeb above.


## Compiling for Questasim

```
$ fusesoc --cores-root . run --no-export --target=sim --tool=modelsim --setup --build openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildsim.log
```
First set the env variable `MODEL_TECH` to your Questasim bin folder.

Questasim version must be >= Questasim 2019.3

You can also use vopt by running:

```
$ fusesoc --cores-root . run --no-export --target=sim_opt --setup --build openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildsim.log
```

## Compiling for Vivado Simulator

Work In Progress


## Compiling Software


First set the `RISCV` env variable to the compiler folder (without the `/bin` included).

Then go to the `./sw` folder and type:

```
make applications/hello_world/hello_world.hex
```

This will create the executable file to be loaded in your target system (ASIC, FPGA, Questasim, Verilator, etc).

## Running Software on Questasim tool

Go to your target system built folder, e.g.

```
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-modelsim/
```

or

```
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim_opt-modelsim/
```

for the vopt version target.

Then type:

```
make run PLUSARGS="c firmware=../../../sw/applications/hello_world/hello_world.hex"
```

Replace the  `.hex` file with your own application if you want to run another pre-compiled application.


## FPGA Xilinx Nexys-A7 100T Flow

Work In Progress and untested!!!

To build and program the bitstream for your FPGA with vivado, type:

```
$ fusesoc --cores-root . run --no-export --target=nexys-a7-100t --setup --build openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildvivado.log
```

If you only need the synthesis implementation:

```
$ fusesoc --cores-root . build --no-export --target=nexys-a7-100t --setup openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildvivado.log
```

then

```
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/nexys-a7-100t-vivado/
make synth
```

at the end of the synthesis, you can export your netlist typing:

```
vivado -notrace -mode batch -source ../../../hw/fpga/scripts/export_verilog_netlist.tcl
```


Only Vivado 2019.1.1 has been tried.


## OpenTitan IPs

These are ips cloned from OpenTitan.
Via the `regtool` application, OpenTitan generates `rtl`, `header` files and more.
Please referes to its documentation available at: https://docs.opentitan.org/doc/rm/register_tool/

## Modified Generator

OpenTitan generates peripherals that are compliant to the TUL bus.
However, in this project we rely on https://github.com/pulp-platform/register_interface
to generate a different bus interface. The register_interface repository contains an OpenTitan
version that has been patched to cope with the register interface instead of the TUL one.

## Generate the UART IP

From the core-v-mini-mcu root folder:

```
$ cd hw/vendor/hw/ip/uart
$ ../../../../pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py -r -t rtl data/uart.hjson
```

Then, manually modify the uart.sv file as:

Replace:

```
`include "prim_assert.sv"
module uart (
```
with

```
`include "common_cells/assertions.svh"

module uart #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic
) (
```

then

```
input  tlul_pkg::tl_h2d_t tl_i,
output tlul_pkg::tl_d2h_t tl_o,
```

with

```
input  reg_req_t reg_req_i,
output reg_rsp_t reg_rsp_o,
```

and on the instance ports of the uart_reg_top module, this

```
.tl_i,
.tl_o,
```
with:

```
.reg_req_i,
.reg_rsp_o,
```

TODO: script this
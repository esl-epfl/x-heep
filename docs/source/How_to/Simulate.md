# Simulate

This project supports simulation with Verilator, Synopsys VCS, Siemens Questasim and Cadence Xcelium.
It relies on `fusesoc` to handle multiple EDA tools and parameters.
For example, if you want to set the `FPU` and `COREV_PULP` parameters of the `cv32e40p` CPU,
you need to add next to your compilation command `FUSESOC_PARAM="--COREV_PULP=1 --FPU=1"`
Below the different EDA examples commands.

## Compiling for Verilator

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

## Compiling for VCS

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

Additional instructions on how to run an analog / mixed-signal simulation of X-HEEP can be found [here](./AnalogMixedSignal.md). To try out the simulation, we provide an example SPICE netlist of an simple 1-bit ADC created by us and exported from [xschem](https://xschem.sourceforge.io/stefan/index.html) and which uses the PTM 65nm bulk CMOS model from [https://ptm.asu.edu](https://ptm.asu.edu/).

## Compiling for Questasim

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

## Compiling for Xcelium

To simulate your application with Xcelium, first compile the HDL:

```
make xcelium-sim
```

then, go to your target system built folder

```
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-xcelium/
```

and type to run your compiled software:

```
make run PLUSARGS="c firmware=../../../sw/build/main.hex"
```

## UART DPI

To simulate the UART, we use the LowRISC OpenTitan [UART DPI](https://github.com/lowRISC/opentitan/tree/master/hw/dv/dpi/uartdpi).
Read how to interact with it in the Section "Running Software on a Verilator Simulation with Bazel" [here](https://opentitan.org/guides/getting_started/setup_verilator.html#running-software-on-a-verilator-simulation-with-bazel).
The output of the UART DPI module is printed in the `uart0.log` file in the simulation folder.

For example, to see the "hello world!" output of the Verilator simulation:

```
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator
./Vtestharness +firmware=../../../sw/build/main.hex
cat uart0.log
```
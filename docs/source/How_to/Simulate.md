# Simulate

This project supports simulation with Verilator, Synopsys VCS, Siemens Questasim and Cadence Xcelium.
We use [FuseSoC](https://github.com/olofk/fusesoc) for all the EDA tools we use. The `fusesoc` commands are used in the targets in the Makefile.
For example, if you want to set the `FPU` and `COREV_PULP` parameters of the `cv32e40p` CPU,
you need to add next to your compilation command `FUSESOC_PARAM="--COREV_PULP=1 --FPU=1"`
Below the different EDA examples commands.

## Simulating with Verilator

To simulate your application with Verilator, first build the Verilator model:

```
make verilator-sim
```

then, go to your target system build folder

```
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator
```

and type to run your compiled application:

```
./Vtestharness +firmware=../../../sw/build/main.hex
```

Finally, you can check the output by running:

```
cat uart0.log
```

You can directly compile the app and run all of the previous steps with:

```
make run-app-verilator
```

```{warning}
The `run-app-verilator` target calls the `app` target, so the application will be recompiled with default parameters unless you add specific ones.
```

## Simulating with VCS

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

## Simulating with Questasim

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

## Simulating with Xcelium

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

## Simulation parameters

You may pass additional simulation parameters to the generated simulation executable, in the form of *plusargs*: `+<parameter>=<value>`.

- `+firmware=<path>`:
  Loads the hex file specified in `<path>` into memory.
  This allows you to run a compiled executable directly, as if it were already written in memory since the beginning of the simulation.
  For example, `./Vtestharness +firmware=../../../sw/build/main.hex` will launch the Verilator simulation and instruct it to load the compiled application executable into memory and run it.

- `+max_sim_time=<time>`:
  Runs the simulation for a maximum of `<time>` clock cycles.
  This is useful in case your application gets stuck in a certain point and never finishes; this parameter will force the simulation to terminate after a certain time so that you can later analyze the generated `waveform.vcd` file.
  In that case, the simulation executable will exit with a return code of 2, indicating premature termination.
  If this parameter is not provided, the simulation will run until the program finishes (the `main()` function ends).

  Alternatively, you may add a time suffix (`s`, `ms`, `us`, `ns`, `ps`) to run the simulation until the specified simulation time has been reached (1 clock cycle = 10ns).
  This is, `+max_sim_time=750us` is the same as `+max_sim_time=75000`.
  (Note that there's no space between the number and the unit, and that fractional values are not supported.)

  If you're launching the Verilator simulation via `make`, you may pass this parameter via the `MAX_SIM_TIME=` command line argument, e.g. `make run-helloworld MAX_SIM_TIME=750us`.

## Simulating the UART DPI

To simulate the UART, we use the LowRISC OpenTitan [UART DPI](https://github.com/lowRISC/opentitan/tree/master/hw/dv/dpi/uartdpi).
Read how to interact with it in the Section "Running Software on a Verilator Simulation with Bazel" [here](https://opentitan.org/guides/getting_started/setup_verilator.html#running-software-on-a-verilator-simulation-with-bazel).
The output of the UART DPI module is printed in the `uart0.log` file in the simulation folder.

For example, to see the "hello world!" output of the Verilator simulation:

```
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator
./Vtestharness +firmware=../../../sw/build/main.hex
cat uart0.log
```

#  Integrate Analog / Mixed-Signal simulations

## About

VCS offers the possibility to run AMS simulations by interfacing analog SPICE files with the rest of the digital RTL design. This is done through a special file located under `hw/ip_examples/ams/analog` named `control.init`.

## The SPICE (.sp) file

SPICE files can be created with the tool of your choice, however they must sometimes be edited manually afterwards to meet the following criteria:

- The format should be HSPICE compatible

- If not already done, one or more library inclusion lines:
```
.LIB /path/to/pdk/lib
```
must be added at the beginning of the file to include the necessary files from the PDK.

- After the .LIB lines, power nets must be manually added in the file and made global. In most cases it should look like this:
```
v_vdd vdd 0 1.2
v_gnd gnd 0 0
.global vdd gnd
```

- Aside from the .LIB inclusions and the power nets, the file must only contain subcircuits.
    - This means that lines describing temperature, parameters, options and so forth should be removed from the file
    - This also means that there should be no top-level elements such as transistors, passive components etc at the top-level. The top-level must thus itself be a subcircuit, with IO pins corresponding to the names defined in the RTL part.

- Keep in mind that all names in HSPICE netlists are treated as lowercase, thus make sure they will properly match with the RTL part.

### Example procedure with Cadence Virtuoso

- Design a schematic that should later be simulated, make sure the toplevel pins match the name of the pins in the RTL part of the peripheral
- Create a toplevel symbol of the schematic you want to simulate
- Run ADE L, select hspiceD in Setup -> Simulator
- Select Simulation -> Netlist -> Create and save the SPICE output in a file
- Open the SPICE file with a text editor, remove lines beginning with .TEMP & .OPTION (remember the file should only
- Add the global power nets tat the beginning of the file:
```
v_vdd vdd 0 1.2
v_gnd gnd 0 0
.global vdd gnd
```
- Go to the bottom of the file, remove toplevel info (the circuit instance `xi` and the `.END`; there must be only subckts left
- The SPICE file is now ready to be simulated. Place it in `hw/ip_examples/ams/analog` and create a `control.init` file (see next section)

## The control.init file

This file should at the very least look similar to this to be able to run a successful simulation:
```
choose xa ../../../hw/ip_examples/ams/analog/adc.sp;
port_connect -cell ams_adc_1b ( vdd => vdd , gnd => gnd );
port_dir -cell ams_adc_1b (input sel; output out);
bus_format <%d>;
```
- The first line tells what analog simulator should VCS be using, and what SPICE file it should be simulating. In this case, by specifying `xa`, we tell it to use CustomSim, which reads SPICE files in the HSPICE format. Note that as this file will be ran from the VCS runtime directory, relative paths pointing to the .sp file should take this into account.
- The second line specifies which RTL cell should be connected to the SPICE top-level subcircuit. VDD & GND must be connected as indicated.
- The third line specifies the direction (input/output) of the ports which are connected from RTL to SPICE. This isn't always necessary to specify, but the simulation might fail without this line
- The last line specifies the bus format used by the SPICE file, which is usually <%d> but can sometimes be [%d] or even (%d) depending on how your SPICE file was written.

Additional lines and options can of course be specified: refer to the official Synopsys Mixed-Signal Simulation User Guide for complete instructions.

## The example AMS peripheral and the interfacing of SPICE netlists within X-HEEP

The example AMS peripheral used by simulations of X-HEEP is located in `hw/ip_examples/ams`. You should edit the port names so they match the top-level connectivity of the SPICE netlist.


### The repository's example SPICE files

![Example ADC](/images/example_adc.svg)

An example `adc.sp` file can be found in `hw/ip_examples/ams/analog`. This is a 1-bit ADC with a threshold that is configured through the 2-bit wide SEL input: an input of 00, 01, 10 and 11 will provide a threshold of 20%, 40%, 60% and 80% of VDD (1.2V) respectively. The input signal of the ADC is a sine wave with a peak-to-peak amplitude of 1.2V directly placed inside the SPICE netlist.

The SPICE netlist uses the [65nm_bulk PTM Bulk CMOS model](http://ptm.asu.edu/modelcard/2006/65nm_bulk.pm) obtained from [https://ptm.asu.edu](https://ptm.asu.edu/) (February 22, 2006 release) ; to be able to simulate this file with VCS/CustomSim, the model file should be placed in `hw/ip_examples/ams/analog/65nm_bulk.pm`.

## Simulating with VCS-AMS and CustomSim

The AMS simulation of X-HEEP can be ran by typing
```
make vcs-ams-sim
```

then going to the target system built folder

```
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-vcs
```

and running the executable

```
./openhwgroup.org_systems_core-v-mini-mcu_0 +firmware=../../../sw/build/main.hex
```

However, due to the analog nature of the simulation, viewing the waveforms is very useful as well.

### Viewing the waveforms with Verdi

To run the simulation through Verdi, make sure to have the `VERDI_HOME` environmental variable  set then run
```
./openhwgroup.org_systems_core-v-mini-mcu_0 +firmware=../../../sw/build/main.hex -gui
```

It may be that you don't see the list signals: click on View -> Signal list. Then, select the desired signals and put the desired simulation time on the box just after the green arrow and click on the green arrow (run Simulation).

In case you cannot add internal signals to the waveform, try to delete the sim-vcs build directory and rebuilding.


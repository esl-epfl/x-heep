# implement on ASIC

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

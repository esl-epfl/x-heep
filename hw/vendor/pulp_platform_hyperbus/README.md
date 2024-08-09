# HyperBus v2

This peripheral implements an AXI4-compliant interface for the HyperBus protocol, described [in its specification](https://www.cypress.com/file/213356/download). HyperBus v2 is part of the [PULP (Parallel Ultra-Low-Power) platform](https://pulp-platform.org/).

Hyperbus is mainly used for off-chip memories (HyperFlash and HyperRAM), but also supported by some generic peripherals.

## Features

The AXI data widths are parameterizable from 16 to 1024, with transfers of all sizes possible. It is also possible to decide how many memories to connect on the same HyperBUS to set the overall available memory.

Multiple memories on the same bus are placed contiguously in the address memory map and selected through their dedicated CS. At runtime, one can communicate to the controller the size
of the HyperRAMs, and the controller will demultiplex the transactions accordingly. Also, one can choose how many HyperBUS interfaces (1 or 2) to expose. Both buses will have the same number of CSs.
When exposing 2 HyperBUSes, the pair of memories on the same chip select will be mapped as interleaved: each memory will be seen as a memory block of 16-bit width. Doing so will double the maximum achievable bandwidth, up to 6.4 Gbps,
doubling the pin count.

The main restrictions are as follows:

- Atomics are not supported.
- Only linear bursts are supported.
- All accesses except byte-size accesses must be aligned to 16-bit boundaries.
- Only communication with HyperRAMs has been tested. Support for flash is WIP.

The address width is also fully parameterizable. We support bursts of any size permitted by AXI and stalling through PHY-level clock stopping and protocol-level burst splitting. We do _not_ buffer bursts to support devices without the clock stop feature; please ensure your device supports clock stop or ensure sufficient buffering upstream.

The configuration register interface uses the minimal Regbus protocol. Data and address widths are parameterizable, but register sizes must be a power of two larger than 16 bits.

## Architecture

The block diagram below outlines the approximate architecture. Note that there are *two* internally provided clock domains,`clk_sys` and `clk_phy`, as well as an incoming clock `clk_rwds_in` which is then internally delayed.
The `clk_phy` and `clk_phy_90` have a 90 degree difference in phase. To obtain the two clocks one can either shift the input `clk_phy_i` with the clock delayer or to use the `ddr_clk` module that halves the frequency and generated 4 90-degree shifted clocks. 

![HyperBus v2 block diagram](./docs/axi_hyper.svg)

## Simulation

† * `models/s27ks0641/s27ks0641.v` will download externally provided peripheral simulation models, some proprietary and with non-free license terms, from their publically accessible sources; see `Makefile` for details. By running `models/s27ks0641/s27ks0641.v` or the default target `run`, you accept this.*

To run a simulation you need [Bender](https://github.com/pulp-platform/bender) and Questasim. Export your path to include your bender binary, and then:

```bash
bender update
make run #(will download proprietary models from Infineon !!)
```

## ToDos

- [ ] Support byte-aligned accesses for non-byte-size transfer
- [ ] Test HyperFlash
- [ ] PSRAM support through additional CA decoder

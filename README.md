<br />
<p align="center"><img src="docs/source/images/x-heep-outline.png" width="500"></p>

`X-HEEP` (eXtendable Heterogeneous Energy-Efficient Platform) is a `RISC-V` microcontroller described in `SystemVerilog`
that can be configured to target small and tiny platforms as well as extended to support accelerators.
The cool thing about `X-HEEP` is that we provide a simple customizable MCU, so CPUs, common peripherals, memories, etc.
so that you can extend it with your own accelerator without modifying the MCU, but just instantiating it in your design.
By doing so, you inherit an IP capable of booting RTOS (such as `freeRTOS`) with the whole FW stack, including `HAL` drivers and `SDK`,
and you can focus on building your special HW supported by the microcontroller.

`X-HEEP` supports simulation with Verilator, Questasim, etc. Morever, FW can be built and linked by using `CMake` either with gcc or with clang. It can be implemented on FPGA, and it supports implementation in Silicon, which is its main (but not only) target. See below for more details.

The block diagram below shows the `X-HEEP` MCU

<p align="center"><img src="block_diagrams/core_v_mini_mcu.svg" width="650"></p>


> :bookmark_tabs: Please refer to the documentation in [Read the Docs](https://x-heep.readthedocs.io/en/latest/index.html)

The original `README` is still available in `docs/source/old_documentation/READAME.md`.

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
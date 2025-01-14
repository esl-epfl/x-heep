# X-HEEP-based eXamples

Here you can find a list of `X-HEEP` based open-source examples. If you want to include your project in this list, please open an issue with a link to your repository.

* [CGRA-X-HEEP](https://github.com/esl-epfl/cgra_x_heep): A CGRA loosely coupled with X-HEEP.
* [F-HEEP](https://github.com/davidmallasen/F-HEEP): System integrating [fpu_ss](https://github.com/pulp-platform/fpu_ss) into X-HEEP via the eXtension interface and cv32e40px.
* [KALIPSO](https://github.com/vlsi-lab/ntt_intt_kyber) and [KRONOS](https://github.com/vlsi-lab/keccak_integration/tree/keccak_xheep): Loosely-coupled, post-quantum cryptography accelerators for NTT/INTT and Keccak hash function integrated into X-HEEP.

## Inter-process communication using Verilator's DPI

The following [repository](https://github.com/specs-feup/x-heep) uses X-HEEP and the Verilator simulator to model a CPU-CGRA hybrid system. This architecture simulates the CPU integrated into the X-HEEP system, and an external Java process simulates the accelerator. Both components require a communication channel to exchange instructions and data. Using the existing infrastructure to to interact with an external OS process is not feasible at first sight, given that the X-HEEP ecosystem's pipeline encapsulates most of the simulation build and execution, with all modules supplied directly to Verilator. 

To circumvent this issue, this project uses [Direct Programming Interface (DPI)](https://verilator.org/guide/latest/connecting.html) calls (defined in `hw/ip_examples/cgraitf/cgraitfdpi.c`) to establish a connection and communicate with an external process through a Unix Domain Socket. This behavior mirrors the UART module (used as the skeleton code) that connects and outputs _printf_ information to the pseudo-terminal. These calls are embedded in a mock CGRA peripheral/interface, located in `hw/ip_examples/cgraitf/cgraitf.sv`. The module overrides reads and writes to the specified peripheral address, with the proper socket-based mechanism (_send_ or _recv_). The _simple_accelerator_ module could also be similarly customized to perform the same operations, using X-HEEP's  interfaces and memory access protocols. A given user program executed in the CPU (such as `sw/applications/cgra_itf/main.c`) must then select assignments to or from the address to trigger the appropriate action.
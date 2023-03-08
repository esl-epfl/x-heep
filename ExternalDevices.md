## Interface for external devices

The top module core_v_mini_mcu (hw/core-v-mini-mcu/corecore_v_mini_mcu.sv) contains 3 external interfaces:

1. ext_xbar_master : N master ports connected to the system bus

2. ext_xbar_slave : 1 slave port connected to the system bus

3. ext_peripheral_slave : 1 peripheral slave port connected to the system bus (through the peripheral interface)

The number of master ports is set by testharness_pkg::EXT_XBAR_NMASTER.
The slave port and peripheral slave port are fixed to one. Multiple slaves can be connected by adding an address decoding stage.

## External device example

One example using the external ports is provided where:

- hw/ip_examples/slow_sram is a memory slave device
- hw/ip_examples/memcopy is a slave peripheral with a master port. It implements a simple memcopy feature (i.e., DMA).

## Run the external device example

To run the external device example, first compile the software example:

```
make app PROJECT=example_external_peripheral
```

By default, the external device example RTL code is disabled. Run fusesoc with the '--flag=use_external_device_example' option to enable it. This example is available for the sim and sim_opt targets.

For example, compile for Verilator with:

```
make verilator-sim FUSESOC_FLAGS="--flag=use_external_device_example"
```

then, go to your target system built folder

```
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator
```

and type to run your compiled software:

```
./Vtestharness +firmware=../../../sw/build/main.hex
```

If you don't compile the platform with the correct fusesoc flag, the simulation will hang forever because the external peripheral is disabled and never replies.

You can display the uart output with:

```
cat uart0.log
```

It should print:

```
--- MEMCOPY EXAMPLE - external peripheral ---
Init the PLIC...success
Set MEMCOPY interrupt priority to 1...success
Enable MEMCOPY interrupt...Success
Memcopy launched...finished
Complete interrupt...success
MEMCOPY SUCCESS
```

## Add an external master/slave or peripheral

1. Master(s): use the obi_pkg (import obi_pkg::\*;) to create your master_req output port (obi_req_t) and master_resp input port (obi_resp_t). Adjust the EXT_XBAR_NMASTER parameter accordingly.

2. Slave(s): similar to adding a master but you have a slave_req input port (obi_req_t) and slave_resp output port (obi_resp_t). If multiple slaves are used, add a decoding stage for address dispatching.

3. Peripheral slave(s): use the reg_pkg (import obi_pkg::\*;) to create your slave_periph_req input port (reg_req_t) and slave_resp output port (reg_rsp_t). If multiple peripheral slaves are used, add a decoding stage for addresses dispatching.

To create and maintain a peripheral unit efficiently, use the reggen tool:

1. Define the registers of your peripheral in a hjson file (read the documentation [here](https://docs.opentitan.org/doc/rm/register_tool/)).

2. Launch the regtool.py script to generate SystemVerilog RTL code and a C header file.

For example, launching the script hw/ip_examples/memcopy_periph/memcopy_periph_gen.sh generates 2 systemverilog files and 1 C header file:

1. memcopy_periph_reg_top.sv
2. memcopy_periph_reg_pkg.sv
3. memcopy_periph_regs.h

The memocopy_periph_reg_top.sv contains the register file module. It can be instantiated inside your peripheral RTL code (e.g., memcopy_periph.sv) and connected to the control logic of your peripheral.

The memcopy_periph_regs.h contains the address offset of the peripheral registers and can be used in a C application. See the example C code in sw/applications/example_external_peripheral folder.

# interface with external devices

The top module [`core_v_mini_mcu`]((./../../../hw/core-v-mini-mcu/corecore_v_mini_mcu.sv) exposes several external interfaces:

- `ext_xbar_master`: N ports to connect external masters to the internal system bus.

- Five external ports to connect internal masters (e.g., CPU instruction and data ports) to external slaves. Every internal master is exposed to the external subsystem:
   1. `ext_core_instr`: CPU instruction interface.
   2. `ext_core_data`: CPU data interface.
   3. `ext_debug_master`: debug interface.
   4. `ext_dma_read_ch0`: DMA read master, channel 0.
   5. `ext_dma_write_ch0`: DMA write master, channel 0.
   6. `ext_dma_addr_ch0`: DMA address (read) master, channel 0.

- `ext_peripheral_slave`: 1 peripheral slave port connected to the system bus (through the peripheral interface).

The number of external master ports is set by the [`EXT_XBAR_NMASTER`](./../../../tb/testharness_pkg.sv#L10) parameter from `testharness_pkg`.
Multiple OBI slaves can be connected to the exposed internal masters using an external bus, as demonstrated in [`testharness.sv`](./../../../tb/testharness.sv#L232).

> NOTE: the internal bus has no master port connected to the external subsystem. Therefore, an external master cannot send a request to an external slave through one of the exposed master ports. All the address decoding must be done by the external bus: the request must be forwarded to one of the `ext_xbar_master` ports only if the target address falls into the space where internal slaves are mapped. This can be achieved using a 1-to-2 crossbar for each external master as done [here](./../../../tb/ext_bus.sv#L131).

Finally, only one peripheral slave port is available to the external subsystem.

## External device example

One example using the external ports is provided where:

- hw/ip_examples/slow_sram is a memory slave device
- hw/ip_examples/memcopy is a slave peripheral with a master port. It implements a simple memcopy feature (i.e., DMA).
- hw/ip_examples/ams is an example AMS peripheral which can interface with SPICE netlists to run mixed-signal simulations (in this repository, the example analog peripheral is a 1-bit ADC)
    - For more information, see [here](AnalogMixedSignal.md)

## Run the external device example

To run the external device example, first compile the software example:

```bash
make app PROJECT=example_external_dma
```

By default, the external device example RTL code is disabled. This example is available for the sim and sim_opt targets.

For example, compile for Verilator with:

```
make verilator-sim
```

then, go to your target system built folder

```bash
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator
```

and type to run your compiled software:

```bash
./Vtestharness +firmware=../../../sw/build/main.hex
```

If you don't compile the platform with the correct fusesoc flag, the simulation will hang forever because the external peripheral is disabled and never replies.

You can display the UART output with:

```bash
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

2. Slave(s): similar to adding a master but you have a slave_req input port (obi_req_t) and slave_resp output port (obi_resp_t). Remember to connect external masters with external slaves through an external bus. The same bus can be used to connect multiple external slaves to the internal `core_v_mini_mcu` masters.

3. Peripheral slave(s): use the reg_pkg (import obi_pkg::\*;) to create your slave_periph_req input port (reg_req_t) and slave_resp output port (reg_rsp_t). If multiple peripheral slaves are used, add a decoding stage for addresses dispatching.

To create and maintain a peripheral unit efficiently, use the `reggen` tool:

1. Define the registers of your peripheral in a `.hjson` file (read the documentation [here](https://docs.opentitan.org/doc/rm/register_tool/)).

2. Launch the `regtool.py` script to generate SystemVerilog RTL code and a C header file.

For example, launching the script [`memcopy_periph_gen.sh`](./../../../hw/ip_examples/memcopy_periph/memcopy_periph_gen.sh) generates 2 SystemVerilog files and one C header file:

1. `memcopy_periph_reg_top.sv`: the register file module. It can be directly instantiated inside your peripheral RTL code (e.g., [`memcopy_periph.sv`](./../../../hw/ip_examples/memcopy_periph/rtl/memcopy_periph.sv)) and connected to the peripheral device controller(s).
2. `memcopy_periph_reg_pkg.sv`: SystemVerilog package containing the definitions used in the SystemVerilog module above.
3. `memcopy_periph_regs.h`: C/C++ header file defining the address offset of the peripheral configuration registers. Take a look at the C code [here](./../../../sw/applications/example_external_peripheral/memcopy_periph.c) for a usage example.

## External Interrupts

X-HEEP includes several empty external interrupts slots that can be assigned both in HW and SW.

Firstly, connect your external device's interrupt to one of the slots of the `external_interrupt_vector` of X-HEEP:

```systemverilog

logic [core_v_mini_mcu_pkg::NEXT_INT-1:0] ext_intr_vector;

always_comb begin
for (int i = 0; i < core_v_mini_mcu_pkg::NEXT_INT; i++) begin
    ext_intr_vector[i] = 1'b0;    // All interrupt lines set to zero by default
end
ext_intr_vector[0] = my_device_int;  // Re-assign the interrupt lines used here
end

x_heep_system #(
    . . .
) x_heep_system_i (
    .intr_vector_ext_i(ext_intr_vector),
    . . .
)

```

Then, when initializing the PLIC system in software, do not forget to assign the corresponding interrupt ID to your custom handler.

```C
#define MY_DEVICE_INTR EXT_INTR_0

void handler_irq_my_device(uint32_t id) {
    my_device_intr_flag = 1;
    // Do whatever you need here
}

void main() {
    plic_Init(); // Init the PLIC, this will clear all external interrupts assigned previously.
    plic_irq_set_priority(MY_DEVICE_INTR, 1); // Set the priority of the external device's interrupt.
    plic_irq_set_enabled(MY_DEVICE_INTR, kPlicToggleEnabled); // Enable the external device's interrupt.
    plic_assign_external_irq_handler( MY_DEVICE_INTR, (void *) &handler_irq_my_device); // Assign a handler taht will be called when this interrupt is triggered.

    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level external interrupts
    const uint32_t mask = 1 << 11;//IRQ_EXT_ENABLE_OFFSET;
    CSR_SET_BITS(CSR_REG_MIE, mask);

    . . .
}
```
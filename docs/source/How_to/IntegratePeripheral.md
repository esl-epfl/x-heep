# Integrate Peripherals

This documentation file summarizes how to integrate a hardware peripheral in the _x-heep_ platform.

## Contents

This document features:

 - Where to place the peripherals description files
 - How to interface the peripheral in the _x-heep_ platform
 - How to implement the registers
 - How to implement an RX window (for data stream)
 - How to access the peripheral via software
 - How to run a simulation

> **Note**
> This document is a stub. You can contribute by detailing the implementation of other peripheral interfaces you encounter.

## Where to place the peripherals description files

Peripherals RTL code are placed under :

```
hw/ip/<peripheral>
```

The module folder typically contains:

```
.
├── data
│   └── <peripheral>.hjson
├── rtl
│   ├── <peripheral>_reg_pkg.sv
│   ├── <peripheral>_reg_top.sv
│   ├── <peripheral>_reg_window.sv
│   ├── <peripheral>.sv
│   ├── <peripheral>_core.sv
│   └── ...
├── <peripheral>.core
├── <peripheral>.sh
└── <peripheral>.vlt
```

The `<peripheral>.hjson` file is a manifest file that provides peripheral's metadata, bus interfaces, registers declaration and data windows declaration.

The `<peripheral>_reg_pkg.sv` and `<peripheral>_reg_top.sv` files are automatically generated at the peripheral build time.

The `<peripheral>_reg_window.sv` file contains the hardware description of the data window. It it necessary only if the peripheral features such a window and can be merged into the peripheral hardware description. The naming is free.

The `<peripheral>.sv` file is the top of the standalone peripheral, without the interface to the _x-heep_ platform. The `<peripheral>_core.sv` is a wrapper that interfaces the standalone peripheral with the signals coming from the auto-generated files. The naming is free and the files can be merged together but it is useful to keep them separated in order to separately simulate the standalone peripheral.

The `<peripheral>.core` file is a manifest file that declares files and dependencies of the peripheral.

The `<peripheral>.sh` file is a script file that is used to build the peripheral. It will generate the auto-generated files.

The `<peripheral>.vlt` is a waiver file. By default, warnings make the simulation compilation fail. When a warning message is proven not to affect the robustness of the peripheral, a waiver can be added to this file in order to ignore the warning and resume the simulation compilation.

## How to interface the peripheral with the _x-heep_ platform

1. A `<peripheral>.hjson` file must be created (see `hw/ip/pdm2pcm/data/pdm2pcm.hjson` for an example). See below for more details.

2. A `<peripheral>.core` file must be created (see `hw/ip/pdm2pcm/pdm2pcm.core` for an example). Dependencies must be declared under `depend:` and peripheral description files must be declared under `files:`.

3. A `<peripheral>.sh` file must be created. It can be adapted from the example given in `OpenTitanIP.md#modified-generator`:

> **Warning**
> When a file has a template (another file that has the same name but with a `.tpl` extension), this file is auto-generated. Therefore, do only edit the template file. Otherwise, the modifications will be overriden at the platform generation.

4. In case of modification of the GPIOs usage, the `hw/fpga/xilinx_core_v_mini_mcu_wrapper.sv` must be adapted.

a. In the `x_heep_system_i` instance, GPIOs can be replaced by the desired signals:

```diff
-      .gpio_X_io(gpio_io[X]),
+      .your_signal_io(your_signal_io),
```

b. The module `xilinx_core_v_mini_mcu_wrapper` should be modified as follows:

```diff
+    inout  logic your_signal_io,

-    inout logic [X:0] gpio_io,
+    inout logic [X-1:0] gpio_io,
```

c. The pads configuration (pad_cfg.json) must be adapted as well:

```diff
         gpio: {
-            num: <N>,
+            num: <N-D>,
             num_offset: 0, #first gpio is gpio0
             type: inout
         },
+        pdm2pcm_pdm: {
+            num: 1,
+            type: inout
+            mux: {
+                <peripheral_io>: {
+                    type: inout
+                },
+                gpio_K: {
+                    type: inout
+                }
+            }
+        },
```

5. The peripheral subsystem (`hw/core-v-mini-mcu/peripheral_subsystem.sv`) must also be adapted:

I. The I/O signals can be added in the `peripheral_subsystem` module:

```systemverilog
    inout  logic your_signal_io,
```

II. The module must be instantiated in the peripheral subsystem:

```systemverilog
  <peripheral> #(
      .reg_req_t(reg_pkg::reg_req_t),
      .reg_rsp_t(reg_pkg::reg_rsp_t)
  ) <peripheral>_i (
      .clk_i,
      .rst_ni,
      <...>
  );
```

6. The core MCU I/O must be adapted as well (`hw/core-v-mini-mcu/core_v_mini_mcu.sv.tpl`). Add the I/Os to the peripherals subsystem instanciation:

```diff
  peripheral_subsystem #(
      <...>
  ) peripheral_subsystem_i (
      <...>
+      .your_signal_io(your_signal_io),
      <...>
  );
```

7. The peripheral package and the waiver files must be declared in the `core-v-mini-mcu.core` manifest:

```yaml
  depend:
       <...>
     - x-heep:ip:<peripheral>

   files:
       <...>
     - hw/ip/<peripheral>/<peripheral>.vlt
```

8. The MCU configuration (mcu_cfg.json) must be adapted:

```diff
    peripherals: {
      <...>
+        <peripheral>: {
+            offset:  0x00060000,
+            length:  0x00010000,
+        },
    },
```

## How to implement the registers

1. Registers must be declared under the `registers:` list in the `<peripheral>.hjson` file. To add a register, append the following:

```
    { name:     "REGISTER_NAME"
      desc:     "Description"
      swaccess: "rw"
      hwaccess: "hro"
      fields: [
        { bits: "15:0",  name: "LSBITS", desc: "Those are the least significant bits." }
        { bits: "31:16", name: "MSBITS", desc: "Those are the most significant bits." }
      ]
    }
```

2. In order to access registers from hardware, the wrapper file (<peripheral>.sv) must be adapted:

a. The module interface must be as follows:

```systemverilog
module <peripheral> #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic,
    <...>
) (
    input logic clk_i,
    input logic rst_ni,

    // Register interface
    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    // I/Os
    inout  logic your_signal_io,
    <...>
);
```

b. The corresponding package must be imported:

```
import <peripheral>_reg_pkg::*;
```

c. There are two objects used to interact with registers:

* `reg2hw`, that contains signals to read values from registers (hardware readable)
* `hw2reg`, that contains signals to write values into the registers (hardware writable).

d. The signals used to interact with registers are the following:

* `q` contains the current value of a register to be read by hardware
* `d` is written to assign a value to the register
* `de` is an enable signal to get the value written by the hardware from software. It must be asserted to `1` to be able to read it from software.

e. A register with only one filed is accessed by its name and a register with several fields is accessed by its fields.

f. Some examples:

```systemverilog
hw2reg.register.field.de // Data enable signal of a hardware-written field
hw2reg.register.d        // Data to be written on a hardware-written register
reg2hw.register.q        // Data to be read from a register
```

## How to implement an RX window

1. Windows must be declared under the `registers:` list in the `<peripheral>.hjson` file. To add a window, append the following:

```
    { window: {
        name: "RX_WINDOW_NAME",
        items: "1",
        validbits: "32",
        desc: '''Window purpose description'''
        swaccess: "ro"
      }
```

2. An RX window typically looks as follows:

```systemverilog

module <peripheral>_window #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic
) (
    input  reg_req_t        rx_win_i,
    output reg_rsp_t        rx_win_o,
    input            [31:0] rx_data_i,
    output logic            rx_ready_o
);

  import <peripheral>_reg_pkg::*;

  logic [BlockAw-1:0] rx_addr;
  logic rx_win_error;

  assign rx_win_error = (rx_win_i.write == 1'b1) && (rx_addr != <peripheral>_reg_pkg::<peripheral>_RXDATA_OFFSET);
  assign rx_ready_o = rx_win_i.valid & ~rx_win_i.write;
  assign rx_win_o.rdata = rx_data_i;
  assign rx_win_o.error = rx_win_error;
  assign rx_win_o.ready = 1'b1;
  assign rx_addr = rx_win_i.addr;

endmodule : <peripheral>_window
```

3. Data is presented on the `rx_data_i` signal and `rx_ready_o` is asserted.

## How to access the peripheral via software

1. Include the following headers to get the peripherals addresses macros and the I/O manipulation functions:

```c
#include "core_v_mini_mcu.h"
#include "<peripheral>_regs.h"

#include "mmio.h"
```

2. The function used to get the base address of the peripheral is the following (the base address is defined in `sw/device/lib/runtime/core_v_mini_mcu.h.tpl`):

```c
mmio_region_t <peripheral>_base_addr = mmio_region_from_addr((uintptr_t)<peripheral>_START_ADDRESS);
```

3. The read and write functions are the following:

```c
mmio_region_write32(pdm2pcm_base_addr, <peripheral>_REGISTERNAME_REG_OFFSET ,<value_to_write>);
uint32_t response = mmio_region_read32(<peripheral>_base_addr, <peripheral>_REGISTERNAME_REG_OFFSET);
```

## How to run a simulation

Use the `hello_world` (`sw/applications/hello_world`) program to quickly test a design.

The hardware platform is contained in the `testharness.sv` (tb/testharness.sv) test bench.

If the GPIOs usage has changed, the testbench must be adapted as follows:

```diff
- .gpio_X_io(gpio[X]),
+ .your_signal_io(gpio[X]),
```

## Add an interrupt
You must register the interrupt in the MCU configuration `mcu_cfg.json`.
```diff
    interrupts: {
        number: 64, // Do not change this number!
        list: {
          ...
+          <interrupt identifier>: <interrupt num>
        }
    }
```
and then connect your signal in `peripheral_subsystem.sv` to the plic
```diff
+   assign intr_vector[${interrupts["<interrupt identifier>"]}] = <your signal>;
```

In software the interrupt gets trigger as "external" interrupt see `rv_plic` documentation, your interrupt number to be used in c is automatically added to the `core_v_mini_mcu.h` under the preprosessor define `<YOUR INTERRUPT>` .

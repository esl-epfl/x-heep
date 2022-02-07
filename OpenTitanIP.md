## OpenTitan IPs

These are ips cloned from OpenTitan.
Via the `regtool` application, OpenTitan generates `rtl`, `header` files and more.
Please referes to its documentation available at: https://docs.opentitan.org/doc/rm/register_tool/

e.g.

```
$ cd hw/vendor/lowrisc_opentitan/hw/ip/uart
$ ../../../util/regtool.py -r -t rtl data/uart.hjson
$ ../../..//util/regtool.py --cdefines -o uart.h  data/uart.hjson
```


## Modified Generator

OpenTitan generates peripherals that are compliant to the TUL bus.
However, in this project we rely on https://github.com/pulp-platform/register_interface
to generate a different bus interface. The register_interface repository contains an OpenTitan
version that has been patched to cope with the register interface instead of the TUL one.

## Generate the UART IP

From the core-v-mini-mcu root folder:

```
$ cd hw/vendor/lowrisc_opentitan/hw/ip/uart
$ ../../../../pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py -r -t rtl data/uart.hjson
```

You can also genereta the C header file as follow:

```
$../../../../pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py --cdefines -o uart.h  data/uart.hjson
```

Then, manually modify the uart.sv file as:

Replace:

```
`include "prim_assert.sv"
module uart (
```
with

```
`include "common_cells/assertions.svh"

module uart #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic
) (
```

then

```
input  tlul_pkg::tl_h2d_t tl_i,
output tlul_pkg::tl_d2h_t tl_o,
```

with

```
input  reg_req_t reg_req_i,
output reg_rsp_t reg_rsp_o,
```

Change the instance of the uart_reg_top module like this:

```
  uart_reg_top #(
    .reg_req_t(reg_req_t),
    .reg_rsp_t(reg_rsp_t)
  ) u_reg (
    .clk_i,
    .rst_ni,
    .reg_req_i,
    .reg_rsp_o,
    .reg2hw,
    .hw2reg,
    .devmode_i  (1'b1)
  );
```

In addition, in the file `uart.core`, change this:

```
filesets:
  files_rtl:
    depend:
      - lowrisc:constants:top_pkg
      - lowrisc:prim:all
      - lowrisc:ip:tlul
```

to this:

```
filesets:
  files_rtl:
    depend:
      - lowrisc:prim:all
      - pulp-platform.org::register_interface
```

and add `lowrisc:ip:uart:0.1` to the `core-v-mini-mcu.core` `depend` section.

Open the file `hw/vendor/hw/ip/lint/common.core` and delete this:

```
files_check_tool_requirements:
  depend:
   - lowrisc:tool:check_tool_requirements
```

and this:

```
- files_check_tool_requirements
```

TODO: script this and update git apply

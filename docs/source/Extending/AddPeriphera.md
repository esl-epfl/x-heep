
# Integrate Peripherals

This file explains how to integrate a hardware peripheral in the _x-heep_ platform.

## Add your IP to the repository

### Vendor

First things first → vendorise your IP by following the next steps:

In the `hw/vendor` folder, create a file named `YOUR_IP.vendor.hjson`.



```json
/*
* Copyright 2023 EPFL
* Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
* SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
*
* Author: YOUR MAJESTY
*/

{
  "name": "YOUR_IP",
  "target_dir": "YOUR_IP", // this will be the folder created in hw/vendor repo with the content of the git repo you specify

  "upstream": {
    "url": "https://github.com/YOUR_MAJESTY/YOUR_IP.git",
    "rev": "v1.1.1" // specify the version 
  },
  "exclude_from_upstream": [
    ".gitignore",
    ".github",
    "test",
    "doc",
    "util",
    ".gitlab-ci.yml",
    "Bender.yml"
  ]
}
```


To vendorize or revendorize the IP repository inside the `BASE/hw/vendor/` folder, run the following command from your `BASE` directory:

```bash
util/vendor.py --update hw/vendor/YOUR_IP.vendor.hjson
```
Now check if the folder and the `.core` and `.lock` files are generated and **not empty** — otherwise, cry.

```{note}
You might also need to create and leave empty the following files:

- `YOUR_IP.core` and `YOUR_IP.lock.hjson` → these files are (should be) automatically generated after you run the python script. If not, create them, leave em empty and rerun the script.

```
---

### Custom RTL

- Insert your IP folder under `hw/ip`.

- If you intend to use it within the testharness block, specify the dependency within `x-heep-tb-ults.core`:

```json
filesets:
  files_examples:
    depend:
    - example:ip:YOUR_IP
    ...
    files:
    file_type: systemVerilogSource
```

## Connect Your IP

Depending on the purpose of your design, you need to modify different filesets.

### Slave Peripheral Domain (also applies to AO Peripheral)
 `mcu_cfg.hjson`:

```json
peripherals: {
  address: 0x30000000,
  length:  0x00100000,
  ...
  YOUR_IP: {
    // The address you are gonna have is PERIPHERAL_START_ADDRESS + offset
    // This is how you are gonna communicate it through the SW
    offset:  0x00080000,
    length:  0x00010000,
    is_included: "yes" // "yes" = will be automatically included and generated with mcu-gen script
  }  
}
```
After running **MCU GEN**, the template will create a relative address space within `core_v_mini_mcu.pkg`.
The ID for request and response will look like YOUR_IP_IDX and is used in your peripheral subsystem as follows:

```Verilog
.reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::YOUR_IP_IDX]),
.reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::YOUR_IP_IDX]),
```
### Integrate Into Peripheral Subsystem

Initiate your module and provide all necessary connections inside:

`hw/core_v_mini_mcu/Peripheral_subsystem.sv.tpl`

```Verilog
// The %if clause is dependent on the .hjson file
% if 'i2s' in peripherals and peripherals['i2s']['is_included'] == 'yes':
  i2s #(
      .YOUR_PARAMS
  ) i2s_i (
      .clk_i,
      .rst_ni,
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::YOUR_IP_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::YOUR_IP_IDX]),
      ...
  );
% else:
  assign peripheral_slv_rsp[core_v_mini_mcu_pkg::I2S_IDX] = '0; // remember to assign yours

  ...
% endif
```
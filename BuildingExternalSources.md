# Building sources from external directories

If `X-HEEP` is vendorized into your project, you can still have software source files in any directory of your convenience and build them using `X-HEEP`'s compilation flow. 

## Proposed repository folder structure

The most seamless way of building sources from external directories begins with having the following repository folder structure:

    BASE
    ├── sw
    │    ├── applications
    │    │    └── my_app
    │    │        ├── main.c
    │    │        └── ...
    │    ├── build -> ../hw/vendor/esl_epfl_x_heep/sw/build
    │    ├── device -> ../hw/vendor/esl_epfl_x_heep/sw/device
    │    └── linker -> ../hw/vendor/esl_epfl_x_heep/sw/linker
    ├── hw
    │    └── vendor
    │        └── esl_epfl_x_heep
    │            ├── hw
    │            ├── sw
    │            │    ├── applications
    │            │    ├── build
    │            │    ├── device
    │            │    └── ...
    │            ├── Makefile
    │            ├── external.mk
    │            └── ...	
    ├── Makefile
    ├── util
    │    └── vendor.py
    └── ...
    
Where `BASE` is your repository's base directory, `esl_epfl_x_heep` is the vendorized `X-HEEP` repository and `my_app` is the name of the application you intend to build. 

## Vendorizing X-HEEP
In order to vendorize X-HEEP create inside `BASE` a `hw/vendor` directory containing a file named `eslepfl_x_heep.vendor.hjson` containing:
```
// Copyright EPFL
// Licensed under the Solderpad Hardware License v2.1, see LICENSE.txt for details
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

{
  name: "esl_epfl_x_heep",
  target_dir: "esl_epfl_x_heep",

  upstream: {
    url: "https://github.com/esl-epfl/x-heep.git",
    rev: "main",
  },

  patch_dir: "patches/esl_epfl_x_heep",

  exclude_from_upstream: [
    ".github",
    "ci",
  ]
}

```

The branch `main` can be replaced with an specific commit. This is the recommended operation to prevent accidental updates to an incompatible version. 

In a directory `BASE/util` add the [vendor python script](https://github.com/lowRISC/opentitan/blob/master/util/vendor.py). 

To vendorize or revendorize the X-HEEP repository inside a `BASE/hw/vendor/esl_epfl_x_heep/` folder run the following command from you `BASE`. 
```
util/vendor.py --update hw/vendor/eslepfl_x_heep.vendor.hjson 
```

## The BASE/sw/ folder

The `BASE/sw/` folder must comply with `X-HEEP` repository structure and therefore include an `applications`, `build`, `device` and `linker` folder.
Inside the `applications` folder different projects can be stored (still respecting the `name_of_project/main.c` structure). 
The `build`, `device` and `linker` should be linked with the vendorized folders inside `X-HEEP`.
```
ln -s hw/vendor/esl_epfl_x_heep/sw/build sw/build
ln -s hw/vendor/esl_epfl_x_heep/sw/device sw/device
ln -s hw/vendor/esl_epfl_x_heep/sw/linker sw/linker
```

## The BASE/Makefile
The `BASE/Makefile` is your own custom Makefile. You can use it as a bridge to access the Makefile from `X-HEEP`. 

To do so, it MUST include the `external.mk` AFTER all your custom rules. 

This is an example `BASE/Makefile`:
```
MAKE     = make

test:
    @echo Nothing is executed from X-HEEP

app:
    @echo This is app being run from the x-heep repo
    $(MAKE) -f $(XHEEP_MAKE) $(MAKECMDGOALS) PROJECT=hello_world SOURCE=.

mcu-gen: 
    @echo You will not mcu-gen

export HEEP_DIR = hw/vendor/esl_epfl_x_heep/
XHEEP_MAKE = $(HEEP_DIR)/external.mk
include $(XHEEP_MAKE)
```

* The `test` rule will not use the `X-HEEP` Makefile in any way.
* The `app` rule will perform actions before calling `X-HEEP` Makefile's `app` rule. In this case, the project and where the source files are to be extracted from is being specified. The `SOURCE=.` argument will set `X-HEEP`'s own `sw/` folder as the directory from which to fetch source files. This is an example of building inner sources from an external directory. 
* The `mcu-gen` rule will override the `X-HEEP` Makefile's one. 
* Any other target will be passed straight to `X-HEEP`'s Makefile, meaning that the following command will build the sources from the external directory:
```
make app PROJECT=my_app
```

## Different use cases
If you plan to vendorize `X-HEEP` in a different directory than the one proposed, just update in your `BASE/Makefile`:
```
export HEEP_DIR = <path_to_x_heep_relative_to_this_directory>
```

If you plan to store source files in a different location that the one proposed, just call `make` making the `SOURCE` path explicit. 
```
make app PROJECT=my_app SOURCE=<path_to_your_sw_relative_to_x_heep_sw>
```
Consider that inside this `sw` folder the same structure than the one proposed is required.  

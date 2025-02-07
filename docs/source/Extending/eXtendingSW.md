# eXtending software

If X-HEEP is vendorized into your project, you can still have software source files in any directory of your top-level repository and build them using X-HEEP's compilation flow.

## Repository structure

The following is an example repository folder structure focusing on extending the software flow.

    BASE
    ├── sw
    │   ├── applications
    │   │   └── your_app
    │   │       ├── main.c
    │   │       ├── your_app.c
    │   │       ├── your_app.h
    │   │       └── ...
    │   ├── build -> ../hw/vendor/esl_epfl_x_heep/sw/build
    │   ├── device -> ../hw/vendor/esl_epfl_x_heep/sw/device
    │   ├── linker -> ../hw/vendor/esl_epfl_x_heep/sw/linker
    │   └── external
    │       ├── drivers
    │       │   └── your_copro
    │       │   	├── your_copro.c
    │       │   	├── your_copro.h
    │       │   	└── your_copro_defs.h -> ../../../../hw/vendor/your_copro/sw/your_copro_defs.h
    │       └── extensions
    │       │   └── your_copro_x_heep.h
    │       └── lib
    │           └── crt
    │               └── external_crt0.S
    ├── hw
    │   └── vendor
    │       ├── your_copro
    │       ├── esl_epfl_x_heep.vendor.hjson
    │       └── esl_epfl_x_heep
    │           ├── hw
    │           ├── sw
    │           │   ├── applications
    │           │   ├── build
    │           │   ├── device
    │           │   └── ...
    │           ├── Makefile
    │           ├── external.mk
    │           └── ...
    ├── Makefile
    ├── util
    │   └── vendor.py
    └── ...

Where `BASE` is your repository's base directory, `esl_epfl_x_heep` is the vendorized X-HEEP repository and `your_app` is the name of the application you intend to build.

## The /sw/ folder

The `BASE/sw/` folder must comply with X-HEEP's repository structure and therefore include an `applications`, `build`, `device` and `linker` folder.
It is not compulsory for it to be on the `BASE` directory, although this is the default structure that X-HEEP's Makefiles will assume if no other path is specified through the `SOURCE` variable. If you plan to store source files in a different location that the one proposed, just call `make` making the `SOURCE` path explicit.
```
make app PROJECT=your_app SOURCE=<path_to_your_sw_relative_to_x_heep_sw>
```
Consider that, inside this `sw` folder, the same structure than the one proposed is required.

Inside the `applications` folder different projects can be stored (still respecting the `name_of_project/main.c` structure of X-HEEP).
The `build`, `device` and `linker` should be linked with the vendorized folders inside X-HEEP.
In this example that is done from the `BASE` directory as follows:
```
ln -s ../hw/vendor/esl_epfl_x_heep/sw/build sw/build
ln -s ../hw/vendor/esl_epfl_x_heep/sw/device sw/device
ln -s ../hw/vendor/esl_epfl_x_heep/sw/linker sw/linker
```

## The /sw/applications folder

Inside the `sw/applications/` folder you may have different applications that can be built separately. Each application is a directory named after your application, containing one and only one `main.c` file which is built during the compilation process. The folder can contain other source or header files (of any name but `main.c`).

## The /sw/external folder

In the (optional) `external` folder you can add whatever is necessary for software to work with your coprocessor/accelerator. This might include:

* Sources and header files.
* Soft links to folders or files.
* A `lib/crt/` directory with an `exteral_crt0.S` file (will be included inside `BASE/sw/device/lib/crt/crt0.S`).

The external folder or any of its subdirectories cannot contain neither a `device` nor an `applications` folder as it would collide with the respective folders inside `BASE/sw/`. It should also not contain a `main.c` file.

## The BASE/Makefile

The `BASE/Makefile` is your own custom Makefile. You can use it as a bridge to access the Makefile from X-HEEP. To do so, it MUST include the `external.mk` AFTER all your custom rules.


<details>
    <summary>Example of BASE/Makefile</summary>

```Makefile
MAKE     = make
.PHONY: custom
custom:
    @echo Nothing is executed from X-HEEP, as custom is not a target inside X-HEEP.

app:
    @echo This target will do something and then call the one inside X-HEEP.
    $(MAKE) -f $(XHEEP_MAKE) $(MAKECMDGOALS) PROJECT=hello_world SOURCE=.

verilator-sim:
    @echo You will not access the verilator-sim target from X-HEEP.

export HEEP_DIR = hw/vendor/esl_epfl_x_heep/
XHEEP_MAKE = $(HEEP_DIR)/external.mk
include $(XHEEP_MAKE)
```

- The `custom` rule will not use the X-HEEP Makefile in any way. Make the target a prerequisite of `.PHONY` to prevent X-HEEP's Makefile from attempting to run a non-existent target.
- The `app` rule will perform actions before calling X-HEEP Makefile's `app` rule. In this case, the project and where the source files are to be extracted from is being specified. The `SOURCE=.` argument will set X-HEEP's own `sw/` folder as the directory from which to fetch source files. This is an example of building inner sources from an external directory.
- The `verilator-sim` rule will override the X-HEEP Makefile's one.
- Any other target will be passed straight to X-HEEP's Makefile. For example
```sh
make mcu-gen CPU=cv32e40px
```

If you plan to vendorize X-HEEP in a different directory than the one proposed, just update:
```
export HEEP_DIR = <path_to_x_heep_relative_to_this_directory>
```
</details><br>

## Excluding files from compilation

If you have files that need to be excluded from the gcc compilation flow, you can add them to a directory containing the keyword `exclude`, and/or rename the file to include the keyword `exclude`. 
In the following example, the files marked with ✅ will be compiled, and the ones marked with ❌ will not.

    BASE
    ├── sw
    │   ├── applications
    │   │   └── your_app
    │   │       ├── ✅ main.c      
    │   │       ├── ✅ your_app.c
    │   │       ├──    your_app.h
    │   │       ├── ❌ my_kernel_exclude.c
    │   │       ├──    my_kernel.h
    │   │       └── exclude_files
    │   │           └── ❌ kernel_asm.S

## Makefile help

If you want that the commands `make` or `make help` show the help for your external Makefile, add the following lines before the first `include` directive or target.

<details>
    <summary>Addition to print the target's help</summary>

```Makefile
# HEEP_DIR might already be defined, you may want to move it to the top
export HEEP_DIR = hw/vendor/esl_epfl_x_heep/

# Get the path of this Makefile to pass to the Makefile help generator
MKFILE_PATH = $(shell dirname "$(realpath $(firstword $(MAKEFILE_LIST)))")
export FILE_FOR_HELP = $(MKFILE_PATH)/Makefile


## Call the help generator. Calling simply
## $ make
## or
## $ make help
## Will print the help of this project.
## With the parameter WHICH you can select to print
## either the help of X-HEEP (WHICH=xheep)
## or both this project's and X-HEEP's (WHICH=all)
help:
ifndef WHICH
	${HEEP_DIR}/util/MakefileHelp
else ifeq ($(filter $(WHICH),xheep x-heep),)
	${HEEP_DIR}/util/MakefileHelp
	$(MAKE) -C $(HEEP_DIR) help
else
	$(MAKE) -C $(HEEP_DIR) help
endif
```

</details><br>

> Remeber to add double hashes `##` on any comment you want printed on the help.
> Use `## @section SectionName` to divide the documentation in sections

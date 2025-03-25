# Understanding the Tools

This section will provide a brief overview of the main tools used in the development of X-HEEP.

## Vendor

In X-HEEP we use the [vendor tool](https://opentitan.org/book/util/doc/vendor.html) from OpenTitan
to manage the code which we include from external repositories. The vendor tool is a simple tool
that allows you to copy upstream sources into the repository. You can find the vendor tool in
`util/vendor.py`, and the vendored repositories in `hw/vendor`.

In `hw/vendor` you will find a `<organization>_<repo_name>.vendor.hjson` file for each vendored
repository. This vendor description file contains the information needed by the vendor tool to fetch
the upstream repository and perform any necessary modifications to it.

The vendor tool allows to apply git patches to the vendored repository. Git patches are files that
capture changes made in a repository, allowing you to share and apply these changes elsewhere. This
is useful when we need to update some files from the vendored repository. You can find all of the
patches we are applying in the `hw/vendor/patches` directory. To create a git patch, you can use the
`git diff` command. For this, first clone the repository you want to create the patch for, make the
changes you want to capture, use `git add` to stage the changes, and then run `git diff main > my_patch.patch`.
This will create a patch file with the changes you made.

To vendorize or revendorize a repository, you can run the following command from the root of the
repository:

```bash
util/vendor.py --update hw/vendor/<organization>_<repo_name>.vendor.hjson
```

For more details on the vendor tool, and a detailed example of a `<organization>_<repo_name>.vendor.hjson`
file, please check the [official vendor documentation](https://opentitan.org/book/util/doc/vendor.html).

## FuseSoC and .core files

[FuseSoC](https://fusesoc.readthedocs.io/en/stable/index.html) is an open-source package manager and
build system for hardware design, primarily focusing on FPGA and ASIC development. It simplifies
working with complex hardware projects by managing dependencies, configurations, and tool flows. It
is the tool we use in X-HEEP to manage the different flows that we support.

The main building blocks of FuseSoC are the `.core` files. A `.core` file is self-contained
description of a hardware module, including its sources, dependencies, and configurations. You can
find the `.core` files for X-HEEP all around the project, especially in the `hw` directory and its
subdirectories. However, the main `.core` file is `core-v-mini-mcu.core`. To understand `.core`
files, modify them, or write your own, please check the documentation on this in the
[FuseSoC documentation](https://fusesoc.readthedocs.io/en/stable/user/build_system/core_files.html).

X-HEEP's `Makefile` uses FuseSoC behind the scenes to build, simulate, and synthesize the design, among
other uses.

For more details on FuseSoC, please check the [official documentation](https://fusesoc.readthedocs.io/en/stable/index.html).

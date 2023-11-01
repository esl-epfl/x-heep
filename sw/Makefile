# Copyright 2019 Clifford Wolf
# Copyright 2019 Robert Balas
# Copyright 2022 Jose Miranda
# Copyright 2023 Juan Sapriza
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

# Author: Juan Sapriza (juan.sapriza@epfl.ch)

MAKE                       = make

# Project options are based on the app to be build (default - hello_world)
PROJECT  ?= hello_world

# Linker options are 'on_chip' (default),'flash_load','flash_exec'
LINKER   ?= on_chip

# Target options are 'sim' (default) and 'pynq-z2'
TARGET   ?= sim

# Compiler options are 'gcc' (default) and 'clang'
COMPILER ?= gcc

# Compiler prefix options are 'riscv32-unknown-' (default)
COMPILER_PREFIX ?= riscv32-unknown-

# Arch options are any RISC-V ISA string supported by the CPU. Default 'rv32imc'
ARCH     ?= rv32imc

# Path relative from the location of sw/Makefile from which to fetch source files. The directory of that file is the default value.
SOURCE 	 ?= $(".")

# riscv toolchain install path
RISCV                      ?= ~/.riscv
RISCV_EXE_PREFIX           = $(RISCV)/bin/${COMPILER_PREFIX}elf-
RISCV_GDB_PATH             = $(RISCV_EXE_PREFIX)gdb

# Get the absolute path
mkfile_path := $(shell dirname "$(realpath $(firstword $(MAKEFILE_LIST)))")

# Check the absolute path
$(info $$You are executing from: $(mkfile_path))

# Get the absolute path of where to fetch sources
source_path := $(realpath $(mkfile_path)/$(SOURCE))
$(info $$You are fetching sources from $(source_path) )


SOURCE_PATH   		       = $(source_path)/
ROOT_PROJECT               = $(mkfile_path)/
INC_FOLDERS                = $(mkfile_path)/device/target/$(TARGET)/
LINK_FOLDER                = $(mkfile_path)/linker

# CMake keyword
CMAKE_DIR=cmake

ifeq (, $(shell which cmake3)) # cmake3 is not defined
CMAKE=cmake
else
CMAKE=cmake3
endif

# Export variables to sub-makefiles
export

# Let's CMake!
include cmake/targets.mak

# GDB connection using RISCV-GDB back-end
gdb_connect:
	  $(RISCV_GDB_PATH) build/main.elf -x gdbInit;

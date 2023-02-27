# Copyright 2019 Clifford Wolf
# Copyright 2019 Robert Balas
# Copyright 2022 Jose Miranda
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

# Author: Jose Miranda (jose.mirandacalero@epfl.ch)

MAKE                       = make

# CMakeLists top file needs to know these variables:
#ifeq ($(origin PROJECT), undefined)
#  $(error $$Please, what is the name of your project?)
#endif
#ifeq ($(origin MAINFILE), undefined)
#  $(error $$Please, what is the name of your main file?)
#endif
#ifeq ($(origin LINKER), undefined)
#  $(error $$Please, what is the linker of your app?)
#endif

# Project options are based on the app to be build (default - hello_world)
PROJECT  ?= hello_world

# Mainfile options are based on the main file to be build (default - hello_world)
MAINFILE ?= hello_world

# Linker options are 'on_chip' (default),'flash_load','flash_exec','freertos'
LINKER   ?= on_chip

# Target options are 'sim' (default) and 'pynq-z2'
TARGET   ?= sim

# riscv toolchain install path
RISCV                      ?= ~/.riscv
RISCV_EXE_PREFIX           = $(RISCV)/bin/riscv32-unknown-elf-
RISCV_GDB_PATH             = $(RISCV_EXE_PREFIX)gdb

# Sanity check
TARGET                     ?= sim

# Get the absolute path
mkfile_path := $(shell dirname "$(realpath $(firstword $(MAKEFILE_LIST)))")

# Check the absolute path
$(info $$You are executing from: $(mkfile_path))

ROOT_PROJECT               = $(mkfile_path)/
LIB_CRT                    = $(mkfile_path)/device/lib/crt/
LIB_CRT_FLASH_EXEC         = $(mkfile_path)/device/lib/crt_flash_exec/
LIB_CRT_FLASH_LOAD         = $(mkfile_path)/device/lib/crt_flash_load/
LIB_CRT_FREERTOS           = $(mkfile_path)/device/lib/crt_freertos/
LIB_BASE                   = $(mkfile_path)/device/lib/base/
LIB_BASE_FREESTD           = $(mkfile_path)/device/lib/base/freestanding/
LIB_RUNTIME                = $(mkfile_path)/device/lib/runtime/
LIB_DRIVERS                = $(mkfile_path)/device/lib/drivers/
INC_FOLDERS                = $(mkfile_path)/device/target/$(TARGET)/
INC_FOLDERS_GCC            = $(addprefix -I ,$(INC_FOLDERS))
LINK_FOLDER                = $(mkfile_path)/linker

# CMake keyword
CMAKE_DIR=cmake

# Let's CMake!
include cmake/targets.mak

# GDB connection using RISCV-GDB back-end
gdb_connect:
	  $(RISCV_GDB_PATH) build/$(MAINFILE).elf -x gdbInit;

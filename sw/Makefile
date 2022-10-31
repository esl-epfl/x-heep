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

# CMakeLists top file needs to know these variables:
ifeq ($(origin PROJECT), undefined)
  $(error $$Please, what is the name of your project?)
endif
ifeq ($(origin MAINFILE), undefined)
  $(error $$Please, what is the name of your main file?)
endif

MAKE                       = make

# riscv toolchain install path
RISCV                      ?= ~/.riscv
RISCV_EXE_PREFIX           = $(RISCV)/bin/riscv32-unknown-elf-

TARGET                     ?= sim

mkfile_path := $(shell dirname "$(realpath $(firstword $(MAKEFILE_LIST)))")
#current_dir := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))

$(info $$You are executing from: $(mkfile_path))

# GCC configuration
CUSTOM_GCC_FLAGS           =

ROOT_PROJECT               = $(mkfile_path)/
LIB_CRT                    = $(mkfile_path)/device/lib/crt/
LIB_CRT_FLASH_EXEC         = $(mkfile_path)/device/lib/crt_flash_exec/
LIB_CRT_FLASH_LOAD         = $(mkfile_path)/device/lib/crt_flash_load/
LIB_BASE                   = $(mkfile_path)/device/lib/base/
LIB_BASE_FREESTD           = $(mkfile_path)/device/lib/base/freestanding/
LIB_RUNTIME                = $(mkfile_path)/device/lib/runtime/
LIB_DRIVERS                = $(mkfile_path)/device/lib/drivers/
INC_FOLDERS                = $(mkfile_path)/device/target/$(TARGET)/
INC_FOLDERS_GCC            = $(addprefix -I ,$(INC_FOLDERS))
LINK_FOLDER                = $(mkfile_path)/linker

# print message to see if wildcard is working
$(info $$LIB_CRT PATH: $(LIB_CRT))

CMAKE_DIR=cmake

$(info $$LIB_RUNTIME PATH: $(LIB_RUNTIME))

# Running programs:
include cmake/targets.mak

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

set(RISCV  /home/$ENV{USER}/tools/riscv)
set(RISCV_GITHUB_GCC_COMPILER  $ENV{RISCV}/bin/riscv32-unknown-elf-gcc)
#message("RISC-V GCC cross-compiler is in : ${RISCV_GCC_COMPILER}") 

if (EXISTS ${RISCV_GITHUB_GCC_COMPILER})
set( RISCV_GCC_COMPILER ${RISCV_GITHUB_GCC_COMPILER})
else()
message(FATAL_ERROR "RISC-V GCC not found. ${RISCV_GITHUB_GCC_COMPILER}") 
endif()

#message( "RISC-V GCC found: ${RISCV_GCC_COMPILER}")

get_filename_component(RISCV_TOOLCHAIN_BIN_PATH ${RISCV_GCC_COMPILER} DIRECTORY)
get_filename_component(RISCV_TOOLCHAIN_BIN_GCC ${RISCV_GCC_COMPILER} NAME_WE)
get_filename_component(RISCV_TOOLCHAIN_BIN_EXT ${RISCV_GCC_COMPILER} EXT)

#message( "RISC-V GCC Path: ${RISCV_TOOLCHAIN_BIN_PATH}" )

STRING(REGEX REPLACE "\-gcc" "-" CROSS_COMPILE ${RISCV_GCC_COMPILER})
#message( "RISC-V Cross Compile: ${CROSS_COMPILE}" )

# The Generic system name is used for embedded targets (targets without OS) in
# CMake
set( CMAKE_SYSTEM_NAME          Generic )
set( CMAKE_SYSTEM_PROCESSOR     rv32imc )
set( CMAKE_EXECUTABLE_SUFFIX    ".elf")

# specify the cross compiler. We force the compiler so that CMake doesn't
# attempt to build a simple test program as this will fail without us using
# the -nostartfiles option on the command line
#CMAKE_FORCE_C_COMPILER( "${RISCV_TOOLCHAIN_BIN_PATH}/${CROSS_COMPILE}gcc${RISCV_TOOLCHAIN_BIN_EXT}" GNU )
#CMAKE_FORCE_CXX_COMPILER( "${RISCV_TOOLCHAIN_BIN_PATH}/${CROSS_COMPILE}g++${RISCV_TOOLCHAIN_BIN_EXT}" GNU )
set(CMAKE_ASM_COMPILER {CROSS_COMPILE}gcc )
set(CMAKE_AR ${CROSS_COMPILE}ar)
set(CMAKE_ASM_COMPILER ${CROSS_COMPILE}gcc)
set(CMAKE_C_COMPILER ${CROSS_COMPILE}gcc)
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)

# We must set the OBJCOPY setting into cache so that it's available to the
# whole project. Otherwise, this does not get set into the CACHE and therefore
# the build doesn't know what the OBJCOPY filepath is
set( CMAKE_OBJCOPY      ${CROSS_COMPILE}objcopy
     CACHE FILEPATH "The toolchain objcopy command " FORCE )
#message( "OBJCOPY PATH: ${CMAKE_OBJCOPY}" )

set( CMAKE_OBJDUMP      ${CROSS_COMPILE}objdump
     CACHE FILEPATH "The toolchain objdump command " FORCE )
#message( "OBJDUMP PATH: ${CMAKE_OBJDUMP}" )

# Dealing with GDB into CMake - TBD
#set( CMAKE_C_GDB        ${CROSS_COMPILE}gdb 
#     CACHE FILEPATH "The gdb toolchain command " FORCE )

# Set the common build flags

# Set the CMAKE C flags (which should also be used by the assembler!
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=${CMAKE_SYSTEM_PROCESSOR}" )

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "" )
set( CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "" )
set( CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "" )

# Set by deafult Linker flags if needed
#set( CMAKE_EXE_LINKER_FLAGS   "${CMAKE_EXE_LINKER_FLAGS}  -march=${CMAKE_SYSTEM_PROCESSOR}    -nostartfiles   " )


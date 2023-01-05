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

cmake_minimum_required(VERSION 3.15)

include(FetchContent)

if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColourReset "${Esc}[m")
  set(ColourBold  "${Esc}[1m")
  set(Red         "${Esc}[31m")
  set(Green       "${Esc}[32m")
  set(Yellow      "${Esc}[33m")
  set(Blue        "${Esc}[34m")
  set(Magenta     "${Esc}[35m")
  set(Cyan        "${Esc}[36m")
  set(White       "${Esc}[37m")
  set(BoldRed     "${Esc}[1;31m")
  set(BoldGreen   "${Esc}[1;32m")
  set(BoldYellow  "${Esc}[1;33m")
  set(BoldBlue    "${Esc}[1;34m")
  set(BoldMagenta "${Esc}[1;35m")
  set(BoldCyan    "${Esc}[1;36m")
  set(BoldWhite   "${Esc}[1;37m")
endif()

# set the project name
project(${PROJECT} ASM C)

set(CMAKE_CXX_STANDARD 14)

# Set MAIN file
SET(TARGET ${MAINFILE})

# Get the correct path for the crt files and linker file
if (${LINKER} STREQUAL "on_chip")
  SET(LIB_CRT_P "${ROOT_PROJECT}device/lib/crt/")
  SET(LINK_FILE "link.ld")
elseif(${LINKER} STREQUAL "flash_load")
  SET(LIB_CRT_P "${ROOT_PROJECT}device/lib/crt_flash_load/")
  SET(LINK_FILE "link_flash_load.ld")
elseif(${LINKER} STREQUAL "flash_exec")
  SET(LIB_CRT_P	"${ROOT_PROJECT}device/lib/crt_flash_exec/")
  SET(LINK_FILE "link_flash_exec.ld")
elseif(${LINKER} STREQUAL "freertos")
  SET(LIB_CRT_P	"${ROOT_PROJECT}device/lib/crt_freertos/")
  SET(LINK_FILE	"link_freertos.ld")
else()
  message( FATAL_ERROR "Linker specification is not correct" )
endif()

# messages to check the paths
message( "${Magenta}Current project: ${PROJECT}${ColourReset}")
message( "${Magenta}Root project: ${ROOT_PROJECT}${ColourReset}")
message( "${Magenta}LIB_CRT PATH for Cmake: ${LIB_CRT_P}${ColourReset}")
message( "${Magenta}LINKER File for Cmake: ${LINK_FILE}${ColourReset}")
message( "${Magenta}LIB_DRIVERS PATH for Cmake: ${LIB_DRIVERS}${ColourReset}")
message( "${Magenta}Targetting main file: ${MAINFILE}${ColourReset}")
message( "${Magenta}Targetting folder: ${INC_FOLDERS}${ColourReset}")

# Get all the folders to include when linking
SET(INCLUDE_FOLDERS "-I ${RISCV}/riscv32-unknown-elf/include \
                     -I ${RISCV}/riscv32-unknown-elf/include/ \
                     -I ${ROOT_PROJECT} \
					 -I ${ROOT_PROJECT}freertos/ \
                     -I ${ROOT_PROJECT}applications/${PROJECT}/ \
                     -I ${INC_FOLDERS} \
                     -I ${LIB_BASE} \
                     -I ${LIB_BASE_FREESTD} \
                     -I ${LIB_RUNTIME} \
                     -I ${LIB_DRIVERS}dma/ \
                     -I ${LIB_DRIVERS}fast_intr_ctrl/ \
                     -I ${LIB_DRIVERS}gpio/ \
                     -I ${LIB_DRIVERS}i2c/ \
                     -I ${LIB_DRIVERS}pad_control/ \
                     -I ${LIB_DRIVERS}power_manager/ \
                     -I ${LIB_DRIVERS}rv_plic/ \
                     -I ${LIB_DRIVERS}rv_timer/ \
                     -I ${LIB_DRIVERS}soc_ctrl/ \
                     -I ${LIB_DRIVERS}spi_host/ \
                     -I ${LIB_DRIVERS}spi_memio/ \
                     -I ${LIB_DRIVERS}uart/ \
                     ")

# Include all those directories for compiling 
include_directories(${TARGET}.elf 
${RISCV}/riscv32-unknown-elf/include
${RISCV}/riscv32-unknown-elf/include/
${ROOT_PROJECT}
${ROOT_PROJECT}freertos/
${ROOT_PROJECT}applications/${PROJECT}/
${INC_FOLDERS}
${LIB_BASE}
${LIB_BASE_FREESTD}
${LIB_RUNTIME}
${LIB_DRIVERS}dma/
${LIB_DRIVERS}fast_intr_ctrl/
${LIB_DRIVERS}gpio/
${LIB_DRIVERS}i2c/
${LIB_DRIVERS}pad_control/
${LIB_DRIVERS}power_manager/
${LIB_DRIVERS}rv_plic/
${LIB_DRIVERS}rv_timer/
${LIB_DRIVERS}soc_ctrl/
${LIB_DRIVERS}spi_host/
${LIB_DRIVERS}spi_memio/
${LIB_DRIVERS}uart/
)

# Get all the files to include when linking
SET(LINKED_FILES    "${LIB_CRT_P}crt0.S \
                     ${LIB_CRT_P}vectors.S \
                     ${LIB_RUNTIME}handler.c \
                     ${LIB_RUNTIME}init.c \
                     ${LIB_RUNTIME}syscalls.c \
                     ${LIB_BASE}bitfield.c \
                     ${LIB_BASE}memory.c \
                     ${LIB_BASE}mmio.c \
                     ${LIB_DRIVERS}soc_ctrl/soc_ctrl.c \
                     ${LIB_DRIVERS}uart/uart.c \
                     ${LIB_DRIVERS}rv_plic/rv_plic.c \
                     ${LIB_DRIVERS}rv_timer/rv_timer.c \
                     ${LIB_DRIVERS}gpio/gpio.c \
                     ${LIB_DRIVERS}i2c/i2c.c \
                     ${LIB_DRIVERS}dma/dma.c \
                     ${LIB_DRIVERS}power_manager/power_manager.c \
                     ${LIB_DRIVERS}fast_intr_ctrl/fast_intr_ctrl.c  \
                     ${LIB_DRIVERS}spi_host/spi_host.c \
                     ${LIB_DRIVERS}pad_control/pad_control.c \
                     ")
					 
# fetch content from freertos kernel repository			 
FetchContent_Declare( freertos_kernel
  GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
  GIT_TAG        99d3d54ac4d17474a81c94ec5bab36f55f470359 #V10.5.1, last commit 16/12/2022
)

# set the freertos version
set(freertos_version "V10.5.1")
add_library(freertos_config  INTERFACE)

# set include dirtectories for freertos
target_include_directories(freertos_config SYSTEM
INTERFACE
    ${ROOT_PROJECT}freertos/
	${LIB_RUNTIME}
    ${INC_FOLDERS}
	${LIB_DRIVERS}rv_timer/
	${LIB_BASE}
)

# set main compilation options for freertos
target_compile_definitions(freertos_config
  INTERFACE
    projCOVERAGE_TEST=0
)
set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)
set(FREERTOS_PORT "GCC_RISC_V" CACHE STRING "" FORCE)

# fetching freertos content
if(${LINK_FILE} STREQUAL "link_freertos.ld")
  FetchContent_MakeAvailable(freertos_kernel)
endif()

# specify the C standard
set(CMAKE_C_FLAGS "\
  -march=${CMAKE_SYSTEM_PROCESSOR} \
  -w -Os -g  -nostdlib  \
  -DHOST_BUILD \
  -DportasmHANDLE_INTERRUPT=vSystemIrqHandler\
")

# In case of wanting to create a library with those subdirectories
#add_subdirectory(device/lib/base)
#add_subdirectory(device/lib/drivers)
#add_subdirectory(device/lib/runtime)

set(SOURCES ${ROOT_PROJECT}applications/${PROJECT}/${TARGET}.c)

# add the executable
add_executable(${TARGET}.elf ${SOURCES})  

#linking the libraries
#target_link_libraries(${TARGET}.elf base)
#target_link_libraries(${TARGET}.elf drivers)
#target_link_libraries(${TARGET}.elf runtime)
if(${LINK_FILE} STREQUAL "link_freertos.ld")
  target_link_libraries(${TARGET}.elf freertos_kernel)
endif()

# Setting-up the linker
SET(LINKER_SCRIPT "${LINK_FOLDER}/${LINK_FILE}")
message( "${Magenta}Linker file: ${LINKER_SCRIPT}${ColourReset}")

# Setting-up the properties, elf is 
set_target_properties(${TARGET}.elf PROPERTIES LINK_DEPENDS "${LINKER_SCRIPT}")

# Linker control
SET(CMAKE_EXE_LINKER_FLAGS  "-T ${LINKER_SCRIPT}  \
                            ${INCLUDE_FOLDERS} \
                             -static ${LINKED_FILES} \
                             -Wl,-Map=${TARGET}.map \
                             -L ${RISCV}/riscv32-unknown-elf/lib \
                             -lc -lm -lgcc -flto \
                            -ffunction-sections -fdata-sections -specs=nano.specs")
message( "${Magenta}Lib Folder RISCV-GCC: ${RISCV}/riscv32-unknown-elf/lib${ColourReset}")

SET(CMAKE_VERBOSE_MAKEFILE on)

# Post processing command to create a disassembly file 
add_custom_command(TARGET ${TARGET}.elf POST_BUILD
        COMMAND ${CMAKE_OBJDUMP} -S  ${TARGET}.elf > ${TARGET}.disasm
        COMMENT "Invoking: Disassemble")

# Post processing command to create a hex file 
add_custom_command(TARGET ${TARGET}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O verilog  ${TARGET}.elf  ${TARGET}.hex 
        COMMENT "Invoking: Hexdump")
add_custom_command(TARGET ${TARGET}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary  ${TARGET}.elf  ${TARGET}.bin
        COMMENT "Invoking: Hexdump")

# Pre-processing command to create disassembly for each source file
foreach (SRC_MODULE ${TARGET} )
  add_custom_command(TARGET ${TARGET}.elf 
                     PRE_LINK
                     COMMAND ${CMAKE_OBJDUMP} -S ${ROOT_PROJECT}build/CMakeFiles/${TARGET}.elf.dir/applications/${PROJECT}/${SRC_MODULE}.c.obj > ${SRC_MODULE}.s
                     COMMENT "Invoking: Disassemble ( CMakeFiles/${TARGET}.dir/${SRC_MODULE}.c.obj)")
endforeach()

SET(DCMAKE_EXPORT_COMPILE_COMMANDS ON)

#message( FATAL_ERROR "You can not do this at all, CMake will exit." )

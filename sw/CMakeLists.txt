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

cmake_minimum_required(VERSION 3.15)

# Include to fetch any vendorised repo (e.g. FreeRTOS)
include(FetchContent)

# Add some colors!
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

# set the required CMake standard
set(CMAKE_CXX_STANDARD 14)

# Set MAIN file
SET(TARGET "main")

# Get the correct path for the crt files and linker file
if (${LINKER} STREQUAL "on_chip")
  SET(LIB_CRT_P "${SOURCE_PATH}device/lib/crt/")
  SET(LINK_FILE "link.ld")
elseif(${LINKER} STREQUAL "flash_load")
  SET(LIB_CRT_P "${SOURCE_PATH}device/lib/crt_flash_load/")
  SET(LINK_FILE "link_flash_load.ld")
elseif(${LINKER} STREQUAL "flash_exec")
  SET(LIB_CRT_P	"${SOURCE_PATH}device/lib/crt_flash_exec/")
  SET(LINK_FILE "link_flash_exec.ld")
else()
  message( FATAL_ERROR "Linker specification is not correct" )
endif()
SET(LIB_VCTR_P	"${LIB_CRT_P}vectors.S")

# Just in case it is a freertos based app
if(${PROJECT} MATCHES "freertos")
  #SET(LIB_CRT_P	"${SOURCE_PATH}device/lib/crt_freertos/")
  SET(LIB_VCTR_P	"${SOURCE_PATH}device/lib/crt_freertos/vectors.S")
endif()

# messages to check the paths
message( "${Magenta}Current project: ${PROJECT}${ColourReset}")
message( "${Magenta}Root project: ${ROOT_PROJECT}${ColourReset}")
message( "${Magenta}Source path: ${SOURCE_PATH}${ColourReset}")
message( "${Magenta}LIB_CRT PATH for Cmake: ${LIB_CRT_P}${ColourReset}")
message( "${Magenta}LINKER File for Cmake: ${LINK_FILE}${ColourReset}")
message( "${Magenta}LIB_DRIVERS PATH for Cmake: ${LIB_DRIVERS}${ColourReset}")
message( "${Magenta}Targetting folder: ${INC_FOLDERS}${ColourReset}")

# Define MACRO to get ALL the *.h files under sw/device (string format)
MACRO(H_FILES_RECURSIVE return_list)
  FILE(GLOB_RECURSE new_list *.h)
  SET(dir_list "")
  FOREACH(file_path ${new_list})
  if(${file_path} MATCHES "device")
    if(${file_path} MATCHES "target")
      if(${file_path} MATCHES "${INC_FOLDERS}")
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} "-I ${dir_path} \
        ")
        string(REPLACE ";" "" dir_list ${dir_list})     
      endif()
    else()
      GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
      SET(dir_list ${dir_list} "-I ${dir_path} \
      ")
      string(REPLACE ";" "" dir_list ${dir_list})
    endif()
  elseif(${file_path} MATCHES "${PROJECT}")
    GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
    SET(dir_list ${dir_list} "-I ${dir_path} \
    ")
    string(REPLACE ";" "" dir_list ${dir_list}) 
  elseif(${file_path} MATCHES "${freertos}")
    if(${PROJECT} MATCHES "freertos")
      GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
      SET(dir_list ${dir_list} "-I ${dir_path} \
      ")
      string(REPLACE ";" "" dir_list ${dir_list}) 
    endif()
  endif()
  ENDFOREACH()
  LIST(REMOVE_DUPLICATES dir_list)
  SET(${return_list} ${dir_list})
ENDMACRO()

# Use the MACRO to get ALL the .h files
H_FILES_RECURSIVE(h_dir_list)

# Check how many you found!
list(LENGTH h_dir_list h_dir_list_count)
message(STATUS "[INFO] ${h_dir_list}")

# Get all the folders to include when linking
SET(INCLUDE_FOLDERS "-I ${RISCV}/riscv32-unknown-elf/include \
                     -I ${RISCV}/riscv32-unknown-elf/include/ \
                     -I ${ROOT_PROJECT} \
                     -I ${SOURCE_PATH} \
					           ${h_dir_list}")

# Define MACRO to get ALL the *.h files under sw/device (list format)
MACRO(H_FILES_RECURSIVE_LIST return_list)
  FILE(GLOB_RECURSE new_list *.h)
  SET(dir_list "")
  FOREACH(file_path ${new_list})
  if(${file_path} MATCHES "device")
    if(${file_path} MATCHES "target")
      if(${file_path} MATCHES "${INC_FOLDERS}")
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} "${dir_path}")    
      endif()
    else()
      GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
      SET(dir_list ${dir_list} "${dir_path}")
    endif()
  elseif(${file_path} MATCHES "${PROJECT}")
    GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
    SET(dir_list ${dir_list} "${dir_path}")
  elseif(${file_path} MATCHES "${freertos}")
    if(${PROJECT} MATCHES "freertos")
      GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
      SET(dir_list ${dir_list} "${dir_path}")
    endif()
  endif()
  ENDFOREACH()
  LIST(REMOVE_DUPLICATES dir_list)
  SET(${return_list} ${dir_list})
ENDMACRO()

# Include all those directories for compiling 
# Use the MACRO to get ALL the .h files
H_FILES_RECURSIVE_LIST(h_dir_list_)

# Define MACRO to get ALL the *.c files under sw/device 
MACRO(C_FILES_RECURSIVE return_list)
    FILE(GLOB_RECURSE new_list *.c)
    SET(dir_list "")
    FOREACH(file_c IN LISTS new_list)
      #message(${file_c})
      if(${file_c} MATCHES "device")
        SET(dir_list ${dir_list} "${file_c} \
        ")
        string(REPLACE ";" "" dir_list ${dir_list})
      elseif(${file_c} MATCHES "${PROJECT}")
        if(NOT ${file_c} MATCHES "main")
          SET(dir_list ${dir_list} "${file_c} \
          ")
          string(REPLACE ";" "" dir_list ${dir_list})          
        endif()
      endif()
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

# Use the MACRO to get ALL the .c files
C_FILES_RECURSIVE(c_dir_list)
# Check how many you found!
list(LENGTH c_dir_list c_dir_list_count)
#message(STATUS "[INFO] ${c_dir_list}")

# Get all the files to include when linking
SET(LINKED_FILES    "${LIB_CRT_P}crt0.S \
                     ${LIB_VCTR_P} \
                     ${c_dir_list}")

message( "${Magenta}Linked files: ${LINKED_FILES}${ColourReset}")
					 
# fetch content from freertos kernel repository			 
FetchContent_Declare( freertos_kernel
  GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
  GIT_TAG        99d3d54ac4d17474a81c94ec5bab36f55f470359 #V10.5.1, last commit 16/12/2022
)

# set the freertos version
set(freertos_version "V10.5.1")
add_library(freertos_config  INTERFACE)

# set include dirtectories for freertos
target_include_directories(freertos_config SYSTEM INTERFACE ${h_dir_list_})

# set main compilation options for freertos
target_compile_definitions(freertos_config
  INTERFACE
    projCOVERAGE_TEST=0
)
set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)
set(FREERTOS_PORT "GCC_RISC_V" CACHE STRING "" FORCE)

# fetching freertos content
if(${PROJECT} MATCHES "freertos")
  FetchContent_MakeAvailable(freertos_kernel)
endif()

# specify the C standard
set(COMPILER_LINKER_FLAGS "\
  -march=${CMAKE_SYSTEM_PROCESSOR} \
  -w -Os -g  -nostdlib  \
  -DHOST_BUILD \
  -DportasmHANDLE_INTERRUPT=vSystemIrqHandler\
")
set(CMAKE_C_FLAGS ${COMPILER_LINKER_FLAGS})

if (${COMPILER} MATCHES "clang")
     set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --target=riscv32 \
                                          -mabi=ilp32 \
                                          --gcc-toolchain=${RISCV} \
                                          --sysroot=${RISCV}/riscv32-unknown-elf \
                                          -static \
                                          -mcmodel=medany \
                                          -fvisibility=hidden \
                                          -nostartfiles \
                                          -Wall \
                                          -Wno-unused-command-line-argument" )
endif()

# In case of wanting to create a library with those subdirectories
#add_subdirectory(device/lib/base)
#add_subdirectory(device/lib/drivers)
#add_subdirectory(device/lib/runtime)

set(SOURCES ${SOURCE_PATH}applications/${PROJECT}/${TARGET}.c)

# add the executable
add_executable(${TARGET}.elf ${SOURCES})  

# add include directories to compilation
target_include_directories(${TARGET}.elf PUBLIC ${h_dir_list_})

# linking the libraries
#target_link_libraries(${TARGET}.elf base)
#target_link_libraries(${TARGET}.elf drivers)
#target_link_libraries(${TARGET}.elf runtime)
if(${PROJECT} MATCHES "freertos")
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

# To make sure that .obj files are created and fetched from the same path. 
# When setting an inner path to add_executable, .obj files are stored in ${TARGET}.elf.dir/<relative path>
# but when an outside directory is provided, they are stored in ${TARGET}.elf.dir/<absolute path>.
# To prevent this, if the SOURCE_PATH is the ROOT_PROJECT (X-HEEP source files will be used), no further path is to be added to the fetch directory.
if( ${SOURCE_PATH} STREQUAL ${ROOT_PROJECT} )
    SET(OBJ_PATH "")
else()
    SET(OBJ_PATH ${SOURCE_PATH})
endif()

# Specify that we want to link with GCC even if we are compiling with clang
if (${COMPILER} MATCHES "clang")
  set( CMAKE_C_LINK_EXECUTABLE "${CMAKE_LINKER} ${COMPILER_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS} \
                                ${SOURCE_PATH}build/CMakeFiles/${TARGET}.elf.dir/${OBJ_PATH}applications/${PROJECT}/${TARGET}.c.obj \
                                -o ${TARGET}.elf")
endif()

# Post processing command to create a disassembly file 
add_custom_command(TARGET ${TARGET}.elf POST_BUILD
        COMMAND ${CMAKE_OBJDUMP} -S  ${TARGET}.elf > ${TARGET}.disasm
        COMMENT "Invoking: Disassemble")

# Post processing command to create a hex file 
if((${LINKER} STREQUAL "flash_load") OR (${LINKER} STREQUAL "flash_exec"))
    add_custom_command(TARGET ${TARGET}.elf POST_BUILD
            COMMAND ${CMAKE_OBJCOPY} -O verilog --adjust-vma=-0x40000000 ${TARGET}.elf  ${TARGET}.hex 
            COMMENT "Invoking: Hexdump")
else()
    add_custom_command(TARGET ${TARGET}.elf POST_BUILD
            COMMAND ${CMAKE_OBJCOPY} -O verilog  ${TARGET}.elf  ${TARGET}.hex 
            COMMENT "Invoking: Hexdump")
endif()

add_custom_command(TARGET ${TARGET}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary  ${TARGET}.elf  ${TARGET}.bin
        COMMENT "Invoking: Hexdump")

# Pre-processing command to create disassembly for each source file
foreach (SRC_MODULE ${TARGET} )
  add_custom_command(TARGET ${TARGET}.elf 
                     PRE_LINK
                     COMMAND ${CMAKE_OBJDUMP} -S ${SOURCE_PATH}build/CMakeFiles/${TARGET}.elf.dir/${OBJ_PATH}applications/${PROJECT}/${SRC_MODULE}.c.obj > ${SRC_MODULE}.s
                     COMMENT "Invoking: Disassemble ( CMakeFiles/${TARGET}.dir/${SRC_MODULE}.c.obj)")
endforeach()

# Adding gdb command - TBD
#add_custom_target(gdb DEPENDS ${TARGET}.elf)
#add_custom_command(TARGET gdb
#    COMMAND ${CMAKE_C_GDB} ${TARGET}.elf -x gdbInit &)

SET(DCMAKE_EXPORT_COMPILE_COMMANDS ON)

#message( FATAL_ERROR "You can not do this at all, CMake will exit." )

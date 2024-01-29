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
SET(MAINFILE "main")


#######################################################################
#      FIND HEADER FILES TO BE INCLUDED
#######################################################################

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Preliminary list of header files inside the source path

# Make a list of the header files that need to be included
FILE(GLOB_RECURSE new_list FOLLOW_SYMLINKS ${SOURCE_PATH}*.h)
SET(dir_list_str "")
FOREACH(file_path ${new_list})
  SET(add 0) # This variable is set to 1 if the file_pth needs to be added to the list
  if(${file_path} MATCHES "/device/")
    if(${file_path} MATCHES "/target/") # Add it if its not in target, or if its in target/${TARGET}
      if(${file_path} MATCHES ${TARGET})
        SET(add 1)
      endif()
    else()
      SET(add 1)
    endif()
  elseif(${file_path} MATCHES ${PROJECT})
    SET(add 1)
  elseif( ( ${file_path} MATCHES "/freertos/" ) AND ( ${PROJECT} MATCHES "freertos" ) )
    SET(add 1)
  elseif( ${file_path} MATCHES "/external/" )
    SET(add 1)
  endif()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Prune list

  if( add EQUAL 1 ) # If the file path mathced one of the criterion, add it to the list
    # Get the path of the obtained directory
    GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)

    # Add it to the string format list
    SET(dir_list_str ${dir_list_str} "-I ${dir_path} \
    ")
    string(REPLACE ";" "" dir_list_str ${dir_list_str})

    # Add it to the header list
    SET(h_dir_list_ ${h_dir_list_} "${dir_path}")
  endif()

ENDFOREACH()

LIST(REMOVE_DUPLICATES dir_list_str)
LIST(REMOVE_DUPLICATES h_dir_list_)

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Add list to the directories to be included

# Get all the folders to include when linking
SET(INCLUDE_FOLDERS "-I ${RISCV}/${COMPILER_PREFIX}elf/include \
                     -I ${RISCV}/${COMPILER_PREFIX}elf/include/ \
                     -I ${ROOT_PROJECT} \
                     -I ${SOURCE_PATH} \
					           ${dir_list_str}")


#######################################################################
#      FIND SOURCE FILES TO BE INCLUDED
#######################################################################

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Preliminary list of source files inside the source path

# Make a list of the source files that need to be linked
FILE(GLOB_RECURSE new_list FOLLOW_SYMLINKS ${SOURCE_PATH}*.c)
SET( c_dir_list "" )
SET( app_found 0 )
FOREACH(file_path IN LISTS new_list)
  SET(add 0) # This variable is set to 1 if the file_pth needs to be added to the list
  if(${file_path} MATCHES "/device/")
    SET(add 1)
  elseif( ${file_path} MATCHES "/external/" )
    SET(add 1)
  elseif( ( ${file_path} MATCHES "/${PROJECT}/" ) AND ( NOT ${file_path} MATCHES ${MAINFILE} ) )
    SET(add 1)
    elseif( ( ${file_path} MATCHES "/${PROJECT}/" ) AND ( ${file_path} MATCHES ${MAINFILE} ) )
    SET(app_found 1)
  endif()

  if( add EQUAL 1 ) # If the file path mathced one of the criterion, add it to the list
    SET(c_dir_list ${c_dir_list} "${file_path}\
    ")
    string(REPLACE ";" "" c_dir_list ${c_dir_list})
  endif()

ENDFOREACH()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# If the app was not found, look into the root project

if( app_found EQUAL 0 )
  SET(SOURCE_PATH ${ROOT_PROJECT})

  # Make a list of the source files that need to be linked
  FILE(GLOB_RECURSE new_list FOLLOW_SYMLINKS ${SOURCE_PATH}*.c)
  SET(c_dir_list "")
  FOREACH(file_path IN LISTS new_list)
    SET(add 0) # This variable is set to 1 if the file_pth needs to be added to the list
    if(${file_path} MATCHES "/device/")
      SET(add 1)
    elseif( ( ${file_path} MATCHES "/${PROJECT}/" ) AND ( NOT ${file_path} MATCHES ${MAINFILE} ) )
      SET(add 1)
    endif()

    if( add EQUAL 1 ) # If the file path mathced one of the criterion, add it to the list
      SET(c_dir_list ${c_dir_list} "${file_path}\
      ")
      string(REPLACE ";" "" c_dir_list ${c_dir_list})
    endif()

    ENDFOREACH()
endif()

LIST(REMOVE_DUPLICATES c_dir_list)

#######################################################################
#      DETERMINE IF APP IS EXTERNAL
#######################################################################

# Determine whether the target app is located in the root project or in an external folder
if( ${SOURCE_PATH} STREQUAL ${ROOT_PROJECT} )
  SET( internal_app 1 )
  message( "${Magenta}Internal application${ColourReset}")
else()
  SET( internal_app 0 )
  message( "${Magenta}External application${ColourReset}")
endif()

#######################################################################
#      FIND CRT FILES TO BE INCLUDED
#######################################################################

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Set the default CRT directories

SET(LIB_CRT_P	  "${ROOT_PROJECT}device/lib/crt/crt0.S")
SET(LIB_VCTR_P  "${ROOT_PROJECT}device/lib/crt/vectors.S")

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Look for external CRT files

SET(CRTO "INTERNAL_CRTO")
SET( LIB_CRT_EXT_P  "" )
if( internal_app EQUAL 0 )
 FILE(GLOB_RECURSE new_list FOLLOW_SYMLINKS "${SOURCE_PATH}/external/lib/crt/*.S")
 FOREACH(file_path IN LISTS new_list)
   if(${file_path} MATCHES "external_crt0.S")
      SET( CRTO "EXTERNAL_CRTO" )
      SET( LIB_CRT_EXT_P  "${file_path}" )
      GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
      SET(INCLUDE_FOLDERS "${INCLUDE_FOLDERS} -I ${dir_path}")
      SET(h_dir_list_ ${h_dir_list_} "${dir_path}")
   endif()
 ENDFOREACH()
endif()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# In case of a freertos based app

if(${PROJECT} MATCHES "freertos")
  #SET(LIB_CRT_P	"${SOURCE_PATH}device/lib/crt_freertos/")
  SET(LIB_VCTR_P	"${SOURCE_PATH}device/lib/crt/vectors_freertos.S")
endif()


#######################################################################
#      SET CRT AND LINKER TYPE
#######################################################################

# Get the correct path for the crt files and linker file
if (${LINKER} STREQUAL "on_chip")
  #SET(LIB_CRT_P "${SOURCE_PATH}device/lib/crt/")
  SET(CRT_TYPE "ON_CHIP")
  SET(LINK_FILE "link.ld")
elseif(${LINKER} STREQUAL "flash_load")
  #SET(LIB_CRT_P "${SOURCE_PATH}device/lib/crt_flash_load/")
  SET(CRT_TYPE "FLASH_LOAD")
  SET(LINK_FILE "link_flash_load.ld")
elseif(${LINKER} STREQUAL "flash_exec")
  #SET(LIB_CRT_P	"${SOURCE_PATH}device/lib/crt_flash_exec/")
  SET(CRT_TYPE "FLASH_EXEC")
  SET(LINK_FILE "link_flash_exec.ld")
else()
  message( FATAL_ERROR "Linker specification is not correct" )
endif()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Debug messages to check the paths

message(STATUS "[INFO] ${INCLUDE_FOLDERS}")
message( "${Magenta}Current project: ${PROJECT}${ColourReset}")
message( "${Magenta}Root project: ${ROOT_PROJECT}${ColourReset}")
message( "${Magenta}Source path: ${SOURCE_PATH}${ColourReset}")
message( "${Magenta}LIB_CRT PATH for Cmake: ${LIB_CRT_P}${ColourReset}")
message( "${Magenta}LINKER File for Cmake: ${LINK_FILE}${ColourReset}")
message( "${Magenta}LIB_DRIVERS PATH for Cmake: ${LIB_DRIVERS}${ColourReset}")
message( "${Magenta}Targetting folder: ${INC_FOLDERS}${ColourReset}")
message( "${Magenta}Target: ${TARGET}${ColourReset}")


#######################################################################
#      SET LINKER PROPERTIES
#######################################################################

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Set linked files

# Get all the files to include when linking
SET(LINKED_FILES    "${LIB_CRT_P} \
                     ${LIB_VCTR_P} \
                     ${LIB_CRT_EXT_P} \
                     ${c_dir_list}")

message( "${Magenta}Linked files: ${LINKED_FILES}${ColourReset}")

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Free-RTOS configurations

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

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Set CMAKE flags

# specify the C standard
set(COMPILER_LINKER_FLAGS "\
  -march=${CMAKE_SYSTEM_PROCESSOR} \
  -w -Os -g  -nostdlib  \
  -ffunction-sections \
  -DHOST_BUILD \
  -D${CRT_TYPE} \
  -D${CRTO} \
  -DportasmHANDLE_INTERRUPT=vSystemIrqHandler\
")
set(CMAKE_C_FLAGS ${COMPILER_LINKER_FLAGS})

if (${COMPILER} MATCHES "clang")
     set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --target=riscv32 \
                                          -mabi=ilp32 \
                                          --gcc-toolchain=${RISCV} \
                                          --sysroot=${RISCV}/${COMPILER_PREFIX}elf \
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

#######################################################################
#      SET TARGETS
#######################################################################

set(SOURCES ${SOURCE_PATH}applications/${PROJECT}/${MAINFILE}.c)

# add the executable
add_executable(${MAINFILE}.elf ${SOURCES})

# add include directories to compilation
target_include_directories(${MAINFILE}.elf PUBLIC ${h_dir_list_})

# linking the libraries
#target_link_libraries(${MAINFILE}.elf base)
#target_link_libraries(${MAINFILE}.elf drivers)
#target_link_libraries(${MAINFILE}.elf runtime)
if(${PROJECT} MATCHES "freertos")
  target_link_libraries(${MAINFILE}.elf freertos_kernel)
endif()

# Setting-up the linker
SET(LINKER_SCRIPT "${LINK_FOLDER}/${LINK_FILE}")
message( "${Magenta}Linker file: ${LINKER_SCRIPT}${ColourReset}")

# Setting-up the properties, elf is
set_target_properties(${MAINFILE}.elf PROPERTIES LINK_DEPENDS "${LINKER_SCRIPT}")

# Linker control
SET(CMAKE_EXE_LINKER_FLAGS  "-T ${LINKER_SCRIPT}  \
                            ${INCLUDE_FOLDERS} \
                             -static ${LINKED_FILES} \
                             -Wl,-Map=${MAINFILE}.map \
                             -L ${RISCV}/${COMPILER_PREFIX}elf/lib \
                             -lc -lm -lgcc -flto \
                            -ffunction-sections -fdata-sections -specs=nano.specs")
message( "${Magenta}Lib Folder RISCV-GCC: ${RISCV}/${COMPILER_PREFIX}elf/lib${ColourReset}")

SET(CMAKE_VERBOSE_MAKEFILE on)

# To make sure that .obj files are created and fetched from the same path.
# When setting an inner path to add_executable, .obj files are stored in ${MAINFILE}.elf.dir/<relative path>
# but when an outside directory is provided, they are stored in ${MAINFILE}.elf.dir/<absolute path>.
# To prevent this, if the SOURCE_PATH is the ROOT_PROJECT (X-HEEP source files will be used), no further path is to be added to the fetch directory.
if( ${SOURCE_PATH} STREQUAL ${ROOT_PROJECT} )
    SET(OBJ_PATH "")
else()
    SET(OBJ_PATH ${SOURCE_PATH})
endif()

# Specify that we want to link with GCC even if we are compiling with clang
if (${COMPILER} MATCHES "clang")
	if( ${PROJECT} MATCHES "freertos" )
		set( CMAKE_C_LINK_EXECUTABLE "${CMAKE_LINKER} ${COMPILER_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS} \
                                ${SOURCE_PATH}build/CMakeFiles/${MAINFILE}.elf.dir/${OBJ_PATH}applications/${PROJECT}/${MAINFILE}.c.obj \
                                -o ${MAINFILE}.elf \
								_deps/freertos_kernel-build/libfreertos_kernel.a \ _deps/freertos_kernel-build/portable/libfreertos_kernel_port.a \ _deps/freertos_kernel-build/libfreertos_kernel.a \ _deps/freertos_kernel-build/portable/libfreertos_kernel_port.a \
								")
	else()
  set( CMAKE_C_LINK_EXECUTABLE "${CMAKE_LINKER} ${COMPILER_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS} \
                                ${SOURCE_PATH}build/CMakeFiles/${MAINFILE}.elf.dir/${OBJ_PATH}applications/${PROJECT}/${MAINFILE}.c.obj \
                                -o ${MAINFILE}.elf")
    endif()
endif()

#######################################################################
#      SET CUSTOM COMMANDS
#######################################################################

# Post processing command to create a disassembly file
add_custom_command(TARGET ${MAINFILE}.elf POST_BUILD
        COMMAND ${CMAKE_OBJDUMP} -S  ${MAINFILE}.elf > ${MAINFILE}.S
        COMMENT "Invoking: Disassemble")

# Post processing command to create a hex file
if((${LINKER} STREQUAL "flash_load") OR (${LINKER} STREQUAL "flash_exec"))
    add_custom_command(TARGET ${MAINFILE}.elf POST_BUILD
            COMMAND ${CMAKE_OBJCOPY} -O verilog --adjust-vma=-0x40000000 ${MAINFILE}.elf  ${MAINFILE}.hex
            COMMENT "Invoking: Hexdump")
else()
    add_custom_command(TARGET ${MAINFILE}.elf POST_BUILD
            COMMAND ${CMAKE_OBJCOPY} -O verilog  ${MAINFILE}.elf  ${MAINFILE}.hex
            COMMENT "Invoking: Hexdump")
endif()

add_custom_command(TARGET ${MAINFILE}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary  ${MAINFILE}.elf  ${MAINFILE}.bin
        COMMENT "Invoking: Hexdump")

# Pre-processing command to create disassembly for each source file
foreach (SRC_MODULE ${MAINFILE} )
  add_custom_command(TARGET ${MAINFILE}.elf
                     PRE_LINK
                     COMMAND ${CMAKE_OBJDUMP} -S ${ROOT_PROJECT}build/CMakeFiles/${MAINFILE}.elf.dir/${OBJ_PATH}applications/${PROJECT}/${SRC_MODULE}.c.obj > ${SRC_MODULE}.s
                     COMMENT "Invoking: Disassemble ( CMakeFiles/${MAINFILE}.dir/${SRC_MODULE}.c.obj)")
endforeach()

# Adding gdb command - TBD
#add_custom_target(gdb DEPENDS ${MAINFILE}.elf)
#add_custom_command(TARGET gdb
#    COMMAND ${CMAKE_C_GDB} ${MAINFILE}.elf -x gdbInit &)

SET(DCMAKE_EXPORT_COMPILE_COMMANDS ON)

#message( FATAL_ERROR "You can not do this at all, CMake will exit." )
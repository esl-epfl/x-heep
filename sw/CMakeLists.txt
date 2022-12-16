
cmake_minimum_required(VERSION 3.10)

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
project(${PROJECT})

set(CMAKE_CXX_STANDARD 14)

# messages to check the paths
message( "${Magenta}Current project: ${PROJECT}${ColourReset}")
message( "${Magenta}LIB_CRT PATH for Cmake: ${LIB_CRT}${ColourReset}")
message( "${Magenta}LIB_DRIVERS PATH for Cmake: ${LIB_DRIVERS}${ColourReset}")
message( "${Magenta}Targetting main file: ${MAINFILE}${ColourReset}")
message( "${Magenta}Targetting folder: ${INC_FOLDERS}${ColourReset}")

#set MAIN file
SET(TARGET ${MAINFILE})

SET(INCLUDE_FOLDERS "-I ${RISCV}/riscv32-unknown-elf/include \
                     -I ${RISCV}/riscv32-unknown-elf/include/ \
                     -I ${ROOT_PROJECT} \
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

SET(LINKED_FILES    "${ROOT_PROJECT}applications/${PROJECT}/test_cpp.cpp \
                     ${LIB_CRT}crt0.S \
                     ${LIB_CRT}vectors.S \
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

# specify the C standard
set(CMAKE_C_FLAGS "\
  -march=${CMAKE_SYSTEM_PROCESSOR} \
  -w -Os -g  -nostdlib  \
  -DHOST_BUILD \
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

# Setting-up the linker
SET(LINKER_SCRIPT "${LINK_FOLDER}/link.ld")
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

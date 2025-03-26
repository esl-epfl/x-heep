# The Generic system name is used for embedded targets (targets without OS) in
# CMake
set( CMAKE_SYSTEM_NAME          Generic )
set( CMAKE_SYSTEM_PROCESSOR     $ENV{ARCH} 
     CACHE STRING "Generate code for given RISC-V ISA string")
set( CMAKE_EXECUTABLE_SUFFIX    ".elf")

set(RISCV_GCC_COMPILER  /home/tools/riscv-corev/bin/riscv32-corev-elf-gcc)
set(RISCV_CLANG_COMPILER  $ENV{RISCV}/bin/clang)

STRING(REGEX REPLACE "\-gcc$" "-" GCC_CROSS_COMPILE ${RISCV_GCC_COMPILER})
STRING(REGEX REPLACE "clang" "" CLANG_CROSS_COMPILE ${RISCV_CLANG_COMPILER})

# specify the cross compiler. We force the compiler so that CMake doesn't
# attempt to build a simple test program as this will fail without us using
# the -nostartfiles option on the command line
#CMAKE_FORCE_C_COMPILER( "${RISCV_TOOLCHAIN_BIN_PATH}/${CROSS_COMPILE}gcc${RISCV_TOOLCHAIN_BIN_EXT}" GNU )
#CMAKE_FORCE_CXX_COMPILER( "${RISCV_TOOLCHAIN_BIN_PATH}/${CROSS_COMPILE}g++${RISCV_TOOLCHAIN_BIN_EXT}" GNU )
if ($ENV{COMPILER} MATCHES "gcc")
     set(CMAKE_C_COMPILER ${GCC_CROSS_COMPILE}gcc)
     set(CMAKE_CXX_COMPILER ${GCC_CROSS_COMPILE}g++)
     set(CMAKE_OBJDUMP ${GCC_CROSS_COMPILE}objdump
          CACHE FILEPATH "The toolchain objdump command " FORCE )
      set(CMAKE_OBJCOPY ${GCC_CROSS_COMPILE}objcopy
          CACHE FILEPATH "The toolchain objcopy command " FORCE )
     set( CMAKE_LINKER ${RISCV_GCC_COMPILER})
elseif ($ENV{COMPILER} MATCHES "clang")

set(CMAKE_OBJDUMP ${CLANG_CROSS_COMPILE}llvm-objdump
     CACHE FILEPATH "The toolchain objdump command " FORCE )
 set(CMAKE_OBJCOPY ${CLANG_CROSS_COMPILE}objcopy
     CACHE FILEPATH "The toolchain objcopy command " FORCE )
set( CMAKE_LINKER ${RISCV_CLANG_COMPILER})
set(CMAKE_C_COMPILER ${CLANG_CROSS_COMPILE}clang)
     set(CMAKE_CXX_COMPILER ${CLANG_CROSS_COMPILE}clang++)
endif()



# We must set the OBJCOPY setting into cache so that it's available to the
# whole project. Otherwise, this does not get set into the CACHE and therefore
# the build doesn't know what the OBJCOPY filepath is
# Even if we compile with clang, this must be GCC's objcopy since we always link with GCC

# Set the common build flags

# Set the CMAKE C flags (which should also be used by the assembler!
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=${CMAKE_SYSTEM_PROCESSOR}" )
if ($ENV{COMPILER} MATCHES "clang")
     set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-command-line-argument --target=riscv32 " )
     # set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-command-line-argument --target=riscv32 --gcc-toolchain=$ENV{RISCV} --sysroot=/home/tools/riscv-corev/riscv32-corev-elf" )
endif()

set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)

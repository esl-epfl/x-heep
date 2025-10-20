#!/bin/bash

# Activate the conda environment
source /opt/conda/etc/profile.d/conda.sh
conda activate core-v-mini-mcu

# Aliases to initialize SW build environment
alias init_gcc='export RISCV=/tools/riscv; export COMPILER=gcc; export COMPILER_PREFIX=riscv32-unknown-; export ARCH=rv32imc_zicsr'
alias init_clang='export RISCV=/tools/riscv-clang; export COMPILER=clang; export COMPILER_PREFIX=riscv32-unknown-; export ARCH=rv32imc'
alias init_corev='export RISCV=/tools/riscv-corev; export COMPILER=gcc; export COMPILER_PREFIX=riscv32-corev-; export ARCH=rv32imc_zicsr_xcvhwlp_xcvmem_xcvmac_xcvbi_xcvalu_xcvsimd_xcvbitmanip'

# Default to GCC toolchain
init_gcc

# Update PATH to include installed tools
export PATH=$TOOL_PATH:$PATH

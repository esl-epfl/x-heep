#!/bin/zsh

conda activate core-v-mini-mcu
export RISCV=/home/linus/tools/riscv
export VERILATOR_VERSION=4.210
export PATH=/home/$USER/tools/verilator/$VERILATOR_VERSION/bin:$PATH
make app PROJECT=l_cnn
make run-helloworld
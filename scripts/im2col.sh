#!/bin/bash

cd ../
if [ $# -lt 1]; then
    echo "Not enough arguments"
    exit 1
fi

if [ "$1" = "-no_verilator" ]; then
    make verilator-sim
fi

make app PROJECT=example_im2col
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator
./Vtestharness +firmware=../../../sw/build/main.hex
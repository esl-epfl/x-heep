#!/bin/bash

MIN_ARGS=1

cd ../

conda activate core-v-mini-mcu

if [ "$#" -lt "$MIN_ARGS" ]; then
    echo "$#"
    make verilator-sim
fi

if [ "$#" -eq "$MIN_ARGS" ] && [ "$1" != "-no_ver" ]; then
    echo "|im2col.sh| Error: unrecognized argument"
    echo "Possible arguments:"
    echo "-no_ver"
    exit 1
fi

make app PROJECT=example_im2col
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator
./Vtestharness +firmware=../../../sw/build/main.hex
cd ../../../scripts
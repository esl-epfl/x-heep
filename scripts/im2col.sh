#!/bin/bash

MIN_ARGS=1

cd ../

conda activate core-v-mini-mcu

STOP=0

while [ $# -gt 0 ]; do
    case "$1" in
        -make_mcu) # If the current argument is -make_gen
        echo "Generating MCU..." # Print that the MCU is being generated
        if [ $# -ne 5 ]; then # If there are not exactly 5 arguments after -make_gen
            echo "Error: -make_gen requires 4 arguments" # Print an error
            STOP=1 # Set the stop flag to 1
        fi
        shift # Shift past the -make_gen argument
        CPU="$1"
        echo "CPU: $CPU"
        case "$CPU" in
            "cv32e20"|"cv32e40p"|"cv32e40x"|"cv32e4px")
            ;;
        *)
                echo "CPU $CPU is not supported."
                STOP=1 # Set the stop flag to 1
                ;;
        esac
        shift # Shift past the first argument
        BUS="$1"
        echo "BUS: $BUS"
        case "$BUS" in
        "onetoM"|"NtoM")
            ;;
        *)  
            echo "BUS $BUS is not supported."
            STOP=1 # Set the stop flag to 1
            ;;
        esac
        shift # Shift past the second argument
        MEM="$1"
        echo "Memory: $MEM"
        shift # Shift past the third argument
        MEM_IL="$1"
        echo "Memory IL: $MEM_IL"
        shift # Shift past the fourth argument
        
        if [ $STOP -eq 0 ]; then # If the stop flag is 0
            make mcu-gen CPU=$CPU BUS=$BUS MEMORY_BANKS=$MEM MEMORY_BANKS_IL=$MEM_IL # Generate the MCU
            make verilator-sim
            break
        fi
        
        ;;
    esac
    shift # Shift past the current argument to move to the next
done

for arg in "$@"
do
    if [ "$arg" = "-make_ver" ]; then
        make verilator-sim
        break
    fi
done

if [ $STOP -eq 0 ]; then 
    make app PROJECT=example_im2col
    cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator
    ./Vtestharness +firmware=../../../sw/build/main.hex
    cd ../../../scripts
else
    cd ./scripts
fi

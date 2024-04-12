#!/bin/bash

MIN_ARGS=1
SIM=0
FPGA=0
STOP=0

cd ../

conda activate core-v-mini-mcu


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
        echo "Memory: $MEM" # 16 max for the pynq-z2
        shift # Shift past the third argument
        MEM_IL="$1"
        echo "Memory IL: $MEM_IL"
        shift # Shift past the fourth argument
        
        if [ $STOP -eq 0 ]; then # If the stop flag is 0
            make mcu-gen CPU=$CPU BUS=$BUS MEMORY_BANKS=$MEM MEMORY_BANKS_IL=$MEM_IL # Generate the MCU
            break
        fi
        ;;

        -make_ver)
        make verilator-sim
        SIM=1
        ;;

        -make_fpga)
        make vivado-fpga FPGA_BOARD=pynq-z2
        make vivado-fpga-pgm FPGA_BOARD=pynq-z2
        FPGA=1
        ;;

        -make_fpga_sw)
        FPGA=1
        ;;
        *)
        echo "Error: Unknown argument $1" # Print an error message
        STOP=1 # Set the stop flag to 1
        ;;
    esac
    shift # Shift past the current argument to move to the next
done

if [ $STOP -eq 0 ]; then 
    # Compile the sw
    if [ $FPGA -eq 1 ]; then
        echo "Compiling the software for the FPGA..."
        make app PROJECT=example_im2col TARGET=pynq-z2 LINKER=flash_load
        make flash-prog
        dmesg --time-format iso | grep FTDI
        echo "Please try one of these USB ports for the PYNQ-Z2 terminal."
        echo "Execute: picocom -b 9600 -r -l --imap lfcrlf /dev/ttyUSB*"
    
    elif [ $SIM -eq 1 ]; then    
        make app PROJECT=example_im2col
        cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator
        FLAG=0

        # Run the FW and capture the output to start the correct UART    
        ./Vtestharness +firmware=../../../sw/build/main.hex | while IFS= read -r line; do
        echo "$line" # Process each line, can also tee to /dev/tty for console output
        # Check for /dev/pts pattern and act upon it
        if [[ $line =~ /dev/pts/([0-9]+) ]]; then
            if [ $FLAG -eq 1 ]; then
                pts_value="${BASH_REMATCH[1]}"
                echo "Extracted PTS value: $pts_value"
                gnome-terminal -- /bin/sh -c "cd '$PWD'; screen /dev/pts/"$pts_value"; exit; exec bash"
            fi
        FLAG=1
    fi
    done
    cd ../../../scripts
    fi

else
    cd /scripts
fi

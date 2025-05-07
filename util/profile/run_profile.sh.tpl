#!/bin/bash
# Script that runs the rv_profile tool to generate a flamegraph after RTL
# simulation.
# Args:
#  <rv_profile> : Path to the rv_profile tool
RV_PROFILE=$1

# Check if the rv_profile tool is provided
if [ -z "$RV_PROFILE" ]; then
    echo "Usage: $0 <rv_profile>"
    exit 1
fi

# Get the upper root directory
ROOT_DIR=$(git rev-parse --show-toplevel)

# Get the waveform file
WAVE_FILE=$(find $ROOT_DIR/build -name "*.vcd")

# Copy WAVE_FILE in a new file with .fst extension
cp $WAVE_FILE $WAVE_FILE.fst

WAVE_FILE=$WAVE_FILE.fst

# Profile report directory
PROFILE_REPORT_DIR=$ROOT_DIR/util/profile

# Config file
PROFILE_CONFIG_FILE=$PROFILE_REPORT_DIR/configs/${cpu_type}.wal

# Run the profiler
$RV_PROFILE --elf $ROOT_DIR/sw/build/main.elf \
            --fst $WAVE_FILE \
            --cfg $PROFILE_CONFIG_FILE \
            --out $PROFILE_REPORT_DIR/flamegraph.svg
#! /usr/bin/bash
echo ========================================
echo ========================================
echo Applications will be tested by trying to
echo build each one of them. 
echo ========================================
echo ========================================

# Create the virtual environment and install the requirements. 
conda init bash
source /root/.bashrc
conda activate core-v-mini-mcu

echo ========================================
echo ========================================
echo Some previous configurations...
echo ========================================
echo ========================================

# The variable could also be obtained from the container.
export RISCV='/tools/riscv' &&\

# All peripherals are included to make sure all apps can be built.
sed 's/is_included: "no",/is_included: "yes",/' -i mcu_cfg.hjson
# The MCU is generated with various memory banks to avoid example code not fitting. 	
make mcu-gen X_HEEP_CFG=configs/ci.hjson

echo ========================================
echo ========================================
echo Starting building script
echo ========================================
echo ========================================

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
chmod u+x $SCRIPT_DIR/build-apps.sh
$SCRIPT_DIR/build-apps.sh


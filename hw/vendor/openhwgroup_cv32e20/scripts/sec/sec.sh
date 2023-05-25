#!/bin/bash

# Copyright 2023 OpenHW Group
#
# Licensed under the Solderpad Hardware Licence, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://solderpad.org/licenses/
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

usage() {                                 # Function: Print a help message.
  echo "Usage: $0 [ -t {cadence,synopsys,mentor} ]" 1>&2
}
exit_abnormal() {                         # Function: Exit with error.
  usage
  exit 1
}

while getopts "t:" flag
do
    case "${flag}" in
        t)
        target_tool=${OPTARG}
        ;;
        :)
        exit_abnormal
        ;;
        *)
        exit_abnormal
        ;;
        ?)
        exit_abnormal
        ;;
    esac
done

if [ ! -d ./reports/ ]; then
    mkdir -p ./reports/
fi

if [[ "${target_tool}" != "cadence" && "${target_tool}" != "synopsys" && "${target_tool}" != "mentor" ]]; then
    exit_abnormal
fi

if [[ -z "${GOLDEN_RTL}" ]]; then
    echo "The env variable GOLDEN_RTL is empty."
    if [ ! -d "./ref_design" ]; then
        echo "Cloning Golden Design...."
        git clone https://github.com/openhwgroup/cve2.git ref_design
    fi
    export GOLDEN_RTL=$(pwd)/ref_design/rtl
else
    echo "SEC: Using ${GOLDEN_RTL} as reference design"
fi

REVISED_DIR=$( readlink -f $(pwd)/../../)

GOLDEN_DIR=$(readlink -f ./ref_design/)


var_golden_rtl=$(awk '{ if ($0 ~ "{DESIGN_RTL_DIR}" && $0 !~ "#" && $0 !~ "tracer" && $0 !~ "wrapper") print $0 }' ${GOLDEN_DIR}/cv32e20_manifest.flist | sed 's|${DESIGN_RTL_DIR}|./ref_design/rtl/|')

var_revised_rtl=$(awk '{ if ($0 ~ "{DESIGN_RTL_DIR}" && $0 !~ "#" && $0 !~ "tracer" && $0 !~ "wrapper") print $0 }' ${REVISED_DIR}/cv32e20_manifest.flist | sed 's|${DESIGN_RTL_DIR}|../../rtl/|')

echo $var_golden_rtl > golden.src
echo $var_revised_rtl > revised.src

report_dir=$(readlink -f $(dirname "${BASH_SOURCE[0]}"))/reports/$(date +%Y-%m-%d/%H-%M)/

if [[ -d ${report_dir} ]]; then
    rm -rf ${report_dir}
fi
mkdir -p ${report_dir}

if [[ "${target_tool}" == "cadence" ]]; then
    tcl_script=$(readlink -f $(dirname "${BASH_SOURCE[0]}"))/cadence/sec.tcl
    jg -sec -proj ${report_dir} -batch -tcl ${tcl_script} -define report_dir ${report_dir} &> ${report_dir}/output.candence.log

    if [ ! -f ${report_dir}/summary.cadence.log ]; then
        echo "Something went wrong during the process"
        exit 1
    fi
    grep -Eq "Overall SEC status[ ]+- Complete" ${report_dir}/summary.cadence.log
    RESULT=$?

elif [[ "${target_tool}" == "synopsys" ]]; then
    echo "Synopsys tool is not implemented yet"
    exit 1

elif [[ "${target_tool}" == "mentor" ]]; then
    echo "Mentor tool is not implemented yet"
    exit 1
fi

if [[ $RESULT == 0 ]]; then
    echo "SEC: The DESIGN IS SEQUENTIAL EQUIVALENT"
    exit 0
else
    echo "SEC: The DESIGN IS NOT SEQUENTIAL EQUIVALENT"
    exit 1
fi


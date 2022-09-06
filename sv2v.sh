#!/bin/bash

# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

# This script converts all SystemVerilog RTL files to Verilog
# using sv2v and then runs LEC (Cadence Conformal) to check if
# the generated Verilog is logically equivalent to the original
# SystemVerilog.  A similar script is used in OpenTitan, any updates
# or fixes here may need to be reflected in the OpenTitan script as well
# https://github.com/lowRISC/opentitan/blob/master/util/syn_yosys.sh
#
# The following tools are required:
#  - sv2v: SystemVerilog-to-Verilog converter from github.com/zachjs/sv2v
#  - yosys
#
# Usage:
#   ./sv2v.sh |& tee sv2v.log

#-------------------------------------------------------------------------
# use fusesoc to generate files and file list
#-------------------------------------------------------------------------

git apply sv2v_openhwgroup_cv32e40p.patch
git apply sv2v_pulp_platform_common_cells.patch

fusesoc --cores-root . run --no-export --target=asic_synthesis --tool=icarus --flag=use_sky130 \
        --setup  openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildicarus.log


cd build/openhwgroup.org_systems_core-v-mini-mcu_0/asic_synthesis-icarus

cp openhwgroup.org_systems_core-v-mini-mcu_0.scr sv2v.scr

# add the prim_assert.sv file as it is marked as include file, but sv2v needs to convert it
echo ../../../hw/vendor/lowrisc_opentitan/hw/ip/prim/rtl/prim_assert.sv >> sv2v.scr

# copy file list and remove incdir, RVFI define, and sim-file
egrep -v 'incdir|parameter|define|_pkg' sv2v.scr > flist_gold

egrep 'pkg' sv2v.scr | egrep -v 'define'  > flist_pkg_gold

egrep 'incdir' sv2v.scr > flist_incdir_gold
sed -i 's/\+incdir+//g' flist_incdir_gold
sed -i ':a;N;$!ba;s/\n/ -I /g' flist_incdir_gold
sed -i '1s/^/-I /' flist_incdir_gold


sv_files=$(cat flist_gold)
packages=$(cat flist_pkg_gold)
incdirs=$(cat flist_incdir_gold)

#-------------------------------------------------------------------------
# convert all RTL files to Verilog using sv2v
#-------------------------------------------------------------------------

printf "\nSV2V VERSION:\n"
sv2v --version

mkdir verilog

printf "\nSV2V ERRORS:\n"

#-------------------------------------------------------------------------
# we put everything in one file otherwise if we loop over each file,
# the module name and the instance name are not consistent due to parameters
#-------------------------------------------------------------------------
sv2v --define=SYNTHESIS --define=SV2V $packages $incdirs $sv_files +RTS -N4 > combined.v

# split files up
modules=`cat combined.v | grep "^module" | sed -e "s/^module //" | sed -e "s/ (.*//"`
echo "$modules" > modules.txt  # for debugging

for module in $modules; do
  sed -n "/^module $module /,/^endmodule/p" < combined.v > verilog/$module.v
done

#rm combined.v

# remove empty files (they do not have a module)
cd verilog
find . -type f -print0 | xargs --null grep -Z -L 'module' > ../empty_files.log
find . -type f -print0 | xargs --null grep -Z -L 'module' | xargs --null rm

cd ../



[ -e yosys_read_verilog.tcl ] && rm yosys_read_verilog.tcl
for i in $(ls verilog)
do
  echo 'read -sv ./verilog/'$i >> yosys_read_verilog.tcl
done


#-------------------------------------------------------------------------
# run yosys
#-------------------------------------------------------------------------
printf "\n\nYosys:\n"
yosys -L yosys_log.log -QTqp "
  script yosys_read_verilog.tcl;
  read_liberty -lib ../../../hw/asic/sky130/sky130_sram_4kbyte_1rw_32x1024_8_TT_1p8V_25C.lib;
  hierarchy -check -top core_v_mini_mcu;
  proc; opt;
  techmap; opt;
  write_verilog yosys_elaborated.v;
  write_json out.json;
"

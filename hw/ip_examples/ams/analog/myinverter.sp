** Copyright EPFL contributors.
** Licensed under the Apache License, Version 2.0, see LICENSE for details.
** SPDX-License-Identifier: Apache-2.0

** Uses the 65nm_bulk PTM Bulk CMOS model, February 22, 2006 release
** (obtained from https://ptm.asu.edu)

v_vdd VDD 0 1.8
v_gnd VSS 0 0
.global VDD VSS

*Model Description
.param temp=27

*Include model file
.lib "/scrap/users/schiavon/gitdir/skywater-pdk/libraries/sky130_fd_pr/latest/models/sky130.lib.spice" tt

*d g s b
.SUBCKT myinverter a z VDD VSS
X1 z a VDD VDD sky130_fd_pr__pfet_01v8 w =0.84 l = 0.15
X2 z a VSS VSS sky130_fd_pr__nfet_01v8 w =0.36 l = 0.15
.ENDS myinverter

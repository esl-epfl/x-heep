# Copyright 2022 OpenHW Group
# Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
# SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1


set SET_LIBS ${SCRIPT_DIR}/set_libs.tcl
set CONSTRAINTS ${SCRIPT_DIR}/set_constraints.tcl

source ${SET_LIBS}

source ${READ_SOURCES}.tcl

source ${CONSTRAINTS}

elaborate ${TOP_MODULE}

link

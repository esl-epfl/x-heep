// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

/* verilator lint_off UNUSED */

`ifndef SYNTHESIS
export "DPI-C" task tb_readHEX;
export "DPI-C" task tb_loadHEX;
export "DPI-C" task tb_writetoSram0;
export "DPI-C" task tb_writetoSram1;
export "DPI-C" task tb_writetoSram2;
export "DPI-C" task tb_writetoSram3;
export "DPI-C" task tb_writetoSram4;
export "DPI-C" task tb_writetoSram5;
export "DPI-C" task tb_writetoSram6;
export "DPI-C" task tb_writetoSram7;
export "DPI-C" task tb_getMemSize;
export "DPI-C" task tb_set_exit_loop;

import core_v_mini_mcu_pkg::*;

task tb_getMemSize;
  output int mem_size;
  mem_size = core_v_mini_mcu_pkg::MEM_SIZE;
endtask

task tb_readHEX;
  input string file;
  output logic [7:0] stimuli[core_v_mini_mcu_pkg::MEM_SIZE];
  $readmemh(file, stimuli);
endtask

task tb_loadHEX;
  input string file;
  logic [7:0] stimuli[core_v_mini_mcu_pkg::MEM_SIZE];
  int i, j, NumBytes;
  tb_readHEX(file, stimuli);
  tb_getMemSize(NumBytes);
  j = 0;
  for (i = 0; i < NumBytes / 8; i = i + 4) begin
    tb_writetoSram0(j, stimuli[i+3], stimuli[i+2], stimuli[i+1], stimuli[i]);
    j++;
  end
  j = 0;
  for (i = 1 * NumBytes / 8; i < 2 * (NumBytes / 8); i = i + 4) begin
    tb_writetoSram1(j, stimuli[i+3], stimuli[i+2], stimuli[i+1], stimuli[i]);
    j++;
  end
  j = 0;
  for (i = 2 * (NumBytes / 8); i < 3 * (NumBytes / 8); i = i + 4) begin
    tb_writetoSram2(j, stimuli[i+3], stimuli[i+2], stimuli[i+1], stimuli[i]);
    j++;
  end
  j = 0;
  for (i = 3 * (NumBytes / 8); i < 4 * (NumBytes / 8); i = i + 4) begin
    tb_writetoSram3(j, stimuli[i+3], stimuli[i+2], stimuli[i+1], stimuli[i]);
    j++;
  end
  j = 0;
  for (i = 4 * (NumBytes / 8); i < 5 * (NumBytes / 8); i = i + 4) begin
    tb_writetoSram4(j, stimuli[i+3], stimuli[i+2], stimuli[i+1], stimuli[i]);
    j++;
  end
  j = 0;
  for (i = 5 * (NumBytes / 8); i < 6 * (NumBytes / 8); i = i + 4) begin
    tb_writetoSram5(j, stimuli[i+3], stimuli[i+2], stimuli[i+1], stimuli[i]);
    j++;
  end
  j = 0;
  for (i = 6 * (NumBytes / 8); i < 7 * (NumBytes / 8); i = i + 4) begin
    tb_writetoSram6(j, stimuli[i+3], stimuli[i+2], stimuli[i+1], stimuli[i]);
    j++;
  end
  j = 0;
  for (i = 7 * (NumBytes / 8); i < 8 * (NumBytes / 8); i = i + 4) begin
    tb_writetoSram7(j, stimuli[i+3], stimuli[i+2], stimuli[i+1], stimuli[i]);
    j++;
  end
endtask

task tb_writetoSram0;
  input integer addr;
  input [7:0] val3;
  input [7:0] val2;
  input [7:0] val1;
  input [7:0] val0;
`ifdef VCS
  force core_v_mini_mcu_i.memory_subsystem_i.ram0_i.tc_ram_i.sram[addr] = {val3, val2, val1, val0};
  release core_v_mini_mcu_i.memory_subsystem_i.ram0_i.tc_ram_i.sram[addr];
`else
  core_v_mini_mcu_i.memory_subsystem_i.ram0_i.tc_ram_i.sram[addr] = {val3, val2, val1, val0};
`endif
endtask

task tb_writetoSram1;
  input integer addr;
  input [7:0] val3;
  input [7:0] val2;
  input [7:0] val1;
  input [7:0] val0;
`ifdef VCS
  force core_v_mini_mcu_i.memory_subsystem_i.ram1_i.tc_ram_i.sram[addr] = {val3, val2, val1, val0};
  release core_v_mini_mcu_i.memory_subsystem_i.ram1_i.tc_ram_i.sram[addr];
`else
  core_v_mini_mcu_i.memory_subsystem_i.ram1_i.tc_ram_i.sram[addr] = {val3, val2, val1, val0};
`endif
endtask

task tb_writetoSram2;
  input integer addr;
  input [7:0] val3;
  input [7:0] val2;
  input [7:0] val1;
  input [7:0] val0;
`ifdef VCS
  force core_v_mini_mcu_i.memory_subsystem_i.ram2_i.tc_ram_i.sram[addr] = {val3, val2, val1, val0};
  release core_v_mini_mcu_i.memory_subsystem_i.ram2_i.tc_ram_i.sram[addr];
`else
  core_v_mini_mcu_i.memory_subsystem_i.ram2_i.tc_ram_i.sram[addr] = {val3, val2, val1, val0};
`endif
endtask

task tb_writetoSram3;
  input integer addr;
  input [7:0] val3;
  input [7:0] val2;
  input [7:0] val1;
  input [7:0] val0;
`ifdef VCS
  force core_v_mini_mcu_i.memory_subsystem_i.ram3_i.tc_ram_i.sram[addr] = {val3, val2, val1, val0};
  release core_v_mini_mcu_i.memory_subsystem_i.ram3_i.tc_ram_i.sram[addr];
`else
  core_v_mini_mcu_i.memory_subsystem_i.ram3_i.tc_ram_i.sram[addr] = {val3, val2, val1, val0};
`endif
endtask

task tb_writetoSram4;
  input integer addr;
  input [7:0] val3;
  input [7:0] val2;
  input [7:0] val1;
  input [7:0] val0;
`ifdef VCS
  force core_v_mini_mcu_i.memory_subsystem_i.ram4_i.tc_ram_i.sram[addr] = {val3, val2, val1, val0};
  release core_v_mini_mcu_i.memory_subsystem_i.ram4_i.tc_ram_i.sram[addr];
`else
  core_v_mini_mcu_i.memory_subsystem_i.ram4_i.tc_ram_i.sram[addr] = {val3, val2, val1, val0};
`endif
endtask

task tb_writetoSram5;
  input integer addr;
  input [7:0] val3;
  input [7:0] val2;
  input [7:0] val1;
  input [7:0] val0;
`ifdef VCS
  force core_v_mini_mcu_i.memory_subsystem_i.ram5_i.tc_ram_i.sram[addr] = {val3, val2, val1, val0};
  release core_v_mini_mcu_i.memory_subsystem_i.ram5_i.tc_ram_i.sram[addr];
`else
  core_v_mini_mcu_i.memory_subsystem_i.ram5_i.tc_ram_i.sram[addr] = {val3, val2, val1, val0};
`endif
endtask

task tb_writetoSram6;
  input integer addr;
  input [7:0] val3;
  input [7:0] val2;
  input [7:0] val1;
  input [7:0] val0;
`ifdef VCS
  force core_v_mini_mcu_i.memory_subsystem_i.ram6_i.tc_ram_i.sram[addr] = {val3, val2, val1, val0};
  release core_v_mini_mcu_i.memory_subsystem_i.ram6_i.tc_ram_i.sram[addr];
`else
  core_v_mini_mcu_i.memory_subsystem_i.ram6_i.tc_ram_i.sram[addr] = {val3, val2, val1, val0};
`endif
endtask

task tb_writetoSram7;
  input integer addr;
  input [7:0] val3;
  input [7:0] val2;
  input [7:0] val1;
  input [7:0] val0;
`ifdef VCS
  force core_v_mini_mcu_i.memory_subsystem_i.ram7_i.tc_ram_i.sram[addr] = {val3, val2, val1, val0};
  release core_v_mini_mcu_i.memory_subsystem_i.ram7_i.tc_ram_i.sram[addr];
`else
  core_v_mini_mcu_i.memory_subsystem_i.ram7_i.tc_ram_i.sram[addr] = {val3, val2, val1, val0};
`endif
endtask

task tb_set_exit_loop;
`ifdef VCS
  force core_v_mini_mcu_i.ao_peripheral_subsystem_i.soc_ctrl_i.testbench_set_exit_loop[0] = 1'b1;
  release core_v_mini_mcu_i.ao_peripheral_subsystem_i.soc_ctrl_i.testbench_set_exit_loop[0];
`else
  core_v_mini_mcu_i.ao_peripheral_subsystem_i.soc_ctrl_i.testbench_set_exit_loop[0] = 1'b1;
`endif
endtask

`endif

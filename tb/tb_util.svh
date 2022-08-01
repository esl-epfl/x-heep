// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

/* verilator lint_off UNUSED */

`ifndef SYNTHESIS
export "DPI-C" task tb_readHEX;
export "DPI-C" task tb_loadHEX;
export "DPI-C" task tb_writetoSram0;
export "DPI-C" task tb_writetoSram1;
export "DPI-C" task tb_writetoSram;
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
  int i, NumBytes;
  tb_readHEX(file, stimuli);
  tb_getMemSize(NumBytes);
  for (i = 0; i < NumBytes / 8; i = i + 4) begin
    tb_writetoSram0(i / 4, stimuli[i+3], stimuli[i+2], stimuli[i+1], stimuli[i]);
  end
  for (i = 1 * NumBytes / 8; i < 2 * (NumBytes / 8); i = i + 4) begin
    tb_writetoSram1(i / 4, stimuli[i+3], stimuli[i+2], stimuli[i+1], stimuli[i]);
  end
  for (i = 2 * (NumBytes / 8); i < 3 * (NumBytes / 8); i = i + 4) begin
    tb_writetoSram2(i / 4, stimuli[i+3], stimuli[i+2], stimuli[i+1], stimuli[i]);
  end
  for (i = 3 * (NumBytes / 8); i < 4 * (NumBytes / 8); i = i + 4) begin
    tb_writetoSram3(i / 4, stimuli[i+3], stimuli[i+2], stimuli[i+1], stimuli[i]);
  end
  for (i = 4 * (NumBytes / 8); i < 5 * (NumBytes / 8); i = i + 4) begin
    tb_writetoSram4(i / 4, stimuli[i+3], stimuli[i+2], stimuli[i+1], stimuli[i]);
  end
  for (i = 5 * (NumBytes / 8); i < 6 * (NumBytes / 8); i = i + 4) begin
    tb_writetoSram5(i / 4, stimuli[i+3], stimuli[i+2], stimuli[i+1], stimuli[i]);
  end
  for (i = 6 * (NumBytes / 8); i < 7 * (NumBytes / 8); i = i + 4) begin
    tb_writetoSram6(i / 4, stimuli[i+3], stimuli[i+2], stimuli[i+1], stimuli[i]);
  end
  for (i = 7 * (NumBytes / 8); i < 8 * (NumBytes / 8); i = i + 4) begin
    tb_writetoSram7(i / 4, stimuli[i+3], stimuli[i+2], stimuli[i+1], stimuli[i]);
  end
endtask

task tb_writetoSram;
  input integer addr;
  input logic [31:0] val;
  output integer retval;
  int NumBytes;
  tb_getMemSize(NumBytes);
  if (|(addr & 32'h03)) begin
    retval = 1;
    $error("Only word-aligned memory access are supported");
  end else begin
    if (addr < NumBytes / 8) begin
      tb_writetoSram0(addr / 4, val[31:24], val[23:16], val[15:8], val[7:0]);
      retval = 0;
    end else if (addr < 2 * (NumBytes / 8)) begin
      tb_writetoSram1(addr / 4, val[31:24], val[23:16], val[15:8], val[7:0]);
      retval = 0;
    end else if (addr < 3 * (NumBytes / 8)) begin
      tb_writetoSram2(addr / 4, val[31:24], val[23:16], val[15:8], val[7:0]);
      retval = 0;
    end else if (addr < 4 * (NumBytes / 8)) begin
      tb_writetoSram3(addr / 4, val[31:24], val[23:16], val[15:8], val[7:0]);
      retval = 0;
    end else if (addr < 5 * (NumBytes / 8)) begin
      tb_writetoSram4(addr / 4, val[31:24], val[23:16], val[15:8], val[7:0]);
      retval = 0;
    end else if (addr < 6 * (NumBytes / 8)) begin
      tb_writetoSram5(addr / 4, val[31:24], val[23:16], val[15:8], val[7:0]);
      retval = 0;
    end else if (addr < 7 * (NumBytes / 8)) begin
      tb_writetoSram6(addr / 4, val[31:24], val[23:16], val[15:8], val[7:0]);
      retval = 0;
    end else if (addr < 8 * (NumBytes / 8)) begin
      tb_writetoSram7(addr / 4, val[31:24], val[23:16], val[15:8], val[7:0]);
      retval = 0;
    end else begin
      retval = 1;
      $error("Out Of Memory");
    end
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

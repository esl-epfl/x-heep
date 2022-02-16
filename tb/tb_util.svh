// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

`ifndef SYNTHESIS
// Task for loading 'mem' with SystemVerilog system task $readmemh()
export "DPI-C" task tb_readHEX;
export "DPI-C" task tb_loadHEX;
export "DPI-C" task tb_writetoSram0;
export "DPI-C" task tb_writetoSram1;
export "DPI-C" task tb_writetoSram;
export "DPI-C" task tb_getMemSize;

import core_v_mini_mcu_pkg::*;


task tb_getMemSize;
  output int mem_size;
  mem_size = core_v_mini_mcu_pkg::MEM_SIZE;
endtask

task tb_readHEX;
  input  string file;
  output logic [7:0] stimuli[core_v_mini_mcu_pkg::MEM_SIZE];
  $readmemh(file,stimuli);
endtask

task tb_loadHEX;
  input  string file;
  logic [7:0] stimuli[core_v_mini_mcu_pkg::MEM_SIZE];
  int i,j,NumBytes;

  tb_readHEX(file,stimuli);
  tb_getMemSize(NumBytes);

  for(i=0;i<NumBytes/2;i=i+4) begin
      tb_writetoSram0(i/4, stimuli[i+3],stimuli[i+2],stimuli[i+1],stimuli[i]);
  end
  for(j=0;j<NumBytes/2;j=j+4) begin
      tb_writetoSram1(j/4, stimuli[i+3],stimuli[i+2],stimuli[i+1],stimuli[i]);
      i = i + 4;
  end
endtask

task tb_writetoSram;
  input integer addr;
  input logic [31:0]  val;
  output integer retval;
  int mem_size;
  tb_getMemSize(mem_size);
  if(|(addr & 32'h03)) begin
    retval = 1;
    $error("Only word-aligned memory access are supported");
  end
  else begin
    if(addr<mem_size/2) begin
      tb_writetoSram0(addr,val[31:24],val[23:16],val[15:8],val[7:0]);
      retval = 0;
    end else if(addr<mem_size) begin
      tb_writetoSram1(addr,val[31:24],val[23:16],val[15:8],val[7:0]);
      retval = 0;
    end else begin
      retval = 1;
      $error("Out Of Memory");
    end
  end
endtask

task tb_writetoSram0;
  input integer addr;
  input [7:0]   val3;
  input [7:0]   val2;
  input [7:0]   val1;
  input [7:0]   val0;
  memory_subsystem_i.ram0_i.tc_ram_i.sram[addr] = {val3, val2, val1, val0};
endtask

task tb_writetoSram1;
  input integer addr;
  input [7:0]   val3;
  input [7:0]   val2;
  input [7:0]   val1;
  input [7:0]   val0;
  memory_subsystem_i.ram1_i.tc_ram_i.sram[addr] = {val3, val2, val1, val0};
endtask




`endif

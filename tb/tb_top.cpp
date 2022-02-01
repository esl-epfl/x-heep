// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1


#include "verilated.h"
#include "verilated_fst_c.h"
#include "Vcore_v_mini_mcu.h"
#include "Vcore_v_mini_mcu__Syms.h"

#include <stdlib.h>
#include <iostream>

vluint64_t sim_time = 0;

std::string getCmdOption(int argc, char* argv[], const std::string& option)
{
    std::string cmd;
     for( int i = 0; i < argc; ++i)
     {
          std::string arg = argv[i];
          size_t arg_size = arg.length();
          size_t option_size = option.length();

          if(arg.find(option)==0){
            cmd = arg.substr(option_size,arg_size-option_size);
          }
     }
     return cmd;
}

void runCycles(unsigned int ncycles, Vcore_v_mini_mcu *dut, VerilatedFstC *m_trace){
  for(unsigned int i = 0; i < ncycles; i++) {
    dut->clk_i ^= 1;
    dut->eval();
    m_trace->dump(sim_time);
    sim_time++;
  }
}

int main (int argc, char * argv[])
{

  std::string firmware;
  svLogicVecVal stimuli[65536]; //2^16
  svLogicVecVal addr;
  int i,j;

  Verilated::commandArgs(argc, argv);

  // Instantiate the model
  Vcore_v_mini_mcu *dut = new Vcore_v_mini_mcu;

  // Open VCD
  Verilated::traceEverOn (true);
  VerilatedFstC *m_trace = new VerilatedFstC;
  dut->trace (m_trace, 99);
  m_trace->open ("waveform.vcd");

  firmware = getCmdOption(argc, argv, "+firmware=");
  if(firmware.empty()){
    std::cout<<"[TESTBENCH]: No firmware  specified"<<std::endl;
    exit(EXIT_FAILURE);
  } else {
    std::cout<<"[TESTBENCH]: loading firmware  "<<firmware<<std::endl;
  }

  svSetScope(svGetScopeFromName("TOP.core_v_mini_mcu"));
  svScope scope = svGetScope();
  if (!scope) {
    std::cout<<"Warning: svGetScope failed"<< std::endl;
    exit(EXIT_FAILURE);
  }

  dut->tb_util_ReadMemh(firmware.c_str(),stimuli);

  dut->clk_i          = 0;
  dut->rst_ni         = 0;
  dut->jtag_tck_i     = 0;
  dut->jtag_tms_i     = 0;
  dut->jtag_trst_ni   = 0;
  dut->jtag_tdi_i     = 0;
  dut->fetch_enable_i = 1;

  dut->eval();
  m_trace->dump(sim_time);
  sim_time++;

  runCycles(20, dut, m_trace);


  dut->rst_ni = 1;
  runCycles(1, dut, m_trace);
  std::cout<<"Reset Released"<< std::endl;

  for(i=0;i<65536/2;i+=4) {
      addr.aval = i/4;
      dut->tb_util_WriteToSram0(&addr,&stimuli[i+3],&stimuli[i+2],&stimuli[i+1],&stimuli[i]);
  }
  for(j=0;j<65536/2;j=j+4) {
      addr.aval = j/4;
      dut->tb_util_WriteToSram1(&addr,&stimuli[i+3],&stimuli[i+2],&stimuli[i+1],&stimuli[i]);
      i = i + 4;
  }

  runCycles(1, dut, m_trace);
  std::cout<<"Memory Loaded"<< std::endl;

  runCycles(20, dut, m_trace);
  std::cout<<"OK Loaded"<< std::endl;

  m_trace->close();
  delete dut;
  exit(EXIT_SUCCESS);

}
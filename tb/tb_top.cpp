// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include "verilated.h"
#include "verilated_fst_c.h"
#include "Vtestharness.h"
#include "Vtestharness__Syms.h"

#include <stdlib.h>
#include <iostream>

#include "XHEEP_CmdLineOptions.hh"

vluint64_t sim_time = 0;

void runCycles(unsigned int ncycles, Vtestharness *dut, VerilatedFstC *m_trace){
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
  unsigned int max_sim_time, boot_sel, exit_val;
  bool use_openocd;
  bool run_all = false;

  Verilated::commandArgs(argc, argv);

  // Instantiate the model
  Vtestharness *dut = new Vtestharness;

  // Open VCD
  Verilated::traceEverOn (true);
  VerilatedFstC *m_trace = new VerilatedFstC;
  dut->trace (m_trace, 99);
  m_trace->open ("waveform.vcd");

  XHEEP_CmdLineOptions* cmd_lines_options = new XHEEP_CmdLineOptions(argc,argv);

  use_openocd = cmd_lines_options->get_use_openocd();
  firmware = cmd_lines_options->get_firmware();

  if(firmware.empty() && use_openocd==false){
      std::cout<<"You must specify the firmware if you are not using OpenOCD"<<std::endl;
      exit(EXIT_FAILURE);
  }

  max_sim_time = cmd_lines_options->get_max_sim_time(run_all);

  boot_sel     = cmd_lines_options->get_boot_sel();

  if(boot_sel == 1) {
    std::cout<<"[TESTBENCH]: ERROR: Executing from SPI is not supported (yet) in Verilator"<<std::endl;
    std::cout<<"exit simulation..."<<std::endl;
    exit(EXIT_FAILURE);
  }

  svSetScope(svGetScopeFromName("TOP.testharness"));
  svScope scope = svGetScope();
  if (!scope) {
    std::cout<<"Warning: svGetScope failed"<< std::endl;
    exit(EXIT_FAILURE);
  }

  dut->clk_i                = 0;
  dut->rst_ni               = 1;
  dut->jtag_tck_i           = 0;
  dut->jtag_tms_i           = 0;
  dut->jtag_trst_ni         = 0;
  dut->jtag_tdi_i           = 0;
  dut->execute_from_flash_i = 1; //this cause boot_sel cannot be 1 anyway
  dut->boot_select_i        = boot_sel;

  dut->eval();
  m_trace->dump(sim_time);
  sim_time++;

  dut->rst_ni               = 1;
  //this creates the negedge
  runCycles(50, dut, m_trace);
  dut->rst_ni               = 0;
  runCycles(50, dut, m_trace);


  dut->rst_ni = 1;
  runCycles(20, dut, m_trace);
  std::cout<<"Reset Released"<< std::endl;

  //dont need to exit from boot loop if using OpenOCD or Boot from Flash
  if(use_openocd==false || boot_sel == 1) {
    dut->tb_loadHEX(firmware.c_str());
    runCycles(1, dut, m_trace);
    dut->tb_set_exit_loop();
    std::cout<<"Set Exit Loop"<< std::endl;
    runCycles(1, dut, m_trace);
    std::cout<<"Memory Loaded"<< std::endl;
  } else {
    std::cout<<"Waiting for GDB"<< std::endl;
  }

  if(run_all==false) {
    runCycles(max_sim_time, dut, m_trace);
  } else {
    while(dut->exit_valid_o!=1) {
      runCycles(500, dut, m_trace);
    }
  }

  if(dut->exit_valid_o==1) {
    std::cout<<"Program Finished with value "<<dut->exit_value_o<<std::endl;
    exit_val = EXIT_SUCCESS;
  } else exit_val = EXIT_FAILURE;

  m_trace->close();
  delete dut;
  delete cmd_lines_options;

  exit(exit_val);

}
// Copyright 2024 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include "verilated.h"
#include "verilated_fst_sc.h"
#include "Vtestharness.h"
#include "Vtestharness__Syms.h"
#include "systemc.h"
#include <stdlib.h>
#include <iostream>
#include "XHEEP_CmdLineOptions.hh"


SC_MODULE(testbench)
{

  sc_in<bool> clk_i;
  sc_out<bool> clk_o;
  sc_out<bool> rst_no;
  sc_out<bool> boot_select_o;
  sc_out<bool> execute_from_flash_o;
  sc_out<bool> jtag_tck_o;
  sc_out<bool> jtag_tms_o;
  sc_out<bool> jtag_trst_n_o;
  sc_out<bool> jtag_tdi_o;

  bool boot_select_option;
  unsigned int reset_cycles = 30;

  void do_reset_cycle () {
    //active low

    //-----
    rst_no.write(false);

    for(int i=0;i<reset_cycles;i++){
      clk_o.write(false);
      //m_trace->dump(1);
      wait();
      clk_o.write(true);
      //m_trace->dump(1);
      wait();
      std::cout<<sc_time_stamp()<<std::endl;
    }


    //-----|||||||
    rst_no.write(true);

    for(int i=0;i<reset_cycles;i++){
      clk_o.write(false);
      //m_trace->dump(1);
      wait();
      clk_o.write(true);
      //m_trace->dump(1);
      wait();
      std::cout<<sc_time_stamp()<<std::endl;
    }

    //-----|||||||------
    rst_no.write(false);

    for(int i=0;i<reset_cycles;i++){
      clk_o.write(false);
      //m_trace->dump(1);
      wait();
      clk_o.write(true);
      //m_trace->dump(1);
      wait();
      std::cout<<sc_time_stamp()<<std::endl;
    }

    //-----|||||||------||||||
    rst_no.write(true);

    for(int i=0;i<reset_cycles;i++){
      clk_o.write(false);
      //m_trace->dump(1);
      wait();
      clk_o.write(true);
      //m_trace->dump(1);
      wait();
      std::cout<<sc_time_stamp()<<std::endl;
    }

  }

  void make_clock () {

    boot_select_o.write(boot_select_option);
    execute_from_flash_o.write(true);
    jtag_tck_o.write(false);
    jtag_tms_o.write(false);
    jtag_trst_n_o.write(false);
    jtag_tdi_o.write(false);

    do_reset_cycle();

    sc_stop();
  }

  SC_CTOR(testbench)
  {

    SC_CTHREAD(make_clock, clk_i.pos());

  }



};

int sc_main (int argc, char * argv[])
{

  std::string firmware;
  unsigned int max_sim_time, boot_sel, exit_val;
  bool use_openocd;
  bool run_all = false;
  Verilated::commandArgs(argc, argv);
  Verilated::traceEverOn(true);

  XHEEP_CmdLineOptions* cmd_lines_options = new XHEEP_CmdLineOptions(argc,argv);

  use_openocd = cmd_lines_options->get_use_openocd();
  firmware = cmd_lines_options->get_firmware();

  if(firmware.empty() && use_openocd==false)
      exit(EXIT_FAILURE);

  max_sim_time = cmd_lines_options->get_max_sim_time(run_all);

  boot_sel     = cmd_lines_options->get_boot_sel();

  if(use_openocd) {
    std::cout<<"[TESTBENCH]: ERROR: Executing from OpenOCD in SystemC is not supported (yet) in X-HEEP"<<std::endl;
    std::cout<<"exit simulation..."<<std::endl;
    exit(EXIT_FAILURE);
  }

  if(boot_sel == 1) {
    std::cout<<"[TESTBENCH]: ERROR: Executing from SPI is not supported (yet) in Verilator"<<std::endl;
    std::cout<<"exit simulation..."<<std::endl;
    exit(EXIT_FAILURE);
  }


  // generate clock
  sc_clock clock_sig("clock", 10, SC_NS, 0.5);

  Vtestharness dut("TOP");
  testbench tb("testbench");

  // static values
  tb.boot_select_option = boot_sel == 1;

  sc_signal<bool, SC_MANY_WRITERS>     clk;
  sc_signal<bool, SC_MANY_WRITERS>     rst_n;
  sc_signal<bool, SC_MANY_WRITERS>     boot_select;
  sc_signal<bool, SC_MANY_WRITERS>     execute_from_flash;
  sc_signal<bool>     jtag_tck;
  sc_signal<bool>     jtag_tms;
  sc_signal<bool>     jtag_trst_n;
  sc_signal<bool>     jtag_tdi;
  sc_signal<bool>     jtag_tdo;
  sc_signal<uint32_t> exit_value;
  sc_signal<bool, SC_MANY_WRITERS>     exit_valid;

  tb.clk_i(clock_sig);
  tb.clk_o(clk);
  tb.rst_no(rst_n);
  tb.boot_select_o(boot_select);
  tb.execute_from_flash_o(execute_from_flash);
  tb.jtag_tck_o(jtag_tck);
  tb.jtag_tms_o(jtag_tms);
  tb.jtag_trst_n_o(jtag_trst_n);
  tb.jtag_tdi_o(jtag_tdi);


  dut.clk_i(clk);
  dut.rst_ni(rst_n);
  dut.boot_select_i(boot_select);
  dut.execute_from_flash_i(execute_from_flash);
  dut.jtag_tck_i(jtag_tck);
  dut.jtag_tms_i(jtag_tms);
  dut.jtag_trst_ni(jtag_trst_n);
  dut.jtag_tdi_i(jtag_tdi);
  dut.jtag_tdo_o(jtag_tdo);
  dut.exit_value_o(exit_value);
  dut.exit_valid_o(exit_valid);

  VerilatedFstC *m_trace = new VerilatedFstC;
  m_trace->open ("waveform.vcd");
  dut.trace (m_trace, 99);

  // You must do one evaluation before enabling waves, in order to allow
  // SystemC to interconnect everything for testing.
  sc_start(SC_ZERO_TIME);

  // Simulate until $finish
  while ((sc_time_stamp() > sc_time(1, SC_NS) && sc_time_stamp() < sc_time(10, SC_NS))) {

      m_trace->flush();
      m_trace->dump(sc_time_stamp().to_double());

      // Simulate 1ns
      sc_start(1, SC_NS);
  }


  //simulation start
  sc_start( );

/*
  svSetScope(svGetScopeFromName("TOP.testharness"));
  svScope scope = svGetScope();
  if (!scope) {
    std::cout<<"Warning: svGetScope failed"<< std::endl;
    exit(EXIT_FAILURE);
  }

  if(use_openocd==false || boot_sel == 1) {
    dut.tb_loadHEX(firmware.c_str());
    std::cout<<"Memory Loaded"<< std::endl;
  } else {
    std::cout<<"Waiting for GDB"<< std::endl;
  }
*/
  m_trace->close();

  return 0;

}

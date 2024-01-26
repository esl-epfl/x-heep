// Copyright 2024 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include "verilated.h"
#include "verilated_fst_c.h"
#include "Vtestharness.h"
#include "systemc.h"
#include <stdlib.h>
#include <iostream>


SC_MODULE(testbench)
{

  sc_in<bool> clk_i;
  sc_out<bool> clk_o;

  void make_clock () {

    for(int i=0;i<30;i++){
      clk_o.write(false);
      wait();
      clk_o.write(true);
      wait();
      std::cout<<sc_time_stamp()<<std::endl;
    }

    sc_stop();
  }

  SC_CTOR(testbench)
  {

    SC_CTHREAD(make_clock, clk_i.pos());

  }
};

int sc_main (int argc, char * argv[])
{

  sc_clock clock_sig("clock", 10, SC_NS, 0.5);

  Verilated::commandArgs(argc, argv);

  Vtestharness dut("testharness");

  testbench tb("testbench");

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


  //tracing
  sc_trace_file *fp;
  fp=sc_create_vcd_trace_file("vcd_trace");
  fp->set_time_unit(1, SC_NS);

  sc_trace(fp, dut.clk_i, "clock");
  sc_trace(fp, dut.rst_ni, "rst_ni");

  //simulation start
  sc_start(100,SC_NS);

  sc_close_vcd_trace_file(fp);

  return 0;

}

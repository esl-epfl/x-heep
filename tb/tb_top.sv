// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module tb_top #(
    parameter PULP_XPULP = 0,
    parameter FPU        = 0,
    parameter PULP_ZFINX = 0,
    parameter JTAG_DPI   = 0,
    parameter BOOT_SEL   = 0
);

  // comment to record execution trace
  //`define TRACE_EXECUTION

  const time          CLK_PHASE_HI = 5ns;
  const time          CLK_PHASE_LO = 5ns;
  const time          CLK_PERIOD = CLK_PHASE_HI + CLK_PHASE_LO;

  const time          STIM_APPLICATION_DEL = CLK_PERIOD * 0.1;
  const time          RESP_ACQUISITION_DEL = CLK_PERIOD * 0.9;
  const time          RESET_DEL = STIM_APPLICATION_DEL;
  const int           RESET_WAIT_CYCLES = 4;

  // clock and reset for tb
  logic               clk = 'b1;
  logic               rst_n = 'b0;

  // cycle counter
  int unsigned        cycle_cnt_q;

  // testbench result
  logic               exit_valid;
  logic        [31:0] exit_value;

  // signals for ri5cy
  logic               fetch_enable;
  logic               jtag_tck;
  logic               jtag_trst_n;
  logic               jtag_tms;
  logic               jtag_tdi;
  logic               jtag_tdo;

  logic               boot_select;

  // make the core start fetching instruction immediately
  assign fetch_enable = '1;

  // allow vcd dump
  initial begin
    if ($test$plusargs("vcd")) begin
      $dumpfile("waveform.vcd");
      $dumpvars(0, tb_top);
    end
  end

  // we either load the provided firmware or execute a small test program that
  // doesn't do more than an infinite loop with some I/O
  initial begin : load_prog
    automatic string firmware;

    wait (rst_n == 1'b1);

    if (JTAG_DPI==0 && BOOT_SEL==0) begin
      if ($value$plusargs("firmware=%s", firmware)) begin
        if ($test$plusargs("verbose"))
          $display("[TESTBENCH] %t: loading firmware %0s ...", $time, firmware);
        testharness_i.tb_loadHEX(firmware);
        #CLK_PHASE_HI
        testharness_i.tb_set_exit_loop();
        #CLK_PHASE_LO
        if ($test$plusargs("verbose"))
          $display("[TESTBENCH] %t: memory loaded", $time);
      end else begin
        $display("No firmware specified");
        $finish;
      end
    end else if (JTAG_DPI==1) begin
      if ($test$plusargs("verbose"))
        $display("Waiting for GDB");
    end else if (BOOT_SEL==1) begin
      if ($test$plusargs("verbose"))
        $display("Booting from flash");
    end
  end

  // clock generation
  initial begin : clock_gen
    forever begin
      #CLK_PHASE_HI clk = 1'b0;
      #CLK_PHASE_LO clk = 1'b1;
    end
  end : clock_gen

  // reset generation
  initial begin : reset_gen
    rst_n = 1'b0;

    // wait a few cycles
    repeat (RESET_WAIT_CYCLES) begin
      @(posedge clk);
    end

    // start running
    #RESET_DEL rst_n = 1'b1;

    if ($test$plusargs("verbose")) $display("reset deasserted", $time);

  end : reset_gen

  // set timing format
  initial begin : timing_format
    $timeformat(-9, 0, "ns", 9);
  end : timing_format

  // abort after n cycles, if we want to
  always_ff @(posedge clk, negedge rst_n) begin
    automatic int maxcycles;
    if ($value$plusargs("maxcycles=%d", maxcycles)) begin
      if (~rst_n) begin
        cycle_cnt_q <= 0;
      end else begin
        cycle_cnt_q <= cycle_cnt_q + 1;
        if (cycle_cnt_q >= maxcycles) begin
          $fatal(2, "Simulation aborted due to maximum cycle limit");
        end
      end
    end
  end

  // check if we succeded
  always_ff @(posedge clk, negedge rst_n) begin
    if (exit_valid) begin
      if (exit_value == 0) $display("EXIT SUCCESS");
      else $display("EXIT FAILURE: %d", exit_value);
      $finish;
    end
  end

  // wrapper for riscv, the memory system and stdout peripheral

  assign boot_select = (BOOT_SEL == 1) ? 1'b1 : 1'b0;
  testharness #(
      .PULP_XPULP(PULP_XPULP),
      .FPU       (FPU),
      .PULP_ZFINX(PULP_ZFINX),
      .JTAG_DPI  (JTAG_DPI)
  ) testharness_i (
      .clk_i         (clk),
      .rst_ni        (rst_n),
      .boot_select_i (boot_select),
      .fetch_enable_i(fetch_enable),
      .exit_valid_o  (exit_valid),
      .exit_value_o  (exit_value),
      .jtag_tck_i    (jtag_tck),
      .jtag_trst_ni  (jtag_trst_n),
      .jtag_tms_i    (jtag_tms),
      .jtag_tdi_i    (jtag_tdi),
      .jtag_tdo_o    (jtag_tdo)
  );


endmodule  // tb_top

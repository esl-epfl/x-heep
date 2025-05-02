// Copyright lowRISC contributors.
// Copyright 2018 ETH Zurich and University of Bologna, see also CREDITS.md.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

`ifdef RISCV_FORMAL
  `define RVFI
`endif

`include "prim_assert.sv"

/**
 * Top level module of the ibex RISC-V core
 */
module cve2_top import cve2_pkg::*; #(
  parameter int unsigned MHPMCounterNum   = 0,
  parameter int unsigned MHPMCounterWidth = 40,
  parameter bit          RV32E            = 1'b0,
  parameter rv32m_e      RV32M            = RV32MFast,
  parameter int unsigned DmHaltAddr       = 32'h1A110800,
  parameter int unsigned DmExceptionAddr  = 32'h1A110808
) (
  // Clock and Reset
  input  logic                         clk_i,
  input  logic                         rst_ni,

  input  logic                         test_en_i,     // enable all clock gates for testing
  input  prim_ram_1p_pkg::ram_1p_cfg_t ram_cfg_i,

  input  logic [31:0]                  hart_id_i,
  input  logic [31:0]                  boot_addr_i,

  // Instruction memory interface
  output logic                         instr_req_o,
  input  logic                         instr_gnt_i,
  input  logic                         instr_rvalid_i,
  output logic [31:0]                  instr_addr_o,
  input  logic [31:0]                  instr_rdata_i,
  input  logic                         instr_err_i,

  // Data memory interface
  output logic                         data_req_o,
  input  logic                         data_gnt_i,
  input  logic                         data_rvalid_i,
  output logic                         data_we_o,
  output logic [3:0]                   data_be_o,
  output logic [31:0]                  data_addr_o,
  output logic [31:0]                  data_wdata_o,
  input  logic [31:0]                  data_rdata_i,
  input  logic                         data_err_i,

  // Interrupt inputs
  input  logic                         irq_software_i,
  input  logic                         irq_timer_i,
  input  logic                         irq_external_i,
  input  logic [15:0]                  irq_fast_i,
  input  logic                         irq_nm_i,       // non-maskeable interrupt

  // Debug Interface
  input  logic                         debug_req_i,
  output crash_dump_t                  crash_dump_o,

  // RISC-V Formal Interface
  // Does not comply with the coding standards of _i/_o suffixes, but follows
  // the convention of RISC-V Formal Interface Specification.
`ifdef RVFI
  output logic                         rvfi_valid,
  output logic [63:0]                  rvfi_order,
  output logic [31:0]                  rvfi_insn,
  output logic                         rvfi_trap,
  output logic                         rvfi_halt,
  output logic                         rvfi_intr,
  output logic [ 1:0]                  rvfi_mode,
  output logic [ 1:0]                  rvfi_ixl,
  output logic [ 4:0]                  rvfi_rs1_addr,
  output logic [ 4:0]                  rvfi_rs2_addr,
  output logic [ 4:0]                  rvfi_rs3_addr,
  output logic [31:0]                  rvfi_rs1_rdata,
  output logic [31:0]                  rvfi_rs2_rdata,
  output logic [31:0]                  rvfi_rs3_rdata,
  output logic [ 4:0]                  rvfi_rd_addr,
  output logic [31:0]                  rvfi_rd_wdata,
  output logic [31:0]                  rvfi_pc_rdata,
  output logic [31:0]                  rvfi_pc_wdata,
  output logic [31:0]                  rvfi_mem_addr,
  output logic [ 3:0]                  rvfi_mem_rmask,
  output logic [ 3:0]                  rvfi_mem_wmask,
  output logic [31:0]                  rvfi_mem_rdata,
  output logic [31:0]                  rvfi_mem_wdata,
  output logic [31:0]                  rvfi_ext_mip,
  output logic                         rvfi_ext_nmi,
  output logic                         rvfi_ext_debug_req,
  output logic [63:0]                  rvfi_ext_mcycle,
`endif

  // CPU Control Signals
  input  logic                         fetch_enable_i,
  output logic                         core_sleep_o
);

  // Scrambling Parameter
  localparam int unsigned NumAddrScrRounds  = 0;

  // Physical Memory Protection
  localparam bit          PMPEnable        = 1'b0;
  localparam int unsigned PMPGranularity   = 0;
  localparam int unsigned PMPNumRegions    = 4;

  // Trigger support
  localparam bit          DbgTriggerEn     = 1'b1;
  localparam int unsigned DbgHwBreakNum    = 1;

  // Bit manipulation extension
  localparam rv32b_e      RV32B            = RV32BNone;

  // Clock signals
  logic                        clk;
  logic                        core_busy_d, core_busy_q;
  logic                        clock_en;
  logic                        fetch_enable_d, fetch_enable_q;
  logic                        irq_pending;

  /////////////////////
  // Main clock gate //
  /////////////////////

  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      core_busy_q <= 1'b0;
      fetch_enable_q <= 1'b0;
    end else begin
      core_busy_q <= core_busy_d;
      fetch_enable_q <= fetch_enable_d;
    end
  end

  assign clock_en = fetch_enable_q & (core_busy_q | debug_req_i | irq_pending | irq_nm_i);
  assign core_sleep_o = fetch_enable_q & !clock_en;
  assign fetch_enable_d = fetch_enable_i ? 1'b1 : fetch_enable_q;

  cve2_clock_gate core_clock_gate_i (
    .clk_i    (clk_i),
    .en_i     (clock_en),
    .scan_cg_en_i(test_en_i),
    .clk_o    (clk)
  );

  ////////////////////////
  // Core instantiation //
  ////////////////////////

  cve2_core #(
    .PMPEnable        (PMPEnable),
    .PMPGranularity   (PMPGranularity),
    .PMPNumRegions    (PMPNumRegions),
    .MHPMCounterNum   (MHPMCounterNum),
    .MHPMCounterWidth (MHPMCounterWidth),
    .RV32E            (RV32E),
    .RV32M            (RV32M),
    .RV32B            (RV32B),
    .DbgTriggerEn     (DbgTriggerEn),
    .DbgHwBreakNum    (DbgHwBreakNum),
    .DmHaltAddr       (DmHaltAddr),
    .DmExceptionAddr  (DmExceptionAddr)
  ) u_cve2_core (
    .clk_i(clk),
    .rst_ni,
    .test_en_i,

    .hart_id_i,
    .boot_addr_i,

    .instr_req_o,
    .instr_gnt_i,
    .instr_rvalid_i,
    .instr_addr_o,
    .instr_rdata_i,
    .instr_err_i,

    .data_req_o,
    .data_gnt_i,
    .data_rvalid_i,
    .data_we_o,
    .data_be_o,
    .data_addr_o,
    .data_wdata_o,
    .data_rdata_i,
    .data_err_i,

    .irq_software_i,
    .irq_timer_i,
    .irq_external_i,
    .irq_fast_i,
    .irq_nm_i,
    .irq_pending_o(irq_pending),

    .debug_req_i,
    .crash_dump_o,

`ifdef RVFI
    .rvfi_valid,
    .rvfi_order,
    .rvfi_insn,
    .rvfi_trap,
    .rvfi_halt,
    .rvfi_intr,
    .rvfi_mode,
    .rvfi_ixl,
    .rvfi_rs1_addr,
    .rvfi_rs2_addr,
    .rvfi_rs3_addr,
    .rvfi_rs1_rdata,
    .rvfi_rs2_rdata,
    .rvfi_rs3_rdata,
    .rvfi_rd_addr,
    .rvfi_rd_wdata,
    .rvfi_pc_rdata,
    .rvfi_pc_wdata,
    .rvfi_mem_addr,
    .rvfi_mem_rmask,
    .rvfi_mem_wmask,
    .rvfi_mem_rdata,
    .rvfi_mem_wdata,
    .rvfi_ext_mip,
    .rvfi_ext_nmi,
    .rvfi_ext_debug_req,
    .rvfi_ext_mcycle,
`endif

    .fetch_enable_i (fetch_enable_q),
    .core_busy_o    (core_busy_d)
  );

  ////////////////////////
  // Rams Instantiation //
  ////////////////////////

  prim_ram_1p_pkg::ram_1p_cfg_t unused_ram_cfg;
  logic unused_ram_inputs;

  assign unused_ram_cfg    = ram_cfg_i;
  assign unused_ram_inputs = (|NumAddrScrRounds);


  // X checks for top-level outputs
  `ASSERT_KNOWN(IbexInstrReqX, instr_req_o)
  `ASSERT_KNOWN_IF(IbexInstrReqPayloadX, instr_addr_o, instr_req_o)

  `ASSERT_KNOWN(IbexDataReqX, data_req_o)
  `ASSERT_KNOWN_IF(IbexDataReqPayloadX,
    {data_we_o, data_be_o, data_addr_o, data_wdata_o}, data_req_o)

  `ASSERT_KNOWN(IbexCoreSleepX, core_sleep_o)

  // X check for top-level inputs
  `ASSERT_KNOWN(IbexTestEnX, test_en_i)
  `ASSERT_KNOWN(IbexRamCfgX, ram_cfg_i)
  `ASSERT_KNOWN(IbexHartIdX, hart_id_i)
  `ASSERT_KNOWN(IbexBootAddrX, boot_addr_i)

  `ASSERT_KNOWN(IbexInstrGntX, instr_gnt_i)
  `ASSERT_KNOWN(IbexInstrRValidX, instr_rvalid_i)
  `ASSERT_KNOWN_IF(IbexInstrRPayloadX,
    {instr_rdata_i, instr_err_i}, instr_rvalid_i)

  `ASSERT_KNOWN(IbexDataGntX, data_gnt_i)
  `ASSERT_KNOWN(IbexDataRValidX, data_rvalid_i)

  `ASSERT_KNOWN(IbexIrqX, {irq_software_i, irq_timer_i, irq_external_i, irq_fast_i, irq_nm_i})

  `ASSERT_KNOWN(IbexDebugReqX, debug_req_i)
endmodule

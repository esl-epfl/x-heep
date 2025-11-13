// Copyright 2025 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Davide Schiavone

/**
 * CVE2 XIF Wrapper that partially translates the X-IF 1.0v to 0.2v
 * v1.0 specs: https://docs.openhwgroup.org/projects/openhw-group-core-v-xif/en/v1.0.0/intro.html
 * v0.2.0 specs: https://docs.openhwgroup.org/projects/openhw-group-core-v-xif/en/v0.2.0/intro.html
 */
module cve2_xif_wrapper
  import cv32e40px_core_v_xif_pkg::*;
  import cve2_pkg::*;
#(
    parameter int unsigned MHPMCounterNum   = 0,
    parameter int unsigned MHPMCounterWidth = 40,
    parameter bit          RV32E            = 1'b0,
    parameter rv32m_e      RV32M            = RV32MFast,
    parameter bit          XInterface       = 1'b0
) (
    // Clock and Reset
    input logic clk_i,
    input logic rst_ni,

    input logic test_en_i,  // enable all clock gates for testing

    input logic [31:0] hart_id_i,
    input logic [31:0] boot_addr_i,

    // Instruction memory interface
    output logic        instr_req_o,
    input  logic        instr_gnt_i,
    input  logic        instr_rvalid_i,
    output logic [31:0] instr_addr_o,
    input  logic [31:0] instr_rdata_i,

    // Data memory interface
    output logic        data_req_o,
    input  logic        data_gnt_i,
    input  logic        data_rvalid_i,
    output logic        data_we_o,
    output logic [ 3:0] data_be_o,
    output logic [31:0] data_addr_o,
    output logic [31:0] data_wdata_o,
    input  logic [31:0] data_rdata_i,

    // CORE-V-XIF
    // Compressed interface
    output logic x_compressed_valid_o,
    input logic x_compressed_ready_i,
    input cv32e40px_core_v_xif_pkg::x_compressed_resp_t x_compressed_resp_i,
    output cv32e40px_core_v_xif_pkg::x_compressed_req_t x_compressed_req_o,

    // Issue Interface
    output logic x_issue_valid_o,
    input logic x_issue_ready_i,
    output cv32e40px_core_v_xif_pkg::x_issue_req_t x_issue_req_o,
    input cv32e40px_core_v_xif_pkg::x_issue_resp_t x_issue_resp_i,

    // Commit Interface
    output logic x_commit_valid_o,
    output cv32e40px_core_v_xif_pkg::x_commit_t x_commit_o,

    // Memory request/response Interface
    input logic x_mem_valid_i,
    input cv32e40px_core_v_xif_pkg::x_mem_req_t x_mem_req_i,

    output logic x_mem_ready_o,
    output cv32e40px_core_v_xif_pkg::x_mem_resp_t x_mem_resp_o,

    // Memory Result Interface
    output logic x_mem_result_valid_o,
    output cv32e40px_core_v_xif_pkg::x_mem_result_t x_mem_result_o,

    // Result Interface
    input logic x_result_valid_i,
    input cv32e40px_core_v_xif_pkg::x_result_t x_result_i,

    output logic x_result_ready_o,

    // Interrupt inputs
    input logic        irq_software_i,
    input logic        irq_timer_i,
    input logic        irq_external_i,
    input logic [15:0] irq_fast_i,

    // Debug Interface
    input  logic        debug_req_i,
    output logic        debug_halted_o,
    input  logic [31:0] dm_halt_addr_i,
    input  logic [31:0] dm_exception_addr_i,

    // CPU Control Signals
    input  logic fetch_enable_i,
    output logic core_sleep_o
);

  logic                    cve2_x_issue_valid;
  logic                    cve2_x_issue_ready;
  cve2_pkg::x_issue_req_t  cve2_x_issue_req;
  cve2_pkg::x_issue_resp_t cve2_x_issue_resp;
  cve2_pkg::x_register_t   cve2_x_register;

  logic                    cve2_x_commit_valid;
  cve2_pkg::x_commit_t     cve2_x_commit;

  logic                    cve2_x_result_valid;
  logic                    cve2_x_result_ready;
  cve2_pkg::x_result_t     cve2_x_result;

  // Issue Interface
  assign x_issue_valid_o = cve2_x_issue_valid;
  assign cve2_x_issue_ready = x_issue_ready_i;

  assign x_issue_req_o.mode = '0;
  assign x_issue_req_o.id = cve2_x_issue_req.id;
  assign x_issue_req_o.instr = cve2_x_issue_req.instr;
  assign x_issue_req_o.rs[0] = cve2_x_register.rs[0];
  assign x_issue_req_o.rs[1] = cve2_x_register.rs[1];
  assign x_issue_req_o.rs[2] = '0;
  assign x_issue_req_o.rs_valid = {1'b0, cve2_x_register.rs_valid};
  assign x_issue_req_o.ecs = '0;
  assign x_issue_req_o.ecs_valid = '0;

  assign cve2_x_issue_resp.accept = x_issue_resp_i.accept;
  assign cve2_x_issue_resp.writeback = x_issue_resp_i.writeback;
  assign cve2_x_issue_resp.register_read = {
    2'b11
  };  //this is suboptimal as forces cve2 to always read all registers - however, cve2 does not have instructions in-flight

  assign x_commit_valid_o = cve2_x_commit_valid;
  assign x_commit_o.id = cve2_x_commit.id;
  assign x_commit_o.commit_kill = cve2_x_commit.commit_kill;

  assign cve2_x_result_valid = x_result_valid_i;
  assign x_result_ready_o = cve2_x_result_ready;

  assign cve2_x_result.id = x_result_i.id;
  assign cve2_x_result.data = x_result_i.data;
  assign cve2_x_result.rd = x_result_i.rd;
  assign cve2_x_result.we = x_result_i.we;

  //ununsed
  assign x_compressed_valid_o = '0;
  assign x_compressed_req_o = '0;
  assign x_mem_ready_o = '0;
  assign x_mem_resp_o = '0;
  assign x_mem_result_valid_o = '0;
  assign x_mem_result_o = '0;
  assign cve2_x_result.hartid = '0;

  logic x_issue_resp_dualwrite = x_issue_resp_i.dualwrite;
  logic [2:0] x_issue_resp_dualread = x_issue_resp_i.dualread;
  logic x_issue_resp_loadstore = x_issue_resp_i.loadstore;
  logic x_issue_resp_ecswrite = x_issue_resp_i.ecswrite;
  logic x_issue_resp_exc = x_issue_resp_i.exc;

  logic [31:0] cve2_x_issue_req_hartid = cve2_x_issue_req.hartid;
  logic [31:0] cve2_x_register_hartid = cve2_x_register.hartid;
  logic [31:0] cve2_x_commit_hartid = cve2_x_commit.hartid;
  logic [3:0] cve2_x_register_id = cve2_x_register.id;

  cve2_top #(
      .MHPMCounterNum(MHPMCounterNum),
      .MHPMCounterWidth(MHPMCounterWidth),
      .RV32E(RV32E),
      .RV32M(RV32M),
      .XInterface(XInterface != '0)
  ) u_cve2_top (
      .clk_i,
      .rst_ni,
      .test_en_i,

      .hart_id_i,
      .boot_addr_i,
      .ram_cfg_i('0),

      .instr_req_o,
      .instr_gnt_i,
      .instr_rvalid_i,
      .instr_addr_o,
      .instr_rdata_i,
      .instr_err_i(1'b0),

      .data_req_o,
      .data_gnt_i,
      .data_rvalid_i,
      .data_we_o,
      .data_be_o,
      .data_addr_o,
      .data_wdata_o,
      .data_rdata_i,
      .data_err_i(1'b0),

      // Core-V Extension Interface (CV-X-IF)
      // Issue Interface
      .x_issue_valid_o(cve2_x_issue_valid),
      .x_issue_ready_i(cve2_x_issue_ready),
      .x_issue_req_o  (cve2_x_issue_req),
      .x_issue_resp_i (cve2_x_issue_resp),

      // Register Interface
      .x_register_o(cve2_x_register),

      // Commit Interface
      .x_commit_valid_o(cve2_x_commit_valid),
      .x_commit_o(cve2_x_commit),

      // Result Interface
      .x_result_valid_i(cve2_x_result_valid),
      .x_result_ready_o(cve2_x_result_ready),
      .x_result_i(cve2_x_result),

      .irq_software_i,
      .irq_timer_i,
      .irq_external_i,
      .irq_fast_i,
      .irq_nm_i(1'b0),

      .debug_req_i,
      .debug_halted_o,
      .dm_halt_addr_i,
      .dm_exception_addr_i,
      .crash_dump_o(),

      .fetch_enable_i,
      .core_sleep_o
  );

endmodule

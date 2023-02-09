// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module cpu_subsystem
  import obi_pkg::*;
  import core_v_mini_mcu_pkg::*;
#(
    parameter BOOT_ADDR = 'h180,
    parameter PULP_XPULP =  0, // PULP ISA Extension (incl. custom CSRs and hardware loop, excl. p.elw)
    parameter FPU = 0,  // Floating Point Unit (interfaced via APU interface)
    parameter PULP_ZFINX = 0,  // Float-in-General Purpose registers
    parameter NUM_MHPMCOUNTERS = 1,
    parameter DM_HALTADDRESS = '0,
    parameter X_EXT = 0,  // eXtension interface in cv32e40x
    parameter core_v_mini_mcu_pkg::cpu_type_e CPU_TYPE = core_v_mini_mcu_pkg::CpuType
) (
    // Clock and Reset
    input logic clk_i,
    input logic rst_ni,

    // Instruction memory interface
    output obi_req_t  core_instr_req_o,
    input  obi_resp_t core_instr_resp_i,

    // Data memory interface
    output obi_req_t  core_data_req_o,
    input  obi_resp_t core_data_resp_i,

    // eXtension interface
    if_xif.cpu_compressed xif_compressed_if,
    if_xif.cpu_issue      xif_issue_if,
    if_xif.cpu_commit     xif_commit_if,
    if_xif.cpu_mem        xif_mem_if,
    if_xif.cpu_mem_result xif_mem_result_if,
    if_xif.cpu_result     xif_result_if,

    // Interrupt inputs
    input  logic [31:0] irq_i,      // CLINT interrupts + CLINT extension interrupts
    output logic        irq_ack_o,
    output logic [ 4:0] irq_id_o,

    // Debug Interface
    input logic debug_req_i,

    // sleep
    output logic core_sleep_o
);


  // CPU Control Signals
  logic fetch_enable;

  assign fetch_enable = 1'b1;

  assign core_instr_req_o.wdata = '0;
  assign core_instr_req_o.we    = '0;
  assign core_instr_req_o.be    = 4'b1111;

  if (CPU_TYPE == cv32e20) begin : gen_cv32e20

    logic [4:0] rf_raddr_a, rf_raddr_b, rf_waddr_wb;
    logic [31:0] rf_rdata_a, rf_rdata_b, rf_wdata_wb;
    logic rf_we_wb;

    import ibex_pkg::*;

    ibex_core #(
        .DmHaltAddr(DM_HALTADDRESS),
        .DmExceptionAddr(32'h0),
        .DbgTriggerEn(1'b1),
        .ResetAll(1'b1)
    ) cv32e20_i (
        .clk_i (clk_i),
        .rst_ni(rst_ni),

        .hart_id_i  (32'h0),
        .boot_addr_i(BOOT_ADDR),

        .instr_addr_o  (core_instr_req_o.addr),
        .instr_req_o   (core_instr_req_o.req),
        .instr_rdata_i (core_instr_resp_i.rdata),
        .instr_gnt_i   (core_instr_resp_i.gnt),
        .instr_rvalid_i(core_instr_resp_i.rvalid),
        .instr_err_i   (1'b0),

        .data_addr_o  (core_data_req_o.addr),
        .data_wdata_o (core_data_req_o.wdata),
        .data_we_o    (core_data_req_o.we),
        .data_req_o   (core_data_req_o.req),
        .data_be_o    (core_data_req_o.be),
        .data_rdata_i (core_data_resp_i.rdata),
        .data_gnt_i   (core_data_resp_i.gnt),
        .data_rvalid_i(core_data_resp_i.rvalid),
        .data_err_i   (1'b0),

        .dummy_instr_id_o (),
        .rf_raddr_a_o     (rf_raddr_a),
        .rf_raddr_b_o     (rf_raddr_b),
        .rf_waddr_wb_o    (rf_waddr_wb),
        .rf_we_wb_o       (rf_we_wb),
        .rf_wdata_wb_ecc_o(rf_wdata_wb),
        .rf_rdata_a_ecc_i (rf_rdata_a),
        .rf_rdata_b_ecc_i (rf_rdata_b),

        .ic_tag_req_o      (),
        .ic_tag_write_o    (),
        .ic_tag_addr_o     (),
        .ic_tag_wdata_o    (),
        .ic_tag_rdata_i    (),
        .ic_data_req_o     (),
        .ic_data_write_o   (),
        .ic_data_addr_o    (),
        .ic_data_wdata_o   (),
        .ic_data_rdata_i   (),
        .ic_scr_key_valid_i(),

        .irq_software_i(irq_i[3]),
        .irq_timer_i   (irq_i[7]),
        .irq_external_i(irq_i[11]),
        .irq_fast_i    (irq_i[30:16]),
        .irq_nm_i      (irq_i[31]),
        .irq_pending_o (),

        .debug_req_i(debug_req_i),
        .crash_dump_o(),
        .double_fault_seen_o(),

        .fetch_enable_i(fetch_enable),
        .alert_minor_o (),
        .alert_major_o (),
        .icache_inval_o(),
        .core_sleep_o
    );

    cv32e40p_register_file #(
        .ADDR_WIDTH(6)
    ) cv32e20_register_file_i (
        // Clock and Reset
        .clk  (clk_i),
        .rst_n(rst_ni),

        .scan_cg_en_i(1'b0),

        //Read port R1
        .raddr_a_i({1'b0, rf_raddr_a}),
        .rdata_a_o(rf_rdata_a),

        //Read port R2
        .raddr_b_i({1'b0, rf_raddr_b}),
        .rdata_b_o(rf_rdata_b),

        //Read port R3
        .raddr_c_i('0),
        .rdata_c_o(),

        // Write port W1
        .waddr_a_i({1'b0, rf_waddr_wb}),
        .wdata_a_i(rf_wdata_wb),
        .we_a_i(rf_we_wb),

        // Write port W2
        .waddr_b_i('0),
        .wdata_b_i('0),
        .we_b_i('0)
    );



    assign irq_ack_o = '0;
    assign irq_id_o  = '0;

  end else if (CPU_TYPE == cv32e40x) begin : gen_cv32e40x

    // instantiate the core
    cv32e40x_core #(
        .NUM_MHPMCOUNTERS(NUM_MHPMCOUNTERS),
        .X_EXT(X_EXT)
    ) cv32e40x_core_i (
        // Clock and reset
        .clk_i(clk_i),
        .rst_ni(rst_ni),
        .scan_cg_en_i(1'b0),

        // Static configuration
        .boot_addr_i(BOOT_ADDR),
        .dm_exception_addr_i(32'h0),
        .dm_halt_addr_i(DM_HALTADDRESS),
        .mhartid_i(32'h0),
        .mimpid_patch_i(4'h0),
        .mtvec_addr_i(32'h0),

        // Instruction memory interface
        .instr_req_o    (core_instr_req_o.req),
        .instr_gnt_i    (core_instr_resp_i.gnt),
        .instr_rvalid_i (core_instr_resp_i.rvalid),
        .instr_addr_o   (core_instr_req_o.addr),
        .instr_memtype_o(),
        .instr_prot_o   (),
        .instr_dbg_o    (),
        .instr_rdata_i  (core_instr_resp_i.rdata),
        .instr_err_i    (1'b0),

        // Data memory interface
        .data_req_o    (core_data_req_o.req),
        .data_gnt_i    (core_data_resp_i.gnt),
        .data_rvalid_i (core_data_resp_i.rvalid),
        .data_addr_o   (core_data_req_o.addr),
        .data_be_o     (core_data_req_o.be),
        .data_we_o     (core_data_req_o.we),
        .data_wdata_o  (core_data_req_o.wdata),
        .data_memtype_o(),
        .data_prot_o   (),
        .data_dbg_o    (),
        .data_atop_o   (),
        .data_rdata_i  (core_data_resp_i.rdata),
        .data_err_i    (1'b0),
        .data_exokay_i (1'b1),

        // Cycle count
        .mcycle_o(),

        // eXtension interface
        .xif_compressed_if,
        .xif_issue_if,
        .xif_commit_if,
        .xif_mem_if,
        .xif_mem_result_if,
        .xif_result_if,

        // Basic interrupt architecture
        .irq_i(irq_i),

        // Event wakeup signal
        .wu_wfe_i(1'b0),

        // Smclic interrupt architecture
        .clic_irq_i      (),
        .clic_irq_id_i   (),
        .clic_irq_level_i(),
        .clic_irq_priv_i (),
        .clic_irq_shv_i  (),

        // Fence.i flush handshake
        .fencei_flush_req_o(),
        .fencei_flush_ack_i(1'b0),

        // Debug interface
        .debug_req_i      (debug_req_i),
        .debug_havereset_o(),
        .debug_running_o  (),
        .debug_halted_o   (),

        // CPU control signals
        .fetch_enable_i(fetch_enable),
        .core_sleep_o
    );

    assign irq_ack_o = '0;
    assign irq_id_o  = '0;

  end else begin : gen_cv32e40p

    // instantiate the core
    cv32e40p_tb_wrapper #(
        .PULP_XPULP      (PULP_XPULP),
        .PULP_CLUSTER    (0),
        .FPU             (FPU),
        .PULP_ZFINX      (PULP_ZFINX),
        .NUM_MHPMCOUNTERS(NUM_MHPMCOUNTERS)
    ) cv32e40p_tb_wrapper_i (
        .clk_i (clk_i),
        .rst_ni(rst_ni),

        .pulp_clock_en_i(1'b1),
        .scan_cg_en_i   (1'b0),

        .boot_addr_i        (BOOT_ADDR),
        .mtvec_addr_i       (32'h0),
        .dm_halt_addr_i     (DM_HALTADDRESS),
        .hart_id_i          (32'h0),
        .dm_exception_addr_i(32'h0),

        .instr_addr_o  (core_instr_req_o.addr),
        .instr_req_o   (core_instr_req_o.req),
        .instr_rdata_i (core_instr_resp_i.rdata),
        .instr_gnt_i   (core_instr_resp_i.gnt),
        .instr_rvalid_i(core_instr_resp_i.rvalid),

        .data_addr_o  (core_data_req_o.addr),
        .data_wdata_o (core_data_req_o.wdata),
        .data_we_o    (core_data_req_o.we),
        .data_req_o   (core_data_req_o.req),
        .data_be_o    (core_data_req_o.be),
        .data_rdata_i (core_data_resp_i.rdata),
        .data_gnt_i   (core_data_resp_i.gnt),
        .data_rvalid_i(core_data_resp_i.rvalid),

        .irq_i    (irq_i),
        .irq_ack_o(irq_ack_o),
        .irq_id_o (irq_id_o),

        .debug_req_i      (debug_req_i),
        .debug_havereset_o(),
        .debug_running_o  (),
        .debug_halted_o   (),

        .fetch_enable_i(fetch_enable),
        .core_sleep_o
    );

  end

endmodule

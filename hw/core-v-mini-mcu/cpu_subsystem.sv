module cpu_subsystem import obi_pkg::*; #(
    parameter BOOT_ADDR     = 'h180,
    parameter PULP_XPULP    =  0,                   // PULP ISA Extension (incl. custom CSRs and hardware loop, excl. p.elw)
    parameter FPU = 0,  // Floating Point Unit (interfaced via APU interface)
    parameter PULP_ZFINX = 0,  // Float-in-General Purpose registers
    parameter NUM_MHPMCOUNTERS = 1,
    parameter DM_HALTADDRESS = '0
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

    // Interrupt inputs
    input  logic [31:0] irq_i,  // CLINT interrupts + CLINT extension interrupts
    output logic        irq_ack_o,
    output logic [ 4:0] irq_id_o,

    // Debug Interface
    input  logic debug_req_i,

    // CPU Control Signals
    input  logic fetch_enable_i
);

  import cv32e40p_apu_core_pkg::*;
  // APU Core to FP Wrapper
  logic                               apu_req;
  logic [    APU_NARGS_CPU-1:0][31:0] apu_operands;
  logic [      APU_WOP_CPU-1:0]       apu_op;
  logic [ APU_NDSFLAGS_CPU-1:0]       apu_flags;


  // APU FP Wrapper to Core
  logic                               apu_gnt;
  logic                               apu_rvalid;
  logic [                 31:0]       apu_rdata;
  logic [ APU_NUSFLAGS_CPU-1:0]       apu_rflags;



  assign core_instr_req_o.wdata = '0;
  assign core_instr_req_o.we    = '0;
  assign core_instr_req_o.be    = 4'b1111;

  // instantiate the core
  cv32e40p_wrapper #(
      .PULP_XPULP      (PULP_XPULP),
      .PULP_CLUSTER    (0),
      .FPU             (FPU),
      .PULP_ZFINX      (PULP_ZFINX),
      .NUM_MHPMCOUNTERS(NUM_MHPMCOUNTERS)
  ) cv32e40p_wrapper_i (
      .clk_i (clk_i),
      .rst_ni(rst_ni),

      .pulp_clock_en_i     (1'b1),
      .scan_cg_en_i        (1'b0),

      .boot_addr_i         (BOOT_ADDR),
      .mtvec_addr_i        (32'h0),
      .dm_halt_addr_i      (DM_HALTADDRESS),
      .hart_id_i           (32'h0),
      .dm_exception_addr_i (32'h0),

      .instr_addr_o        (core_instr_req_o.addr),
      .instr_req_o         (core_instr_req_o.req),
      .instr_rdata_i       (core_instr_resp_i.rdata),
      .instr_gnt_i         (core_instr_resp_i.gnt),
      .instr_rvalid_i      (core_instr_resp_i.rvalid),

      .data_addr_o         (core_data_req_o.addr),
      .data_wdata_o        (core_data_req_o.wdata),
      .data_we_o           (core_data_req_o.we),
      .data_req_o          (core_data_req_o.req),
      .data_be_o           (core_data_req_o.be),
      .data_rdata_i        (core_data_resp_i.rdata),
      .data_gnt_i          (core_data_resp_i.gnt),
      .data_rvalid_i       (core_data_resp_i.rvalid),

      .apu_req_o           (apu_req),
      .apu_gnt_i           (apu_gnt),
      .apu_operands_o      (apu_operands),
      .apu_op_o            (apu_op),
      .apu_flags_o         (apu_flags),
      .apu_rvalid_i        (apu_rvalid),
      .apu_result_i        (apu_rdata),
      .apu_flags_i         (apu_rflags),

      .irq_i               (irq_i),
      .irq_ack_o           (irq_ack_o),
      .irq_id_o            (irq_id_o),

      .debug_req_i         (debug_req_i),
      .debug_havereset_o   (),
      .debug_running_o     (),
      .debug_halted_o      (),

      .fetch_enable_i      (fetch_enable_i),
      .core_sleep_o        ()
  );

  generate
    if (FPU) begin
      cv32e40p_fp_wrapper fp_wrapper_i (
          .clk_i         (clk_i),
          .rst_ni        (rst_ni),
          .apu_req_i     (apu_req),
          .apu_gnt_o     (apu_gnt),
          .apu_operands_i(apu_operands),
          .apu_op_i      (apu_op),
          .apu_flags_i   (apu_flags),
          .apu_rvalid_o  (apu_rvalid),
          .apu_rdata_o   (apu_rdata),
          .apu_rflags_o  (apu_rflags)
      );
    end else begin
      assign apu_gnt      = '0;
      assign apu_operands = '0;
      assign apu_op       = '0;
      assign apu_flags    = '0;
      assign apu_rvalid   = '0;
      assign apu_rdata    = '0;
      assign apu_rflags   = '0;
    end
  endgenerate

endmodule

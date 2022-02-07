// Copyright 2018 Robert Balas <balasr@student.ethz.ch>
// Copyright and related rights are licensed under the Solderpad Hardware
// License, Version 0.51 (the "License"); you may not use this file except in
// compliance with the License.  You may obtain a copy of the License at
// http://solderpad.org/licenses/SHL-0.51. Unless required by applicable law
// or agreed to in writing, software, hardware and materials distributed under
// this License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

// Contributor: Robert Balas <balasr@student.ethz.ch>

module core_v_mini_mcu #(
    parameter JTAG_IDCODE = 32'h10001c05,
    parameter BOOT_ADDR = 'h180,
    parameter PULP_XPULP = 0,
    parameter PULP_CLUSTER = 0,
    parameter FPU = 0,
    parameter PULP_ZFINX = 0,
    parameter NUM_MHPMCOUNTERS = 1
) (
    input logic clk_i,
    input logic rst_ni,

    input  logic        jtag_tck_i,
    input  logic        jtag_tms_i,
    input  logic        jtag_trst_ni,
    input  logic        jtag_tdi_i,
    output logic        jtag_tdo_o,

    input  logic        fetch_enable_i,
    output logic [31:0] exit_value_o,
    output logic        exit_valid_o
);

  `include "tb_util.svh"

  import core_v_mini_mcu_pkg::*;
  import obi_pkg::*;
  import cv32e40p_apu_core_pkg::*;

  localparam NUM_BYTES      = 2**16; //must be 2**16, it is not a parameter!!!
  localparam DM_HALTADDRESS = core_v_mini_mcu_pkg::DEBUG_START_ADDRESS + 32'h00000800; //debug rom code (section .text in linker) starts at 0x800

  // signals connecting core to memory

  obi_req_t     core_instr_req;
  obi_resp_t    core_instr_resp;
  obi_req_t     core_data_req;
  obi_resp_t    core_data_resp;
  obi_req_t     debug_master_req;
  obi_resp_t    debug_master_resp;

  obi_req_t     ram0_slave_req;
  obi_resp_t    ram0_slave_resp;
  obi_req_t     ram1_slave_req;
  obi_resp_t    ram1_slave_resp;
  obi_req_t     debug_slave_req;
  obi_resp_t    debug_slave_resp;
  obi_req_t     peripheral_slave_req;
  obi_resp_t    peripheral_slave_resp;

  // signals to debug unit
  logic                               debug_core_req;

  // irq signals
  logic                               irq_ack;
  logic [                  4:0]       irq_id_out;
  logic                               irq_software;
  logic                               irq_timer;
  logic                               irq_external;
  logic [                 15:0]       irq_fast;

  logic                               core_sleep_o;

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



  assign core_instr_req.wdata = '0;
  assign core_instr_req.we    = '0;
  assign core_instr_req.be    = 4'b1111;

  // instantiate the core
  cv32e40p_wrapper #(
      .PULP_XPULP      (PULP_XPULP),
      .PULP_CLUSTER    (PULP_CLUSTER),
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

      .instr_addr_o        (core_instr_req.addr),
      .instr_req_o         (core_instr_req.req),
      .instr_rdata_i       (core_instr_resp.rdata),
      .instr_gnt_i         (core_instr_resp.gnt),
      .instr_rvalid_i      (core_instr_resp.rvalid),

      .data_addr_o         (core_data_req.addr),
      .data_wdata_o        (core_data_req.wdata),
      .data_we_o           (core_data_req.we),
      .data_req_o          (core_data_req.req),
      .data_be_o           (core_data_req.be),
      .data_rdata_i        (core_data_resp.rdata),
      .data_gnt_i          (core_data_resp.gnt),
      .data_rvalid_i       (core_data_resp.rvalid),

      .apu_req_o           (apu_req),
      .apu_gnt_i           (apu_gnt),
      .apu_operands_o      (apu_operands),
      .apu_op_o            (apu_op),
      .apu_flags_o         (apu_flags),
      .apu_rvalid_i        (apu_rvalid),
      .apu_result_i        (apu_rdata),
      .apu_flags_i         (apu_rflags),

      .irq_i               ({irq_fast, 4'b0, irq_external, 3'b0, irq_timer, 3'b0, irq_software, 3'b0}),
      .irq_ack_o           (irq_ack),
      .irq_id_o            (irq_id_out),

      .debug_req_i         (debug_core_req),
      .debug_havereset_o   (),
      .debug_running_o     (),
      .debug_halted_o      (),

      .fetch_enable_i      (fetch_enable_i),
      .core_sleep_o        (core_sleep_o)
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


  debug_subsystem #(
      .JTAG_IDCODE(JTAG_IDCODE)
  ) debug_subsystem_i (
    .clk_i               ( clk_i            ),
    .rst_ni              ( rst_ni           ),

    .jtag_tck_i          ( jtag_tck_i       ),
    .jtag_tms_i          ( jtag_tms_i       ),
    .jtag_trst_ni        ( jtag_trst_ni     ),
    .jtag_tdi_i          ( jtag_tdi_i       ),
    .jtag_tdo_o          ( jtag_tdo_o       ),

    .debug_core_req_o    ( debug_core_req   ),

    .debug_slave_req_i   ( debug_slave_req   ),
    .debug_slave_resp_o  ( debug_slave_resp  ),
    .debug_master_req_o  ( debug_master_req  ),
    .debug_master_resp_i ( debug_master_resp )

  );

  system_bus #(
      .NUM_BYTES(NUM_BYTES)
  ) system_bus_i (
    .clk_i (clk_i),
    .rst_ni(rst_ni),

    .core_instr_req_i        ( core_instr_req        ),
    .core_instr_resp_o       ( core_instr_resp       ),
    .core_data_req_i         ( core_data_req         ),
    .core_data_resp_o        ( core_data_resp        ),
    .debug_master_req_i      ( debug_master_req      ),
    .debug_master_resp_o     ( debug_master_resp     ),

    .ram0_req_o              ( ram0_slave_req        ),
    .ram0_resp_i             ( ram0_slave_resp       ),
    .ram1_req_o              ( ram1_slave_req        ),
    .ram1_resp_i             ( ram1_slave_resp       ),
    .debug_slave_req_o       ( debug_slave_req       ),
    .debug_slave_resp_i      ( debug_slave_resp      ),
    .peripheral_slave_req_o  ( peripheral_slave_req  ),
    .peripheral_slave_resp_i ( peripheral_slave_resp )

  );

  memory_subsystem #(
      .NUM_BYTES(NUM_BYTES)
  ) memory_subsystem_i (
    .clk_i,
    .rst_ni,
    .ram0_req_i(ram0_slave_req),
    .ram0_resp_o(ram0_slave_resp),
    .ram1_req_i(ram1_slave_req),
    .ram1_resp_o(ram1_slave_resp)
  );

  peripheral_subsystem peripheral_subsystem_i
  (
    .clk_i,
    .rst_ni,

    .slave_req_i(peripheral_slave_req),
    .slave_resp_o(peripheral_slave_resp),

    .exit_valid_o  (exit_valid_o),
    .exit_value_o  (exit_value_o),

    .uart_rx_i('0),
    .uart_tx_o(),
    .uart_tx_en_o(),
    .uart_intr_tx_watermark_o() ,
    .uart_intr_rx_watermark_o() ,
    .uart_intr_tx_empty_o()  ,
    .uart_intr_rx_overflow_o()  ,
    .uart_intr_rx_frame_err_o() ,
    .uart_intr_rx_break_err_o() ,
    .uart_intr_rx_timeout_o()   ,
    .uart_intr_rx_parity_err_o()
  );

  assign irq_ack      = '0;
  assign irq_id_out   = '0;
  assign irq_software = '0;
  assign irq_timer    = '0;
  assign irq_external = '0;
  assign irq_fast     = '0;


endmodule  // cv32e40p_tb_subsystem

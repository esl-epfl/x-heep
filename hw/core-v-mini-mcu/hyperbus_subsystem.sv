// Copyright 2024 EPFL
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Davide Schiavone <davide.schiavone@epfl.ch>

module hyperbus_subsystem
  import obi_pkg::*;
  import reg_pkg::*;
  import core_v_mini_mcu_pkg::*;
#(
    parameter int unsigned NumChips = core_v_mini_mcu_pkg::HyperRamNumChips,
    parameter int unsigned NumPhys  = core_v_mini_mcu_pkg::HyperRamNumPhys
) (
    input  logic                                  clk_i,
    input  logic                                  clk_per_i,
    input  logic                                  rst_ni,
    // OBI bus
    input  obi_req_t                              obi_req_i,
    output obi_resp_t                             obi_resp_o,
    // Reg bus
    input  reg_req_t                              reg_req_i,
    output reg_rsp_t                              reg_rsp_o,
    // Physical interace: facing HyperBus PADs
    output logic      [NumPhys-1:0][NumChips-1:0] hyper_cs_no,
    output logic      [NumPhys-1:0]               hyper_ck_o,
    output logic      [NumPhys-1:0]               hyper_ck_no,
    output logic      [NumPhys-1:0]               hyper_rwds_o,
    input  logic      [NumPhys-1:0]               hyper_rwds_i,
    output logic      [NumPhys-1:0]               hyper_rwds_oe_o,
    input  logic      [NumPhys-1:0][         7:0] hyper_dq_i,
    output logic      [NumPhys-1:0][         7:0] hyper_dq_o,
    output logic      [NumPhys-1:0]               hyper_dq_oe_o,
    output logic      [NumPhys-1:0]               hyper_reset_no
);

  import axi_pkg::*;

  core_v_mini_mcu_pkg::axi_req_t  axi_req;
  core_v_mini_mcu_pkg::axi_resp_t axi_resp;
  localparam int unsigned AddrMapHyperBusIdxWidth = (NumChips > 1) ? $clog2(NumChips) : 1;

  typedef struct packed {
    logic [AddrMapHyperBusIdxWidth-1:0] idx;
    logic [1:0] start_addr;
    logic [1:0] end_addr;
  } addr_map_hyperbus_t;

  // Instantiate the HyperBus controller
  hyperbus #(
      .NumChips(NumChips),
      .NumPhys(NumPhys),
      .AxiAddrWidth(core_v_mini_mcu_pkg::AxiAddrWidth),
      .AxiDataWidth(core_v_mini_mcu_pkg::AxiDataWidth),
      .AxiIdWidth(core_v_mini_mcu_pkg::AxiIdWidth),
      .AxiUserWidth(core_v_mini_mcu_pkg::AxiUserWidth),
      .axi_req_t(core_v_mini_mcu_pkg::axi_req_t),
      .axi_rsp_t(core_v_mini_mcu_pkg::axi_resp_t),
      .axi_w_chan_t(core_v_mini_mcu_pkg::axi_w_t),
      .axi_b_chan_t(core_v_mini_mcu_pkg::axi_b_t),
      .axi_ar_chan_t(core_v_mini_mcu_pkg::axi_ar_t),
      .axi_r_chan_t(core_v_mini_mcu_pkg::axi_r_t),
      .axi_aw_chan_t(core_v_mini_mcu_pkg::axi_aw_t),
      .RegDataWidth(32),
      .reg_req_t(reg_pkg::reg_req_t),
      .reg_rsp_t(reg_pkg::reg_rsp_t),
      .axi_rule_t(addr_map_hyperbus_t)
  ) hyperbus_i (
      .clk_phy_i  (clk_per_i),
      .rst_phy_ni (rst_ni),
      .clk_sys_i  (clk_i),
      .rst_sys_ni (rst_ni),
      .test_mode_i(1'b0),
      .axi_req_i  (axi_req),
      .axi_rsp_o  (axi_resp),
      .reg_req_i,
      .reg_rsp_o,
      .hyper_cs_no,
      .hyper_ck_o,
      .hyper_ck_no,
      .hyper_rwds_o,
      .hyper_rwds_i,
      .hyper_rwds_oe_o,
      .hyper_dq_i,
      .hyper_dq_o,
      .hyper_dq_oe_o,
      .hyper_reset_no
  );

  axi_from_mem #(
      .MemAddrWidth(32),
      .AxiAddrWidth(32),
      .DataWidth(32),
      .MaxRequests(1),
      .axi_req_t(core_v_mini_mcu_pkg::axi_req_t),
      .axi_rsp_t(core_v_mini_mcu_pkg::axi_resp_t)
  ) axi_from_mem_i (
      .clk_i,
      .rst_ni,
      .mem_req_i(obi_req_i.req),
      .mem_addr_i(obi_req_i.addr),
      .mem_we_i(obi_req_i.we),
      .mem_wdata_i(obi_req_i.wdata),
      .mem_be_i(obi_req_i.be),
      .mem_gnt_o(obi_resp_o.gnt),
      .mem_rsp_valid_o(obi_resp_o.rvalid),
      .mem_rsp_rdata_o(obi_resp_o.rdata),
      .mem_rsp_error_o(),
      .slv_aw_cache_i('0),
      .slv_ar_cache_i('0),
      .axi_req_o(axi_req),
      .axi_rsp_i(axi_resp)
  );

`ifndef SYNTHESIS

  /*
    this version of axi from mem uses axi_lite_from_mem
    as axi lite does not support any transaction size but the buswidth (32b for X-HEEP)
    you cannot access the HyperRam with data bidwidth < 32b (e.g. 8 and 16)

    Future fixes will come by bypassing the axi_lite bridge and using the OBI BE (byte enable) to infer the size.
    For now, this limitation must be taken into account in SW - so if 8b or 16b data needs to be used, we recocomend threat them packet

    further constraints have been added by checking that the OBI BE must be always 4'b1111 in Simulation (but not FPGA or ASIC - thus be careful)

  */

  always_ff @(posedge clk_i, negedge rst_ni) begin : check_out_of_bound
    if (rst_ni) begin
      if (obi_req_i.req && ((obi_req_i.addr[1:0] != 2'b00) || obi_req_i.be != 4'b1111)) begin
        $display("%t wrong HyperRam access 0x%08x (be: %x)", $time, obi_req_i.addr, obi_req_i.be);
        $stop;
      end
    end
  end
`endif

endmodule : hyperbus_subsystem

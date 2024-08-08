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
      .RegAddrWidth(32),
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

  per2axi #(
      .NB_CORES(1),
      .AXI_ADDR_WIDTH(core_v_mini_mcu_pkg::AxiAddrWidth),
      .AXI_DATA_WIDTH(core_v_mini_mcu_pkg::AxiDataWidth),
      .AXI_USER_WIDTH(core_v_mini_mcu_pkg::AxiUserWidth),
      .AXI_ID_WIDTH(core_v_mini_mcu_pkg::AxiIdWidth),
      .AXI_STRB_WIDTH(core_v_mini_mcu_pkg::AxiStrbWidth)
  ) per2axi_i (

      .clk_i,
      .rst_ni,
      .test_en_i(1'b0),

      //OBI CHANNEL
      .per_slave_req_i(obi_req_i.req),
      .per_slave_add_i(obi_req_i.addr),
      .per_slave_we_i(obi_req_i.we),
      .per_slave_wdata_i(obi_req_i.wdata),
      .per_slave_be_i(obi_req_i.be),
      .per_slave_id_i('0),  //TODO check if correct
      .per_slave_gnt_o(obi_resp_o.gnt),
      .per_slave_r_valid_o(obi_resp_o.rvalid),
      .per_slave_r_rdata_o(obi_resp_o.rdata),
      .per_slave_r_opc_o(),
      .per_slave_r_id_o(),

      // AXI4 CHANNEL

      // WRITE ADDRESS CHANNEL
      .axi_master_aw_valid_o(axi_req.aw_valid),
      .axi_master_aw_addr_o(axi_req.aw.addr),
      .axi_master_aw_prot_o(axi_req.aw.prot),
      .axi_master_aw_region_o(axi_req.aw.region),
      .axi_master_aw_len_o(axi_req.aw.len),
      .axi_master_aw_size_o(axi_req.aw.size),
      .axi_master_aw_burst_o(axi_req.aw.burst),
      .axi_master_aw_lock_o(axi_req.aw.lock),
      .axi_master_aw_cache_o(axi_req.aw.cache),
      .axi_master_aw_qos_o(axi_req.aw.qos),
      .axi_master_aw_id_o(axi_req.aw.id),
      .axi_master_aw_user_o(axi_req.aw.user),
      .axi_master_aw_ready_i(axi_resp.aw_ready),

      // READ ADDRESS CHANNEL
      .axi_master_ar_valid_o(axi_req.ar_valid),
      .axi_master_ar_addr_o(axi_req.ar.addr),
      .axi_master_ar_prot_o(axi_req.ar.prot),
      .axi_master_ar_region_o(axi_req.ar.region),
      .axi_master_ar_len_o(axi_req.ar.len),
      .axi_master_ar_size_o(axi_req.ar.size),
      .axi_master_ar_burst_o(axi_req.ar.burst),
      .axi_master_ar_lock_o(axi_req.ar.lock),
      .axi_master_ar_cache_o(axi_req.ar.cache),
      .axi_master_ar_qos_o(axi_req.ar.qos),
      .axi_master_ar_id_o(axi_req.ar.id),
      .axi_master_ar_user_o(axi_req.ar.user),
      .axi_master_ar_ready_i(axi_resp.ar_ready),

      // WRITE DATA CHANNEL
      .axi_master_w_valid_o(axi_req.w_valid),
      .axi_master_w_data_o (axi_req.w.data),
      .axi_master_w_strb_o (axi_req.w.strb),
      .axi_master_w_user_o (axi_req.w.user),
      .axi_master_w_last_o (axi_req.w.last),
      .axi_master_w_ready_i(axi_resp.w_ready),

      // READ DATA CHANNEL
      .axi_master_r_valid_i(axi_resp.r_valid),
      .axi_master_r_data_i(axi_resp.r.data),
      .axi_master_r_resp_i(axi_resp.r.resp),
      .axi_master_r_last_i(axi_resp.r.last),
      .axi_master_r_id_i(axi_resp.r.id),
      .axi_master_r_user_i(axi_resp.r.user),
      .axi_master_r_ready_o(axi_req.r_ready),

      // WRITE RESPONSE CHANNEL
      .axi_master_b_valid_i(axi_resp.b_valid),
      .axi_master_b_resp_i(axi_resp.b.resp),
      .axi_master_b_id_i(axi_resp.b.id),
      .axi_master_b_user_i(axi_resp.b.user),
      .axi_master_b_ready_o(axi_req.b_ready),

      .busy_o()
  );

endmodule : hyperbus_subsystem

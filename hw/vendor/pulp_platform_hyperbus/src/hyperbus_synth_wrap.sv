// Copyright 2023 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Thomas Benz <paulsc@iis.ee.ethz.ch>
// Paul Scheffler <paulsc@iis.ee.ethz.ch>

`include "axi/assign.svh"
`include "axi/typedef.svh"
`include "register_interface/typedef.svh"

module hyperbus_lint_wrap #(
    // HyperBus parameters
    parameter int unsigned  NumChips        = 2,
    parameter int unsigned  NumPhys         = 2,
    parameter int unsigned  IsClockODelayed = 0,
    // AXI parameters
    parameter int unsigned  AxiIdWidth      = 6,
    parameter int unsigned  AxiAddrWidth    = 48,
    parameter int unsigned  AxiDataWidth    = 128,
    parameter int unsigned  AxiUserWidth    = 1,
    // Regbus parameters
    parameter int unsigned  RegAddrWidth    = 32,
    parameter int unsigned  RegDataWidth    = 32,
    // Dependent parameters; do not override!
    parameter type axi_addr_t   = logic [AxiAddrWidth-1:0],
    parameter type axi_data_t   = logic [AxiDataWidth-1:0],
    parameter type axi_strb_t   = logic [AxiDataWidth/8-1:0],
    parameter type axi_id_t     = logic [AxiIdWidth-1:0],
    parameter type axi_user_t   = logic [AxiUserWidth-1:0],
    parameter type reg_addr_t   = logic [RegAddrWidth-1:0],
    parameter type reg_data_t   = logic [RegDataWidth-1:0],
    parameter type reg_strb_t   = logic [RegDataWidth/8-1:0]
) (
    // SoC
    input  logic                        clk_phy_i,
    input  logic                        rst_phy_ni,
    input  logic                        clk_sys_i,
    input  logic                        rst_sys_ni,
    input  logic                        test_mode_i,

    // AXI bus
    input  axi_id_t                     axi_aw_id_i,
    input  axi_addr_t                   axi_aw_addr_i,
    input  axi_pkg::len_t               axi_aw_len_i,
    input  axi_pkg::size_t              axi_aw_size_i,
    input  axi_pkg::burst_t             axi_aw_burst_i,
    input  logic                        axi_aw_lock_i,
    input  axi_pkg::cache_t             axi_aw_cache_i,
    input  axi_pkg::prot_t              axi_aw_prot_i,
    input  axi_pkg::qos_t               axi_aw_qos_i,
    input  axi_pkg::region_t            axi_aw_region_i,
    input  axi_pkg::atop_t              axi_aw_atop_i,
    input  axi_user_t                   axi_aw_user_i,
    input  logic                        axi_aw_valid_i,
    output logic                        axi_aw_ready_o,
    input  axi_data_t                   axi_w_data_i,
    input  axi_strb_t                   axi_w_strb_i,
    input  logic                        axi_w_last_i,
    input  axi_user_t                   axi_w_user_i,
    input  logic                        axi_w_valid_i,
    output logic                        axi_w_ready_o,
    output axi_id_t                     axi_b_id_o,
    output axi_pkg::resp_t              axi_b_resp_o,
    output axi_user_t                   axi_b_user_o,
    output logic                        axi_b_valid_o,
    input  logic                        axi_b_ready_i,
    input  axi_id_t                     axi_ar_id_i,
    input  axi_addr_t                   axi_ar_addr_i,
    input  axi_pkg::len_t               axi_ar_len_i,
    input  axi_pkg::size_t              axi_ar_size_i,
    input  axi_pkg::burst_t             axi_ar_burst_i,
    input  logic                        axi_ar_lock_i,
    input  axi_pkg::cache_t             axi_ar_cache_i,
    input  axi_pkg::prot_t              axi_ar_prot_i,
    input  axi_pkg::qos_t               axi_ar_qos_i,
    input  axi_pkg::region_t            axi_ar_region_i,
    input  axi_user_t                   axi_ar_user_i,
    input  logic                        axi_ar_valid_i,
    output logic                        axi_ar_ready_o,
    output axi_id_t                     axi_r_id_o,
    output axi_data_t                   axi_r_data_o,
    output axi_pkg::resp_t              axi_r_resp_o,
    output logic                        axi_r_last_o,
    output axi_user_t                   axi_r_user_o,
    output logic                        axi_r_valid_o,
    input  logic                        axi_r_ready_i,

    // Reg bus
    input  reg_addr_t                   rbus_req_addr_i,
    input  logic                        rbus_req_write_i,
    input  reg_data_t                   rbus_req_wdata_i,
    input  reg_strb_t                   rbus_req_wstrb_i,
    input  logic                        rbus_req_valid_i,
    output reg_data_t                   rbus_rsp_rdata_o,
    output logic                        rbus_rsp_ready_o,
    output logic                        rbus_rsp_error_o,

    // PHY interface
    output logic [NumPhys-1:0][NumChips-1:0] hyper_cs_no,
    output logic [NumPhys-1:0]               hyper_ck_o,
    output logic [NumPhys-1:0]               hyper_ck_no,
    output logic [NumPhys-1:0]               hyper_rwds_o,
    input  logic [NumPhys-1:0]               hyper_rwds_i,
    output logic [NumPhys-1:0]               hyper_rwds_oe_o,
    input  logic [NumPhys-1:0][7:0]          hyper_dq_i,
    output logic [NumPhys-1:0][7:0]          hyper_dq_o,
    output logic [NumPhys-1:0]               hyper_dq_oe_o,
    output logic [NumPhys-1:0]               hyper_reset_no
);

    // Types
    `AXI_TYPEDEF_AW_CHAN_T(aw_chan_t, axi_addr_t, axi_id_t, axi_user_t)
    `AXI_TYPEDEF_W_CHAN_T(w_chan_t, axi_data_t, axi_strb_t, axi_user_t)
    `AXI_TYPEDEF_B_CHAN_T(b_chan_t, axi_id_t, axi_user_t)
    `AXI_TYPEDEF_AR_CHAN_T(ar_chan_t, axi_addr_t, axi_id_t, axi_user_t)
    `AXI_TYPEDEF_R_CHAN_T(r_chan_t, axi_data_t, axi_id_t, axi_user_t)
    `AXI_TYPEDEF_REQ_T(axi_req_t, aw_chan_t, w_chan_t, ar_chan_t)
    `AXI_TYPEDEF_RESP_T(axi_rsp_t, b_chan_t, r_chan_t)

    axi_req_t   axi_req;
    axi_rsp_t  axi_rsp;

    `REG_BUS_TYPEDEF_REQ(reg_req_t, reg_addr_t, reg_data_t, reg_strb_t)
    `REG_BUS_TYPEDEF_RSP(reg_rsp_t, reg_data_t)

    reg_req_t   reg_req;
    reg_rsp_t   reg_rsp;

    typedef axi_pkg::xbar_rule_32_t axi_rule_t;

    // Wrapped instance
    hyperbus #(
        .NumChips        ( NumChips        ),
        .NumPhys         ( NumPhys         ),
        .IsClockODelayed ( IsClockODelayed ),
        .AxiAddrWidth    ( AxiAddrWidth    ),
        .AxiDataWidth    ( AxiDataWidth    ),
        .AxiIdWidth      ( AxiIdWidth      ),
        .AxiUserWidth    ( AxiUserWidth    ),
        .axi_req_t       ( axi_req_t       ),
        .axi_rsp_t       ( axi_rsp_t       ),
        .axi_w_chan_t    ( w_chan_t        ),
        .axi_b_chan_t    ( b_chan_t        ),
        .axi_ar_chan_t   ( ar_chan_t       ),
        .axi_r_chan_t    ( r_chan_t        ),
        .axi_aw_chan_t   ( aw_chan_t       ),
        .RegAddrWidth    ( RegAddrWidth    ),
        .RegDataWidth    ( RegDataWidth    ),
        .reg_req_t       ( reg_req_t       ),
        .reg_rsp_t       ( reg_rsp_t       ),
        .axi_rule_t      ( axi_rule_t      )
    ) i_hyperbus (
        .clk_phy_i       ( clk_phy_i       ),
        .rst_phy_ni      ( rst_phy_ni      ),
        .clk_sys_i       ( clk_sys_i       ),
        .rst_sys_ni      ( rst_sys_ni      ),
        .test_mode_i     ( test_mode_i     ),
        .axi_req_i       ( axi_req         ),
        .axi_rsp_o       ( axi_rsp         ),
        .reg_req_i       ( reg_req         ),
        .reg_rsp_o       ( reg_rsp         ),
        .hyper_cs_no     ( hyper_cs_no     ),
        .hyper_ck_o      ( hyper_ck_o      ),
        .hyper_ck_no     ( hyper_ck_no     ),
        .hyper_rwds_o    ( hyper_rwds_o    ),
        .hyper_rwds_i    ( hyper_rwds_i    ),
        .hyper_rwds_oe_o ( hyper_rwds_oe_o ),
        .hyper_dq_i      ( hyper_dq_i      ),
        .hyper_dq_o      ( hyper_dq_o      ),
        .hyper_dq_oe_o   ( hyper_dq_oe_o   ),
        .hyper_reset_no  ( hyper_reset_no  )
    );

    // AXI Slave
    assign axi_req.aw.id        = axi_aw_id_i;
    assign axi_req.aw.addr      = axi_aw_addr_i;
    assign axi_req.aw.len       = axi_aw_len_i;
    assign axi_req.aw.size      = axi_aw_size_i;
    assign axi_req.aw.burst     = axi_aw_burst_i;
    assign axi_req.aw.lock      = axi_aw_lock_i;
    assign axi_req.aw.cache     = axi_aw_cache_i;
    assign axi_req.aw.prot      = axi_aw_prot_i;
    assign axi_req.aw.qos       = axi_aw_qos_i;
    assign axi_req.aw.region    = axi_aw_region_i;
    assign axi_req.aw.atop      = axi_aw_atop_i;
    assign axi_req.aw.user      = axi_aw_user_i;
    assign axi_req.aw_valid     = axi_aw_valid_i;
    assign axi_aw_ready_o       = axi_rsp.aw_ready;
    assign axi_req.w.data       = axi_w_data_i;
    assign axi_req.w.strb       = axi_w_strb_i;
    assign axi_req.w.last       = axi_w_last_i;
    assign axi_req.w.user       = axi_w_user_i;
    assign axi_req.w_valid      = axi_w_valid_i;
    assign axi_w_ready_o        = axi_rsp.w_ready;
    assign axi_b_id_o           = axi_rsp.b.id;
    assign axi_b_resp_o         = axi_rsp.b.resp;
    assign axi_b_user_o         = axi_rsp.b.user;
    assign axi_b_valid_o        = axi_rsp.b_valid;
    assign axi_req.b_ready      = axi_b_ready_i;
    assign axi_req.ar.id        = axi_ar_id_i;
    assign axi_req.ar.addr      = axi_ar_addr_i;
    assign axi_req.ar.len       = axi_ar_len_i;
    assign axi_req.ar.size      = axi_ar_size_i;
    assign axi_req.ar.burst     = axi_ar_burst_i;
    assign axi_req.ar.lock      = axi_ar_lock_i;
    assign axi_req.ar.cache     = axi_ar_cache_i;
    assign axi_req.ar.prot      = axi_ar_prot_i;
    assign axi_req.ar.qos       = axi_ar_qos_i;
    assign axi_req.ar.region    = axi_ar_region_i;
    assign axi_req.ar.user      = axi_ar_user_i;
    assign axi_req.ar_valid     = axi_ar_valid_i;
    assign axi_ar_ready_o       = axi_rsp.ar_ready;
    assign axi_r_id_o           = axi_rsp.r.id;
    assign axi_r_data_o         = axi_rsp.r.data;
    assign axi_r_resp_o         = axi_rsp.r.resp;
    assign axi_r_last_o         = axi_rsp.r.last;
    assign axi_r_user_o         = axi_rsp.r.user;
    assign axi_r_valid_o        = axi_rsp.r_valid;
    assign axi_req.r_ready      = axi_r_ready_i;

    // Regbus slave
    assign reg_req.addr         = rbus_req_addr_i;
    assign reg_req.write        = rbus_req_write_i;
    assign reg_req.wdata        = rbus_req_wdata_i;
    assign reg_req.wstrb        = rbus_req_wstrb_i;
    assign reg_req.valid        = rbus_req_valid_i;
    assign rbus_rsp_rdata_o     = reg_rsp.rdata;
    assign rbus_rsp_ready_o     = reg_rsp.ready;
    assign rbus_rsp_error_o     = reg_rsp.error;

endmodule : hyperbus_lint_wrap

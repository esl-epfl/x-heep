// Copyright 2023 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Thomas Benz <paulsc@iis.ee.ethz.ch>
// Paul Scheffler <paulsc@iis.ee.ethz.ch>

module hyperbus #(
    parameter int unsigned  NumChips        = -1,
    parameter int unsigned  AxiDataWidth    = -1,
    parameter int unsigned  AxiIdWidth      = -1,
    parameter type          axi_req_t       = logic,
    parameter type          axi_rsp_t       = logic,
    parameter type          axi_rule_t      = logic
) (
    input  logic                        clk_phy_i,
    input  logic                        clk_sys_i,
    input  logic                        rst_ni,
    input  logic                        test_mode_i,
    // AXI bus
    input  axi_req_t                    axi_req_i,
    output axi_rsp_t                    axi_rsp_o,
    // Reg bus
    input  reg_intf_pkg::req_a32_d32    reg_req_i,
    output reg_intf_pkg::rsp_d32        reg_rsp_o,
    // PHY interface
    output logic [NumChips-1:0]         hyper_cs_no,
    output logic                        hyper_ck_o,
    output logic                        hyper_ck_no,
    output logic                        hyper_rwds_o,
    input  logic                        hyper_rwds_i,
    output logic                        hyper_rwds_oe_o,
    input  logic [7:0]                  hyper_dq_i,
    output logic [7:0]                  hyper_dq_o,
    output logic                        hyper_dq_oe_o,
    output logic                        hyper_reset_no,
    // Debug interface
    output logic                        debug_hyper_rwds_oe_o,
    output logic                        debug_hyper_dq_oe_o,
    output logic [3:0]                  debug_hyper_phy_state_o
);

    assign axi_rsp_o                 = '0;
    assign reg_rsp_o                 = '0;
    assign hyper_cs_no               = '0;
    assign hyper_ck_o                = '0;
    assign hyper_ck_no               = '0;
    assign hyper_rwds_o              = '0;
    assign hyper_rwds_oe_o           = '0;
    assign hyper_dq_o                = '0;
    assign hyper_dq_oe_o             = '0;
    assign hyper_reset_no            = '0;
    assign debug_hyper_rwds_oe_o     = '0;
    assign debug_hyper_dq_oe_o       = '0;
    assign debug_hyper_phy_state_o   = '0;

endmodule : hyperbus

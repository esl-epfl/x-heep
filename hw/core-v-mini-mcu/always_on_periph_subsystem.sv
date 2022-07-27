// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module always_on_periph_subsystem
  import obi_pkg::*;
  import reg_pkg::*;
(
    input logic clk_i,
    input logic rst_ni,

    input  obi_req_t  slave_req_i,
    output obi_resp_t slave_resp_o,

    //SOC CTRL
    output logic        exit_valid_o,
    output logic [31:0] exit_value_o,

    //RV TIMER
    output logic rv_timer_irq_timer_o
);

  import core_v_mini_mcu_pkg::*;
  import tlul_pkg::*;
  import rv_plic_reg_pkg::*;

  reg_pkg::reg_req_t peripheral_req;
  reg_pkg::reg_rsp_t peripheral_rsp;

  reg_pkg::reg_req_t [core_v_mini_mcu_pkg::ALWAYS_ON_PERIPHERALS-1:0] peripheral_slv_req;
  reg_pkg::reg_rsp_t [core_v_mini_mcu_pkg::ALWAYS_ON_PERIPHERALS-1:0] peripheral_slv_rsp;

  tlul_pkg::tl_h2d_t rv_timer_tl_h2d;
  tlul_pkg::tl_d2h_t rv_timer_tl_d2h;

  //Address Decoder
  logic [ALWAYS_ON_PERIPHERALS_PORT_SEL_WIDTH-1:0] peripheral_select;

  periph_to_reg #(
      .req_t(reg_pkg::reg_req_t),
      .rsp_t(reg_pkg::reg_rsp_t),
      .IW(1)
  ) periph_to_reg_i (
      .clk_i,
      .rst_ni,
      .req_i(slave_req_i.req),
      .add_i(slave_req_i.addr),
      .wen_i(~slave_req_i.we),
      .wdata_i(slave_req_i.wdata),
      .be_i(slave_req_i.be),
      .id_i('0),
      .gnt_o(slave_resp_o.gnt),
      .r_rdata_o(slave_resp_o.rdata),
      .r_opc_o(),
      .r_id_o(),
      .r_valid_o(slave_resp_o.rvalid),
      .reg_req_o(peripheral_req),
      .reg_rsp_i(peripheral_rsp)
  );

  addr_decode #(
      .NoIndices(core_v_mini_mcu_pkg::ALWAYS_ON_PERIPHERALS),
      .NoRules(core_v_mini_mcu_pkg::ALWAYS_ON_PERIPHERALS),
      .addr_t(logic [31:0]),
      .rule_t(addr_map_rule_pkg::addr_map_rule_t)
  ) i_addr_decode_soc_regbus_periph_xbar (
      .addr_i(peripheral_req.addr),
      .addr_map_i(core_v_mini_mcu_pkg::ALWAYS_ON_PERIPHERALS_ADDR_RULES),
      .idx_o(peripheral_select),
      .dec_valid_o(),
      .dec_error_o(),
      .en_default_idx_i(1'b0),
      .default_idx_i('0)
  );

  reg_demux #(
      .NoPorts(core_v_mini_mcu_pkg::ALWAYS_ON_PERIPHERALS),
      .req_t  (reg_pkg::reg_req_t),
      .rsp_t  (reg_pkg::reg_rsp_t)
  ) reg_demux_i (
      .clk_i,
      .rst_ni,
      .in_select_i(peripheral_select),
      .in_req_i(peripheral_req),
      .in_rsp_o(peripheral_rsp),
      .out_req_o(peripheral_slv_req),
      .out_rsp_i(peripheral_slv_rsp)
  );

  soc_ctrl #(
      .reg_req_t(reg_pkg::reg_req_t),
      .reg_rsp_t(reg_pkg::reg_rsp_t)
  ) soc_ctrl_i (
      .clk_i,
      .rst_ni,
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::SOC_CTRL_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::SOC_CTRL_IDX]),
      .exit_valid_o,
      .exit_value_o
  );

  reg_to_tlul rv_timer_reg_to_tlul_i (
      .tl_o(rv_timer_tl_h2d),
      .tl_i(rv_timer_tl_d2h),
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::RV_TIMER_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::RV_TIMER_IDX])
  );

  rv_timer rv_timer_i (
      .clk_i,
      .rst_ni,
      .tl_i(rv_timer_tl_h2d),
      .tl_o(rv_timer_tl_d2h),
      .intr_timer_expired_0_0_o(rv_timer_irq_timer_o)
  );

endmodule : always_on_periph_subsystem

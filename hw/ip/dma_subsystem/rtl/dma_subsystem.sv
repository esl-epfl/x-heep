/*
 * Copyright 2024 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *
 * Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *  
 * Info: DMA subsystem, it instantiates 1 to 8 DMA channels and manages the data transfers.
 */


module dma_subsystem #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic,
    parameter type obi_req_t = logic,
    parameter type obi_resp_t = logic,
    parameter int unsigned SLOT_NUM = 0
) (
    input logic clk_i,
    input logic rst_ni,

    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    output obi_req_t  dma_read_ch0_req_o,
    input  obi_resp_t dma_read_ch0_resp_i,

    output obi_req_t  dma_write_ch0_req_o,
    input  obi_resp_t dma_write_ch0_resp_i,

    output obi_req_t  dma_addr_ch0_req_o,
    input  obi_resp_t dma_addr_ch0_resp_i,

    input logic [SLOT_NUM-1:0] trigger_slot_i,
    input reg_req_t peripheral_req_i,  // This comes from the peripheral_to_reg of the ao per

    output dma_done_intr_o,
    output dma_window_intr_o
);


  import obi_pkg::*;
  import reg_pkg::*;
  import m2s_dma_reg_pkg::*;

  /*_________________________________________________________________________________________________________________________________ */

  /* Signals declaration */

  /* Masters requests to the bus */
  obi_req_t [core_v_mini_mcu_pkg::M2S_DMA_CH_NUM-1:0] xbar_write_req;
  obi_req_t [core_v_mini_mcu_pkg::M2S_DMA_CH_NUM-1:0] xbar_read_req;
  obi_req_t [core_v_mini_mcu_pkg::M2S_DMA_CH_NUM-1:0] xbar_address_req;

  /* Masters response from the bus*/
  obi_resp_t [core_v_mini_mcu_pkg::M2S_DMA_CH_NUM-1:0] xbar_write_resp;
  obi_resp_t [core_v_mini_mcu_pkg::M2S_DMA_CH_NUM-1:0] xbar_read_resp;
  obi_resp_t [core_v_mini_mcu_pkg::M2S_DMA_CH_NUM-1:0] xbar_address_resp;

  /* Interrupt signals */
  logic [core_v_mini_mcu_pkg::M2S_DMA_CH_NUM-1:0] dma_trans_done;
  logic [core_v_mini_mcu_pkg::M2S_DMA_CH_NUM-1:0] dma_window_done;

  /* Register interface routing signals */
  logic [core_v_mini_mcu_pkg::M2S_DMA_PORT_SEL_WIDTH:0] submodules_select;

  /* Register interfaces from register demux to DMAs */
  reg_pkg::reg_req_t [core_v_mini_mcu_pkg::M2S_DMA_CH_NUM:0] submodules_req;
  reg_pkg::reg_rsp_t [core_v_mini_mcu_pkg::M2S_DMA_CH_NUM:0] submodules_rsp;

  /*_________________________________________________________________________________________________________________________________ */

  /* Module instantiation */

  /* DMA modules */
  generate
    for (genvar i = 0; i < core_v_mini_mcu_pkg::M2S_DMA_CH_NUM; i++) begin : dma_i_gen
      dma #(
          .reg_req_t (reg_pkg::reg_req_t),
          .reg_rsp_t (reg_pkg::reg_rsp_t),
          .obi_req_t (obi_pkg::obi_req_t),
          .obi_resp_t(obi_pkg::obi_resp_t),
          .SLOT_NUM  (SLOT_NUM)
      ) dma_i (
          .clk_i,
          .rst_ni,
          .reg_req_i(submodules_req[i+1]),  // +1 because the first register is the M2S DMA register
          .reg_rsp_o(submodules_rsp[i+1]),
          .dma_read_ch0_req_o(xbar_read_req[i]),
          .dma_read_ch0_resp_i(xbar_read_resp[i]),
          .dma_write_ch0_req_o(xbar_write_req[i]),
          .dma_write_ch0_resp_i(xbar_write_resp[i]),
          .dma_addr_ch0_req_o(xbar_address_req[i]),
          .dma_addr_ch0_resp_i(xbar_address_resp[i]),
          .trigger_slot_i(trigger_slot_i),
          .dma_done_o(dma_trans_done[i]),
          .dma_window_o(dma_window_done[i])
      );
    end
  endgenerate

  /* Read operations xbar */
  generate
    if (core_v_mini_mcu_pkg::M2S_DMA_CH_NUM > 1) begin : xbar_varlat_n_to_one_gen
      xbar_varlat_n_to_one #(
          .XBAR_NMASTER(core_v_mini_mcu_pkg::M2S_DMA_CH_NUM)
      ) xbar_read_i (
          .clk_i        (clk_i),
          .rst_ni       (rst_ni),
          .master_req_i (xbar_read_req),
          .master_resp_o(xbar_read_resp),
          .slave_req_o  (m2s_dma_read_ch0_req_o),  // Request TO the slave, not by the slave
          .slave_resp_i (m2s_dma_read_ch0_resp_i)
      );
    end
  endgenerate

  /* Write operations xbar */
  generate
    if (core_v_mini_mcu_pkg::M2S_DMA_CH_NUM > 1) begin : xbar_varlat_n_to_one_gen
      xbar_varlat_n_to_one #(
          .XBAR_NMASTER(core_v_mini_mcu_pkg::M2S_DMA_CH_NUM)
      ) xbar_write_i (
          .clk_i        (clk_i),
          .rst_ni       (rst_ni),
          .master_req_i (xbar_write_req),
          .master_resp_o(xbar_write_resp),
          .slave_req_o  (m2s_dma_write_ch0_req_o),
          .slave_resp_i (m2s_dma_write_ch0_resp_i)
      );
    end
  endgenerate

  /* Address mode operations xbar */
  generate
    if (core_v_mini_mcu_pkg::M2S_DMA_CH_NUM > 1) begin : xbar_varlat_n_to_one_gen
      xbar_varlat_n_to_one #(
          .XBAR_NMASTER(core_v_mini_mcu_pkg::M2S_DMA_CH_NUM)
      ) xbar_address_i (
          .clk_i        (clk_i),
          .rst_ni       (rst_ni),
          .master_req_i (xbar_address_req),
          .master_resp_o(xbar_address_resp),
          .slave_req_o  (m2s_dma_addr_ch0_req_o),
          .slave_resp_i (m2s_dma_addr_ch0_resp_i)
      );
    end
  endgenerate

  /* Bus ports routing in the case of a single DMA */
  generate
    if (core_v_mini_mcu_pkg::M2S_DMA_CH_NUM == 1) begin
      assign m2s_dma_read_ch0_req_o = xbar_read_req[0];
      assign xbar_read_resp[0] = m2s_dma_read_ch0_resp_i;
      assign m2s_dma_write_ch0_req_o = xbar_write_req[0];
      assign xbar_write_resp[0] = m2s_dma_write_ch0_resp_i;
      assign m2s_dma_addr_ch0_req_o = xbar_address_req[0];
      assign xbar_address_resp[0] = m2s_dma_addr_ch0_resp_i;
    end
  endgenerate

  /* Internal address decoder */
  addr_decode #(
      .NoIndices(core_v_mini_mcu_pkg::M2S_DMA_CH_NUM + 1),
      .NoRules(core_v_mini_mcu_pkg::M2S_DMA_CH_NUM + 1), // +1 because the first map space is for the M2S DMA registers
      .addr_t(logic [31:0]),
      .rule_t(addr_map_rule_pkg::addr_map_rule_t)
  ) addr_dec_i (
      .addr_i(peripheral_req_i.addr),
      .addr_map_i(core_v_mini_mcu_pkg::M2S_DMA_ADDR_RULES),
      .idx_o(submodules_select),
      .dec_valid_o(),
      .dec_error_o(),
      .en_default_idx_i(1'b0),
      .default_idx_i('0)
  );

  /* Register demux */
  reg_demux #(
      .NoPorts(core_v_mini_mcu_pkg::M2S_DMA_CH_NUM + 1),
      .req_t  (reg_pkg::reg_req_t),
      .rsp_t  (reg_pkg::reg_rsp_t)
  ) reg_demux_i (
      .clk_i,
      .rst_ni,
      .in_select_i(submodules_select),
      .in_req_i(reg_req_i),
      .in_rsp_o(reg_rsp_o),
      .out_req_o(submodules_req),
      .out_rsp_i(submodules_rsp)
  );

  /*_________________________________________________________________________________________________________________________________ */

  /* Signal assignments */

  /* MUX between AO_peripheral register interface or external SDCs */

  // TBD

  assign dma_done_intr_o   = |(dma_trans_done);
  assign dma_window_intr_o = |(dma_window_done);

endmodule

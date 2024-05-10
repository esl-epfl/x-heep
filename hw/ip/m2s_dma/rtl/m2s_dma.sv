/*
 * Copyright 2024 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *
 *  Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *  
 *  Info: Multichannel Multidimensional Smart DMA subsystem, in instantiates 1 to 8 DMA modules and
 *  manages the data transfers, the configuration registers transfers, the window counters and the 
 *  interrupt generation. 
 */

module m2s_dma #(
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

    output obi_req_t  m2s_dma_read_ch0_req_o,
    input  obi_resp_t m2s_dma_read_ch0_resp_i,

    output obi_req_t  m2s_dma_write_ch0_req_o,
    input  obi_resp_t m2s_dma_write_ch0_resp_i,

    output obi_req_t  m2s_dma_addr_ch0_req_o,
    input  obi_resp_t m2s_dma_addr_ch0_resp_i,

    input logic [SLOT_NUM-1:0] trigger_slot_i,

    input reg_pkg::reg_req_t peripheral_req_i, // This comes from the peripheral_to_reg of the ao per
    output reg_pkg::reg_rsp_t peripheral_rsp_o,  // This goes to the peripheral_to_reg of the ao per

    output m2s_dma_done_intr_o,
    output m2s_dma_window_intr_o
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
  obi_req_t [core_v_mini_mcu_pkg::M2S_DMA_CH_NUM-1:0] xbar_write_resp;
  obi_req_t [core_v_mini_mcu_pkg::M2S_DMA_CH_NUM-1:0] xbar_read_resp;
  obi_req_t [core_v_mini_mcu_pkg::M2S_DMA_CH_NUM-1:0] xbar_address_resp;

  /* DMA to bus/xbars signals */
  obi_req_t dma_read_ch0_req[core_v_mini_mcu_pkg::M2S_DMA_CH_NUM];
  obi_resp_t dma_read_ch0_resp[core_v_mini_mcu_pkg::M2S_DMA_CH_NUM];
  obi_req_t dma_write_ch0_req[core_v_mini_mcu_pkg::M2S_DMA_CH_NUM];
  obi_resp_t dma_write_ch0_resp[core_v_mini_mcu_pkg::M2S_DMA_CH_NUM];
  obi_req_t dma_addr_ch0_req[core_v_mini_mcu_pkg::M2S_DMA_CH_NUM];
  obi_resp_t dma_addr_ch0_resp[core_v_mini_mcu_pkg::M2S_DMA_CH_NUM];

  /* Interrupt signals */
  logic [core_v_mini_mcu_pkg::M2S_DMA_CH_NUM-1:0] dma_done_intr;
  logic [core_v_mini_mcu_pkg::M2S_DMA_CH_NUM-1:0] dma_window_intr;
  logic [7:0] transaction_ifr;
  logic [7:0] window_ifr;

  /* Register interface routing signals */
  logic [core_v_mini_mcu_pkg::M2S_DMA_CH_NUM:0] submodules_select; // It's channel_num and not channel_num -1 because for 2 dmas we have 2 registers + 1 for the M2SD

  /* Register interfaces from register demux to DMAs */
  reg_pkg::reg_req_t [core_v_mini_mcu_pkg::M2S_DMA_CH_NUM:0] submodules_req;
  reg_pkg::reg_rsp_t [core_v_mini_mcu_pkg::M2S_DMA_CH_NUM:0] submodules_rsp;

  /* Regtool register interface */
  m2s_dma_reg2hw_t reg2hw;
  m2s_dma_hw2reg_t hw2reg;

  /*_________________________________________________________________________________________________________________________________ */

  /* Module instantiation */

  /* DMA modules */
  generate
    for (i = 0; i < core_v_mini_mcu_pkg::M2S_DMA_CH_NUM; i++) begin : proc_gen_dma
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
          .dma_addr_ch0_req_o(dma_read_ch0_req[i]),
          .dma_addr_ch0_resp_i(dma_read_ch0_resp[i]),
          .dma_write_ch0_req_o(dma_write_ch0_req[i]),
          .dma_write_ch0_resp_i(dma_write_ch0_resp[i]),
          .dma_addr_ch0_req_o(dma_addr_ch0_req[i]),
          .dma_addr_ch0_resp_i(dma_addr_ch0_resp[i]),
          .trigger_slot_i(trigger_slots_i),
          .dma_done_o(dma_done_intr[i]),
          .dma_window_o(dma_window_intr[i]),
          .dma_trans_intr_en_i(transaction_ifr[i]),
          .dma_window_intr_en_i(window_ifr[i])
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
      assign m2s_dma_read_ch0_req_o = dma_read_ch0_req[0];
      assign dma_read_ch0_resp[0] = m2s_dma_read_ch0_resp_i;
      assign m2s_dma_write_ch0_req_o = dma_write_ch0_req[0];
      assign dma_write_ch0_resp[0] = m2s_dma_write_ch0_resp_i;
      assign m2s_dma_addr_ch0_req_o = dma_addr_ch0_req[0];
      assign dma_addr_ch0_resp[0] = m2s_dma_addr_ch0_resp_i;
    end
  endgenerate

  /* Internal address decoder */
  if (core_v_mini_mcu_pkg::M2S_DMA_CH_NUM > 1) begin : addr_decode_gen
    addr_decode #(
        .NoIndices(core_v_mini_mcu_pkg::M2S_DMA_CH_NUM),
        .NoRules(core_v_mini_mcu_pkg::M2S_DMA_CH_NUM),
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
  end

  /* Register demux */
  reg_demux #(
      .NoPorts(core_v_mini_mcu_pkg::M2S_DMA_CH_NUM),
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

  /* M2S DMA registers */
  m2s_dma_reg #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) dma_reg_i (
      .clk_i,
      .rst_ni,
      .reg_req_i(submodules_req[core_v_mini_mcu_pkg::M2S_DMA_SYS_REGS_IDX]), //@ToD0: add the indexes to core_v_mini_mcu_pkg
      .reg_req_i(submodules_rsp[core_v_mini_mcu_pkg::M2S_DMA_SYS_REGS_IDX]),
      .reg2hw,
      .hw2reg,
      .devmode_i(1'b1)
  );


  /*_________________________________________________________________________________________________________________________________ */

  /* Signal assignments */

  /* MUX between AO_peripheral register interface or external SDCs */

  // TBD

  /* Interrupt logic 
 * The interrupt signals are ORed together to generate the final interrupt signals. To check which DMA raised the interrupt,
 * the user has to read the status register of the M2S DMA, which holds the values of the dma_done and window_done signals.
 */
  assign m2s_dma_done_intr_o = |dma_done_intr;
  assign m2s_dma_window_intr_o = |dma_window_intr;
  assign hw2reg.transaction_ifr.d = transaction_ifr;
  assign hw2reg.window_ifr.d = window_ifr;

  /*_________________________________________________________________________________________________________________________________ */

  /* Sequential statements */

  /* Transaction IFR update */
  always_ff @(posedge clk_i, negedge rst_ni) begin
    if (~rst_ni) begin
      transaction_ifr <= '0;
      window_ifr <= '0;
    end else begin
      for (int i = 0; i < core_v_mini_mcu_pkg::M2S_DMA_CH_NUM; i++) begin
        if (dma_done_intr[i] == 1'b1) begin
          transaction_ifr[i] <= 1'b0;
        end
      end
    end
  end

  /* Window IFR update */
  always_ff @(posedge clk_i, negedge rst_ni) begin
    if (~rst_ni) begin
      window_ifr <= '0;
    end else begin
      for (int i = 0; i < core_v_mini_mcu_pkg::M2S_DMA_CH_NUM; i++) begin
        if (dma_window_intr[i] == 1'b1) begin
          window_ifr[i] <= 1'b0;
        end
      end
    end
  end

  /*_________________________________________________________________________________________________________________________________ */

  /* Combinational statements */

  /*
TO BE DEFINED
always_comb begin : proc_mux_reg_intfc
   if (reg2hw.control.q == 1'b1) begin
       
   end
end
*/
endmodule

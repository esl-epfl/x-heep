/*
 * Copyright 2024 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *
 * Author: Tommaso Terzano <tommaso.terzano@epfl.ch> 
 *                         <tommaso.terzano@gmail.com>
 *  
 * Info: DMA subsystem, it instantiates 1 to 8 DMA channels and manages the data transfers.
 */


module dma_subsystem #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic,
    parameter type obi_req_t = logic,
    parameter type obi_resp_t = logic,
    parameter int unsigned GLOBAL_SLOT_NUM = 0,
    parameter int unsigned EXT_SLOT_NUM = 0
) (
    input logic clk_i,
    input logic rst_ni,
    input logic clk_gate_en_ni [core_v_mini_mcu_pkg::DMA_CH_NUM-1:0],

    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    output obi_req_t  [core_v_mini_mcu_pkg::DMA_NUM_MASTER_PORTS-1:0] dma_read_req_o,
    input  obi_resp_t [core_v_mini_mcu_pkg::DMA_NUM_MASTER_PORTS-1:0] dma_read_resp_i,

    output obi_req_t  [core_v_mini_mcu_pkg::DMA_NUM_MASTER_PORTS-1:0] dma_write_req_o,
    input  obi_resp_t [core_v_mini_mcu_pkg::DMA_NUM_MASTER_PORTS-1:0] dma_write_resp_i,

    output obi_req_t  [core_v_mini_mcu_pkg::DMA_NUM_MASTER_PORTS-1:0] dma_addr_req_o,
    input  obi_resp_t [core_v_mini_mcu_pkg::DMA_NUM_MASTER_PORTS-1:0] dma_addr_resp_i,

    input logic [GLOBAL_SLOT_NUM-1:0] global_trigger_slot_i,
    input logic [EXT_SLOT_NUM-1:0] ext_trigger_slot_i,

    input logic [core_v_mini_mcu_pkg::DMA_CH_NUM-1:0] ext_dma_stop_i,

    output dma_done_intr_o,
    output dma_window_intr_o,

    output logic [core_v_mini_mcu_pkg::DMA_CH_NUM-1:0] dma_done_o
);

  /*_________________________________________________________________________________________________________________________________ */

  /* Imports and parameters */

  import obi_pkg::*;
  import reg_pkg::*;

  /* 
   * It's possible to define individually the sizes of each DMA channel's FIFOs.
   * For example, a FIFO_CH_ARRAY like this {L, M, M, S} means that CH0 will be small,
   * CH1 and CH2 will be medium and CH3 will be large.
   *
   * This is useful in applications that require different bandwidths for different channels.
   * To enable this functionality, decomment EN_SET_FIFO_CH_SIZE and set the desired sizes.
   */

  //`define EN_SET_FIFO_CH_SIZE;

  `ifdef EN_SET_FIFO_CH_SIZE 

  localparam int unsigned LARGE_FIFO_CH_SIZE = 8;
  localparam int unsigned MEDIUM_FIFO_CH_SIZE = 4;
  localparam int unsigned SMALL_FIFO_CH_SIZE = 2;

  typedef enum {L, M, S} fifo_ch_size_t;

  localparam fifo_ch_size_t FIFO_CH_ARRAY [core_v_mini_mcu_pkg::DMA_CH_NUM] = '{L, M, M, S};

  `endif

  /*_________________________________________________________________________________________________________________________________ */

  /* Signals declaration */

  /* Masters requests to the bus */
  obi_req_t [core_v_mini_mcu_pkg::DMA_CH_NUM-1:0] xbar_write_req;
  obi_req_t [core_v_mini_mcu_pkg::DMA_CH_NUM-1:0] xbar_read_req;
  obi_req_t [core_v_mini_mcu_pkg::DMA_CH_NUM-1:0] xbar_address_req;

  /* Masters response from the bus*/
  obi_resp_t [core_v_mini_mcu_pkg::DMA_CH_NUM-1:0] xbar_write_resp;
  obi_resp_t [core_v_mini_mcu_pkg::DMA_CH_NUM-1:0] xbar_read_resp;
  obi_resp_t [core_v_mini_mcu_pkg::DMA_CH_NUM-1:0] xbar_address_resp;

  /* Interrupt signals */
  logic [core_v_mini_mcu_pkg::DMA_CH_NUM-1:0] dma_trans_done;
  logic [core_v_mini_mcu_pkg::DMA_CH_NUM-1:0] dma_window_done;

  /* Register interfaces from register demux to DMAs */
  reg_pkg::reg_req_t [core_v_mini_mcu_pkg::DMA_CH_NUM-1:0] submodules_req;
  reg_pkg::reg_rsp_t [core_v_mini_mcu_pkg::DMA_CH_NUM-1:0] submodules_rsp;

  /*_________________________________________________________________________________________________________________________________ */

  /* Module instantiation */

  /* DMA modules */
  generate
    for (genvar i = 0; i < core_v_mini_mcu_pkg::DMA_CH_NUM; i++) begin : dma_i_gen

      /* FIFO size assignment */
      localparam int fifo_size = 
      `ifdef EN_SET_FIFO_CH_SIZE
        (FIFO_CH_ARRAY[i] == L) ? LARGE_FIFO_CH_SIZE :
        ((FIFO_CH_ARRAY[i] == M) ? MEDIUM_FIFO_CH_SIZE : SMALL_FIFO_CH_SIZE);
      `else
        4;
      `endif

      dma #(
          .reg_req_t (reg_pkg::reg_req_t),
          .reg_rsp_t (reg_pkg::reg_rsp_t),
          .obi_req_t (obi_pkg::obi_req_t),
          .obi_resp_t(obi_pkg::obi_resp_t),
          .SLOT_NUM  (GLOBAL_SLOT_NUM + 2),
          .FIFO_DEPTH (fifo_size)
      ) dma_i (
          .clk_i,
          .rst_ni,
          .clk_gate_en_ni(clk_gate_en_ni[i]),
          .ext_dma_stop_i(ext_dma_stop_i[i]),
          .reg_req_i(submodules_req[i]),
          .reg_rsp_o(submodules_rsp[i]),
          .dma_read_req_o(xbar_read_req[i]),
          .dma_read_resp_i(xbar_read_resp[i]),
          .dma_write_req_o(xbar_write_req[i]),
          .dma_write_resp_i(xbar_write_resp[i]),
          .dma_addr_req_o(xbar_address_req[i]),
          .dma_addr_resp_i(xbar_address_resp[i]),
          .trigger_slot_i({
            ext_trigger_slot_i[2*i+1], ext_trigger_slot_i[2*i], global_trigger_slot_i
          }),
          .dma_done_intr_o(dma_trans_done[i]),
          .dma_window_intr_o(dma_window_done[i]),
          .dma_done_o(dma_done_o[i])
      );
    end
  endgenerate


  generate
    if (core_v_mini_mcu_pkg::DMA_CH_NUM > 1) begin : xbar_varlat_gen

      /* Register interface routing signals */
      logic [core_v_mini_mcu_pkg::DMA_CH_PORT_SEL_WIDTH-1:0] submodules_select;

      if (core_v_mini_mcu_pkg::DMA_NUM_MASTER_PORTS > 1 && core_v_mini_mcu_pkg::DMA_NUM_MASTER_PORTS != core_v_mini_mcu_pkg::DMA_CH_NUM) begin : xbar_n_to_m_gen

        /* Read, write & address mode operations xbar*/
        dma_NtoM_xbar #(
            .XBAR_NMASTER(core_v_mini_mcu_pkg::DMA_CH_NUM),
            .XBAR_MSLAVE(core_v_mini_mcu_pkg::DMA_NUM_MASTER_PORTS)
        ) xbar_read_i (
            .clk_i(clk_i),
            .rst_ni(rst_ni),
            .master_req_i(xbar_read_req),
            .master_resp_o(xbar_read_resp),
            .slave_req_o(dma_read_req_o),
            .slave_resp_i(dma_read_resp_i)
        );

        dma_NtoM_xbar #(
            .XBAR_NMASTER(core_v_mini_mcu_pkg::DMA_CH_NUM),
            .XBAR_MSLAVE(core_v_mini_mcu_pkg::DMA_NUM_MASTER_PORTS)
        ) xbar_write_i (
            .clk_i(clk_i),
            .rst_ni(rst_ni),
            .master_req_i(xbar_write_req),
            .master_resp_o(xbar_write_resp),
            .slave_req_o(dma_write_req_o),
            .slave_resp_i(dma_write_resp_i)
        );

        dma_NtoM_xbar #(
            .XBAR_NMASTER(core_v_mini_mcu_pkg::DMA_CH_NUM),
            .XBAR_MSLAVE(core_v_mini_mcu_pkg::DMA_NUM_MASTER_PORTS)
        ) xbar_address_i (
            .clk_i(clk_i),
            .rst_ni(rst_ni),
            .master_req_i(xbar_address_req),
            .master_resp_o(xbar_address_resp),
            .slave_req_o(dma_addr_req_o),
            .slave_resp_i(dma_addr_resp_i)
        );
      end else if (core_v_mini_mcu_pkg::DMA_NUM_MASTER_PORTS > 1 && core_v_mini_mcu_pkg::DMA_NUM_MASTER_PORTS == core_v_mini_mcu_pkg::DMA_CH_NUM) begin : xbar_n_to_n_gen
        for (genvar i = 0 ; i < core_v_mini_mcu_pkg::DMA_NUM_MASTER_PORTS; i++) begin
          assign dma_read_req_o[i] = xbar_read_req[i];
          assign xbar_read_resp[i] = dma_read_resp_i[i];
          assign dma_write_req_o[i] = xbar_write_req[i];
          assign xbar_write_resp[i] = dma_write_resp_i[i];
          assign dma_addr_req_o[i] = xbar_address_req[i];
          assign xbar_address_resp[i] = dma_addr_resp_i[i];
        end

      end else begin : xbar_n_to_1_gen
        /* Read, write & address mode operations xbar*/
        xbar_varlat_n_to_one #(
            .XBAR_NMASTER(core_v_mini_mcu_pkg::DMA_CH_NUM)
        ) xbar_read_i (
            .clk_i        (clk_i),
            .rst_ni       (rst_ni),
            .master_req_i (xbar_read_req),
            .master_resp_o(xbar_read_resp),
            .slave_req_o  (dma_read_req_o[0]),
            .slave_resp_i (dma_read_resp_i[0])
        );

        xbar_varlat_n_to_one #(
            .XBAR_NMASTER(core_v_mini_mcu_pkg::DMA_CH_NUM)
        ) xbar_write_i (
            .clk_i        (clk_i),
            .rst_ni       (rst_ni),
            .master_req_i (xbar_write_req),
            .master_resp_o(xbar_write_resp),
            .slave_req_o  (dma_write_req_o[0]),
            .slave_resp_i (dma_write_resp_i[0])
        );

        xbar_varlat_n_to_one #(
            .XBAR_NMASTER(core_v_mini_mcu_pkg::DMA_CH_NUM)
        ) xbar_address_i (
            .clk_i        (clk_i),
            .rst_ni       (rst_ni),
            .master_req_i (xbar_address_req),
            .master_resp_o(xbar_address_resp),
            .slave_req_o  (dma_addr_req_o[0]),
            .slave_resp_i (dma_addr_resp_i[0])
        );
      end

      /* Internal address decoder */
      addr_decode #(
          .NoIndices(core_v_mini_mcu_pkg::DMA_CH_NUM),
          .NoRules(core_v_mini_mcu_pkg::DMA_CH_NUM),
          .addr_t(logic [7:0]),
          .rule_t(addr_map_rule_pkg::addr_map_rule_8bit_t)
      ) addr_dec_i (
          .addr_i(reg_req_i.addr[15:8]),
          .addr_map_i(core_v_mini_mcu_pkg::DMA_ADDR_RULES),
          .idx_o(submodules_select),
          .dec_valid_o(),
          .dec_error_o(),
          .en_default_idx_i(1'b0),
          .default_idx_i('0)
      );

      /* Register demux */
      reg_demux #(
          .NoPorts(core_v_mini_mcu_pkg::DMA_CH_NUM),
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

    end else begin

      /* Bus ports routing in the case of a single DMA */
      assign dma_read_req_o[0] = xbar_read_req[0];
      assign xbar_read_resp[0] = dma_read_resp_i[0];
      assign dma_write_req_o[0] = xbar_write_req[0];
      assign xbar_write_resp[0] = dma_write_resp_i[0];
      assign dma_addr_req_o[0] = xbar_address_req[0];
      assign xbar_address_resp[0] = dma_addr_resp_i[0];
      assign submodules_req[0] = reg_req_i;
      assign reg_rsp_o = submodules_rsp[0];
    end
  endgenerate


  /*_________________________________________________________________________________________________________________________________ */

  /* Signal assignments */

  assign dma_done_intr_o   = |(dma_trans_done);
  assign dma_window_intr_o = |(dma_window_done);

endmodule

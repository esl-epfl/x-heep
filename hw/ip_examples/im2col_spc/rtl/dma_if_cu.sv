/*
 * Copyright 2024 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *
 * Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *  
 * Info: This module enables the im2col SPC to interface with the Always On Peripheral
 *       subsystem to perform DMA transactions. It communicates with the DMA queue, it asserts a request
 *       which is answered with a valid when the queue is not empty.
 */

 module dma_if_cu #(
    parameter int unsigned NUM_CH = 2
 ) (
    input logic clk_i,
    input logic rst_ni,
    input logic en_i,
    input dma_if_t dma_if_i,
    input logic [core_v_mini_mcu_pkg::DMA_CH_NUM-1:0] dma_done_i,
    input obi_req_t obi_req_i,
    input logic dma_queue_valid_i,
    output logic dma_queue_req_o,
    output obi_resp_t obi_resp_o,
    output logic dma_queue_en_o
 );

  import obi_pkg::*;
  import dma_queue_pkg::*;
  import core_v_mini_mcu_pkg::*;

  /*_________________________________________________________________________________________________________________________________ */

  /* Signals declaration */
  


  
 endmodule
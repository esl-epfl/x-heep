/*
 * Copyright 2024 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *
 * Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *  
 * Info: This module enables the im2col SPC DMA interface unit to manage multiple channels 
         and poll between them.
 */

module dma_queue # (
    parameter int unsigned NUM_CH = 2
) (
    input logic clk_i,
    input logic rst_ni,
    input logic sample_en,
    input dma_if_t dma_if_i,
    input logic dma_if_req_i,
    output logic dma_channel_o,
    output dma_if_t dma_if_o,
    output dma_queue_full_o,
    output dma_queue_empty_o
)

  /*_________________________________________________________________________________________________________________________________ */

  /* Signals declaration */

  dma_if_t [NUM_CH-1:0] dma_channels;
  logic [clog2(NUM_CH)-1:0] dma_channels_full;
  logic [clog2(NUM_CH)-1:0] dma_channel_head_id;
  logic [clog2(NUM_CH)-1:0] dma_tail_id;
  logic dma_queue_full;
  logic dma_queue_empty;

  /*_________________________________________________________________________________________________________________________________ */

  /* Module & FSMs instantiation */

  /* 
   * DMA Queue manager: when the sample_en signal is on, the manager samples the data into the first free
   * channel. It is implemented as a circular buffer: the channel_tail_id signal is used to keep track of the
   * first free channel for write operations while channel_head_id is used for read operations.
   */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_dma_queue_manager
    if (!rst_ni) begin
      for (int i=0; i<NUM_CH; i++) begin
        dma_channels[i] <= '0;
      end
      dma_channel_tail_id <= '0;
      dma_channel_head_id <= '0;
      dma_channels_full <= '0;
      dma_queue_full <= 1'b0;
      dma_queue_empty <= 1'b1;
    end else begin
      /*
       * Read operations: sample a new dma_if if the sample signal is asserted 
       * & if the queue isn't full.
       */
      if (sample_en == 1'b1 && dma_queue_full == 1'b0) begin
        dma_channels[channel_free_id] <= dma_if_i;
        dma_channels_full <= dma_channels_full + 1'b1;
        dma_tail_id <= dma_tail_id + 1'b1;
        if (dma_tail_id == NUM_CH-1) begin
          dma_tail_id <= '0;
        end
      end
      
      /* 
       * Pop a dma_if if the queue isn't empty 
       */
      if (dma_if_req_i == 1'b1 && dma_queue_empty == 1'b0) begin
        dma_if_o <= dma_channels[channel_head_id];
        dma_channels_full <= dma_channels_full - 1'b1;
        dma_head_id <= dma_head_id + 1'b1;
        if (dma_head_id == NUM_CH-1) begin
          dma_head_id <= '0;
        end
      end
    end
  end

  /*_________________________________________________________________________________________________________________________________ */

  /* Signal assignments */

  assign dma_queue_full_o = dma_queue_full;
  assign dma_queue_full = (dma_channels_full == NUM_CH);
  assign dma_queue_empty = (dma_channels_full == '0);

endmodule
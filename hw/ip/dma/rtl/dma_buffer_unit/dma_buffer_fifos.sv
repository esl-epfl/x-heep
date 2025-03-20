/*
 * Copyright 2025 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *
 * Author: Tommaso Terzano <tommaso.terzano@gmail.com>
 *  
 * Info: This module contains all the FIFOs needed by the DMA. The idea is to make the DMA unit more modular, which improves
 *       code readability and increases the options for design exploration. 
 *       This module is declared in `dma_buffer_unit.sv`, together with the `dma_buffer_control.sv`.
 */

module dma_buffer_fifos
  import dma_reg_pkg::*;
  import hw_fifo_pkg::*;
#(
    parameter SUBADDR_MODE_EN = 1,
    parameter ADDR_MODE_EN = 1,
    parameter FIFO_DEPTH = 4,
    parameter NUM_READ_FIFOS = (SUBADDR_MODE_EN ? 4 : 1)
) (
    input logic clk_i,
    input logic rst_ni,    

    input dma_reg2hw_t reg2hw_i,

    input logic [31:0] read_fifo_input_i,
    input logic [31:0] read_addr_fifo_input_i,
    input logic [31:0] write_fifo_input_i,

    input logic fifo_flush_i,

    input logic read_fifo_push_i,
    input logic read_addr_fifo_push_i,
    input logic write_fifo_push_i,

    input logic [NUM_READ_FIFOS-1:0] read_fifo_pop_i,
    input logic                      read_addr_fifo_pop_i,
    input logic                      write_fifo_pop_i,

    output logic [31:0] read_fifo_output_o,
    output logic [31:0] read_addr_fifo_output_o,
    output logic [31:0] write_fifo_output_o,

    output logic read_fifo_alm_full_o,
    output logic read_addr_fifo_alm_full_o,
    output logic write_fifo_alm_full_o,

    output logic read_fifo_full_o,
    output logic read_addr_fifo_full_o,
    output logic write_fifo_full_o,

    output logic read_fifo_empty_o,
    output logic read_addr_fifo_empty_o,
    output logic write_fifo_empty_o
);

  localparam int unsigned LastFifoUsage = FIFO_DEPTH - 1;
  localparam int unsigned AddrFifoDepth = (FIFO_DEPTH > 1) ? $clog2(FIFO_DEPTH) : 1;

  logic subaddr_mode;

  logic fifo_flush;

  logic read_fifo_push;
  logic read_addr_fifo_push;
  logic write_fifo_push;

  logic read_fifo_pop;
  logic read_addr_fifo_pop;
  logic write_fifo_pop;

  logic read_addr_fifo_full;
  logic write_fifo_full;
  
  logic read_fifo_alm_full;
  logic read_addr_fifo_alm_full;
  logic write_fifo_alm_full;

  logic read_addr_fifo_empty;
  logic write_fifo_empty;

  logic [AddrFifoDepth-1:0] read_addr_fifo_usage;
  logic [AddrFifoDepth-1:0] write_fifo_usage;

  logic [31:0] read_fifo_input;
  logic [31:0] read_addr_fifo_input;
  logic [31:0] write_fifo_input;

  logic [31:0] read_fifo_output;
  logic [31:0] read_addr_fifo_output;
  logic [31:0] write_fifo_output;

  /* Generate Read FIFOs */
  generate
    if (SUBADDR_MODE_EN == 1) begin
      genvar i;

      logic [3:0] read_fifo_full;
      logic [3:0] read_fifo_empty;
      logic [3:0] read_fifo_pop;

      logic [3:0][AddrFifoDepth-1:0] read_fifo_usage;

      for (i=0; i<4; i++) begin : FIFO_GEN
        /* Read FIFO */
        fifo_v3 #(
            .DEPTH(FIFO_DEPTH),
            .FALL_THROUGH(1'b0),
            .DATA_WIDTH(8)
        ) dma_read_fifo_i (
            .clk_i,
            .rst_ni,
            .flush_i(fifo_flush),
            .testmode_i(1'b0),
            .full_o(read_fifo_full[i]),
            .empty_o(read_fifo_empty[i]),
            .usage_o(read_fifo_usage[i]),
            .data_i(read_fifo_input[(8*(i+1)-1):8*i]),
            .push_i(read_fifo_push),
            .data_o(read_fifo_output[(8*(i+1)-1):8*i]),
            .pop_i(read_fifo_pop[i])
        );
      end

      assign read_fifo_full_o = |(read_fifo_full);
      assign read_fifo_empty_o = &(read_fifo_empty);
      assign read_fifo_alm_full = (read_fifo_usage[0] == LastFifoUsage[Addr_Fifo_Depth-1:0]) & 
                                  (read_fifo_usage[1] == LastFifoUsage[Addr_Fifo_Depth-1:0]) &
                                  (read_fifo_usage[2] == LastFifoUsage[Addr_Fifo_Depth-1:0]) &
                                  (read_fifo_usage[3] == LastFifoUsage[Addr_Fifo_Depth-1:0]);

    end else begin
      logic read_fifo_full;
      logic read_fifo_empty;
      logic read_fifo_pop;

      logic [AddrFifoDepth-1:0] read_fifo_usage;

      /* Read FIFO */
      fifo_v3 #(
            .DEPTH(FIFO_DEPTH),
            .FALL_THROUGH(1'b0),
            .DATA_WIDTH(32)
        ) dma_read_fifo_i (
            .clk_i,
            .rst_ni,
            .flush_i(fifo_flush),
            .testmode_i(1'b0),
            .full_o(read_fifo_full),
            .empty_o(read_fifo_empty),
            .usage_o(read_fifo_usage),
            .data_i(read_fifo_input[31:0]),
            .push_i(read_fifo_push),
            .data_o(read_fifo_output[31:0]),
            .pop_i(read_fifo_pop)
        );
      
      assign read_fifo_full_o = read_fifo_full;
      assign read_fifo_empty_o = read_fifo_empty;
      assign read_fifo_alm_full_o = (read_fifo_usage == LastFifoUsage[Addr_Fifo_Depth-1:0]);
    end
  endgenerate

  /* Generate Read Address Mode FIFOs */
  generate
    if (ADDR_MODE_EN == 1) begin
      /* Read address mode FIFO */
      fifo_v3 #(
          .DEPTH(FIFO_DEPTH),
          .FALL_THROUGH(1'b0)
      ) dma_read_addr_fifo_i (
          .clk_i,
          .rst_ni,
          .flush_i(fifo_flush),
          .testmode_i(1'b0),
          .full_o(read_addr_fifo_full),
          .empty_o(read_addr_fifo_empty),
          .usage_o(read_addr_fifo_usage),
          .data_i(read_addr_fifo_input),
          .push_i(read_addr_fifo_push),
          .data_o(read_addr_fifo_output),
          .pop_i(read_addr_fifo_pop)
      );

      assign read_addr_fifo_alm_full_o = (read_addr_fifo_usage == LastFifoUsage[Addr_Fifo_Depth-1:0]);

    end else begin
      /* Tie to 0s the Address mode signals */
      assign read_addr_fifo_output = '0;
      assign read_addr_fifo_usage = '0;
      assign read_addr_fifo_empty = '0;
      assign read_addr_fifo_full = '0;
      assign read_addr_fifo_alm_full_o = '0;
    end
  endgenerate
  
  /* Write FIFO */
  fifo_v3 #(
      .DEPTH(FIFO_DEPTH),
      .FALL_THROUGH(1'b1)
  ) dma_write_fifo_i (
      .clk_i,
      .rst_ni,
      .flush_i(fifo_flush),
      .testmode_i(1'b0),
      .full_o(write_fifo_full),
      .empty_o(write_fifo_empty),
      .usage_o(write_fifo_usage),
      .data_i(write_fifo_input),
      .push_i(write_fifo_push),
      .data_o(write_fifo_output),
      .pop_i(write_fifo_pop)
  );

  /* Read FIFO mask logic for non-WORD transactions */

  /* Re-ordering of data for read FIFO input with sign-extension */
  
  assign write_fifo_alm_full = (write_fifo_usage == LastFifoUsage[Addr_Fifo_Depth-1:0]);
  
  /* Port signals assignments */
  assign read_fifo_pop = read_fifo_pop_i;
  assign read_addr_fifo_pop = read_addr_fifo_pop_i;
  assign write_fifo_pop = write_fifo_pop_i;

  assign read_fifo_push = read_fifo_push_i;
  assign read_addr_fifo_push = read_addr_fifo_push_i;
  assign write_fifo_push = write_fifo_push_i;

  assign read_addr_fifo_full_o = read_addr_fifo_full;
  assign write_fifo_full_o = write_fifo_full;
  
  assign read_fifo_alm_full_o = read_fifo_alm_full;
  assign read_addr_fifo_alm_full_o = read_addr_fifo_alm_full;
  assign write_fifo_alm_full_o = write_fifo_alm_full;
  
  assign fifo_flush = fifo_flush_i;
  
  assign read_fifo_output_o = read_fifo_output;
  assign read_addr_fifo_output_o = read_addr_fifo_output;
  assign write_fifo_output_o = write_fifo_output;

  assign read_fifo_input = read_fifo_input_i;
  assign read_addr_fifo_input = read_addr_fifo_input_i;
  assign write_fifo_input = write_fifo_input_i;

endmodule
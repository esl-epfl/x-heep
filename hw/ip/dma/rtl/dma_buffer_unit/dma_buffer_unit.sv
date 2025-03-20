/*
 * Copyright 2025 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *
 * Author: Tommaso Terzano <tommaso.terzano@gmail.com>
 *  
 * Info: This module instantiates the DMA FIFOs and the logic that drive them. 
 */

 module dma_buffer_unit
  import dma_reg_pkg::*;
#(
    parameter SUBADDR_MODE_EN = 1,
    parameter ADDR_MODE_EN = 1,
    parameter FIFO_DEPTH = 4,
    parameter NUM_READ_FIFOS = (SUBADDR_MODE_EN ? 4 : 1)
) (
    input logic clk_i,
    input logic rst_ni,   

    input dma_reg2hw_t reg2hw_i,
    
    // To be modified with mcu_gen
    input  hw_fifo_resp_t hw_fifo_resp_i,
    output hw_fifo_req_t  hw_fifo_req_o,

    input logic dma_start_i,

    input logic [31:0] read_data_input_i,
    input logic [31:0] read_addr_data_input_i,
    input logic [31:0] write_data_input_i,

    input logic buffer_flush_i,

    input logic read_data_push_i,
    input logic read_addr_data_push_i,
    input logic write_data_push_i,

    input logic [NUM_READ_FIFOS-1:0] read_data_pop_i,
    input logic                      read_addr_data_pop_i,
    input logic                      write_data_pop_i,

    output logic [31:0] read_data_output_o,
    output logic [31:0] read_addr_data_output_o,
    output logic [31:0] write_data_output_o,
// TODO: write some logic to produce these signals directly here in the top.
    output logic read_buffer_push_stall_o,
    output logic read_addr_buffer_push_stall_o,
    output logic write_buffer_push_stall_o,

    output logic read_buffer_pop_stall_o,
    output logic read_addr_buffer_pop_stall_o,
    output logic write_buffer_pop_stall_o
)
  logic fifo_flush;

  logic read_fifo_push;
  logic read_addr_fifo_push;
  logic write_fifo_push_control;
  logic write_fifo_push;
  
  logic read_fifo_pop;
  logic read_addr_fifo_pop;
  logic write_fifo_pop;

  logic [AddrFifoDepth-1:0] read_addr_fifo_usage;
  logic [AddrFifoDepth-1:0] write_fifo_usage;

  logic [31:0] read_fifo_input;
  logic [31:0] read_addr_fifo_input;
  logic [31:0] write_fifo_input;

  logic [31:0] read_fifo_output;
  logic [31:0] read_addr_fifo_output;
  logic [31:0] write_fifo_output;

  logic [31:0] write_data_mask;

  logic read_fifo_alm_full;
  logic read_addr_fifo_alm_full;
  logic write_fifo_alm_full;
  logic write_fifo_internal_alm_full;

  logic read_fifo_full;
  logic read_addr_fifo_full;
  logic write_fifo_full;
  logic write_fifo_internal_full;

  logic read_fifo_empty;
  logic read_addr_fifo_empty;
  logic write_fifo_empty;
  logic write_fifo_internal_empty;

  dma_buffer_fifos #(
      .SUBADDR_MODE_EN,
      .ADDR_MODE_EN,
      .FIFO_DEPTH,
      .NUM_READ_FIFOS
  ) dma_buffer_fifos_i (
      .clk_i,
      .rst_ni,    
      .reg2hw_i,

      .read_fifo_input_i(read_fifo_input),
      .read_addr_fifo_input_i(read_addr_fifo_input),
      .write_fifo_input_i(write_fifo_input),

      .fifo_flush_i(fifo_flush),

      .read_fifo_push_i(read_fifo_push),
      .read_addr_fifo_push_i(read_addr_fifo_push),
      .write_fifo_push_i(write_fifo_push_control),

      .read_fifo_pop_i(read_fifo_pop),
      .read_addr_fifo_pop_i(read_addr_fifo_pop),
      .write_fifo_pop_i(write_fifo_pop),

      .read_fifo_output_o(read_fifo_output),
      .read_addr_fifo_output_o(read_addr_fifo_output),
      .write_fifo_output_o(write_fifo_output),

      .read_fifo_alm_full_o(read_fifo_alm_full),
      .read_addr_fifo_alm_full_o(read_addr_fifo_alm_full),
      .write_fifo_alm_full_o(write_fifo_internal_alm_full),

      .read_fifo_full_o(read_fifo_full),
      .read_addr_fifo_full_o(read_addr_fifo_full),
      .write_fifo_full_o(write_fifo_internal_full),

      .read_fifo_empty_o(read_fifo_empty),
      .read_addr_fifo_empty_o(read_addr_fifo_empty),
      .write_fifo_empty_o(write_fifo_internal_empty)
  );

  dma_buffer_control #(
    .SUBADDR_MODE_EN,
    .ADDR_MODE_EN,
    .HW_FIFO_MODE_EN,
    .NUM_READ_FIFOS
  ) dma_buffer_control_i (
    .clk_i,
    .rst_ni,
    .dma_start_i,
    .reg2hw_i,
    .read_fifo_alm_full_i(read_fifo_alm_full),
    .read_addr_fifo_alm_full_i(read_addr_fifo_alm_full),
    .write_fifo_alm_full_i(write_fifo_alm_full),

    .read_fifo_full_i(read_fifo_full),
    .read_addr_fifo_full_i(read_addr_fifo_full),
    .write_fifo_full_i(write_fifo_full),

    .read_fifo_empty_i(read_fifo_empty),
    .read_addr_fifo_empty_i(read_addr_fifo_empty),
    .write_fifo_empty_i(write_fifo_empty),

    .read_data_pop_i(read_data_pop),
    .read_data_push_i(read_data_push),
    .read_addr_data_pop_i(read_addr_data_pop),
    .read_addr_data_push_i(read_addr_data_push),
    .write_data_pop_i(write_data_pop),
    .write_data_push_i(write_data_push),

    .read_fifo_push_o(read_fifo_push),
    .read_fifo_pop_o(read_fifo_pop),
    .read_addr_fifo_push_o(read_addr_fifo_push),
    .read_addr_fifo_pop_o(read_addr_fifo_pop),
    .write_fifo_push_o(write_fifo_push),
    .write_fifo_pop_o(write_fifo_pop),

    .write_data_mask_o(write_data_mask)
 );

 /* Generate HW FIFO logic when Subaddressing mode is enabled */
  generate
    if (SUBADDR_MODE_EN == 1) begin
      logic subaddr_push;
      logic subaddr_pop;
      logic subaddr_full;
      logic subaddr_empty;

      assign hw_fifo_req_o.pop = write_data
      
    end
  endgenerate

  /* Generate HW FIFO logic when Subaddressing mode is enabled */
  generate
    if (SUBADDR_MODE_EN == 1) begin
      logic subaddr_push;
      logic subaddr_pop;
      logic subaddr_full;
      logic subaddr_empty;

      always_comb begin
        if (subaddr_mode == 1) begin
          write_fifo_push_control = 0;
          write_fifo_empty = hw_fifo_resp_i.empty;
          write_fifo_full = hw_fifo_resp_i.full;
          hw_fifo_req_o.push = write_fifo_push;
        end else begin
          write_fifo_push_control = write_fifo_push;
          hw_fifo_req_o.push = '0;
        end
      end
    end
  endgenerate

  assign write_fifo_input = write_fifo_input_i & write_data_mask;
  assign hw_fifo_req_o.data = write_fifo_input;

endmodule
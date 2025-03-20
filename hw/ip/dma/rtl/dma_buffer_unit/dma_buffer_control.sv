/*
 * Copyright 2025 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *
 * Author: Tommaso Terzano <tommaso.terzano@gmail.com>
 *  
 * Info: This module contains all the logic that governs the DMA FIFO control, which means POP & PUSH signals. 
 *       With the HW FIFO mode, the internal read FIFO(s) of the buffer unit are replaced by external FIFOs of a tighly
 *       coupled accelerator. This is the data flow:
 *       TO BE CONTINUED...
 *       This module is declared in `dma_buffer_unit.sv`, together with the `dma_buffer_fifos.sv`.
 */

module dma_buffer_control
  import dma_reg_pkg::*;
#(
  parameter SUBADDR_MODE_EN = 1,
  parameter ADDR_MODE_EN = 1,
  parameter HW_FIFO_MODE_EN = 1,
  parameter NUM_READ_FIFOS = (SUBADDR_MODE_EN ? 4 : 1)
 ) (
    input logic clk_i,
    input logic rst_ni,  
    input logic dma_start_i,  

    input dma_reg2hw_t reg2hw_i,

    input logic read_fifo_alm_full_i,
    input logic read_addr_fifo_alm_full_i,
    input logic write_fifo_alm_full_i,

    input logic read_fifo_full_i,
    input logic read_addr_fifo_full_i,
    input logic write_fifo_full_i,

    input logic read_fifo_empty_i,
    input logic read_addr_fifo_empty_i,
    input logic write_fifo_empty_i,
    
    input logic read_data_pop_i,
    input logic read_data_push_i,
    input logic read_addr_data_pop_i,
    input logic read_addr_data_push_i,
    input logic write_data_pop_i,
    input logic write_data_push_i,

    output logic read_fifo_push_o,
    output logic read_addr_fifo_push_o,
    output logic write_fifo_push_o,

    output logic [NUM_READ_FIFOS-1:0] read_fifo_pop_o,
    output logic                      read_addr_fifo_pop_o,
    output logic                      write_fifo_pop_o,

    output logic [31:0] write_data_mask_o

 );

  logic dma_start;
  logic read_data_pop;
  logic [31:0] write_data_mask;
  logic [NUM_READ_FIFOS-1:0] read_fifo_pop;

  /* This logic enables Subaddressing Mode, when enabled */
  generate 
    if (SUBADDR_MODE_EN == 1) begin
      logic [3:0] mask_read_fifo_pop;

      always_ff @(posedge clk_i, negedge rst_ni) begin
        if (~rst_ni) begin
          mask_read_fifo_pop <= 4'b0000;
        end else begin
          if (subaddr_mode == 1'b1) begin
            case (src_data_type)
              DMA_DATA_TYPE_HALF_WORD: begin
                if (dma_start == 1'b1) begin
                  mask_read_fifo_pop <= 4'b0011;
                end else if (read_data_pop == 1'b1) begin
                  if (mask_read_fifo_pop == 4'b1100) begin
                    mask_read_fifo_pop <= 4'b0011;
                  end else begin
                    mask_read_fifo_pop <= 4'b1100;
                  end
                end
              end

              DMA_DATA_TYPE_BYTE: begin
                if (dma_start == 1'b1) begin
                  mask_read_fifo_pop <= 4'b0001;
                end else if (read_data_pop == 1'b1) begin
                  if (mask_read_fifo_pop == 4'b1000) begin
                    mask_read_fifo_pop <= 4'b0001;
                  end else begin
                    mask_read_fifo_pop <= mask_read_fifo_pop << 1;
                  end
                end
              end

              default: mask_read_fifo_pop <= {4{read_data_pop}};
            endcase
          end else begin
            mask_read_fifo_pop <= {4{read_data_pop}};
          end
        end
      end
    end
  endgenerate

  /* This adds the logic for the HW FIFO when enabled */
  // TODO: fix this shit, it makes no sense
  generate
    if (HW_FIFO_MODE_EN == 1) begin
      logic hw_fifo_read_fifo_pop;

      always_ff @(posedge clk_i, negedge rst_ni) begin
        if (~rst_ni) begin
          hw_fifo_read_fifo_pop <= 1'b0;
        end else begin
          if (hw_fifo_mode == 1'b1) begin
            hw_fifo_read_fifo_pop <= 1'b0;
          end else begin
            hw_fifo_read_fifo_pop <= read_data_pop;
          end
        end
      end
    end
  endgenerate

  /* This combines the pop signals with different modes combinations */
  // TODO: fix this shit, it makes no sense
  generate
    if (SUBADDR_MODE_EN == 1 & HW_FIFO_MODE_EN == 1) begin
      read_fifo_pop = mask_read_fifo_pop & {4, hw_fifo_read_fifo_pop};
    end else if (SUBADDR_MODE_EN == 1 & HW_FIFO_MODE_EN == 0) begin
      read_fifo_pop = mask_read_fifo_pop;
    end else if (SUBADDR_MODE_EN == 0 & HW_FIFO_MODE_EN == 1) begin
      read_fifo_pop = hw_fifo_read_fifo_pop;
    end else begin
      read_fifo_pop = read_data_pop;
    end
  endgenerate

  /* This adds the logic for the write mask when Subaddressing Mode enabled */
  always_comb begin
    write_data_mask = 1';
    if (subaddr_mode == 1'b1) begin
      write_data_mask = {{8{mask_read_fifo_pop[3]}},{8{mask_read_fifo_pop[2]}},{8{mask_read_fifo_pop[1]}},{8{mask_read_fifo_pop[0]}}};
    end
  end

  always_comb begin
    if (subaddr_mode == 1'b1) begin
      write_fifo_input = pad_write_fifo_input;
      if (read_data_pop_i == 1'b1) begin
        case (src_data_type)
          DMA_DATA_TYPE_HALF_WORD: begin

            if (mask_read_fifo_pop == 4'b1100) begin
              write_fifo_input = {{16{1'b0}}, pad_write_fifo_input[31:16]};
            end else if (mask_read_fifo_pop == 4'b0011) begin
              write_fifo_input = {{16{1'b0}}, pad_write_fifo_input[15:0]};
            end else begin
              write_fifo_input = pad_write_fifo_input;
            end

          end

          DMA_DATA_TYPE_BYTE: begin

            else if (mask_read_fifo_pop == 4'b1000) begin
              write_fifo_input = {{24{1'b0}}, pad_write_fifo_input[31:24]};
            end else if (mask_read_fifo_pop == 4'b0100) begin
              write_fifo_input = {{24{1'b0}}, pad_write_fifo_input[23:16]};
            end else if (mask_read_fifo_pop == 4'b0010) begin
              write_fifo_input = {{24{1'b0}}, pad_write_fifo_input[15:8]};
            end else if (mask_read_fifo_pop == 4'b0001) begin
              write_fifo_input = {{24{1'b0}}, pad_write_fifo_input[7:0]};
            end else begin
              write_fifo_input = pad_write_fifo_input;
            end

          end

          default: begin
            write_fifo_input = pad_write_fifo_input;
            read_fifo_pop = {4{read_data_pop_i}};
          end

        endcase
      end else begin
        // no pop from read fifo issued by padding fsm
        write_fifo_input = '0;
        write_fifo_push = 1'b0;
        read_fifo_pop = {4{read_data_pop_i}};
      end
    end else if (hw_fifo_mode == 1'b1) begin
      // hw fifo mode: no pop and no push are needed from the internal read fifo
      // and to the internal write fifo respectively
      write_fifo_input = '0;
      write_fifo_push = 1'b0;
      read_fifo_pop = {4{1'b0}};
    end else begin
      // other modes
      write_fifo_input = pad_write_fifo_input;
      write_fifo_push = pad_write_fifo_push;
      read_fifo_pop = {4{read_data_pop_i}};
    end
  end

  assign single_mode = reg2hw.mode.q == 0;
  assign circular_mode = reg2hw.mode.q == 1;
  assign dma_start = dma_start_i;
  assign read_data_pop = read_data_pop_i;

  generate
    if (SUBADDR_MODE_EN == 1) begin
      assign subaddr_mode = reg2hw.mode.q == 2;
    end

    if (ADDR_MODE_EN == 1) begin
      assign addr_mode = reg2hw.mode.q == 3;
    end

    if (HW_FIFO_MODE_EN == 1) begin
      assign hw_fifo_mode = reg2hw.mode.q == 4;
    end
  endgenerate
  
 endmodule
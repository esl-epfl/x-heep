/*
 * Copyright 2024 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *
 * Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *                         <tommaso.terzano@gmail.com>
 *  
 * Info: Reading FSM for DMA channel in address mode, controls the input FIFO.
 */

module dma_obiread_addr_fsm
  import dma_reg_pkg::*;
#(
) (
    input logic clk_i,
    input logic rst_ni,
    input dma_reg2hw_t reg2hw_i,
    input logic dma_start_i,
    input logic ext_dma_stop_i,
    input logic read_fifo_addr_full_i,
    input logic read_fifo_addr_alm_full_i,
    input logic address_mode_i,
    input logic data_in_gnt_i,
    output logic data_addr_in_req_o,
    output logic data_addr_in_we_o,
    output logic [3:0] data_addr_in_be_o,
    output logic [31:0] data_addr_in_addr_o
);

  /*_________________________________________________________________________________________________________________________________ */

  /* Parameter definition */

  import dma_reg_pkg::*;

  /*_________________________________________________________________________________________________________________________________ */

  /* Signals declaration */

  /* Registers */
  dma_reg2hw_t reg2hw;

  /* Control signals */
  logic dma_start;
  logic ext_dma_stop;

  enum logic {
    DMA_READ_FSM_IDLE,
    DMA_READ_FSM_ON
  }
      dma_read_addr_fsm_state, dma_read_addr_fsm_n_state;

  logic fifo_addr_full;
  logic fifo_addr_alm_full;

  logic data_addr_in_gnt;
  logic data_addr_in_req;
  logic data_addr_in_we;
  logic [3:0] data_addr_in_be;
  logic [31:0] data_addr_in_addr;

  logic address_mode;
  logic [31:0] addr_ptr_reg;
  logic [31:0] dma_addr_cnt;

  /*_________________________________________________________________________________________________________________________________ */

  /* FSMs instantiation */

  // Store address data pointer and increment everytime read request is granted - only in address mode
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_ptr_addr_reg
    if (~rst_ni) begin
      addr_ptr_reg <= '0;
    end else begin
      if (dma_start == 1'b1 && address_mode) begin
        addr_ptr_reg <= reg2hw.addr_ptr.q;
      end else if (data_addr_in_gnt == 1'b1 && address_mode) begin
        addr_ptr_reg <= addr_ptr_reg + 32'h4;  //always continuos in 32b
      end
    end
  end

  // Store dma transfer size for the address port
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_dma_addr_cnt_reg
    if (~rst_ni) begin
      dma_addr_cnt <= '0;
    end else begin
      if (dma_start == 1'b1 && address_mode) begin
        dma_addr_cnt <= {16'h0, reg2hw.size_d1.q};
      end else if (data_addr_in_gnt == 1'b1 && address_mode) begin
        dma_addr_cnt <= dma_addr_cnt - 1;  //address always 32b
      end
    end
  end

  // FSM state update
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_fsm_state
    if (~rst_ni) begin
      dma_read_addr_fsm_state <= DMA_READ_FSM_IDLE;
    end else begin
      dma_read_addr_fsm_state <= dma_read_addr_fsm_n_state;
    end
  end

  // Read address master FSM
  always_comb begin : proc_dma_addr_read_fsm_logic

    dma_read_addr_fsm_n_state = DMA_READ_FSM_IDLE;

    data_addr_in_req = '0;
    data_addr_in_we = '0;
    data_addr_in_be = '0;
    data_addr_in_addr = '0;


    unique case (dma_read_addr_fsm_state)

      DMA_READ_FSM_IDLE: begin
        // Wait for start signal
        if (dma_start == 1'b1 && address_mode) begin
          dma_read_addr_fsm_n_state = DMA_READ_FSM_ON;
        end else begin
          dma_read_addr_fsm_n_state = DMA_READ_FSM_IDLE;
        end
      end
      // Read one word
      DMA_READ_FSM_ON: begin
        if (ext_dma_stop == 1'b0) begin
          // If all input data read exit
          if (|dma_addr_cnt == 1'b0) begin
            dma_read_addr_fsm_n_state = DMA_READ_FSM_IDLE;
          end else begin
            dma_read_addr_fsm_n_state = DMA_READ_FSM_ON;
            // Wait if fifo is full, almost full (last data), or if the SPI RX does not have valid data (only in SPI mode 1).
            if (fifo_addr_full == 1'b0 && fifo_addr_alm_full == 1'b0) begin
              data_addr_in_req  = 1'b1;
              data_addr_in_we   = 1'b0;
              data_addr_in_be   = 4'b1111;  // always read all bytes
              data_addr_in_addr = addr_ptr_reg;
            end
          end
        end else begin
          dma_read_addr_fsm_n_state = DMA_READ_FSM_IDLE;
        end
      end
    endcase
  end

  /*_________________________________________________________________________________________________________________________________ */

  /* Signal assignments */

  /* Renaming */
  assign address_mode = address_mode_i;
  assign reg2hw = reg2hw_i;
  assign data_addr_in_gnt = data_in_gnt_i;
  assign fifo_addr_full = read_fifo_addr_full_i;
  assign fifo_addr_alm_full = read_fifo_addr_alm_full_i;
  assign dma_start = dma_start_i;
  assign ext_dma_stop = ext_dma_stop_i;
  assign data_addr_in_be_o = data_addr_in_be;
  assign data_addr_in_addr_o = data_addr_in_addr;
  assign data_addr_in_req_o = data_addr_in_req;
  assign data_addr_in_we_o = data_addr_in_we;


endmodule

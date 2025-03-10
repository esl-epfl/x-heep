/*
 * Copyright 2024 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *
 * Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *                         <tommaso.terzano@gmail.com>
 *  
 * Info: Reading FSM for DMA channel, controls the input FIFO.
 */

module dma_obiread_fsm
  import dma_reg_pkg::*;
#(
) (
    input logic clk_i,
    input logic rst_ni,
    input dma_reg2hw_t reg2hw_i,
    input logic dma_start_i,
    input logic dma_done_i,
    input logic ext_dma_stop_i,
    input logic read_fifo_full_i,
    input logic read_fifo_alm_full_i,
    input logic wait_for_rx_i,
    input logic data_in_gnt_i,
    input logic data_in_rvalid_i,
    input logic [31:0] data_in_rdata_i,

    output logic [31:0] fifo_input_o,
    output logic data_in_req_o,
    output logic data_in_we_o,
    output logic [3:0] data_in_be_o,
    output logic [31:0] data_in_addr_o,
    output logic read_fifo_flush_o
);

  /*_________________________________________________________________________________________________________________________________ */

  /* Parameter definition */

  import dma_reg_pkg::*;

  /*_________________________________________________________________________________________________________________________________ */

  /* Signals declaration */

  /* Registers */
  dma_reg2hw_t reg2hw;

  enum logic {
    DMA_READ_FSM_IDLE,
    DMA_READ_FSM_ON
  }
      dma_read_fsm_state, dma_read_fsm_n_state;

  logic data_in_gnt;
  logic fifo_full;
  logic fifo_alm_full;
  logic fifo_flush;
  logic dma_start;
  logic ext_dma_stop;
  logic read_ptr_update_sel;
  logic dma_conf_1d;
  logic dma_conf_2d;
  logic data_in_rvalid;

  logic wait_for_rx;

  logic [16:0] dma_src_cnt_d1;
  logic [16:0] dma_src_cnt_d2;

  logic [31:0] trsp_src_ptr_reg;
  logic [31:0] read_ptr_reg;

  logic data_in_req;
  logic data_in_we;
  logic [3:0] data_in_be;
  logic [31:0] data_in_addr;
  logic [31:0] data_in_rdata;

  logic [31:0] read_ptr_valid_reg;
  logic [31:0] dma_src_d1_inc;
  logic [31:0] dma_src_d2_inc;

  /* FIFO signals */
  logic [31:0] fifo_input;

  /*_________________________________________________________________________________________________________________________________ */

  /* FSMs instantiation */

  /* Sign extension of the increments */
  always_comb begin
    dma_src_d1_inc = {{26{reg2hw.src_ptr_inc_d1.q[5]}}, reg2hw.src_ptr_inc_d1.q};
    dma_src_d2_inc = {{9{reg2hw.src_ptr_inc_d2.q[22]}}, reg2hw.src_ptr_inc_d2.q};
  end

  /* Counters for the reading fsm */
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_dma_src_cnt_reg
    if (~rst_ni) begin
      dma_src_cnt_d1 <= '0;
      dma_src_cnt_d2 <= '0;
    end else begin
      if (dma_start == 1'b1) begin
        dma_src_cnt_d1 <= {1'h0, reg2hw.size_d1.q};
        dma_src_cnt_d2 <= {1'h0, reg2hw.size_d2.q};
      end else if (dma_done_i == 1'b1) begin
        dma_src_cnt_d1 <= '0;
        dma_src_cnt_d2 <= '0;
      end else if (data_in_gnt == 1'b1) begin
        if (dma_conf_1d == 1'b1) begin
          // 1D case
          dma_src_cnt_d1 <= dma_src_cnt_d1 - 1;
        end else if (dma_conf_2d == 1'b1) begin
          // 2D case
          if (dma_src_cnt_d1 == 1) begin
            // In this case, the d1 is finished, so we need to decrement the d2 size and reset the d2 size
            dma_src_cnt_d2 <= dma_src_cnt_d2 - 1;
            dma_src_cnt_d1 <= {1'h0, reg2hw.size_d1.q};
          end else begin
            // In this case, the d1 isn't finished, so we need to decrement the d1 size
            dma_src_cnt_d1 <= dma_src_cnt_d1 - 1;
          end
        end
      end
    end
  end

  /* 
   * Store input data pointer in source_ptr_reg and increment it every time read request is granted, 
   * if the d1 has finished reading and the read pointer update is set to 1'b1 
   */

  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_src_ptr_reg
    if (~rst_ni) begin
      trsp_src_ptr_reg <= '0;
    end else begin
      if (dma_start == 1'b1) begin
        trsp_src_ptr_reg <= reg2hw.src_ptr.q + dma_src_d1_inc;
      end else if (data_in_gnt == 1'b1 && dma_conf_2d == 1'b1 && read_ptr_update_sel == 1'b1 &&
                    (dma_src_cnt_d1 == 1 && |dma_src_cnt_d2 == 1'b1)) begin
        trsp_src_ptr_reg <= trsp_src_ptr_reg + dma_src_d1_inc;
      end
    end
  end

  /* Store input data pointer and increment everytime read request is granted */
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_ptr_in_reg
    if (~rst_ni) begin
      read_ptr_reg <= '0;
    end else begin
      if (dma_start == 1'b1) begin
        read_ptr_reg <= reg2hw.src_ptr.q;
      end else if (data_in_gnt == 1'b1) begin
        if (dma_conf_1d == 1'b1) begin
          /* Increase the pointer by the amount written in ptr_inc */
          read_ptr_reg <= read_ptr_reg + dma_src_d1_inc;
        end else if (dma_conf_2d == 1'b1) begin
          if (read_ptr_update_sel == 1'b0) begin
            if (dma_src_cnt_d1 == 1 && |dma_src_cnt_d2 == 1'b1) begin
              /* In this case, the d1 is almost finished, so we need to increment the pointer by sizeof(d1)*data_unit */
              read_ptr_reg <= read_ptr_reg + dma_src_d2_inc;
            end else begin
              read_ptr_reg <= read_ptr_reg + dma_src_d1_inc; /* Increment of the d1 increment (stride) */
            end
          end else begin
            /* In this case, perform the transposition */
            if (dma_src_cnt_d1 == 1 && |dma_src_cnt_d2 == 1'b1) begin
              /* In this case, the d1 is almost finished, so we need to increment the pointer by sizeof(d1)*data_unit */
              read_ptr_reg <= trsp_src_ptr_reg;
            end else begin
              read_ptr_reg <= read_ptr_reg + dma_src_d2_inc; /* Increment of the d2 increment (stride) */
            end
          end
        end
      end
    end
  end

  // FSM state update
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_fsm_state
    if (~rst_ni) begin
      dma_read_fsm_state <= DMA_READ_FSM_IDLE;
    end else begin
      dma_read_fsm_state <= dma_read_fsm_n_state;
    end
  end

  // Read master FSM
  always_comb begin : proc_dma_read_fsm_logic

    dma_read_fsm_n_state = DMA_READ_FSM_IDLE;

    data_in_req = '0;
    data_in_we = '0;
    data_in_be = '0;
    data_in_addr = '0;

    fifo_flush = 1'b0;

    unique case (dma_read_fsm_state)

      DMA_READ_FSM_IDLE: begin
        // Wait for start signal
        if (dma_start == 1'b1) begin
          dma_read_fsm_n_state = DMA_READ_FSM_ON;
        end else begin
          dma_read_fsm_n_state = DMA_READ_FSM_IDLE;
        end
      end
      // Read one word
      DMA_READ_FSM_ON: begin
        // If all input data read exit
        if (ext_dma_stop == 1'b0) begin
          if (dma_conf_1d == 1'b1) begin
            // 1D DMA case
            if (|dma_src_cnt_d1 == 1'b0) begin
              dma_read_fsm_n_state = DMA_READ_FSM_IDLE;
            end else begin
              dma_read_fsm_n_state = DMA_READ_FSM_ON;
              // Wait if fifo is full, almost full (last data), or if the SPI RX does not have valid data (only in SPI mode 1).
              if (fifo_full == 1'b0 && fifo_alm_full == 1'b0 && wait_for_rx == 1'b0) begin
                data_in_req  = 1'b1;
                data_in_we   = 1'b0;
                data_in_be   = 4'b1111;  // always read all bytes
                data_in_addr = read_ptr_reg;
              end
            end
          end else if (dma_conf_2d == 1'b1) begin
            // 2D DMA case: exit only if both 1d and 2d counters are at 0
            if (dma_src_cnt_d1 == {1'h0, reg2hw.size_d1.q} && |dma_src_cnt_d2 == 1'b0) begin
              dma_read_fsm_n_state = DMA_READ_FSM_IDLE;
            end else begin
              // The read operation is the same in both cases
              dma_read_fsm_n_state = DMA_READ_FSM_ON;
              // Wait if fifo is full, almost full (last data), or if the SPI RX does not have valid data (only in SPI mode 1).
              if (fifo_full == 1'b0 && fifo_alm_full == 1'b0 && wait_for_rx == 1'b0) begin
                data_in_req  = 1'b1;
                data_in_we   = 1'b0;
                data_in_be   = 4'b1111;  // always read all bytes
                data_in_addr = read_ptr_reg;
              end
            end
          end
        end else begin
          dma_read_fsm_n_state = DMA_READ_FSM_IDLE;
        end
      end
    endcase
  end

  // Only update read_ptr_valid_reg when the data is stored in the fifo.
  // Since every input grant is followed by a rvalid, the read_ptr_valid_reg is a mere sample of the read_ptr_reg
  // synched with the rvalid signal.
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_ptr_valid_in_reg
    if (~rst_ni) begin
      read_ptr_valid_reg <= '0;
    end else begin
      if (dma_start == 1'b1) begin
        read_ptr_valid_reg <= reg2hw.src_ptr.q;
      end else if (data_in_rvalid == 1'b1) begin
        read_ptr_valid_reg <= read_ptr_reg;
      end
    end
  end

  // Input data shift: shift the input data to be on the LSB of the fifo
  always_comb begin : proc_input_data

    fifo_input[7:0]   = data_in_rdata[7:0];
    fifo_input[15:8]  = data_in_rdata[15:8];
    fifo_input[23:16] = data_in_rdata[23:16];
    fifo_input[31:24] = data_in_rdata[31:24];

    case (read_ptr_valid_reg[1:0])
      2'b00: ;
      2'b01: fifo_input[7:0] = data_in_rdata[15:8];

      2'b10: begin
        fifo_input[7:0]  = data_in_rdata[23:16];
        fifo_input[15:8] = data_in_rdata[31:24];
      end

      2'b11: fifo_input[7:0] = data_in_rdata[31:24];
    endcase
  end

  /*_________________________________________________________________________________________________________________________________ */

  /* Signal assignments */

  /* Renaming */
  assign reg2hw = reg2hw_i;
  assign data_in_gnt = data_in_gnt_i;
  assign fifo_full = read_fifo_full_i;
  assign fifo_alm_full = read_fifo_alm_full_i;
  assign dma_start = dma_start_i;
  assign ext_dma_stop = ext_dma_stop_i;
  assign read_ptr_update_sel = reg2hw.dim_inv.q;
  assign dma_conf_1d = reg2hw.dim_config.q == 0;
  assign dma_conf_2d = reg2hw.dim_config.q == 1;
  assign data_in_be_o = data_in_be;
  assign data_in_addr_o = data_in_addr;
  assign data_in_req_o = data_in_req;
  assign data_in_we_o = data_in_we;
  assign read_fifo_flush_o = fifo_flush;
  assign wait_for_rx = wait_for_rx_i;
  assign data_in_rvalid = data_in_rvalid_i;
  assign data_in_rdata = data_in_rdata_i;
  assign fifo_input_o = fifo_input;


endmodule

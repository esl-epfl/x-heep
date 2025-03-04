/*
 * Copyright 2024 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *
 * Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *                         <tommaso.terzano@gmail.com>
 *  
 * Info: Writing FSM for DMA channel, process data coming out of the output FIFO. Performs the sign extension if needed.
 */

module dma_obiwrite_fsm
  import dma_reg_pkg::*;
#(
) (
    input logic clk_i,
    input logic rst_ni,
    input dma_reg2hw_t reg2hw_i,
    input logic dma_start_i,
    input logic write_fifo_empty_i,
    input logic read_addr_fifo_empty_i,
    input logic [31:0] fifo_output_i,
    input logic wait_for_tx_i,
    input logic address_mode_i,
    input logic padding_fsm_done_i,
    input logic data_out_gnt_i,
    input logic [31:0] fifo_addr_output_i,

    output logic data_out_req_o,
    output logic data_out_we_o,
    output logic [3:0] data_out_be_o,
    output logic [31:0] data_out_addr_o,
    output logic [31:0] data_out_wdata_o,
    output logic dma_done_o
);

  /*_________________________________________________________________________________________________________________________________ */

  /* Parameter definition */

  import dma_reg_pkg::*;

  /*_________________________________________________________________________________________________________________________________ */

  /* Signals declaration */

  /* Registers */
  dma_reg2hw_t reg2hw;

  logic dma_conf_1d;
  logic dma_conf_2d;
  logic dma_done;
  logic address_mode;
  logic dma_start;

  enum logic {
    DMA_WRITE_FSM_IDLE,
    DMA_WRITE_FSM_ON
  }
      dma_write_fsm_state, dma_write_fsm_n_state;

  typedef enum logic [1:0] {
    DMA_DATA_TYPE_WORD,
    DMA_DATA_TYPE_HALF_WORD,
    DMA_DATA_TYPE_BYTE,
    DMA_DATA_TYPE_BYTE_
  } dma_data_type_t;

  dma_data_type_t dst_data_type;
  dma_data_type_t src_data_type;

  logic data_out_req;
  logic data_out_we;
  logic data_out_gnt;
  logic [3:0] data_out_be;
  logic [31:0] data_out_addr;
  logic [31:0] data_out_wdata;
  logic [31:0] write_address;
  logic [31:0] write_ptr_reg;
  logic [3:0] byte_enable_out;

  logic [31:0] dma_dst_d1_inc;
  logic [31:0] dma_dst_d2_inc;
  logic [16:0] dma_dst_cnt_d1;

  logic wait_for_tx;

  /* Sign extension signals */
  logic sign_extend;

  /* FIFO signals */
  logic write_fifo_empty;
  logic read_addr_fifo_empty;
  logic [31:0] fifo_output;

  /*_________________________________________________________________________________________________________________________________ */

  /* FSMs instantiation */

  /* Sign extension of the increments */
  always_comb begin
    dma_dst_d1_inc = {{26{reg2hw.dst_ptr_inc_d1.q[5]}}, reg2hw.dst_ptr_inc_d1.q};
    dma_dst_d2_inc = {{9{reg2hw.dst_ptr_inc_d2.q[22]}}, reg2hw.dst_ptr_inc_d2.q};
  end

  /* Counters for the reading fsm */
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_dma_dst_cnt_reg
    if (~rst_ni) begin
      dma_dst_cnt_d1 <= '0;
    end else begin
      if (dma_start == 1'b1) begin
        dma_dst_cnt_d1 <= {1'h0, reg2hw.size_d1.q} + {11'h0, reg2hw.pad_left.q} + {11'h0, reg2hw.pad_right.q};
      end else if (dma_done == 1'b1) begin
        dma_dst_cnt_d1 <= '0;
      end else if (data_out_gnt == 1'b1) begin
        if (dma_conf_1d == 1'b1) begin
          // 1D case
          dma_dst_cnt_d1 <= dma_dst_cnt_d1 - 1;
        end else if (dma_conf_2d == 1'b1) begin
          // 2D case
          if (dma_dst_cnt_d1 == 1) begin
            // In this case, the d1 is finished, so we need to reset the d2 size
            dma_dst_cnt_d1 <= {1'h0, reg2hw.size_d1.q} + {11'h0, reg2hw.pad_left.q} + {11'h0, reg2hw.pad_right.q};
          end else begin
            // In this case, the d1 isn't finished, so we need to decrement the d1 size
            dma_dst_cnt_d1 <= dma_dst_cnt_d1 - 1;
          end
        end
      end
    end
  end

  /* Determine the byte enable depending on the datatype */
  always_comb begin : proc_byte_enable_out
    case (dst_data_type)  // Data type 00 Word, 01 Half word, 11,10 byte
      DMA_DATA_TYPE_WORD: byte_enable_out = 4'b1111;  // Writing a word (32 bits)

      DMA_DATA_TYPE_HALF_WORD: begin  // Writing a half-word (16 bits)
        case (write_ptr_reg[1])
          1'b0: byte_enable_out = 4'b0011;
          1'b1: byte_enable_out = 4'b1100;
        endcase
        ;  // case(write_ptr_reg[1:0])
      end

      DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_BYTE_: begin  // Writing a byte (8 bits)
        case (write_ptr_reg[1:0])
          2'b00: byte_enable_out = 4'b0001;
          2'b01: byte_enable_out = 4'b0010;
          2'b10: byte_enable_out = 4'b0100;
          2'b11: byte_enable_out = 4'b1000;
        endcase
        ;  // case(write_ptr_reg[1:0])
      end
    endcase
    ;  // case (dst_data_type)
  end

  /* Store output data pointer and increment everytime write request is granted */
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_ptr_out_reg
    if (~rst_ni) begin
      write_ptr_reg <= '0;
    end else begin
      if (dma_start == 1'b1) begin
        write_ptr_reg <= reg2hw.dst_ptr.q;
      end else if (data_out_gnt == 1'b1) begin
        if (dma_conf_1d == 1'b1) begin
          write_ptr_reg <= write_ptr_reg + dma_dst_d1_inc;
        end else if (dma_conf_2d == 1'b1) begin
          if (dma_dst_cnt_d1 == 1) begin
            // In this case, the d1 is finished, so we need to increment the pointer by sizeof(d1)*data_unit*strides
            write_ptr_reg <= write_ptr_reg + dma_dst_d2_inc;
          end else begin
            write_ptr_reg <= write_ptr_reg + dma_dst_d1_inc; // Increment just of one du, since we need to increase the 1d
          end
        end
      end
    end
  end

  /* FSM state update */
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_fsm_state
    if (~rst_ni) begin
      dma_write_fsm_state <= DMA_WRITE_FSM_IDLE;
    end else begin
      dma_write_fsm_state <= dma_write_fsm_n_state;
    end
  end

  /* Write master FSM */
  always_comb begin : proc_dma_write_fsm_logic

    dma_write_fsm_n_state = DMA_WRITE_FSM_IDLE;
    dma_done = 1'b0;

    data_out_req = '0;
    data_out_we = '0;
    data_out_be = '0;
    data_out_addr = '0;

    unique case (dma_write_fsm_state)

      DMA_WRITE_FSM_IDLE: begin
        // Wait for start signal
        if (dma_start == 1'b1) begin
          dma_write_fsm_n_state = DMA_WRITE_FSM_ON;
        end else begin
          dma_write_fsm_n_state = DMA_WRITE_FSM_IDLE;
        end
      end
      // Read one word
      DMA_WRITE_FSM_ON: begin
        // If all input data read exit
        if (padding_fsm_done_i == 1'b1 && write_fifo_empty == 1'b1) begin
          dma_done = (write_fifo_empty == 1'b1);
          // If all input data has been processed and written, exit, otherwise finish storing the data
          dma_write_fsm_n_state = dma_done ? DMA_WRITE_FSM_IDLE : DMA_WRITE_FSM_ON;
        end else begin
          dma_write_fsm_n_state = DMA_WRITE_FSM_ON;
          // Wait if write fifo is empty or if the SPI TX is not ready for new data (only in SPI mode 2).
          if (write_fifo_empty == 1'b0 && wait_for_tx == 1'b0 && (read_addr_fifo_empty && address_mode) == 1'b0) begin
            data_out_req  = 1'b1;
            data_out_we   = 1'b1;
            data_out_be   = byte_enable_out;
            data_out_addr = write_address;
          end
        end
      end
    endcase
  end

  /* Perform the data shift */
  always_comb begin : proc_output_data

    data_out_wdata[7:0]   = fifo_output[7:0];
    data_out_wdata[15:8]  = fifo_output[15:8];
    data_out_wdata[23:16] = fifo_output[23:16];
    data_out_wdata[31:24] = fifo_output[31:24];

    if (address_mode == 1'b0) begin
      case (write_ptr_reg[1:0])
        2'b00: begin
          if (sign_extend) begin
            case ({
              src_data_type, dst_data_type
            })
              {DMA_DATA_TYPE_WORD, DMA_DATA_TYPE_WORD} : ;
              {
                DMA_DATA_TYPE_HALF_WORD, DMA_DATA_TYPE_WORD
              } :
              data_out_wdata[31:16] = {16{fifo_output[15]}};
              {
                DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_WORD
              }, {
                DMA_DATA_TYPE_BYTE_, DMA_DATA_TYPE_WORD
              } :
              data_out_wdata[31:8] = {24{fifo_output[7]}};
              {DMA_DATA_TYPE_HALF_WORD, DMA_DATA_TYPE_HALF_WORD} : ;
              {
                DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_HALF_WORD
              }, {
                DMA_DATA_TYPE_BYTE_, DMA_DATA_TYPE_HALF_WORD
              } :
              data_out_wdata[15:8] = {8{fifo_output[7]}};
              default: ;
            endcase
          end else begin
            case ({
              src_data_type, dst_data_type
            })
              {DMA_DATA_TYPE_WORD, DMA_DATA_TYPE_WORD} : ;
              {DMA_DATA_TYPE_HALF_WORD, DMA_DATA_TYPE_WORD} : data_out_wdata[31:16] = 16'b0;
              {
                DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_WORD
              }, {
                DMA_DATA_TYPE_BYTE_, DMA_DATA_TYPE_WORD
              } :
              data_out_wdata[31:8] = 24'b0;
              {DMA_DATA_TYPE_HALF_WORD, DMA_DATA_TYPE_HALF_WORD} : ;
              {
                DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_HALF_WORD
              }, {
                DMA_DATA_TYPE_BYTE_, DMA_DATA_TYPE_HALF_WORD
              } :
              data_out_wdata[15:8] = 8'b0;
              default: ;
            endcase
          end
        end
        2'b01:
        data_out_wdata[15:8] = fifo_output[7:0];  // Writing a byte, no need for sign extension
        2'b10: begin  // Writing a half-word or a byte
          data_out_wdata[23:16] = fifo_output[7:0];
          data_out_wdata[31:24] = fifo_output[15:8];

          if (sign_extend) begin
            case ({
              src_data_type, dst_data_type
            })
              {DMA_DATA_TYPE_HALF_WORD, DMA_DATA_TYPE_HALF_WORD} : ;
              {
                DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_HALF_WORD
              }, {
                DMA_DATA_TYPE_BYTE_, DMA_DATA_TYPE_HALF_WORD
              } :
              data_out_wdata[31:24] = {8{fifo_output[7]}};
              default: ;
            endcase
          end else begin
            case ({
              src_data_type, dst_data_type
            })
              {DMA_DATA_TYPE_HALF_WORD, DMA_DATA_TYPE_HALF_WORD} : ;
              {
                DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_HALF_WORD
              }, {
                DMA_DATA_TYPE_BYTE_, DMA_DATA_TYPE_HALF_WORD
              } :
              data_out_wdata[31:24] = 8'b0;
              default: ;
            endcase
          end
        end
        2'b11:
        data_out_wdata[31:24] = fifo_output[7:0];  // Writing a byte, no need for sign extension
      endcase
    end
  end

  /*_________________________________________________________________________________________________________________________________ */

  /* Signal assignments */

  /* Renaming */
  assign dma_start = dma_start_i;
  assign reg2hw = reg2hw_i;
  assign data_out_gnt = data_out_gnt_i;
  assign dma_done_o = dma_done;
  assign dst_data_type = dma_data_type_t'(reg2hw.dst_data_type.q);
  assign src_data_type = dma_data_type_t'(reg2hw.src_data_type.q);
  assign data_out_wdata_o = data_out_wdata;
  assign write_fifo_empty = write_fifo_empty_i;
  assign fifo_output = fifo_output_i;
  assign wait_for_tx = wait_for_tx_i;
  assign data_out_be_o = data_out_be;
  assign data_out_addr_o = data_out_addr;
  assign data_out_req_o = data_out_req;
  assign data_out_we_o = data_out_we;
  assign read_addr_fifo_empty = read_addr_fifo_empty_i;
  assign address_mode = address_mode_i;
  assign dma_conf_1d = reg2hw.dim_config.q == 0;
  assign dma_conf_2d = reg2hw.dim_config.q == 1;

  /* Write address */
  assign write_address = address_mode ? fifo_addr_output_i : write_ptr_reg;

  /* Sign extension */
  assign sign_extend = reg2hw.sign_ext.q & ( (src_data_type[1] & ~dst_data_type[1]) | ((src_data_type[1] == dst_data_type[1]) & (src_data_type[0] & ~dst_data_type[0])));


endmodule

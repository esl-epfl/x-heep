/*
 * Copyright 2024 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *
 * Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *                         <tommaso.terzano@gmail.com>
 *  
 * Info: Im2col accelerator implemented as a Smart Peripheral Controller. It accesses the DMA CH0 to perform
 *       the matrix manipulation operation known as "image to column" (im2col), which enables efficient
 *       CNN inference by transforming the input tensor to use the GEMM library.
 */

module im2col_spc
  import obi_pkg::*;
  import reg_pkg::*;
#(
) (
    input logic clk_i,
    input logic rst_ni,

    input  obi_resp_t aopx2im2col_resp_i,
    output obi_req_t  im2col2aopx_req_o,

    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    input logic [core_v_mini_mcu_pkg::DMA_CH_NUM-1:0] dma_done_i,
    output logic im2col_spc_done_int_o
);

  import core_v_mini_mcu_pkg::*;
  import dma_if_pkg::*;
  import im2col_spc_reg_pkg::*;

  /*_________________________________________________________________________________________________________________________________ */

  /* Parameter definition */

  /* DMA register offsets */
  localparam DMA_DIMENSIONALITY_OFFSET = 32'h34;
  localparam DMA_SRC_PTR_OFFSET = 32'h0;
  localparam DMA_DST_PTR_OFFSET = 32'h4;
  localparam DMA_INC_SRC_D1_OFFSET = 32'h18;
  localparam DMA_INC_SRC_D2_OFFSET = 32'h1C;
  localparam DMA_INC_DST_D1_OFFSET = 32'h20;
  localparam DMA_INC_DST_D2_OFFSET = 32'h24;
  localparam DMA_SIZE_D2_OFFSET = 32'h10;
  localparam DMA_SIZE_D1_OFFSET = 32'hC;
  localparam DMA_DATATYPE_OFFSET = 32'h2C;
  localparam DMA_TOP_PAD_OFFSET = 32'h3C;
  localparam DMA_BOTTOM_PAD_OFFSET = 32'h40;
  localparam DMA_RIGHT_PAD_OFFSET = 32'h44;
  localparam DMA_LEFT_PAD_OFFSET = 32'h48;

  /* FIFO dimension */
  localparam FIFO_DEPTH = 4;

  /*_________________________________________________________________________________________________________________________________ */

  /* Signals declaration */

  /* Control Unit signals */
  logic im2col_start;
  logic im2col_fsms_done;
  logic im2col_done;

  /* Interrupt management signals */
  logic im2col_spc_ifr;

  /* DMA FIFO signals */
  logic fifo_flush;
  logic fifo_full;
  logic fifo_empty;
  dma_if_t fifo_input;
  logic fifo_push;
  dma_if_t fifo_output;
  logic fifo_pop;
  logic [$clog2(FIFO_DEPTH)-1:0] fifo_usage;

  /* DMA interface unit signals */
  logic im2col_param_done;
  logic [DMA_CH_NUM-1:0] dma_if_channels;
  logic [DMA_CH_NUM-1:0] dma_ch_first_write;
  logic [(DMA_CH_NUM == 1) ? 0 : ($clog2(DMA_CH_NUM) - 1):0] dma_free_channel;
  logic [(DMA_CH_NUM == 1) ? 0 : ($clog2(DMA_CH_NUM) - 1):0] dma_trans_free_channel;
  logic [31:0] dma_ch_en_mask;

  logic dma_if_load;
  logic dma_if_loaded;
  logic dma_if_load_continue;
  logic dma_if_load_req;
  logic dma_if_load_rvalid;
  logic dma_channels_full;

  /* Regtool signals */
  im2col_spc_reg2hw_t reg2hw;
  im2col_spc_hw2reg_t hw2reg;

  /* Parameters computation signals */
  logic [31:0] w_offset;
  logic [31:0] h_offset;
  logic [31:0] im_c;
  logic [31:0] im_row;
  logic [31:0] im_col;
  logic [31:0] fw_min_w_offset;
  logic [31:0] fh_min_h_offset;
  logic [31:0] n_zeros_left;
  logic [31:0] n_zeros_right;
  logic [31:0] n_zeros_top;
  logic [31:0] n_zeros_bottom;
  logic [31:0] size_transfer_1d;
  logic [31:0] size_transfer_2d;
  logic [31:0] index;
  logic [31:0] h_offset_counter;
  logic [31:0] im_c_counter;
  logic [31:0] ch_col_counter;
  logic [31:0] batch_counter;
  logic [31:0] input_data_ptr;
  logic [31:0] output_data_ptr;
  logic [31:0] source_inc_d2;

  /* 
   * Every "comp" state refers to variable computations that precede the DMA call in the 
   * loop, while any "param" state refers to the DMA parameters computation that follows the DMA call.
   */
  enum {
    IDLE,
    F_MIN_OFFSET_COMP,
    IM_COORD_COMP,
    N_ZEROS_COMP,
    DMA_RUN_PARAM_COMP_1,
    DMA_RUN_PARAM_COMP_2,
    OUT_PTR_UPDATE,
    IM_OFFSET_UPDATE,
    START_DMA_RUN
  }
      param_state_q, param_state_d;

  enum {
    IDLE_IF_CU,
    GET_TRANSACTION,
    LOAD_TRANSACTION
  }
      dma_if_cu_q, dma_if_cu_d;

  enum {
    IDLE_IF_LOAD,
    WRITE_DIMENSIONALITY,
    WRITE_DATATYPE,
    WRITE_TOP_PAD,
    WRITE_BOTTOM_PAD,
    WRITE_LEFT_PAD,
    WRITE_RIGHT_PAD,
    WRITE_INPUT_PTR,
    WRITE_OUTPUT_PTR,
    WRITE_INC_SRC_D1,
    WRITE_INC_SRC_D2,
    WRITE_INC_DST_D1,
    WRITE_INC_DST_D2,
    WRITE_SIZE_D2,
    WRITE_SIZE_D1,
    DONE
  }
      dma_if_cu_load_q, dma_if_cu_load_d;

  /*_________________________________________________________________________________________________________________________________ */

  /* Module instantiation */

  /* Regtool top module */
  im2col_spc_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) im2col_spc_reg_top_i (
      .clk_i,
      .rst_ni,
      .reg_req_i,
      .reg_rsp_o,
      .reg2hw,
      .hw2reg,
      .devmode_i(1'b1)
  );

  /* DMA FIFO */
  fifo_v3 #(
      .DEPTH(FIFO_DEPTH),
      .FALL_THROUGH(1'b1),
      .dtype(dma_if_t)
  ) dma_fifo_i (
      .clk_i,
      .rst_ni,
      .flush_i(fifo_flush),
      .testmode_i(1'b0),
      // status flags
      .full_o(fifo_full),
      .empty_o(fifo_empty),
      .usage_o(fifo_usage),
      // as long as the queue is not full we can push new data
      .data_i(fifo_input),
      .push_i(fifo_push),
      // as long as the queue is not empty we can pop new elements
      .data_o(fifo_output),
      .pop_i(fifo_pop)
  );

  /*_________________________________________________________________________________________________________________________________ */

  /* FSMs instantiation */

  /* Parameter computation state transition FSM */
  always_comb begin : proc_comb_param_state_fsm
    unique case (param_state_d)
      IDLE: begin
        fifo_push = 1'b0;
        if (im2col_start == 1'b1) begin
          param_state_q = F_MIN_OFFSET_COMP;
        end else begin
          param_state_q = IDLE;
        end
      end

      F_MIN_OFFSET_COMP: begin
        fifo_push = 1'b0;
        param_state_q = IM_COORD_COMP;
      end

      IM_COORD_COMP: begin
        fifo_push = 1'b0;
        param_state_q = N_ZEROS_COMP;
      end

      N_ZEROS_COMP: begin
        fifo_push = 1'b0;
        param_state_q = DMA_RUN_PARAM_COMP_1;
      end

      DMA_RUN_PARAM_COMP_1: begin
        fifo_push = 1'b0;
        param_state_q = DMA_RUN_PARAM_COMP_2;
      end

      DMA_RUN_PARAM_COMP_2: begin
        fifo_push = 1'b0;
        if (fifo_full == 1'b0) begin
          param_state_q = START_DMA_RUN;
        end else begin
          param_state_q = DMA_RUN_PARAM_COMP_2;
        end
      end

      OUT_PTR_UPDATE: begin
        fifo_push = 1'b0;
        if (batch_counter == reg2hw.batch.q - 1) begin
          param_state_q = IM_OFFSET_UPDATE;
        end else begin
          param_state_q = IM_COORD_COMP;
        end
      end

      IM_OFFSET_UPDATE: begin
        fifo_push = 1'b0;
        if (im2col_param_done == 1'b1) begin
          param_state_q = IDLE;
        end else begin
          param_state_q = F_MIN_OFFSET_COMP;
        end
      end

      START_DMA_RUN: begin
        fifo_push = 1'b1;
        param_state_q = OUT_PTR_UPDATE;
      end
    endcase
  end

  /* Parameter computation logic */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_param_comp_fsm
    if (!rst_ni) begin
      ch_col_counter <= '0;
      h_offset_counter <= '0;
      im_c_counter <= '0;
      batch_counter <= '0;
      w_offset <= '0;
      h_offset <= '0;
      im_c <= '0;
      n_zeros_left <= '0;
      n_zeros_right <= '0;
      n_zeros_top <= '0;
      n_zeros_bottom <= '0;
      size_transfer_1d <= '0;
      size_transfer_2d <= '0;
      index <= '0;
      input_data_ptr <= '0;
      output_data_ptr <= reg2hw.dst_ptr.q;
    end else begin
      unique case (param_state_d)
        IDLE: begin
          ch_col_counter <= '0;
          h_offset_counter <= '0;
          im_c_counter <= '0;
          batch_counter <= '0;
          w_offset <= '0;
          h_offset <= '0;
          im_c <= '0;
          n_zeros_left <= '0;
          n_zeros_right <= '0;
          n_zeros_top <= '0;
          n_zeros_bottom <= '0;
          size_transfer_1d <= '0;
          size_transfer_2d <= '0;
          index <= '0;
          input_data_ptr <= '0;
          output_data_ptr <= reg2hw.dst_ptr.q;
        end

        F_MIN_OFFSET_COMP: begin
          fw_min_w_offset <= reg2hw.fw.q - 1 - w_offset;  // fw_minus_w_offset = FW - 1 - w_offset;
          fh_min_h_offset <= reg2hw.fh.q - 1 - h_offset;  // fh_minus_h_offset = FH - 1 - h_offset;
          batch_counter   <= '0;  // Reset the batch counter before starting a new batch
        end

        IM_COORD_COMP: begin
          im_row <= h_offset - {26'h0, reg2hw.pad_top.q};  // im_row = h_offset - TOP_PAD;
          im_col <= w_offset - {26'h0, reg2hw.pad_left.q};  // im_col = w_offset - LEFT_PAD;
        end

        N_ZEROS_COMP: begin

          /* Left zeros computation */
          if (w_offset >= reg2hw.pad_left.q) begin
            n_zeros_left <= 0;
          end else if (({26'h0, reg2hw.pad_left.q} - w_offset) % reg2hw.strides_d1.q == 0) begin
            n_zeros_left <= ({26'h0, reg2hw.pad_left.q} - w_offset) / reg2hw.strides_d1.q; // n_zeros_left = LEFT_PAD - w_offset;
          end else begin
            n_zeros_left <= ({26'h0, reg2hw.pad_left.q} - w_offset) / reg2hw.strides_d1.q + 1;
          end

          /* Top zeros computation */
          if (h_offset >= reg2hw.pad_top.q) begin
            n_zeros_top <= 0;
          end else if (({26'h0, reg2hw.pad_top.q} - h_offset) % reg2hw.strides_d2.q == 0) begin
            n_zeros_top <= ({26'h0, reg2hw.pad_top.q} - h_offset) / reg2hw.strides_d2.q; // n_zeros_top = TOP_PAD - h_offset;
          end else begin
            n_zeros_top <= ({26'h0, reg2hw.pad_top.q} - h_offset) / reg2hw.strides_d2.q + 1;
          end

          /* Right zeros computation */
          if (fw_min_w_offset >= reg2hw.pad_right.q || reg2hw.adpt_pad_right.q == 0) begin
            n_zeros_right <= 0;
          end else if ((reg2hw.adpt_pad_right.q - fw_min_w_offset) % reg2hw.strides_d1.q == 0) begin
            n_zeros_right <= (reg2hw.adpt_pad_right.q - fw_min_w_offset) / reg2hw.strides_d1.q;
          end else begin
            n_zeros_right <= (reg2hw.adpt_pad_right.q - fw_min_w_offset) / reg2hw.strides_d1.q + 1;
          end

          /* Bottom zeros computation */
          if (fh_min_h_offset >= reg2hw.pad_bottom.q || reg2hw.adpt_pad_bottom.q == 0) begin
            n_zeros_bottom <= 0;
          end else if ((reg2hw.adpt_pad_bottom.q - fh_min_h_offset) % reg2hw.strides_d2.q == 0) begin
            n_zeros_bottom <= (reg2hw.adpt_pad_bottom.q - fh_min_h_offset) / reg2hw.strides_d2.q;
          end else begin
            n_zeros_bottom <= (reg2hw.adpt_pad_bottom.q - fh_min_h_offset) / reg2hw.strides_d2.q + 1;
          end
        end

        DMA_RUN_PARAM_COMP_1: begin
          size_transfer_1d <= reg2hw.n_patches_w.q - n_zeros_left - n_zeros_right;
          size_transfer_2d <= reg2hw.n_patches_h.q - n_zeros_top - n_zeros_bottom;
          index <= ((batch_counter * reg2hw.num_ch.q + im_c) * reg2hw.ih.q + (im_row + n_zeros_top * reg2hw.strides_d2.q)) * reg2hw.iw.q + im_col + n_zeros_left * reg2hw.strides_d1.q;
        end

        DMA_RUN_PARAM_COMP_2: begin
          source_inc_d2 <= ((reg2hw.strides_d2.q * reg2hw.iw.q) - (size_transfer_1d - 1 + (reg2hw.strides_d1.q- 1) * (size_transfer_1d - 1)));
          input_data_ptr <= reg2hw.src_ptr.q + index * 4;
        end

        OUT_PTR_UPDATE: begin
          output_data_ptr <= output_data_ptr + (reg2hw.n_patches_h.q * reg2hw.n_patches_w.q) * 4;
          batch_counter   <= batch_counter + 1;
        end

        IM_OFFSET_UPDATE: begin
          /* w_offset update */
          if (w_offset == reg2hw.fw.q - 1) begin
            w_offset <= '0;
          end else begin
            w_offset <= w_offset + 1;
          end

          /* h_offset update */
          if (h_offset_counter == reg2hw.fw.q - 1) begin
            h_offset_counter <= '0;
            if (h_offset == reg2hw.fh.q - 1) begin
              h_offset <= '0;
            end else begin
              h_offset <= h_offset + 1;
            end
          end else begin
            h_offset_counter <= h_offset_counter + 1;
          end

          /* im_c update */
          if (im_c_counter == reg2hw.fh.q * reg2hw.fw.q - 1) begin
            im_c_counter <= '0;
            im_c <= im_c + 1;
          end else begin
            im_c_counter <= im_c_counter + 1;
          end

          /* ch_col_counter update */
          if (ch_col_counter == reg2hw.ch_col.q - 1) begin
            ch_col_counter <= '0;
          end else begin
            ch_col_counter <= ch_col_counter + 1;
          end
        end
      endcase
    end
  end

  /* Parameter computation state transition ff */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_param_state_dq
    if (!rst_ni) begin
      param_state_d <= IDLE;
    end else begin
      param_state_d <= param_state_q;
    end
  end

  /* DMA interface unit state transition logic */
  always_comb begin : proc_comb_dma_if_fsm
    unique case (dma_if_cu_d)
      IDLE_IF_CU: begin
        dma_if_load = 1'b0;
        if (fifo_empty == 1'b0 && im2col_fsms_done == 1'b0 && dma_channels_full == 1'b0) begin
          dma_if_cu_q = GET_TRANSACTION;
        end else begin
          dma_if_cu_q = IDLE_IF_CU;
        end
      end

      GET_TRANSACTION: begin
        dma_if_load = 1'b0;
        if (fifo_empty == 1'b0 && dma_channels_full == 1'b0) begin
          dma_if_cu_q = LOAD_TRANSACTION;
        end else begin
          dma_if_cu_q = IDLE_IF_CU;
        end
      end

      LOAD_TRANSACTION: begin
        dma_if_load = 1'b1;
        if (dma_if_loaded == 1'b1) begin
          if (im2col_fsms_done == 1'b0) begin
            dma_if_cu_q = GET_TRANSACTION;
          end else begin
            dma_if_cu_q = IDLE_IF_CU;
          end
        end else begin
          dma_if_cu_q = LOAD_TRANSACTION;
        end
      end
    endcase
  end

  /* DMA interface unit state transition ff */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_dma_if_fsm
    if (!rst_ni) begin
      dma_if_cu_d <= IDLE_IF_CU;
    end else begin
      dma_if_cu_d <= dma_if_cu_q;
    end
  end

  /* DMA interface unit transaction loading state transition logic */
  always_comb begin : proc_comb_dma_if_trans_load_fsm
    unique case (dma_if_cu_load_d)
      IDLE_IF_LOAD: begin
        fifo_pop = 1'b0;
        if (dma_if_cu_d == GET_TRANSACTION && im2col_fsms_done == 1'b0 && dma_channels_full == 1'b0) begin
          if (dma_ch_first_write[dma_trans_free_channel] == 1'b0) begin
            dma_if_cu_load_q = WRITE_DIMENSIONALITY;
          end else begin
            dma_if_cu_load_q = WRITE_TOP_PAD;
          end
        end else begin
          dma_if_cu_load_q = IDLE_IF_LOAD;
        end
      end

      WRITE_DIMENSIONALITY: begin
        fifo_pop = 1'b0;
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_DATATYPE;
        end else if (dma_if_cu_d == IDLE_IF_CU) begin
          dma_if_cu_load_q = IDLE_IF_LOAD;
        end else begin
          dma_if_cu_load_q = WRITE_DIMENSIONALITY;
        end
      end

      WRITE_DATATYPE: begin
        fifo_pop = 1'b0;
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_TOP_PAD;
        end else begin
          dma_if_cu_load_q = WRITE_DATATYPE;
        end
      end

      WRITE_TOP_PAD: begin
        fifo_pop = 1'b0;
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_BOTTOM_PAD;
        end else begin
          dma_if_cu_load_q = WRITE_TOP_PAD;
        end
      end

      WRITE_BOTTOM_PAD: begin
        fifo_pop = 1'b0;
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_LEFT_PAD;
        end else begin
          dma_if_cu_load_q = WRITE_BOTTOM_PAD;
        end
      end

      WRITE_LEFT_PAD: begin
        fifo_pop = 1'b0;
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_RIGHT_PAD;
        end else begin
          dma_if_cu_load_q = WRITE_LEFT_PAD;
        end
      end

      WRITE_RIGHT_PAD: begin
        fifo_pop = 1'b0;
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_INPUT_PTR;
        end else begin
          dma_if_cu_load_q = WRITE_RIGHT_PAD;
        end
      end

      WRITE_INPUT_PTR: begin
        fifo_pop = 1'b0;
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_OUTPUT_PTR;
        end else begin
          dma_if_cu_load_q = WRITE_INPUT_PTR;
        end
      end

      WRITE_OUTPUT_PTR: begin
        fifo_pop = 1'b0;
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_INC_SRC_D1;
        end else begin
          dma_if_cu_load_q = WRITE_OUTPUT_PTR;
        end
      end

      WRITE_INC_SRC_D1: begin
        fifo_pop = 1'b0;
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_INC_SRC_D2;
        end else begin
          dma_if_cu_load_q = WRITE_INC_SRC_D1;
        end
      end

      WRITE_INC_SRC_D2: begin
        fifo_pop = 1'b0;
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_INC_DST_D1;
        end else begin
          dma_if_cu_load_q = WRITE_INC_SRC_D2;
        end
      end

      WRITE_INC_DST_D1: begin
        fifo_pop = 1'b0;
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_INC_DST_D2;
        end else begin
          dma_if_cu_load_q = WRITE_INC_DST_D1;
        end
      end

      WRITE_INC_DST_D2: begin
        fifo_pop = 1'b0;
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_SIZE_D2;
        end else begin
          dma_if_cu_load_q = WRITE_INC_DST_D2;
        end
      end

      WRITE_SIZE_D2: begin
        fifo_pop = 1'b0;
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_SIZE_D1;
        end else begin
          dma_if_cu_load_q = WRITE_SIZE_D2;
        end
      end

      WRITE_SIZE_D1: begin
        fifo_pop = 1'b0;
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = DONE;
        end else begin
          dma_if_cu_load_q = WRITE_SIZE_D1;
        end
      end

      DONE: begin
        fifo_pop = 1'b1;
        dma_if_cu_load_q = IDLE_IF_LOAD;
      end
    endcase
  end

  /* DMA interface unit transaction loading FSM */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_dma_if_trans_load_fsm
    if (!rst_ni) begin
      dma_if_loaded <= 1'b0;
    end else begin
      unique case (dma_if_cu_load_d)
        IDLE_IF_LOAD: begin
          dma_if_loaded <= 1'b0;
          im2col2aopx_req_o.req <= 1'b0;
          dma_if_load_req <= 1'b0;
        end

        WRITE_DIMENSIONALITY: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            im2col2aopx_req_o.req <= 1'b1;
            im2col2aopx_req_o.we <= 1'b1;
            im2col2aopx_req_o.be <= 4'b1111;
            im2col2aopx_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_trans_free_channel * core_v_mini_mcu_pkg::DMA_CH_SIZE + 
                                    DMA_DIMENSIONALITY_OFFSET;
            im2col2aopx_req_o.wdata <= 32'h1;
          end else if (dma_if_load_rvalid == 1'b1) begin
            dma_if_load_continue <= 1'b1;
            dma_if_load_req <= 1'b0;
          end else begin
            dma_if_load_continue  <= 0;
            im2col2aopx_req_o.req <= 1'b0;
          end
        end

        WRITE_DATATYPE: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            im2col2aopx_req_o.req <= 1'b1;
            im2col2aopx_req_o.we <= 1'b1;
            im2col2aopx_req_o.be <= 4'b1111;
            im2col2aopx_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_trans_free_channel * core_v_mini_mcu_pkg::DMA_CH_SIZE + 
                                    DMA_DATATYPE_OFFSET;
            im2col2aopx_req_o.wdata <= {30'h0, reg2hw.data_type.q} & 32'h3;  // Mask
          end else if (dma_if_load_rvalid == 1'b1) begin
            dma_if_load_continue <= 1'b1;
            dma_if_load_req <= 1'b0;
          end else begin
            dma_if_load_continue  <= 0;
            im2col2aopx_req_o.req <= 1'b0;
          end
        end

        WRITE_TOP_PAD: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            im2col2aopx_req_o.req <= 1'b1;
            im2col2aopx_req_o.we <= 1'b1;
            im2col2aopx_req_o.be <= 4'b1111;
            im2col2aopx_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_trans_free_channel * core_v_mini_mcu_pkg::DMA_CH_SIZE + 
                                    DMA_TOP_PAD_OFFSET;
            im2col2aopx_req_o.wdata <= fifo_output.n_zeros_top * (4 >> reg2hw.data_type.q) & 32'h3f; // Mask
          end else if (dma_if_load_rvalid == 1'b1) begin
            dma_if_load_continue <= 1'b1;
            dma_if_load_req <= 1'b0;
          end else begin
            dma_if_load_continue  <= 0;
            im2col2aopx_req_o.req <= 1'b0;
          end
        end

        WRITE_BOTTOM_PAD: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            im2col2aopx_req_o.req <= 1'b1;
            im2col2aopx_req_o.we <= 1'b1;
            im2col2aopx_req_o.be <= 4'b1111;
            im2col2aopx_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_trans_free_channel * core_v_mini_mcu_pkg::DMA_CH_SIZE + 
                                    DMA_BOTTOM_PAD_OFFSET;
            im2col2aopx_req_o.wdata <= fifo_output.n_zeros_bottom * (4 >> reg2hw.data_type.q) & 32'h3f; // Mask
          end else if (dma_if_load_rvalid == 1'b1) begin
            dma_if_load_continue <= 1'b1;
            dma_if_load_req <= 1'b0;
          end else begin
            dma_if_load_continue  <= 0;
            im2col2aopx_req_o.req <= 1'b0;
          end
        end

        WRITE_LEFT_PAD: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            im2col2aopx_req_o.req <= 1'b1;
            im2col2aopx_req_o.we <= 1'b1;
            im2col2aopx_req_o.be <= 4'b1111;
            im2col2aopx_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_trans_free_channel * core_v_mini_mcu_pkg::DMA_CH_SIZE + 
                                    DMA_LEFT_PAD_OFFSET;
            im2col2aopx_req_o.wdata <= fifo_output.n_zeros_left * (4 >> reg2hw.data_type.q) & 32'h3f; // Mask
          end else if (dma_if_load_rvalid == 1'b1) begin
            dma_if_load_continue <= 1'b1;
            dma_if_load_req <= 1'b0;
          end else begin
            dma_if_load_continue  <= 0;
            im2col2aopx_req_o.req <= 1'b0;
          end
        end

        WRITE_RIGHT_PAD: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            im2col2aopx_req_o.req <= 1'b1;
            im2col2aopx_req_o.we <= 1'b1;
            im2col2aopx_req_o.be <= 4'b1111;
            im2col2aopx_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_trans_free_channel * core_v_mini_mcu_pkg::DMA_CH_SIZE + 
                                    DMA_RIGHT_PAD_OFFSET;
            im2col2aopx_req_o.wdata <= fifo_output.n_zeros_right * (4 >> reg2hw.data_type.q) & 32'h3f; // Mask
          end else if (dma_if_load_rvalid == 1'b1) begin
            dma_if_load_continue <= 1'b1;
            dma_if_load_req <= 1'b0;
          end else begin
            dma_if_load_continue  <= 0;
            im2col2aopx_req_o.req <= 1'b0;
          end
        end

        WRITE_INPUT_PTR: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            im2col2aopx_req_o.req <= 1'b1;
            im2col2aopx_req_o.we <= 1'b1;
            im2col2aopx_req_o.be <= 4'b1111;
            im2col2aopx_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_trans_free_channel * core_v_mini_mcu_pkg::DMA_CH_SIZE + 
                                    DMA_SRC_PTR_OFFSET;
            im2col2aopx_req_o.wdata <= fifo_output.input_ptr;
          end else if (dma_if_load_rvalid == 1'b1) begin
            dma_if_load_continue <= 1'b1;
            dma_if_load_req <= 1'b0;
          end else begin
            dma_if_load_continue  <= 0;
            im2col2aopx_req_o.req <= 1'b0;
          end
        end

        WRITE_OUTPUT_PTR: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            im2col2aopx_req_o.req <= 1'b1;
            im2col2aopx_req_o.we <= 1'b1;
            im2col2aopx_req_o.be <= 4'b1111;
            im2col2aopx_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_trans_free_channel * core_v_mini_mcu_pkg::DMA_CH_SIZE + 
                                    DMA_DST_PTR_OFFSET;
            im2col2aopx_req_o.wdata <= fifo_output.output_ptr;
          end else if (dma_if_load_rvalid == 1'b1) begin
            dma_if_load_continue <= 1'b1;
            dma_if_load_req <= 1'b0;
          end else begin
            dma_if_load_continue  <= 0;
            im2col2aopx_req_o.req <= 1'b0;
          end
        end

        WRITE_INC_SRC_D1: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            im2col2aopx_req_o.req <= 1'b1;
            im2col2aopx_req_o.we <= 1'b1;
            im2col2aopx_req_o.be <= 4'b1111;
            im2col2aopx_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_trans_free_channel * core_v_mini_mcu_pkg::DMA_CH_SIZE + 
                                    DMA_INC_SRC_D1_OFFSET;
            im2col2aopx_req_o.wdata <= reg2hw.strides_d1.q * (4 >> reg2hw.data_type.q) & 32'h3f; // Mask;
          end else if (dma_if_load_rvalid == 1'b1) begin
            dma_if_load_continue <= 1'b1;
            dma_if_load_req <= 1'b0;
          end else begin
            dma_if_load_continue  <= 0;
            im2col2aopx_req_o.req <= 1'b0;
          end
        end

        WRITE_INC_SRC_D2: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            im2col2aopx_req_o.req <= 1'b1;
            im2col2aopx_req_o.we <= 1'b1;
            im2col2aopx_req_o.be <= 4'b1111;
            im2col2aopx_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_trans_free_channel * core_v_mini_mcu_pkg::DMA_CH_SIZE + 
                                    DMA_INC_SRC_D2_OFFSET;
            im2col2aopx_req_o.wdata <= fifo_output.in_inc_d2 * (4 >> reg2hw.data_type.q) & 32'h7fffff; // Mask
          end else if (dma_if_load_rvalid == 1'b1) begin
            dma_if_load_continue <= 1'b1;
            dma_if_load_req <= 1'b0;
          end else begin
            dma_if_load_continue  <= 0;
            im2col2aopx_req_o.req <= 1'b0;
          end
        end

        WRITE_INC_DST_D1: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            im2col2aopx_req_o.req <= 1'b1;
            im2col2aopx_req_o.we <= 1'b1;
            im2col2aopx_req_o.be <= 4'b1111;
            im2col2aopx_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_trans_free_channel * core_v_mini_mcu_pkg::DMA_CH_SIZE + 
                                    DMA_INC_DST_D1_OFFSET;
            im2col2aopx_req_o.wdata <= (4 >> reg2hw.data_type.q) & 32'h3f;  // Mask;
          end else if (dma_if_load_rvalid == 1'b1) begin
            dma_if_load_continue <= 1'b1;
            dma_if_load_req <= 1'b0;
          end else begin
            dma_if_load_continue  <= 0;
            im2col2aopx_req_o.req <= 1'b0;
          end
        end

        WRITE_INC_DST_D2: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            im2col2aopx_req_o.req <= 1'b1;
            im2col2aopx_req_o.we <= 1'b1;
            im2col2aopx_req_o.be <= 4'b1111;
            im2col2aopx_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_trans_free_channel * core_v_mini_mcu_pkg::DMA_CH_SIZE + 
                                    DMA_INC_DST_D2_OFFSET;
            im2col2aopx_req_o.wdata <= (4 >> reg2hw.data_type.q) & 32'h7fffff;  // Mask
          end else if (dma_if_load_rvalid == 1'b1) begin
            dma_if_load_continue <= 1'b1;
            dma_if_load_req <= 1'b0;
          end else begin
            dma_if_load_continue  <= 0;
            im2col2aopx_req_o.req <= 1'b0;
          end
        end

        WRITE_SIZE_D2: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            im2col2aopx_req_o.req <= 1'b1;
            im2col2aopx_req_o.we <= 1'b1;
            im2col2aopx_req_o.be <= 4'b1111;
            im2col2aopx_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_trans_free_channel * core_v_mini_mcu_pkg::DMA_CH_SIZE + 
                                    DMA_SIZE_D2_OFFSET;
            im2col2aopx_req_o.wdata <= fifo_output.size_du_d2 * (4 >> reg2hw.data_type.q) & 32'hffff; // Mask
          end else if (dma_if_load_rvalid == 1'b1) begin
            dma_if_load_continue <= 1'b1;
            dma_if_load_req <= 1'b0;
          end else begin
            dma_if_load_continue  <= 0;
            im2col2aopx_req_o.req <= 1'b0;
          end
        end

        WRITE_SIZE_D1: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            im2col2aopx_req_o.req <= 1'b1;
            im2col2aopx_req_o.we <= 1'b1;
            im2col2aopx_req_o.be <= 4'b1111;
            im2col2aopx_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_trans_free_channel * core_v_mini_mcu_pkg::DMA_CH_SIZE + 
                                    DMA_SIZE_D1_OFFSET;
            im2col2aopx_req_o.wdata <= fifo_output.size_du_d1 * (4 >> reg2hw.data_type.q) & 32'hffff; // Mask
          end else if (dma_if_load_rvalid == 1'b1) begin
            dma_if_load_continue <= 1'b1;
            dma_if_load_req <= 1'b0;
          end else begin
            dma_if_load_continue  <= 0;
            im2col2aopx_req_o.req <= 1'b0;
          end
        end

        DONE: begin
          dma_if_loaded   <= 1'b1;
          dma_if_load_req <= 1'b0;
        end
      endcase
    end
  end

  /* DMA interface unit transaction loading state transition ff */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_dma_if_trans_load_dq
    if (!rst_ni) begin
      dma_if_cu_load_d <= IDLE_IF_LOAD;
    end else begin
      dma_if_cu_load_d <= dma_if_cu_load_q;
    end
  end

  /* Free channel finder */
  always_comb begin : proc_comb_free_channel
    dma_free_channel = 0;
    for (int i = 0; i < 32; i = i + 1) begin
      if (dma_if_channels[i] == 1'b0 && dma_ch_en_mask[i] == 1'b1) begin
        dma_free_channel = i[(DMA_CH_NUM==1)?0 : ($clog2(DMA_CH_NUM)-1):0];
        break;
      end
    end
  end

  /* Channel tracker */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_control_unit
    if (!rst_ni) begin
      dma_trans_free_channel <= 0;
      for (int i = 0; i < 32; i = i + 1) begin
        dma_if_channels[i] <= 1'b0;
        dma_ch_first_write[i] <= 1'b0;
      end
    end else begin
      /* Reset the first write flags when the im2col spc is done */
      if (im2col_fsms_done == 1'b1) begin
        for (int i = 0; i < 32; i = i + 1) begin
          dma_ch_first_write[i] <= 1'b0;
        end
      end

      /* If an occupied channel asserts a done signal, free it up */
      for (int i = 0; i < 32; i = i + 1) begin
        if (dma_if_channels[i] == 1'b1 && dma_done_i[i] == 1'b1) begin
          dma_if_channels[i] <= 1'b0;
        end
      end

      /* If a transaction has to take place, occupy the free channel */
      if (dma_if_cu_q == GET_TRANSACTION) begin
        dma_trans_free_channel <= dma_free_channel;
      end

      if (dma_if_cu_d == GET_TRANSACTION) begin
        dma_if_channels[dma_trans_free_channel] <= 1'b1;
        dma_ch_first_write[dma_trans_free_channel] <= 1'b1;
      end
    end
  end

  /* FIFO reset */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_fifo_reset
    if (!rst_ni) begin
      fifo_flush <= 1'b1;
    end else begin
      fifo_flush <= 1'b0;
    end
  end

  /* Channels full flag logic */
  always_comb begin : proc_comb_channels_full
    dma_channels_full = 1'b1;
    for (int i = 0; i < 32; i = i + 1) begin
      if (dma_if_channels[i] == 1'b0 && dma_ch_en_mask[i] == 1'b1) begin
        dma_channels_full = 1'b0;
        break;
      end
    end
  end

  /* Transaction IFR update */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_spc_ifr
    if (~rst_ni) begin
      im2col_spc_ifr <= '0;
    end else if (reg2hw.interrupt_en.q == 1'b1) begin
      // Enter here only if the im2col_done interrupt is enabled
      if (im2col_done == 1'b1) begin
        im2col_spc_ifr <= 1'b1;
      end else if (reg2hw.spc_ifr.re == 1'b1) begin
        // If the IFR bit is read, we must clear the transaction_ifr
        im2col_spc_ifr <= 1'b0;
      end
    end
  end

  /* Parameter computation done signal update */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_param_done
    if (!rst_ni) begin
      im2col_param_done <= 1'b0;
    end else begin
      if (ch_col_counter == (reg2hw.ch_col.q - 1) && batch_counter == reg2hw.batch.q) begin
        im2col_param_done <= 1'b1;
      end else if (im2col_start == 1'b1) begin
        im2col_param_done <= 1'b0;
      end
    end
  end

  /* Global fsm done signal update logic */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_spc_done
    if (!rst_ni) begin
      im2col_fsms_done <= 1'b0;
    end else begin
      if (dma_if_cu_load_d == DONE && im2col_param_done == 1'b1 && fifo_usage == 1) begin
        im2col_fsms_done <= 1'b1;
      end else if (im2col_start == 1'b1) begin
        im2col_fsms_done <= 1'b0;
      end
    end
  end

  /* Global done signal update logic */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_done
    if (!rst_ni) begin
      im2col_done <= 1'b0;
    end else begin
      if (im2col_fsms_done == 1'b1 && |dma_if_channels == 1'b0) begin
        im2col_done <= 1'b1;
      end else if (im2col_start == 1'b1) begin
        im2col_done <= 1'b0;
      end
    end
  end

  /*_________________________________________________________________________________________________________________________________ */

  /* Signal assignments */

  /* Start signal assignment */
  assign im2col_start = (reg2hw.num_ch.qe == 1'b1);

  /* Parameter computation FSM */
  assign fifo_input.input_ptr = input_data_ptr;
  assign fifo_input.output_ptr = output_data_ptr;
  assign fifo_input.in_inc_d2 = source_inc_d2;
  assign fifo_input.n_zeros_top = n_zeros_top;
  assign fifo_input.n_zeros_bottom = n_zeros_bottom;
  assign fifo_input.n_zeros_left = n_zeros_left;
  assign fifo_input.n_zeros_right = n_zeros_right;
  assign fifo_input.size_du_d1 = size_transfer_1d;
  assign fifo_input.size_du_d2 = size_transfer_2d;

  /* Transaction loading process */
  assign dma_if_load_rvalid = aopx2im2col_resp_i.rvalid && dma_if_load;
  assign hw2reg.status.d = im2col_done;

  /* Interrupt management */
  assign hw2reg.spc_ifr.d = im2col_spc_ifr;
  assign im2col_spc_done_int_o = im2col_spc_ifr;

  /* DMA channels mask register */
  assign dma_ch_en_mask = reg2hw.spc_ch_mask.q;

endmodule

/*
 * Copyright 2024 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *
 * Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *  
 * Info: Im2col accelerator implemented as a Smart Peripheral Controller. It accesses the DMA CH0 to perform
 *       the matrix manipulation operation known as "image to column" (im2col), which enables efficient
 *       CNN inference by transforming the input tensor to use the GEMM library.
 */

module im2col_spc #(
    parameter int unsigned NUM_CH_SPC = 2
) (
    input logic clk_i,
    input logic rst_ni,

    input obi_resp_t aopx2im2col_resp_i,
    output obi_req_t im2col2aopx_req_o,

    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    input logic [core_v_mini_mcu_pkg::DMA_CH_NUM-1:0] dma_done_i
);

  import obi_pkg::*;
  import reg_pkg::*;
  import dma_queue_pkg::*;
  import core_v_mini_mcu_pkg::*;

  /*_________________________________________________________________________________________________________________________________ */

  /* Parameter definition */
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
  localparam DMA_SPC_RESERVED_OFFSET = 32'h5c;
  /*_________________________________________________________________________________________________________________________________ */

  /* Signals declaration */
  
  /* Control Unit signals */
  logic stall_parameters;
  logic stall_queue;
  logic stall_dma_if;

  /* DMA queue signals */
  dma_if_t dma_queue_if_in;
  dma_if_t dma_queue_if_out;
  logic dma_queue_if_req;
  logic dma_queue_sample_en;
  logic dma_queue_valid;
  logic [clog2(NUM_CH_SPC)-1:0] dma_queue_channel;
  logic dma_queue_full;
  logic dma_queue_empty;

  /* DMA interface unit signals */
  logic dma_run_en;
  logic [NUM_CH_SPC-1:0] dma_if_channel;
  logic dma_if_load;
  logic dma_if_loaded;
  logic dma_if_load_continue;
  logic dma_if_load_req;
  logic dma_if_load_rvalid;
  
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
  logic [31:0] ch_col;
  logic [31:0] ch_col_counter;
  logic [31:0] batch_counter;
  logic [31:0] stray_elements;
  logic [31:0] last_position_right;
  logic [31:0] tmp_pad_right;
  logic [31:0] last_position_bottom;
  logic [31:0] tmp_pad_bottom;
  logic [31:0] input_data_ptr;
  logic [31:0] output_data_ptr;
  logic [31:0] source_inc_d2;

  /* 
   * Every "comp" state refers to variable computations that precede the DMA call in the 
   * loop, while any "param" state refers to the DMA parameters computation that follows the DMA call.
   */
  enum {
    IDLE,
    CH_COL_COMP,
    F_MIN_OFFSET_COMP,
    IM_COORD_COMP,
    N_ZEROS_COMP,
    DMA_RUM_PARAM_COMP,
    OUT_PTR_UPDATE,
    IM_OFFSET_UPDATE,
    START_DMA_RUN
  } param_state_q, param_state_d;

  enum {
    IDLE,
    GET_TRANSACTION,
    LOAD_TRANSACTION
  } dma_if_cu_q, dma_if_cu_d;

  enum {
    IDLE,
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
  } dma_if_cu_load_q, dma_if_cu_load_d;

  /*_________________________________________________________________________________________________________________________________ */

  /* Module instantiation */

  /* Regtool top module */
  im2col_spc_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) im2col_spc_top_i (
      .clk_i,
      .rst_ni,
      .reg_req_i,
      .reg_rsp_o,
      .reg2hw,
      .hw2reg,
      .devmode_i(1'b1)
  );

  /* DMA queue */
  dma_queue #(
    .NUM_CH(NUM_CH_SPC)
  ) dma_queue_i (
    .clk_i(clk_i),
    .rst_ni(rst_ni),
    .sample_en(dma_queue_sample_en),
    .dma_if_i(dma_queue_if_in),
    .dma_if_req_i(dma_queue_if_req),
    .dma_channel_o(dma_queue_channel),
    .dma_if_o(dma_queue_if_out),
    .dma_if_valid_o(dma_queue_valid),
    .dma_queue_full_o(dma_queue_full),
    .dma_queue_empty_o(dma_queue_empty)
  );

  /*_________________________________________________________________________________________________________________________________ */

  /* FSMs instantiation */

  /* Parameter computation state transition FSM */
  always_comb begin: proc_comb_param_state_fsm
    unique case (param_state_d)
      IDLE: begin
        dma_queue_sample_en = 1'b0;
        if (im2col_start == 1'b1) begin
          param_state_q = CH_COL_COMP;
        end else begin
          param_state_q = IDLE;
        end
      end
      
      CH_COL_COMP: begin
        param_state_q = F_MIN_OFFSET_COMP;
        dma_queue_sample_en = 1'b0;
      end

      F_MIN_OFFSET_COMP: begin
        param_state_q = IM_COORD_COMP;
      end

      IM_COORD_COMP: begin
        param_state_q = N_ZEROS_COMP;
      end

      N_ZEROS_COMP: begin
        param_state_q = DMA_RUN_PARAM_COMP;
      end

      DMA_RUN_PARAM_COMP: begin
        param_state_q = OUT_PTR_UPDATE;
      end

      OUT_PTR_UPDATE: begin
        if (batch_counter == reg2hw.batch.d) begin
          param_state_q = IM_OFFSET_UPDATE;
        end else begin
          param_state_q = IM_COORD_COMP;
        end
      end

      IM_OFFSET_UPDATE: begin
        if (ch_col_counter == ch_col) begin
          param_state_q = START_DMA_RUN;
        end else begin
          param_state_q = F_MIN_OFFSET_COMP;
        end
      end

      START_DMA_RUN: begin
        dma_queue_sample_en = 1'b1;
        if (dma_queue_full == 1'b0 && ch_col_counter != ch_col) begin
          param_state_q = CH_COL_COMP;
        end else if (dma_queue_full == 1'b0 && ch_col_counter == ch_col) begin
          param_state_q = IDLE;
        end else begin
          param_state_q = START_DMA_RUN;
        end
      end
    endcase
  end

  /* Parameter computation logic */
  always_ff @(posedge clk_i, negedge rst_ni) begin: proc_ff_param_comp_fsm
    if(!rst_ni) begin
      // ToD0: initialize all the signals
      output_data_ptr <= reg2hw.output_ptr.d;
    end else begin
      unique case (param_state_d)
        CH_COL_COMP: begin
          ch_col <= reg2hw.num_ch.d * reg2hw.fw.d * reg2hw.fh.d; // CH_COL = CH * FW *FH
          n_patches_w <= (reg2hw.iw.d + (reg2hw.right_pad.d + reg2hw.left_pad.d) - reg2hw.fw.d)/ reg2hw.stride_d1.d + 1; 
          n_patches_h <= (reg2hw.ih.d + (reg2hw.top_pad.d + reg2hw.bottom_pad.d) - reg2hw.fh.d)/ reg2hw.stride_d2.d + 1; 
        end 

        F_MIN_OFFSET_COMP: begin
          fw_min_w_offset <= reg2hw.fw. - 1 - w_offset; // fw_minus_w_offset = FW - 1 - w_offset;
          fh_min_h_offset <= reg2hw.fh.d - 1 - h_offset; // fh_minus_h_offset = FH - 1 - h_offset;
        end

        IM_COORD_COMP: begin
          im_row <= h_offset - reg2hw.pad_top.d; // im_row = h_offset - TOP_PAD;
          im_col <= w_offset - reg2hw.pad_left.d; // im_col = w_offset - LEFT_PAD;
        end

        N_ZEROS_COMP: begin

          /* Left zeros computation */
          if (w_offset >= reg2hw.pad_left.d) begin
            n_zeros_left <= 0;
          end else if ((reg2hw.pad_left.d - w_offset) % reg2hw.strides_1d.d == 1'b0) begin
            n_zeros_left <= (reg2hw.pad_left.d - w_offset) / reg2hw.strides_1d.d; // n_zeros_left = LEFT_PAD - w_offset;
          end else begin
            n_zeros_left <= (reg2hw.pad_left.d - w_offset) / reg2hw.strides_1d.d + 1;
          end

          /* Top zeros computation */
          if (h_offset >= reg2hw.pad_top.d) begin
            n_zeros_top <= 0;
          end else if ((reg2hw.pad_top.d - h_offset) % reg2hw.strides_1d.d == 1'b0) begin
            n_zeros_top <= (reg2hw.pad_top.d - h_offset) / reg2hw.strides_1d.d; // n_zeros_top = TOP_PAD - h_offset;
          end else begin
            n_zeros_top <= (reg2hw.pad_top.d - h_offset) / reg2hw.strides_1d.d + 1;
          end

          /* Right & bottom zeros computation */
          tmp_pad_right <= reg2hw.stride_d1.d * (n_patches_w - 1) + reg2hw.fw.d - reg2hw.iw.d - reg2hw.pad_left.d;
          tmp_pad_bottom <= reg2hw.stride_d1.d * (n_patches_h - 1) + reg2hw.fh.d - reg2hw.ih.d - reg2hw.pad_left.d;

          if (fw_min_w_offset >= tmp_pad_right) begin
            n_zeros_right <= 0;
          end else if ((tmp_pad_right - fw_min_w_offset) % reg2hw.strides_1d.d == 1'b0) begin
            n_zeros_right <= (tmp_pad_right - fw_min_w_offset) / reg2hw.strides_1d.d;
          end else begin
            n_zeros_right <= (tmp_pad_right - fw_min_w_offset) / reg2hw.strides_1d.d + 1;
          end

          if (fh_min_h_offset >= tmp_pad_bottom) begin
            n_zeros_bottom <= 0;
          end else if ((tmp_pad_bottom - fh_min_h_offset) % reg2hw.strides_1d.d == 1'b0) begin
            n_zeros_bottom <= (tmp_pad_bottom - fh_min_h_offset) / reg2hw.strides_1d.d;
          end else begin
            n_zeros_bottom <= (tmp_pad_bottom - fh_min_h_offset) / reg2hw.strides_1d.d + 1;
          end
        end

        DMA_RUN_PARAM_COMP: begin
          size_transfer_1d <= n_patches_w - n_zeros_left - n_zeros_right; 
          size_transfer_2d <= n_patches_h - n_zeros_top - n_zeros_bottom;
          index <= ((b * CH + im_c) * reg2hw.ih.d + (im_row + n_zeros_top * reg2hw.strides_2d.d)) * reg2hw.iw.d + im_col + n_zeros_left * reg2hw.strides_1d.d;
          input_data_ptr <= reg2hw.input_ptr.d + index;
          source_inc_d2 <= (reg2hw.strides_2d.d * reg2hw.iw.d - (size_transfer - 1 + (reg2hw.strides_1d.d- 1) * (size_transfer - 1)));
        end

        OUT_PTR_UPDATE: begin
          output_data_ptr <= output_data_ptr + n_patches_h * n_patches_w;
          if (batch_counter == reg2hw.batch.d - 1) begin
            batch_counter <= 0;
          end else begin
            batch_counter <= batch_counter + 1;
          end
        end

        IM_OFFSET_UPDATE: begin
          /* w_offset update */
          if (w_offset == reg2hw.fw.d - 1) begin
            w_offset <= '0;
          end else begin
            w_offset <= w_offset + 1;
          end

          /* h_offset update */
          if (h_offset_counter == 0) begin
            h_offset_counter <= '0;
            if (h_offset == reg2hw.fh.d - 1) begin
              h_offset <= '0;
            end else begin
              h_offset <= h_offset + 1;
            end
          end else begin
            h_offset_counter <= h_offset_counter + 1;
          end

          /* im_c update */
          if (im_c_counter == reg2hw.fh.d * reg2hw.fw.d) begin
            im_c_counter <= '0;
            im_c <= '0;
          end else begin
            im_c_counter <= im_c_counter + 1;
          end

          /* ch_col_counter update */
          if (ch_col_counter == ch_col - 1) begin
            ch_col_counter <= '0;
          end else begin
            ch_col_counter <= ch_col_counter + 1;
          end
        end
      endcase
    end
  end

  /* Parameter computation state transition ff */
  always_ff @(posedge clk_i, negedge rst_ni) begin: proc_ff_param_state_dq
    if(!rst_ni) begin
      param_state_d <= IDLE;
    end else begin
      param_state_d <= param_state_q;
    end
  end

  /* DMA interface unit state transition logic */
  always_comb begin: proc_comb_dma_if_fsm
    unique case (dma_if_cu_d)
      IDLE: begin
        dma_queue_if_req = 1'b0;
        dma_if_load = 1'b0;
        if (dma_queue_empty == 1'b0 && dma_run_en == 1'b1) begin
          dma_if_cu_q = GET_TRANSACTION;
        end else begin
          dma_if_cu_q = IDLE;
        end
      end

      GET_TRANSACTION: begin
        dma_queue_if_req = 1'b1;
        dma_if_load = 1'b0;
        if (dma_queue_empty == 1'b0 && dma_queue_valid == 1'b1) begin
          dma_if_cu_q = LOAD_TRANSACTION;
        end else begin
          dma_if_cu_q = GET_TRANSACTION;
        end
      end

      LOAD_TRANSACTION: begin
        dma_queue_if_req = 1'b0;
        dma_if_load = 1'b1;
        if (dma_if_loaded == 1'b1) begin
          if (dma_run_en == 1'b1) begin
            dma_if_cu_q = GET_TRANSACTION;
          end else begin
            dma_if_cu_q = IDLE;
          end
        end else begin
          dma_if_cu_q = LOAD_TRANSACTION;
        end
      end
    endcase
  end
  
  /* DMA interface unit state transition ff */
  always_ff @(posedge clk_i, negedge rst_ni) begin: proc_ff_dma_if_fsm
    if(!rst_ni) begin
      dma_if_cu_d <= IDLE;
    end else begin
      dma_if_cu_d <= dma_if_cu_q;
    end
  end

  /* DMA interface unit transaction loading state transition logic */
  always_comb begin: proc_comb_dma_if_trans_load_fsm
    unique case (dma_if_cu_load_d)
      IDLE: begin
        if (dma_if_load == 1'b1) begin
          dma_if_cu_load_q = WRITE_DIMENSIONALITY;
        end else begin
          dma_if_cu_load_q = IDLE;
        end
      end

      WRITE_DIMENSIONALITY: begin
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_DATATYPE;
        end else begin
          dma_if_cu_load_q = WRITE_DIMENSIONALITY;
        end
      end

      WRITE_DATATYPE: begin
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_TOP_PAD;
        end else begin
          dma_if_cu_load_q = WRITE_DATATYPE;
        end
      end

      WRITE_TOP_PAD: begin
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_BOTTOM_PAD;
        end else begin
          dma_if_cu_load_q = WRITE_TOP_PAD;
        end
      end

      WRITE_BOTTOM_PAD: begin
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_LEFT_PAD;
        end else begin
          dma_if_cu_load_q = WRITE_BOTTOM_PAD;
        end
      end

      WRITE_LEFT_PAD: begin
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_RIGHT_PAD;
        end else begin
          dma_if_cu_load_q = WRITE_LEFT_PAD;
        end
      end

      WRITE_RIGHT_PAD: begin
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_INPUT_PTR;
        end else begin
          dma_if_cu_load_q = WRITE_RIGHT_PAD;
        end
      end

      WRITE_INPUT_PTR: begin
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_OUTPUT_PTR;
        end else begin
          dma_if_cu_load_q = WRITE_INPUT_PTR;
        end
      end

      WRITE_OUTPUT_PTR: begin
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_INC_SRC_D1;
        end else begin
          dma_if_cu_load_q = WRITE_OUTPUT_PTR;
        end
      end

      WRITE_INC_SRC_D1: begin
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_INC_SRC_D2;
        end else begin
          dma_if_cu_load_q = WRITE_INC_SRC_D1;
        end
      end 

      WRITE_INC_SRC_D2: begin
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_INC_DST_D1;
        end else begin
          dma_if_cu_load_q = WRITE_INC_SRC_D2;
        end
      end

      WRITE_INC_DST_D1: begin
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_INC_DST_D2;
        end else begin
          dma_if_cu_load_q = WRITE_INC_DST_D1;
        end
      end

      WRITE_INC_DST_D2: begin
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_SIZE_D2;
        end else begin
          dma_if_cu_load_q = WRITE_INC_DST_D2;
        end
      end

      WRITE_SIZE_D2: begin
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = WRITE_SIZE_D1;
        end else begin
          dma_if_cu_load_q = WRITE_SIZE_D2;
        end
      end

      WRITE_SIZE_D1: begin
        if (dma_if_load_continue == 1'b1) begin
          dma_if_cu_load_q = DONE;
        end else begin
          dma_if_cu_load_q = WRITE_SIZE_D1;
        end
      end

      DONE: begin
        dma_if_cu_load_q = TURN_OFF_SPC_RESERVED;
      end
    endcase
  end

  /* DMA interface unit transaction loading FSM */
  always_ff @(posedge clk_i, negedge rst_ni) begin: proc_ff_dma_if_trans_load_fsm
    if(!rst_ni) begin
      dma_if_loaded <= 1'b0;
    end else begin
      unique case (dma_if_cu_load_d)
        IDLE: begin
          dma_if_loaded <= 1'b0;
        end

        WRITE_DIMENSIONALITY: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            aopx2im2col_req_o.we <= 1'b1;
            aopx2im2col_req_o.be <= 4'b1111;
            aopx2im2col_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_queue_channel * core_v_mini_mcu_pkg::DMA_CHANNEL_SIZE + 
                                    DMA_DIMENSIONALITY_OFFSET;
            aopx2im2col_req_o.wdata <= 32'h1;
          end else if (dma_if_load_rvalid == 1'b1) begin 
            dma_if_load_continue <= 1'b1;
          end else begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b0;
          end
        end

        WRITE_DATATYPE: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            aopx2im2col_req_o.we <= 1'b1;
            aopx2im2col_req_o.be <= 4'b1111;
            aopx2im2col_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_queue_channel * core_v_mini_mcu_pkg::DMA_CHANNEL_SIZE + 
                                    DMA_DATATYPE_OFFSET;
            aopx2im2col_req_o.wdata <= reg2hw.datatype.d;
          end else if (dma_if_load_rvalid == 1'b1) begin 
            dma_if_load_continue <= 1'b1;
          end else begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b0;
          end
        end

        WRITE_TOP_PAD: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            aopx2im2col_req_o.we <= 1'b1;
            aopx2im2col_req_o.be <= 4'b1111;
            aopx2im2col_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_queue_channel * core_v_mini_mcu_pkg::DMA_CHANNEL_SIZE + 
                                    DMA_TOP_PAD_OFFSET;
            aopx2im2col_req_o.wdata <= dma_queue_if_out.n_zeros_top;
          end else if (dma_if_load_rvalid == 1'b1) begin 
            dma_if_load_continue <= 1'b1;
          end else begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b0;
          end
        end

        WRITE_BOTTOM_PAD: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            aopx2im2col_req_o.we <= 1'b1;
            aopx2im2col_req_o.be <= 4'b1111;
            aopx2im2col_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_queue_channel * core_v_mini_mcu_pkg::DMA_CHANNEL_SIZE + 
                                    DMA_BOTTOM_PAD_OFFSET;
            aopx2im2col_req_o.wdata <= dma_queue_if_out.n_zeros_bottom;
          end else if (dma_if_load_rvalid == 1'b1) begin 
            dma_if_load_continue <= 1'b1;
          end else begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b0;
          end
        end

        WRITE_LEFT_PAD: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            aopx2im2col_req_o.we <= 1'b1;
            aopx2im2col_req_o.be <= 4'b1111;
            aopx2im2col_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_queue_channel * core_v_mini_mcu_pkg::DMA_CHANNEL_SIZE + 
                                    DMA_LEFT_PAD_OFFSET;
            aopx2im2col_req_o.wdata <= dma_queue_if_out.n_zeros_left;
          end else if (dma_if_load_rvalid == 1'b1) begin 
            dma_if_load_continue <= 1'b1;
          end else begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b0;
          end
        end

        WRITE_RIGHT_PAD: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            aopx2im2col_req_o.we <= 1'b1;
            aopx2im2col_req_o.be <= 4'b1111;
            aopx2im2col_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_queue_channel * core_v_mini_mcu_pkg::DMA_CHANNEL_SIZE + 
                                    DMA_RIGHT_PAD_OFFSET;
            aopx2im2col_req_o.wdata <= dma_queue_if_out.n_zeros_right;
          end else if (dma_if_load_rvalid == 1'b1) begin 
            dma_if_load_continue <= 1'b1;
          end else begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b0;
          end
        end

        WRITE_INPUT_PTR: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            aopx2im2col_req_o.we <= 1'b1;
            aopx2im2col_req_o.be <= 4'b1111;
            aopx2im2col_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_queue_channel * core_v_mini_mcu_pkg::DMA_CHANNEL_SIZE + 
                                    DMA_SRC_PTR_OFFSET;
            aopx2im2col_req_o.wdata <= dma_queue_if_out.input_ptr;
          end else if (dma_if_load_rvalid == 1'b1) begin 
            dma_if_load_continue <= 1'b1;
          end else begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b0;
          end
        end

        WRITE_OUTPUT_PTR: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            aopx2im2col_req_o.we <= 1'b1;
            aopx2im2col_req_o.be <= 4'b1111;
            aopx2im2col_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_queue_channel * core_v_mini_mcu_pkg::DMA_CHANNEL_SIZE + 
                                    DMA_DST_PTR_OFFSET;
            aopx2im2col_req_o.wdata <= dma_queue_if_out.output_ptr;
          end else if (dma_if_load_rvalid == 1'b1) begin 
            dma_if_load_continue <= 1'b1;
          end else begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b0;
          end
        end

        WRITE_INC_SRC_D1: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            aopx2im2col_req_o.we <= 1'b1;
            aopx2im2col_req_o.be <= 4'b1111;
            aopx2im2col_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_queue_channel * core_v_mini_mcu_pkg::DMA_CHANNEL_SIZE + 
                                    DMA_INC_SRC_D1_OFFSET;
            aopx2im2col_req_o.wdata <= dma_queue_if_out.in_inc_d1;
          end else if (dma_if_load_rvalid == 1'b1) begin 
            dma_if_load_continue <= 1'b1;
          end else begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b0;
          end
        end

        WRITE_INC_SRC_D2: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            aopx2im2col_req_o.we <= 1'b1;
            aopx2im2col_req_o.be <= 4'b1111;
            aopx2im2col_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_queue_channel * core_v_mini_mcu_pkg::DMA_CHANNEL_SIZE + 
                                    DMA_INC_SRC_D2_OFFSET;
            aopx2im2col_req_o.wdata <= dma_queue_if_out.in_inc_d2;
          end else if (dma_if_load_rvalid == 1'b1) begin 
            dma_if_load_continue <= 1'b1;
          end else begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b0;
          end
        end

        WRITE_INC_DST_D1: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            aopx2im2col_req_o.we <= 1'b1;
            aopx2im2col_req_o.be <= 4'b1111;
            aopx2im2col_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_queue_channel * core_v_mini_mcu_pkg::DMA_CHANNEL_SIZE + 
                                    DMA_INC_DST_D1_OFFSET;
            aopx2im2col_req_o.wdata <= dma_queue_if_out.out_inc_d1;
          end else if (dma_if_load_rvalid == 1'b1) begin 
            dma_if_load_continue <= 1'b1;
          end else begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b0;
          end
        end

        WRITE_INC_DST_D2: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            aopx2im2col_req_o.we <= 1'b1;
            aopx2im2col_req_o.be <= 4'b1111;
            aopx2im2col_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_queue_channel * core_v_mini_mcu_pkg::DMA_CHANNEL_SIZE + 
                                    DMA_INC_DST_D2_OFFSET;
            aopx2im2col_req_o.wdata <= dma_queue_if_out.out_inc_d2;
          end else if (dma_if_load_rvalid == 1'b1) begin 
            dma_if_load_continue <= 1'b1;
          end else begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b0;
          end
        end

        WRITE_SIZE_D2: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            aopx2im2col_req_o.we <= 1'b1;
            aopx2im2col_req_o.be <= 4'b1111;
            aopx2im2col_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_queue_channel * core_v_mini_mcu_pkg::DMA_CHANNEL_SIZE + 
                                    DMA_SIZE_D2_OFFSET;
            aopx2im2col_req_o.wdata <= dma_queue_if_out.out_size_du_d2;
          end else if (dma_if_load_rvalid == 1'b1) begin 
            dma_if_load_continue <= 1'b1;
          end else begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b0;
          end
        end

        WRITE_SIZE_D1: begin
          if (dma_if_load_req == 1'b0) begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b1;
            aopx2im2col_req_o.we <= 1'b1;
            aopx2im2col_req_o.be <= 4'b1111;
            aopx2im2col_req_o.addr <= core_v_mini_mcu_pkg::DMA_START_ADDRESS + 
                                    dma_queue_channel * core_v_mini_mcu_pkg::DMA_CHANNEL_SIZE + 
                                    DMA_SIZE_D1_OFFSET;
            aopx2im2col_req_o.wdata <= dma_queue_if_out.out_size_du_d1;
          end else if (dma_if_load_rvalid == 1'b1) begin 
            dma_if_load_continue <= 1'b1;
          end else begin
            dma_if_load_continue <= 0;
            dma_if_load_req <= 1'b0;
          end
        end

        DONE: begin
          dma_if_loaded <= 1'b1;
          dma_if_load_req <= 1'b0;
        end
      endcase
    end
  end

  always_ff @(posedge clk_i, negedge rst_ni) begin: proc_ff_dma_if_trans_load_dq
    if(!rst_ni) begin
      dma_run_en <= 1'b0;
    end else begin
      dma_run_en <= im2col_start;
    end
  end
  /*_________________________________________________________________________________________________________________________________ */

  /* Signal assignments */

  /* Start signal assignment */
  assign im2col_start = (reg2hw.num_ch.qe == 1'b1);

  /* Parameter computation FSM */
  assign dma_queue_if_in.input_ptr_i = input_data_ptr;
  assign dma_queue_if_in.output_ptr_i = output_data_ptr;
  assign dma_queue_if_in.in_inc_d1_i = reg2hw.strides_1d.d;
  assign dma_queue_if_in.in_inc_d2_i = source_inc_d2;
  assign dma_queue_if_in.out_inc_d1_i = 32'h1;
  assign dma_queue_if_in.out_inc_d2_i = 32'h1;
  assign dma_queue_if_in.n_zeros_top_i = n_zeros_top;
  assign dma_queue_if_in.n_zeros_bottom_i = n_zeros_bottom;
  assign dma_queue_if_in.n_zeros_left_i = n_zeros_left;
  assign dma_queue_if_in.n_zeros_right_i = n_zeros_right;
  assign dma_queue_if_in.in_size_du_d1_i = size_transfer_1d;
  assign dma_queue_if_in.in_size_du_d2_i = size_transfer_2d;
  assign dma_queue_if_in.out_size_du_d1_i = size_transfer_1d;
  assign dma_queue_if_in.out_size_du_d2_i = size_transfer_2d;

  /* Transaction loading process */
  assign aopx2im2col_req_o.req = dma_if_load_req;
  assign dma_if_load_rvalid = aopx2im2col_resp_i.rvalid;

endmodule
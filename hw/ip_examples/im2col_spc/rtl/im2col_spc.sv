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

 /*_________________________________________________________________________________________________________________________________ */

  /* Signals declaration */
  
  /* DMA queue signals */
  dma_if_t dma_queue_if_in;
  dma_if_t dma_queue_if_out;
  logic dma_if_req;
  logic dma_queue_channel;
  logic dma_queue_full;
  logic dma_queue_empty;
  
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

  /* 
   * Every "comp" state refers to variable computations that precede the DMA call in the 
   * loop, while any "param" state refers to the DMA parameters computation that follows the DMA call.
   */
  enum {
    CH_COL_COMP,
    F_MIN_OFFSET_COMP,
    IM_COORD_COMP,
    N_ZEROS_COMP,
    DMA_RUM_PARAM_COMP,
    OUT_PTR_UPDATE,
    IM_OFFSET_UPDATE,
    IDLE
  } param_state_q, param_state_d;

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
    .sample_en(1'b1),
    .dma_if_i(dma_queue_if_in),
    .dma_if_req_i(dma_if_req),
    .dma_channel_o(dma_queue_channel),
    .dma_if_o(dma_queue_if_out),
    .dma_queue_full_o(dma_queue_full),
    .dma_queue_empty_o(dma_queue_empty)
  );

  /*_________________________________________________________________________________________________________________________________ */

  /* FSMs instantiation */

  /* Parameter computation state transition FSM */
  always_comb begin: proc_comb_param_state_fsm
    unique case (param_state_d)
      
      CH_COL_COMP: begin
        param_state_q = F_MIN_OFFSET_COMP;
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
        if (batch_counter == reg2hw.batch.d)
          param_state_q = IM_OFFSET_UPDATE;
        else
          param_state_q = IM_COORD_COMP;
      end

      IM_OFFSET_UPDATE: begin
        if (ch_col_counter == ch_col)
          param_state_q = IDLE;
        else
          param_state_q = F_MIN_OFFSET_COMP;
      end
    endcase
  end

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

  /*_________________________________________________________________________________________________________________________________ */

  /* Signal assignments */


endmodule
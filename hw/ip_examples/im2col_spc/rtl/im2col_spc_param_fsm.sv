/*
 * Copyright 2024 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *
 * Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *                         <tommaso.terzano@gmail.com>
 *  
 * Info: Parameter FSM of the im2col accelerator.
 */

module im2col_spc_param_fsm
  import im2col_spc_reg_pkg::*;
  import dma_if_pkg::*;
(
    input logic clk_i,
    input logic rst_ni,
    input im2col_spc_reg2hw_t reg2hw_i,
    input logic im2col_done_i,
    input logic fifo_full_i,
    input logic im2col_start_i,
    output logic fifo_push_o,
    output logic im2col_param_done_o,
    output dma_if_t fifo_input_o

);
  /*_________________________________________________________________________________________________________________________________ */

  /* Signals declaration */

  /* Register interface */
  im2col_spc_reg2hw_t reg2hw;

  /* 
   * Every "comp" state refers to variable computations that precede the DMA call in the 
   * loop, while any "param" state refers to the DMA parameters computation that follows the DMA call.
   */
  enum {
    IDLE,
    N_ZEROS_PRECOMP,
    ZEROS_COND_EVAL,
    N_ZEROS_COMP_1,
    N_ZEROS_COMP_2,
    N_ZEROS_COMP_3,
    OUT_PTR_UPDATE,
    IM_OFFSET_UPDATE,
    INDEX_COMP_1,
    INDEX_COMP_2,
    INDEX_COMP_3,
    START_DMA_RUN
  }
      param_state_q, param_state_d;

  /* Parameters computation signals */
  logic [7:0] w_offset;
  logic [7:0] h_offset;
  logic [7:0] im_c;
  logic [7:0] im_row;
  logic [7:0] im_col;
  logic [7:0] fw_min_w_offset;
  logic [7:0] fh_min_h_offset;
  logic [7:0] n_zeros_left;
  logic [7:0] n_zeros_right;
  logic [7:0] n_zeros_top;
  logic [7:0] n_zeros_bottom;
  logic [15:0] size_transfer_1d;
  logic [15:0] size_transfer_2d;
  logic [31:0] index;
  logic [31:0] h_offset_counter;
  logic [31:0] im_c_counter;
  logic [15:0] ch_col_counter;
  logic [7:0] batch_counter;
  logic [31:0] input_data_ptr;
  logic [31:0] output_data_ptr;
  logic [31:0] source_inc_d2;
  logic [31:0] out_data_ptr_inc;
  logic left_zero_cond;
  logic top_zero_cond;
  logic right_zero_cond;
  logic bottom_zero_cond;
  logic [7:0] n_zeros_left_std;
  logic [7:0] n_zeros_top_std;
  logic [7:0] n_zeros_right_std;
  logic [7:0] n_zeros_bottom_std;

  /* Pipelining control signals */
  logic output_data_ptr_en;
  logic im_offset_en;
  logic batch_inc_en;
  logic batch_rst;
  logic zeros_phase1_en;
  logic zeros_phase2_en;
  logic zeros_rst;
  logic output_data_ptr_rst;
  logic pipe_rst;

  /* Status signals */
  logic im2col_param_done;
  logic im2col_done;
  logic im2col_start;
  logic fifo_push;
  logic fifo_full;

  /* Pipelining signals */
  logic [7:0] n_zeros_left_comp1_n;
  logic [7:0] n_zeros_top_comp1_n;
  logic [7:0] n_zeros_right_comp1_n;
  logic [7:0] n_zeros_bottom_comp1_n;
  logic [7:0] n_zeros_left_comp1;
  logic [7:0] n_zeros_top_comp1;
  logic [7:0] n_zeros_right_comp1;
  logic [7:0] n_zeros_bottom_comp1;
  logic [31:0] index_comp1_n;
  logic [31:0] index_comp2_n;
  logic [31:0] index_comp3_n;
  logic [31:0] index_comp4_n;
  logic [31:0] index_comp1;
  logic [31:0] index_comp2;
  logic [31:0] index_comp3;
  logic [31:0] index_comp4;

  /*_________________________________________________________________________________________________________________________________ */

  /* FSMs instantiation */

  /* Parameter computation state transition ff */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_param_state_dq
    if (!rst_ni) begin
      param_state_d <= IDLE;
    end else begin
      param_state_d <= param_state_q;
    end
  end

  /* Parameter computation done signal update */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_param_done
    if (!rst_ni) begin
      im2col_param_done <= 1'b0;
    end else begin
      if (im2col_done == 1'b1) begin
        im2col_param_done <= 1'b0;
      end else if (ch_col_counter == (reg2hw.ch_col.q - 1) && batch_counter == reg2hw.batch.q) begin
        im2col_param_done <= 1'b1;
      end
    end
  end

  /* Parameter computation state transition FSM */
  always_comb begin : proc_comb_param_state_fsm
    fifo_push = 1'b0;
    batch_inc_en = 1'b0;
    batch_rst = 1'b0;
    output_data_ptr_en = 1'b0;
    im_offset_en = 1'b0;
    zeros_phase1_en = 1'b0;
    zeros_phase2_en = 1'b0;
    zeros_rst = 1'b0;
    output_data_ptr_rst = 1'b0;

    unique case (param_state_d)

      IDLE: begin
        batch_rst = 1'b1;
        zeros_rst = 1'b1;
        output_data_ptr_rst = 1'b1;
        if (im2col_start == 1'b1) begin
          param_state_q = N_ZEROS_PRECOMP;
        end else begin
          param_state_q = IDLE;
        end
      end

      /* Cycle needed to precompute parts of both the zero condition and the number of zero computations
       * and to reduce the datapath for FPGA targets. Every signal is identical to the IDLE state.
       * @Tod0: is this really necessary?
       */
      N_ZEROS_PRECOMP: begin
        batch_rst = 1'b1;
        zeros_rst = 1'b1;
        output_data_ptr_rst = 1'b1;
        param_state_q = ZEROS_COND_EVAL;
      end

      ZEROS_COND_EVAL: begin
        if (im2col_param_done == 1'b1) begin
          param_state_q = IDLE;
        end else begin
          param_state_q = N_ZEROS_COMP_1;
        end
      end

      N_ZEROS_COMP_1: begin
        zeros_phase1_en = 1'b1;
        param_state_q   = N_ZEROS_COMP_2;
      end

      N_ZEROS_COMP_2: begin
        zeros_phase2_en = 1'b1;
        param_state_q   = N_ZEROS_COMP_3;
      end

      N_ZEROS_COMP_3: begin
        zeros_phase2_en = 1'b1;
        param_state_q   = INDEX_COMP_1;
      end

      INDEX_COMP_1: begin
        param_state_q = INDEX_COMP_2;
      end

      INDEX_COMP_2: begin
        param_state_q = INDEX_COMP_3;
      end

      INDEX_COMP_3: begin
        if (fifo_full == 1'b0) begin
          param_state_q = START_DMA_RUN;
        end else begin
          param_state_q = INDEX_COMP_3;
        end
      end

      OUT_PTR_UPDATE: begin
        batch_inc_en = 1'b1;
        output_data_ptr_en = 1'b1;
        if (batch_counter == reg2hw.batch.q - 1) begin
          param_state_q = IM_OFFSET_UPDATE;
        end else begin
          param_state_q = ZEROS_COND_EVAL;
        end
      end

      IM_OFFSET_UPDATE: begin
        batch_rst = 1'b1;
        im_offset_en = 1'b1;
        param_state_q = ZEROS_COND_EVAL;
      end

      START_DMA_RUN: begin
        fifo_push = 1'b1;
        param_state_q = OUT_PTR_UPDATE;
      end

      default: begin
        param_state_q = IDLE;
      end

    endcase
  end

  /* Number of zeros computation */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_zeros_phase1_comp
    if (!rst_ni) begin
      fw_min_w_offset <= '0;
      fh_min_h_offset <= '0;
    end else begin
      if (zeros_phase1_en == 1'b1) begin
        fw_min_w_offset <= reg2hw.fw.q - 1 - w_offset;
        fh_min_h_offset <= reg2hw.fh.q - 1 - h_offset;
      end else if (zeros_rst == 1'b1) begin
        fw_min_w_offset <= '0;
        fh_min_h_offset <= '0;
      end
    end
  end

  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_zeros_phase2_comp
    if (!rst_ni) begin
      n_zeros_left <= '0;
      n_zeros_right <= '0;
      n_zeros_top <= '0;
      n_zeros_bottom <= '0;
    end else begin
      if (zeros_phase2_en == 1'b1) begin
        /* Left zeros computation */
        if (w_offset >= {2'h0, reg2hw.pad_left.q}) begin
          n_zeros_left <= 0;
        end else begin
          n_zeros_left <= n_zeros_left_std + {7'h0, left_zero_cond};  // Sum a 1 if the condition is true
        end

        /* Top zeros computation */
        if (h_offset >= {2'h0, reg2hw.pad_top.q}) begin
          n_zeros_top <= 0;
        end else begin
          n_zeros_top <= n_zeros_top_std + {7'h0, top_zero_cond};
        end

        /* Right zeros computation */
        if (fw_min_w_offset >= {2'h0, reg2hw.pad_right.q} || reg2hw.adpt_pad_right.q == 0) begin
          n_zeros_right <= 0;
        end else begin
          n_zeros_right <= n_zeros_right_std + {7'h0, right_zero_cond};
        end

        /* Bottom zeros computation */
        if (fh_min_h_offset >= {2'h0, reg2hw.pad_bottom.q} || reg2hw.adpt_pad_bottom.q == 0) begin
          n_zeros_bottom <= 0;
        end else begin
          n_zeros_bottom <= n_zeros_bottom_std + {7'h0, bottom_zero_cond};
        end
      end else if (zeros_rst == 1'b1) begin
        n_zeros_left <= '0;
        n_zeros_right <= '0;
        n_zeros_top <= '0;
        n_zeros_bottom <= '0;
      end
    end
  end


  /* Batch counter */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_batch_counter
    if (!rst_ni) begin
      batch_counter <= '0;
    end else begin
      if (batch_inc_en == 1'b1) begin
        batch_counter <= batch_counter + 1;
      end else if (batch_rst == 1'b1) begin
        batch_counter <= '0;
      end
    end
  end

  /* w_offset counter */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_w_offset_counter
    if (!rst_ni) begin
      w_offset <= '0;
    end else begin
      if (im2col_param_done == 1'b1) begin
        w_offset <= '0;
      end else if (im_offset_en == 1'b1) begin
        if (w_offset == reg2hw.fw.q - 1) begin
          w_offset <= '0;
        end else begin
          w_offset <= w_offset + 1;
        end
      end
    end
  end

  /* h_offset counter */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_h_offset_counter
    if (!rst_ni) begin
      h_offset <= '0;
      h_offset_counter <= '0;
    end else begin
      if (im2col_param_done == 1'b1) begin
        h_offset <= '0;
        h_offset_counter <= '0;
      end else if (im_offset_en == 1'b1) begin
        if (h_offset_counter == {24'h0, reg2hw.fw.q} - 1) begin
          h_offset_counter <= '0;
          if (h_offset == reg2hw.fh.q - 1) begin
            h_offset <= '0;
          end else begin
            h_offset <= h_offset + 1;
          end
        end else begin
          h_offset_counter <= h_offset_counter + 1;
        end
      end
    end
  end

  /* im_c counter */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_im_c_counter
    if (!rst_ni) begin
      im_c <= '0;
      im_c_counter <= '0;
    end else begin
      if (im2col_param_done == 1'b1) begin
        im_c <= '0;
        im_c_counter <= '0;
      end else if (im_offset_en == 1'b1) begin
        if (im_c_counter == {24'h0, reg2hw.fh.q} * {24'h0, reg2hw.fw.q} - 1) begin
          im_c_counter <= '0;
          im_c <= im_c + 1;
        end else begin
          im_c_counter <= im_c_counter + 1;
        end
      end
    end
  end

  /* ch_col counter */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_ch_col_counter
    if (!rst_ni) begin
      ch_col_counter <= '0;
    end else begin
      if (im2col_param_done == 1'b1) begin
        ch_col_counter <= '0;
      end else if (im_offset_en == 1'b1) begin
        if (ch_col_counter == reg2hw.ch_col.q - 1) begin
          ch_col_counter <= '0;
        end else begin
          ch_col_counter <= ch_col_counter + 1;
        end
      end
    end
  end

  /* Output data pointer counter */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_output_data_ptr
    if (!rst_ni) begin
      output_data_ptr <= reg2hw.dst_ptr.q;
    end else begin
      if (output_data_ptr_en == 1'b1) begin
        output_data_ptr <= output_data_ptr + out_data_ptr_inc;
      end else if (output_data_ptr_rst == 1'b1) begin
        output_data_ptr <= reg2hw.dst_ptr.q;
      end
    end
  end

  /*_________________________________________________________________________________________________________________________________ */

  /* Modules instantiation */

  /* Pipeline modules */
  pipe_reg #(
      .WIDTH(8)
  ) pipe_reg_left_zeros (
      .clk_i,
      .rst_ni  (pipe_rst),
      .data_in (n_zeros_left_comp1_n),
      .data_out(n_zeros_left_comp1)
  );

  pipe_reg #(
      .WIDTH(8)
  ) pipe_reg_top_zeros (
      .clk_i,
      .rst_ni  (pipe_rst),
      .data_in (n_zeros_top_comp1_n),
      .data_out(n_zeros_top_comp1)
  );

  pipe_reg #(
      .WIDTH(8)
  ) pipe_reg_right_zeros (
      .clk_i,
      .rst_ni  (pipe_rst),
      .data_in (n_zeros_right_comp1_n),
      .data_out(n_zeros_right_comp1)
  );

  pipe_reg #(
      .WIDTH(8)
  ) pipe_reg_bottom_zeros (
      .clk_i,
      .rst_ni  (pipe_rst),
      .data_in (n_zeros_bottom_comp1_n),
      .data_out(n_zeros_bottom_comp1)
  );

  pipe_reg #(
      .WIDTH(32)
  ) pipe_reg_index_comp1 (
      .clk_i,
      .rst_ni  (pipe_rst),
      .data_in (index_comp1_n),
      .data_out(index_comp1)
  );

  pipe_reg #(
      .WIDTH(32)
  ) pipe_reg_index_comp2 (
      .clk_i,
      .rst_ni  (pipe_rst),
      .data_in (index_comp2_n),
      .data_out(index_comp2)
  );

  pipe_reg #(
      .WIDTH(32)
  ) pipe_reg_index_comp3 (
      .clk_i,
      .rst_ni  (pipe_rst),
      .data_in (index_comp3_n),
      .data_out(index_comp3)
  );

  pipe_reg #(
      .WIDTH(32)
  ) pipe_reg_index_comp4 (
      .clk_i,
      .rst_ni  (pipe_rst),
      .data_in (index_comp4_n),
      .data_out(index_comp4)
  );


  /*_________________________________________________________________________________________________________________________________ */

  /* Signal assignments */

  assign im_row = h_offset - {2'h0, reg2hw.pad_top.q};  // im_row = h_offset - TOP_PAD;
  assign im_col = w_offset - {2'h0, reg2hw.pad_left.q};  // im_col = w_offset - LEFT_PAD;

  assign n_zeros_left_comp1_n = ({2'h0, reg2hw.pad_left.q} - w_offset);
  assign n_zeros_left_std = n_zeros_left_comp1 >> {4'h0, reg2hw.log_strides_d1.q};

  assign n_zeros_top_comp1_n = ({2'h0, reg2hw.pad_top.q} - h_offset);
  assign n_zeros_top_std = n_zeros_top_comp1 >> {4'h0, reg2hw.log_strides_d2.q};

  assign n_zeros_right_comp1_n = (reg2hw.adpt_pad_right.q - fw_min_w_offset);
  assign n_zeros_right_std = n_zeros_right_comp1 >> {4'h0, reg2hw.log_strides_d1.q};

  assign n_zeros_bottom_comp1_n = (reg2hw.adpt_pad_bottom.q - fh_min_h_offset);
  assign n_zeros_bottom_std = n_zeros_bottom_comp1 >> {4'h0, reg2hw.log_strides_d2.q};

  assign size_transfer_1d = reg2hw.n_patches_w.q - {8'h0, n_zeros_left} - {8'h0, n_zeros_right};
  assign size_transfer_2d = reg2hw.n_patches_h.q - {8'h0, n_zeros_top} - {8'h0, n_zeros_bottom};

  assign index_comp1_n = ({24'h0, batch_counter} * {24'h0, reg2hw.num_ch.q} + {24'h0, im_c});
  assign index_comp2_n = index_comp1 * {16'h0, reg2hw.ih.q};
  assign index_comp3_n = (index_comp2 + ({{24{im_row[7]}}, im_row} + ({24'h0, n_zeros_top} << {4'h0, reg2hw.log_strides_d2.q})));
  assign index_comp4_n = index_comp3 * {16'h0, reg2hw.iw.q} + {{24{im_col[7]}}, im_col};
  assign index = index_comp4 + ({24'h0, n_zeros_left} << {24'h0, reg2hw.log_strides_d1.q});

  assign source_inc_d2 = (({16'h0, reg2hw.iw.q} << {24'h0, reg2hw.log_strides_d2.q}) - (({16'h0, size_transfer_1d} - 1) << {24'h0, reg2hw.log_strides_d1.q}));
  assign input_data_ptr = reg2hw.src_ptr.q + (index << (2 - reg2hw.data_type.q));
  assign out_data_ptr_inc = ({16'h0, reg2hw.n_patches_h.q} * {16'h0, reg2hw.n_patches_w.q}) << (2 - reg2hw.data_type.q);

  assign left_zero_cond = |((n_zeros_left_comp1) & ((1 << {reg2hw.log_strides_d1.q}) - 1));
  assign top_zero_cond = |((n_zeros_top_comp1) & ((1 << {reg2hw.log_strides_d2.q}) - 1));
  assign right_zero_cond = |((n_zeros_right_comp1) & ((1 << {reg2hw.log_strides_d1.q}) - 1));
  assign bottom_zero_cond = |((n_zeros_bottom_comp1) & ((1 << {reg2hw.log_strides_d2.q}) - 1));

  assign pipe_rst = (rst_ni && param_state_d != IDLE);

  /* Output signals */
  assign im2col_param_done_o = im2col_param_done;
  assign reg2hw = reg2hw_i;
  assign im2col_done = im2col_done_i;
  assign im2col_start = im2col_start_i;
  assign fifo_push_o = fifo_push;
  assign fifo_full = fifo_full_i;

  assign fifo_input_o.input_ptr = input_data_ptr;
  assign fifo_input_o.output_ptr = output_data_ptr;
  assign fifo_input_o.in_inc_d2 = source_inc_d2[22:0];
  assign fifo_input_o.n_zeros_top = n_zeros_top;
  assign fifo_input_o.n_zeros_bottom = n_zeros_bottom;
  assign fifo_input_o.n_zeros_left = n_zeros_left;
  assign fifo_input_o.n_zeros_right = n_zeros_right;
  assign fifo_input_o.size_du_d1 = size_transfer_1d;
  assign fifo_input_o.size_du_d2 = size_transfer_2d;

endmodule

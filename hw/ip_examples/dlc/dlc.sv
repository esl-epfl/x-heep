// Copyright 2025 EPFL and Politecnico di Torino.
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: dlc.sv
// Author: Alessio Naclerio
// Date: 17/02/2025
// Description: Digital Level Crossing Block

module dlc #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic,
    parameter type hw_fifo_req_t = logic,
    parameter type hw_fifo_resp_t = logic
) (
    input logic clk_i,
    input logic rst_ni,
    // width of the level window (it must be a power of 2)
    input reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,
    // fifo interface
    input hw_fifo_req_t hw_fifo_req_i,
    output hw_fifo_resp_t hw_fifo_resp_o
);

  // Signals Declaration

  // ------------------------- FSM

  typedef enum logic [1:0] {
    DLC_RUN,
    DLC_DLVL_OVF,
    DLC_DT_OVF
  } dlc_state_t;

  dlc_state_t dlc_state_n;
  dlc_state_t dlc_state;

  // ------------------------- Write and Read Fifos

  logic hw_r_fifo_empty;
  logic hw_r_fifo_pop;
  logic signed [15:0] hw_r_fifo_data_out;
  logic [15:0] hw_w_fifo_data_in;
  logic hw_w_fifo_full;
  logic hw_w_fifo_push;
  logic valid;

  // ------------------------- Registers

  logic [9:0] reg_log_wl_q;  // log2 of the level width
  logic [7:0] reg_dlvl_bits_q;  // number of bits for the delta levels in the output data
  logic [7:0] reg_dt_bits_q;  // number of bits for the delta time output data
  logic [15:0] reg_mask_dlvl_q;  // mask for delta levels
  logic [15:0] reg_mask_dt_q;  // mask for delta time
  logic reg_dlvl_signed_q;  // if '1' delta levels are in 2s complement, else sign|abs_value
  logic reg_output_format_q;  // output format of the data
  logic reg_read_write_q;  // base response push on read/write data

  // ------------------------- Level Crossing Logic

  logic [15:0] curr_lvl;  // current level
  logic xing;  // crossing signal
  logic signed [15:0] din_lvl;  // input data level

  // ------------------------- Sample Skip Counter

  logic [15:0] skip_cnt;  // skip counter for the number of samples that have been skipped
  logic skip_cnt_en;  // skip counter enable signal
  logic skip_cnt_rst;  // skip counter reset signal
  logic [15:0] dt_out;  // delta time actual output

  // ------------------------- Delta Levels Management

  logic [15:0] dlvl;  // difference between din level and the current level (signed)
  logic signed [15:0] dlvl_abs;  // abolute value delta levels
  logic dlvl_ovf;  // delta levels overflow bit
  logic [15:0] dlvl_out;  // delta levels actual output
  logic [15:0] ovf_dwn_cnt;  // overflow down counter
  logic [15:0] ovf_dwn_cnt_n;  // next value of the overflow down counter
  logic [15:0] ovf_dwn_cnt_op1;  // operand 1 of the overflow down counter
  logic check_end_ovf;  // '1' if overflow data must not be sent anymore
  logic dir;  // direction of the delta levels
  logic dir_reg;  // direction register
  logic [16:0] dir_ext;  // direction extended on 16 bits
  logic dir_out;  // dir actual output
  logic [16:0] dlvl_dir_out;  // combined delta levels and direction output




  // ------------------------- FSM

  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (~rst_ni) begin
      dlc_state <= DLC_RUN;
    end else begin
      dlc_state <= dlc_state_n;
    end
  end

  always_comb begin
    if (dlc_state == DLC_RUN) begin
      // dlvl has overflowed and write fifo is not full
      // (if the hw write fifo is full, no need to stall because din_lvl will be the preserved
      // as no pop operation will be issued from the hw read fifo)
      if (dlvl_ovf && !hw_w_fifo_full) begin
        dlc_state_n = DLC_DLVL_OVF;
        // dt has overflowed but write fifo is full, so we have to wait
      end else if (skip_cnt_en && (skip_cnt == reg_mask_dt_q)) begin
        dlc_state_n = DLC_DT_OVF;
      end else begin
        dlc_state_n = DLC_RUN;
      end
    end else if (dlc_state == DLC_DLVL_OVF) begin
      // the end of the overflow has been reached so we can go back to the run state
      if (check_end_ovf == 1'b1) begin
        dlc_state_n = DLC_RUN;
      end else begin
        dlc_state_n = DLC_DLVL_OVF;
      end
    end else if (dlc_state == DLC_DT_OVF) begin
      // if the write fifo is not full anymore we can go back to the run state
      // otherwise we stay in the stall state
      if (!hw_w_fifo_full) begin
        dlc_state_n = DLC_RUN;
      end else begin
        dlc_state_n = DLC_DT_OVF;
      end
    end else begin
      dlc_state_n = DLC_RUN;
    end
  end

  // ------------------------- Write and Read Fifos

  assign hw_fifo_resp_o.data[31:16] = '0;

  /* Hardware Read Fifo */
  fifo_v3 #(
      .DEPTH(4),
      .FALL_THROUGH(1'b0),
      .DATA_WIDTH(16)
  ) hw_r_fifo_i (
      .clk_i(clk_i),
      .rst_ni,
      .flush_i(),
      .testmode_i(1'b0),
      // status flags
      .full_o(hw_fifo_resp_o.full),
      .empty_o(hw_r_fifo_empty),
      .usage_o(),
      // as long as the queue is not full we can push new data
      .data_i(hw_fifo_req_i.data[15:0]),
      .push_i(hw_fifo_req_i.push),
      // as long as the queue is not empty we can pop new elements
      .data_o(hw_r_fifo_data_out),
      .pop_i(hw_r_fifo_pop)
  );

  /* Hardware Write Fifo */
  fifo_v3 #(
      .DEPTH(4),
      .FALL_THROUGH(1'b0),
      .DATA_WIDTH(16)
  ) hw_w_fifo_i (
      .clk_i(clk_i),
      .rst_ni,
      .flush_i(),
      .testmode_i(1'b0),
      // status flags
      .full_o(hw_w_fifo_full),
      .empty_o(hw_fifo_resp_o.empty),
      .usage_o(),
      // as long as the queue is not full we can push new data
      .data_i(hw_w_fifo_data_in),
      .push_i(hw_w_fifo_push),
      // as long as the queue is not empty we can pop new elements
      .data_o(hw_fifo_resp_o.data[15:0]),
      .pop_i(hw_fifo_req_i.pop)
  );

  // ------------------------- Registers

  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (~rst_ni) begin
      reg_log_wl_q <= '0;
      reg_dlvl_bits_q <= '0;
      reg_dt_bits_q <= '0;
      reg_dlvl_signed_q <= '0;
      reg_mask_dlvl_q <= '0;
      reg_mask_dt_q <= '0;
    end else begin
      if (reg_req_i.valid && reg_req_i.write) begin
        if (reg_req_i.addr[7:0] == 8'h0) begin
          reg_log_wl_q <= reg_req_i.wdata[9:0];
        end else if (reg_req_i.addr[7:0] == 8'h4) begin
          reg_dlvl_bits_q <= reg_req_i.wdata[7:0];
        end else if (reg_req_i.addr[7:0] == 8'h8) begin
          reg_dt_bits_q <= reg_req_i.wdata[7:0];
        end else if (reg_req_i.addr[7:0] == 8'hc) begin
          reg_dlvl_signed_q <= reg_req_i.wdata[0];
        end else if (reg_req_i.addr[7:0] == 8'h10) begin
          reg_output_format_q <= reg_req_i.wdata[0];
        end else if (reg_req_i.addr[7:0] == 8'h14) begin
          reg_mask_dlvl_q[15:0] <= reg_req_i.wdata[15:0];
        end else if (reg_req_i.addr[7:0] == 8'h18) begin
          reg_mask_dt_q <= reg_req_i.wdata[15:0];
        end else if (reg_req_i.addr[7:0] == 8'h1c) begin
          reg_read_write_q <= reg_req_i.wdata[0];
        end
      end
    end
  end

  always_comb begin
    reg_rsp_o.rdata = '0;
    reg_rsp_o.error = 1'b0;
    reg_rsp_o.ready = 1'b1;

    if (reg_req_i.valid) begin
      if (reg_req_i.addr[7:0] == 8'h0) begin
        reg_rsp_o.rdata[9:0] = reg_log_wl_q;
      end else if (reg_req_i.addr[7:0] == 8'h4) begin
        reg_rsp_o.rdata[7:0] = reg_dlvl_bits_q;
      end else if (reg_req_i.addr[7:0] == 8'h8) begin
        reg_rsp_o.rdata[7:0] = reg_dt_bits_q;
      end else if (reg_req_i.addr[7:0] == 8'hc) begin
        reg_rsp_o.rdata[0] = reg_dlvl_signed_q;
      end else if (reg_req_i.addr[7:0] == 8'h10) begin
        reg_rsp_o.rdata[0] = reg_output_format_q;
      end else if (reg_req_i.addr[7:0] == 8'h14) begin
        reg_rsp_o.rdata[15:0] = reg_mask_dlvl_q;
      end else if (reg_req_i.addr[7:0] == 8'h18) begin
        reg_rsp_o.rdata[15:0] = reg_mask_dt_q;
      end else if (reg_req_i.addr[7:0] == 8'h1c) begin
        reg_rsp_o.rdata[0] = reg_read_write_q;
      end else begin
        reg_rsp_o.error = 1'b1;
      end
    end
  end

  // ------------------------- Fifo Control Logic

  // Pop from HW Read FIFO
  always_comb begin
    /*
      Popping from the hw read fifo is allowed only when (in parallel):
        -> the hw read fifo is not empty
        -> the hw write fifo is not full
        -> the dlc state is in the run state

      The valid signal is set to 1 when the above conditions are met, too.
    */
    hw_r_fifo_pop = 1'b0;
    valid = 1'b0;
    if (hw_r_fifo_empty == 1'b0 &&
        hw_w_fifo_full == 1'b0 &&
        (dlc_state != DLC_DLVL_OVF && dlc_state != DLC_DT_OVF)) begin
      hw_r_fifo_pop = 1'b1;
      valid = 1'b1;
    end
  end

  /*
    This signal is used to make the downcounter inside the DMA decrement
    even if the data is not actually pushed into the hw write fifo.
  */
  assign hw_fifo_resp_o.push = reg_read_write_q ? valid : hw_w_fifo_push;

  // Push to HW Write FIFO
  always_comb begin

    /*
      Pushing to the hw write fifo is allowed only when:
        -> the hw write fifo is not full AND
          -> the dlc state is DLC_RUN and a crossing is detected (standard crossing case)
          -> the delta levels have just overflowed (dlvl_ovf) or the state is DLC_DLVL_OVF
             (this means that the overflow data is being pushed to hw write fifo)
          -> the delta time has overflowed (skip_cnt_ovf) and the state is DLC_DT_OVF
             (this means that the overflow data is waiting to being pushed to hw write fifo
              as soon as the hw write fifo is not full anymore)

      The hw write fifo is pushed only when one of the above conditions is met.
    */

    hw_w_fifo_push = 1'b0;
    if (hw_w_fifo_full == 1'b0 && ((xing && dlc_state == DLC_RUN) ||  // crossing detected
        (dlc_state == DLC_DLVL_OVF || dlvl_ovf) ||  // delta levels overflows
        (dlc_state == DLC_DT_OVF))) begin  // delta time overflows (CHECK)
      hw_w_fifo_push = 1'b1;
    end
  end

  // ------------------------- Level Crossing Logic

  // Input data level detection:
  // input data shifted right by (log2 of the level width) positions.
  // the resulting signal indicates the level the input data belongs to.
  always_comb begin
    din_lvl = hw_r_fifo_data_out >>> reg_log_wl_q;
  end

  // Threshold updating:
  // the current level is updated only when a crossing is detected.
  // the current level is updated with the the input data level.
  always_ff @(posedge clk_i, negedge rst_ni) begin
    if (~rst_ni) begin
      curr_lvl <= '0;
    end else begin
      if (xing) begin
        curr_lvl <= din_lvl;
      end else begin
        curr_lvl <= curr_lvl;
      end
    end
  end

  // Level crossing detection:
  // the crossing signal is set to 1 difference of levels between the input data level and the current level
  // is not equal to 0. This either means that the input data belongs to a level above or below the current level.
  always_comb begin
    // delta levels calculation
    dlvl = (valid) ? ($signed(din_lvl) - $signed(curr_lvl)) : '0;
    xing = dlvl != 0;
  end

  // ------------------------- Sample Skip Counter

  always_comb begin
    /*
      Skip counter enable:
        the skip counter is enabled when no crossing happened and the hw read fifo is being popped.
        This means that the skip counter is incremented only when a crossing is not detected on a valid input data.
    */
    skip_cnt_en  = !xing && hw_r_fifo_pop;

    /*
      Skip counter reset:
        the skip counter is reset when a crossing is detected or the skip counter has overflowed.
    */
    skip_cnt_rst = xing || (dlc_state == DLC_DT_OVF && !hw_w_fifo_full);
  end

  // Sample Skip Counter
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (~rst_ni) begin
      skip_cnt <= 16'd0;
    end else begin
      if (skip_cnt_rst == 1'b1) begin
        skip_cnt <= 16'd1;
      end else if (skip_cnt_en == 1'b1) begin
        skip_cnt <= skip_cnt + 1;
      end else begin
        skip_cnt <= skip_cnt;
      end
    end
  end

  /* Delta Time Output
      -> if the skip counter has overflowed
         the delta time output is set to the skip counter mask (all 1s).
      -> if the delta levels have just overflowed, delta time is the sample skip counter value.
      -> if delta levels have overflowed and a stall due to them is happening, delta time is 0
         because more thatn one packets must be sent to represent one data, but they should appear
         one right after the other, with zero time interval.
      -> if the delta levels are valid, delta time is the sample skip counter value.
  */
  always_comb begin
    if (dlc_state == DLC_DT_OVF) begin  // skip cnt overflow or stall due to it
      dt_out = reg_mask_dt_q;
    end else if (dlvl_ovf) begin  // dlvl just overflowed
      dt_out = skip_cnt;
    end else if (dlc_state == DLC_DLVL_OVF) begin  // stall due to dlvl
      dt_out = '0;
    end else begin
      dt_out = skip_cnt;  // standard run case
    end
  end

  // ------------------------- Delta Levels Management

  always_comb begin
    // delta levels direction is the sign of the delta levels
    dir = dlvl[15];
    // absolute value of the delta levels
    dlvl_abs = (dlvl[15]) ? (~dlvl + 1) : (dlvl);

    /* delta levels overflow detection:
        the overflow bit is set to 1 when the delta levels are valid and the delta levels
        are greater than the delta levels mask.
        This is detected by ANDing the delta levels with the inverted delta levels mask,
        and then checking if any of the bits is set to 1.
        If this is the case, it means that dlvl cannot be represented with the current
        number of bits.
    */
    dlvl_ovf = valid & (|(dlvl_abs & ~reg_mask_dlvl_q));

    /*
      Overflow down counter operand 1 is equal to the absolute value of the delta levels
      if the delta levels have just overflowed,
      otherwise it is equal to the down counter value.
    */
    ovf_dwn_cnt_op1 = (dlvl_ovf) ? (dlvl_abs) : (ovf_dwn_cnt);

    /*
      The overflow down counter is decremented by the delta levels mask.
      If the overflow down counter is less than 0, the remainder must be sent and then stop.
    */
    ovf_dwn_cnt_n = $signed(ovf_dwn_cnt_op1) - $signed(reg_mask_dlvl_q);
    check_end_ovf = hw_w_fifo_full ? '0 : (ovf_dwn_cnt_n[15] == 1'b1 || ovf_dwn_cnt_n == 16'h0);
  end

  /*
    Overflow down counter and direction register
      -> the overflow down counter is updated to its next value if the dlvl just overflowed
         or a stall is happening due to dlvl. Otherwise, it is set to 0.
      -> the direction register is updated to the direction of the delta levels if the dlvl just overflowed.
         If a stall is in act, dir keeps its valid. Otherwise, it is set to 0.
  */
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (~rst_ni) begin
      ovf_dwn_cnt <= '0;
      dir_reg <= 1'b0;
    end else begin
      if (!hw_w_fifo_full) begin
        if (dlvl_ovf) begin
          ovf_dwn_cnt <= ovf_dwn_cnt_n;
          dir_reg <= dir;
        end else if (dlc_state == DLC_DLVL_OVF) begin
          if (check_end_ovf) begin
            ovf_dwn_cnt <= '0;
          end else begin
            ovf_dwn_cnt <= ovf_dwn_cnt_n;
          end
          dir_reg <= dir_reg;
        end else begin
          ovf_dwn_cnt <= '0;
          dir_reg <= 1'b0;
        end
      end
    end
  end

  /* dlvl and dir output construction
      -> if the delta levels have just overflowed, the delta levels output is set to the delta levels mask
         and the direction is set to the direction of the delta levels.
      -> if a stall is happening due to delta levels and it is ongoing, the delta levels output is set to the delta levels mask
         and the direction is set to the direction register.
      -> if a stall is happening due to delta levels and it has finished, the delta levels output is set to the
         operand 1 of the overflow down counter because it is the remainder of the delta levels
         and the direction is set to the direction register.
      -> if the delta levels are valid and the skip counter has not overflowed, the delta levels output is set to
         the delta levels if the delta levels are signed, otherwise it is set to the absolute value of the delta levels.
         The direction is set to the direction of the delta levels. If the skip counter has overflowed, the delta levels
         and the direction outputs is set to 0.
  */
  always_comb begin
    if (dlvl_ovf) begin
      dlvl_out = reg_mask_dlvl_q;
      dir_out  = dir;
    end else if (dlc_state == DLC_DLVL_OVF && check_end_ovf == 1'b0) begin
      dlvl_out = reg_mask_dlvl_q;
      dir_out  = dir_reg;
    end else if (dlc_state == DLC_DLVL_OVF && check_end_ovf == 1'b1) begin
      dlvl_out = ovf_dwn_cnt_op1 & reg_mask_dlvl_q;
      dir_out  = dir_reg;
    end else begin
      dlvl_out = (dlc_state == DLC_DT_OVF) ? '0 : ((reg_dlvl_signed_q) ? (dlvl & reg_mask_dlvl_q) : (dlvl_abs & reg_mask_dlvl_q));
      dir_out = (dlc_state == DLC_DT_OVF) ? '0 : (dir);
    end
  end

  // Delta levels and direction output construction
  // direction must be shift left by reg_dlvl_bits_q and then combined with the delta levels
  always_comb begin
    dir_ext[16:1] = (reg_dlvl_signed_q) ? {1'b0, dt_out[15:1]} : dt_out[15:0];
    dir_ext[0] = (reg_dlvl_signed_q) ? dt_out[0] : dir_out;
    dlvl_dir_out = (dir_ext << reg_dlvl_bits_q) | {1'b0, dlvl_out};
  end

  // ------------------------- Write Fifo Input Data Construction

  //assign hw_w_fifo_data_in[3:0]   = dlvl_dir_out[3:0];
  //assign hw_w_fifo_data_in[7:4]   = (reg_output_format_q) ? (dlvl_dir_out[7:4]) : (dt_out[3:0]);
  //assign hw_w_fifo_data_in[11:8]  = (reg_output_format_q) ? (dt_out[3:0]) : ('0);
  //assign hw_w_fifo_data_in[15:12] = (reg_output_format_q) ? (dt_out[7:4]) : ('0);
  assign hw_w_fifo_data_in = dlvl_dir_out[15:0];
endmodule

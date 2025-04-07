// Copyright 2025 EPFL and Politecnico di Torino.
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: dlc.sv
// Author: Alessio Naclerio
// Date: 17/02/2025
// Description: Digital Level Crossing Block

module dlc (
    input logic clk_i,
    input logic rst_ni,
    // interrupt
    output logic dlc_xing_intr_o,
    // width of the level window (it must be a power of 2)
    input reg_pkg::reg_req_t reg_req_i,
    output reg_pkg::reg_rsp_t reg_rsp_o,
    // fifo interface
    input hw_fifo_pkg::hw_fifo_req_t hw_fifo_req_i,
    output hw_fifo_pkg::hw_fifo_resp_t hw_fifo_resp_o
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

  // ------------------------- Interrupt

  logic dlc_xing_intr;

  // ------------------------- Write and Read Fifos

  logic hw_r_fifo_empty;
  logic hw_r_fifo_pop;
  logic signed [15:0] hw_r_fifo_data_out;
  logic [15:0] hw_w_fifo_data_in;
  logic hw_w_fifo_full;
  logic hw_w_fifo_push;

  // ------------------------- Registers

  dlc_reg_pkg::dlc_reg2hw_t reg2hw;
  dlc_reg_pkg::dlc_hw2reg_t hw2reg;
  logic [3:0] reg_log_wl;  // log2 of the level width
  logic [3:0] reg_dlvl_bits;  // number of bits for the delta levels in the output data
  logic [15:0] reg_dlvl_mask;  // mask for delta levels, it has as many 1s as the number of bits for the delta levels
  logic [15:0] reg_dt_mask;  // mask for delta time, it has as many 1s as the number of bits for the delta time
  logic reg_dlvl_twoscomp_n_sgnmod;  // if '1' delta levels are in 2s complement, else sign|abs_value
  logic reg_rnw;  // base response push on read/write data
  logic reg_bypass;  // bypass mode

  // ------------------------- Level Crossing Logic

  logic [15:0] curr_lvl;  // current level
  logic xing;  // crossing signal asserted when a crossing occurs
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
  logic [16:0] dt_dir_out;  // delta-time and direction packet extended on 17 bits
  logic dir_out;  // dir actual output
  logic [16:0] dlc_output;  // dLC output packet

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
      /* 
        dlvl has overflowed and write fifo is not full
        (if the hw write fifo is full, no need to stall because din_lvl will be preserved
        as no pop operation will be issued from the hw read fifo)
      */
      if (dlvl_ovf && !hw_w_fifo_full) begin
        dlc_state_n = DLC_DLVL_OVF;
        /* 
        delta time has overflowed
      */
      end else if (skip_cnt_en && (skip_cnt == reg_dt_mask)) begin
        dlc_state_n = DLC_DT_OVF;
      end else begin
        dlc_state_n = DLC_RUN;
      end
    end else if (dlc_state == DLC_DLVL_OVF) begin
      /*
        the end of delta level overflow has been reached so we can go back to the run state
      */
      if (check_end_ovf == 1'b1) begin
        dlc_state_n = DLC_RUN;
      end else begin
        dlc_state_n = DLC_DLVL_OVF;
      end
    end else if (dlc_state == DLC_DT_OVF) begin
      /*
        if the write fifo is not full anymore we can go back to the run state
        otherwise we stay in the stall state due to delta time overflow
      */
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

  // The dLC response packet is at most 16-bit wide
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
      .full_o(hw_fifo_resp_o.full),
      .empty_o(hw_r_fifo_empty),
      .usage_o(),
      .data_i(hw_fifo_req_i.data[15:0]),
      .push_i(hw_fifo_req_i.push),
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
      .full_o(hw_w_fifo_full),
      .empty_o(hw_fifo_resp_o.empty),
      .usage_o(),
      .data_i(hw_w_fifo_data_in),
      .push_i(hw_w_fifo_push),
      .data_o(hw_fifo_resp_o.data[15:0]),
      .pop_i(hw_fifo_req_i.pop)
  );

  // ------------------------- Registers

  parameter type reg_req_t = reg_pkg::reg_req_t;
  parameter type reg_rsp_t = reg_pkg::reg_rsp_t;
  dlc_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) dlc_reg_top_i (
      .clk_i,
      .rst_ni,
      .reg2hw,
      .hw2reg,
      .reg_req_i,
      .reg_rsp_o,
      .devmode_i(1'b0)
  );

  /*
    Delta-Levels Configuration
  */
  assign reg_log_wl = reg2hw.dlvl_log_level_width.q;
  assign reg_dlvl_bits = reg2hw.dlvl_n_bits.q;
  assign reg_dlvl_mask = reg2hw.dlvl_mask.q;
  assign reg_dlvl_twoscomp_n_sgnmod = reg2hw.dlvl_format.q;
  /*
    Delta-Time Configuration
  */
  assign reg_dt_mask = reg2hw.dt_mask.q;
  /*
    Response 'Push' Configuration
  */
  assign reg_rnw = reg2hw.readnotwrite.q;
  assign reg_bypass = reg2hw.bypass.q;

  // ------------------------- Fifo Control Logic

  // Management of popping from HW Read FIFO
  always_comb begin
    /*
      Popping from the hw read fifo is allowed only when (at the same time):
        -> the hw read fifo is not empty
        -> the hw write fifo is not full
        -> the dlc state is in the run state
    */
    hw_r_fifo_pop = 1'b0;
    if (hw_r_fifo_empty == 1'b0 &&
        hw_w_fifo_full == 1'b0 &&
        (dlc_state != DLC_DLVL_OVF && dlc_state != DLC_DT_OVF)) begin
      hw_r_fifo_pop = 1'b1;
    end
  end

  /*
    Response dLC Push:
      -> if 1, the dma downcounter in dma_padding_fsm downcounts each time data is read from the HW Read Fifo
      -> if 0, the dma downcounter in dma_padding_fsm downcounts each time data is written into the HW Write Fifo      
  */
  assign hw_fifo_resp_o.push = reg_rnw ? hw_r_fifo_pop : hw_w_fifo_push;

  // Management of pushing to HW Write FIFO
  always_comb begin

    /*
      Pushing to the hw write fifo is allowed only when:
        -> the hw write fifo is not full AND
          -> the dlc state is DLC_RUN and a crossing is detected (standard crossing case)
          -> delta levels have just overflowed (dlvl_ovf) or the state is DLC_DLVL_OVF
             (this means that the overflow data is being pushed to hw write fifo)
          -> delta time has overflowed (skip_cnt_ovf) and the state is DLC_DT_OVF
             (this means that the overflow data is waiting to being pushed to hw write fifo
              as soon as the hw write fifo is not full anymore)

      The hw write fifo is pushed only when one of the above conditions is met.
    */

    hw_w_fifo_push = 1'b0;
    if (hw_w_fifo_full == 1'b0 && ((xing && dlc_state == DLC_RUN) ||  // crossing detected
        (dlc_state == DLC_DLVL_OVF || dlvl_ovf) ||  // delta levels overflows
        (dlc_state == DLC_DT_OVF))) begin  // delta time overflows
      hw_w_fifo_push = 1'b1;
    end
  end

  // ------------------------- Level Crossing Logic

  /*
    Input data level detection:
      input data shifted right by (log2 of the level width) positions.
      the resulting signal indicates the level the input data belongs to.
      4-bit arithmetic right shifter.
  */
  always_comb begin
    din_lvl = hw_r_fifo_data_out >>> reg_log_wl;
  end

  /* 
    Threshold updating:
      the current level is updated only when a crossing is detected.
      the current level is updated with the the input data level.
  */
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

  /* 
    Level crossing detection:
      the crossing signal is set to 1 when difference of levels between the input data level and the current level
      is not equal to 0. This either means that the input data belongs to a level above or below the current level.
  */
  always_comb begin
    // delta levels calculation
    dlvl = (hw_r_fifo_pop) ? ($signed(din_lvl) - $signed(curr_lvl)) : '0;
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

  // Sample Skip 16-bit Counter
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (~rst_ni) begin
      skip_cnt <= 16'd0;
    end else begin
      if (reg_bypass == 1'b0) begin
        if (skip_cnt_rst == 1'b1) begin
          skip_cnt <= 16'd1;
        end else if (skip_cnt_en == 1'b1) begin
          skip_cnt <= skip_cnt + 1;
        end else begin
          skip_cnt <= skip_cnt;
        end
      end
    end
  end

  /* Delta Time Output:
      -> if the skip counter has overflowed
         delta time output is set to the skip counter mask (all 1s).
      -> if delta levels have just overflowed, delta time is the sample skip counter value.
      -> if delta levels have overflowed and a stall due to them is happening, delta time is 0
         because more than one packets must be sent to represent one data, but they should appear
         one right after the other, with zero time interval.
      -> if the delta levels are valid, delta time is the sample skip counter value.
  */
  always_comb begin
    if (dlc_state == DLC_DT_OVF) begin  // skip cnt overflow or stall due to it
      dt_out = reg_dt_mask;
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
    dlvl_ovf = (reg_bypass == 1'b0) ? (hw_r_fifo_pop & (|(dlvl_abs & ~reg_dlvl_mask))) : 1'b0;

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
    ovf_dwn_cnt_n = $signed(ovf_dwn_cnt_op1) - $signed(reg_dlvl_mask);
    check_end_ovf = hw_w_fifo_full ? '0 : (ovf_dwn_cnt_n[15] == 1'b1 || ovf_dwn_cnt_n == 16'h0);
  end

  /*
    Overflow down counter and Direction register:
      -> the overflow down counter is updated to its next value if delta level just overflowed
         or if the dLC is stalling due to an delta-level overflow. Otherwise, it is set to 0.
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

  /* dlvl and dir output construction:
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
      dlvl_out = reg_dlvl_mask;
      dir_out  = dir;
    end else if (dlc_state == DLC_DLVL_OVF && check_end_ovf == 1'b0) begin
      dlvl_out = reg_dlvl_mask;
      dir_out  = dir_reg;
    end else if (dlc_state == DLC_DLVL_OVF && check_end_ovf == 1'b1) begin
      dlvl_out = ovf_dwn_cnt_op1 & reg_dlvl_mask;
      dir_out  = dir_reg;
    end else begin
      dlvl_out = (dlc_state == DLC_DT_OVF) ? '0 : ((reg_dlvl_twoscomp_n_sgnmod) ? (dlvl & reg_dlvl_mask) : (dlvl_abs & reg_dlvl_mask));
      dir_out = (dlc_state == DLC_DT_OVF) ? '0 : (dir);
    end
  end

  // ------------------------- Write Fifo Input Data Construction

  /* Output packet construction:
      -> dt_dir_out contains:
        - if the delta levels are in 2s complement
          - the direction of the delta levels in the LSB
          - delta time in the 15 MSBs
        - if the delta levels are in sign|abs_value
          - just delta time in the 16 MSBs
      -> dlc_output contains:
        - dt_dir_out shifted left by the number of bits for the delta levels
        - delta levels in the LSBs
  */
  always_comb begin
    dt_dir_out[16:0] = (reg_dlvl_twoscomp_n_sgnmod) ? {1'b0, dt_out[15:0]} : {dt_out[15:0], dir_out};
    // 4-bit left shifter
    dlc_output = (dt_dir_out << reg_dlvl_bits) | {1'b0, dlvl_out};
  end

  assign hw_w_fifo_data_in = reg_bypass ? hw_r_fifo_data_out : dlc_output[15:0];

  // ------------------------- Interrupt Generation
  always_ff @(posedge clk_i, negedge rst_ni) begin
    if (~rst_ni) begin
      dlc_xing_intr <= '0;
    end else if (reg2hw.interrupt_en.q == 1'b1) begin
      if (xing == 1'b1) begin
        dlc_xing_intr <= 1'b1;
      end else if (reg2hw.xing_intr.re == 1'b1) begin
        dlc_xing_intr <= 1'b0;
      end
    end
  end

  assign hw2reg.xing_intr.d = dlc_xing_intr;
  assign dlc_xing_intr_o = dlc_xing_intr;
endmodule

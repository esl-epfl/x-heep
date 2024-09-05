/*
 * Copyright 2022 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 */

/* DMA assume a read request is not granted before previous request rvalid is asserted */

module dma #(
    parameter int unsigned FIFO_DEPTH = 4,
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic,
    parameter type obi_req_t = logic,
    parameter type obi_resp_t = logic,
    parameter int unsigned SLOT_NUM = 0
) (
    input logic clk_i,
    input logic rst_ni,

    input logic ext_dma_stop_i,

    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    output obi_req_t  dma_read_ch0_req_o,
    input  obi_resp_t dma_read_ch0_resp_i,

    output obi_req_t  dma_write_ch0_req_o,
    input  obi_resp_t dma_write_ch0_resp_i,

    output obi_req_t  dma_addr_ch0_req_o,
    input  obi_resp_t dma_addr_ch0_resp_i,

    input logic [SLOT_NUM-1:0] trigger_slot_i,

    output dma_done_intr_o,
    output dma_window_intr_o
);

  import dma_reg_pkg::*;

  localparam int unsigned LastFifoUsage = FIFO_DEPTH - 1;
  localparam int unsigned Addr_Fifo_Depth = (FIFO_DEPTH > 1) ? $clog2(FIFO_DEPTH) : 1;

  dma_reg2hw_t                       reg2hw;
  dma_hw2reg_t                       hw2reg;

  logic        [               31:0] src_ptr_reg;
  logic        [               31:0] read_ptr_reg;
  logic        [               31:0] addr_ptr_reg;
  logic        [               31:0] read_ptr_valid_reg;
  logic        [               31:0] write_ptr_reg;
  logic        [               31:0] write_address;
  logic        [               31:0] dma_addr_cnt;
  logic        [                2:0] dma_src_cnt_du;
  logic        [                2:0] dma_dst_cnt_du;
  logic                              dma_start;
  logic                              dma_done;
  logic                              dma_window_event;

  logic                              window_done_q;

  logic        [Addr_Fifo_Depth-1:0] fifo_usage;
  logic                              fifo_alm_full;

  logic        [Addr_Fifo_Depth-1:0] fifo_addr_usage;
  logic                              fifo_addr_alm_full;

  logic                              data_in_req;
  logic                              data_in_we;
  logic        [                3:0] data_in_be;
  logic        [               31:0] data_in_addr;
  logic                              data_in_gnt;
  logic                              data_in_rvalid;
  logic        [               31:0] data_in_rdata;

  logic                              data_addr_in_req;
  logic                              data_addr_in_we;
  logic        [                3:0] data_addr_in_be;
  logic        [               31:0] data_addr_in_addr;
  logic                              data_addr_in_gnt;
  logic                              data_addr_in_rvalid;
  logic        [               31:0] data_addr_in_rdata;

  logic                              data_out_req;
  logic                              data_out_we;
  logic        [                3:0] data_out_be;
  logic        [               31:0] data_out_addr;
  logic        [               31:0] data_out_wdata;
  logic                              data_out_gnt;
  logic                              data_out_rvalid;
  logic        [               31:0] data_out_rdata;

  /* Sign extension signals */
  logic                              sign_extend;

  /* 2D signals */

  /* Dimensionality configuration */
  logic                              dma_conf_1d;  // Dimensionality configuration: 0-> 1D, 1-> 2D
  logic                              dma_conf_2d;  // Dimensionality configuration: 0-> 1D, 1-> 2D

  /* Counters */
  logic        [               16:0] dma_src_cnt_d1;  // d1 src counter
  logic        [               16:0] dma_src_cnt_d2;  // d2 src counter
  logic        [               16:0] dma_dst_cnt_d1;  // d2 dst counter

  /* Increments */
  logic        [                5:0] dma_src_d1_inc;  // d1 source increment
  logic        [               22:0] dma_src_d2_inc;  // d2 source increment
  logic        [                5:0] dma_dst_d1_inc;  // d1 destination increment
  logic        [               22:0] dma_dst_d2_inc;  // d2 destination increment

  /* Flags */
  logic                              pad_fifo_on;  // Padding flag for FIFO
  logic                              pad_cnt_on;  // Padding flag for counters
  logic                              read_ptr_update_sel;  // Select the read pointer update source

  /* Padding FSM conditions */
  logic                              idle_to_left_ex;
  logic                              idle_to_top_ex;
  logic                              idle_to_right_ex;
  logic                              idle_to_bottom_ex;
  logic                              top_ex_to_top_dn;
  logic                              top_ex_to_left_ex;
  logic                              top_dn_to_right_ex;
  logic                              top_dn_to_bottom_ex;
  logic                              top_dn_to_idle;
  logic                              left_ex_to_left_dn;
  logic                              left_dn_to_left_ex;
  logic                              left_dn_to_right_ex;
  logic                              left_dn_to_bottom_ex;
  logic                              left_dn_to_idle;
  logic                              right_ex_to_right_dn;
  logic                              right_ex_to_left_ex;
  logic                              right_dn_to_right_ex;
  logic                              right_dn_to_idle;
  logic                              right_ex_to_bottom_ex;
  logic                              bottom_ex_to_idle;

  /* Padding synchronization signals */
  logic                              data_in_rvalid_virt;
  logic                              data_in_rvalid_virt_n;
  logic                              data_in_rvalid_virt_n_n;
  logic                              data_in_gnt_virt;
  logic                              data_in_gnt_virt_n;
  logic                              data_in_gnt_virt_n_n;

  /* Interrupt Flag Register signals */
  logic                              transaction_ifr;
  logic                              dma_done_intr_n;
  logic                              dma_done_intr;
  logic                              window_ifr;
  logic                              dma_window_intr;
  logic                              dma_window_intr_n;

  /* FIFO signals */
  logic                              fifo_flush;
  logic                              fifo_full;
  logic                              fifo_empty;

  logic                              fifo_addr_flush;
  logic                              fifo_addr_full;
  logic fifo_addr_empty, fifo_addr_empty_check;

  logic wait_for_rx;
  logic wait_for_tx;

  typedef enum logic [1:0] {
    DMA_DATA_TYPE_WORD,
    DMA_DATA_TYPE_HALF_WORD,
    DMA_DATA_TYPE_BYTE,
    DMA_DATA_TYPE_BYTE_
  } dma_data_type_t;

  dma_data_type_t        dst_data_type;
  dma_data_type_t        src_data_type;

  logic           [31:0] fifo_input;
  logic           [31:0] fifo_addr_input;
  logic           [31:0] fifo_output;
  logic           [31:0] fifo_addr_output;

  logic           [ 3:0] byte_enable_out;

  logic                  circular_mode;
  logic                  address_mode;

  logic                  dma_start_pending;

  enum {
    DMA_READY,
    DMA_STARTING,
    DMA_RUNNING
  }
      dma_state_q, dma_state_d;

  /* Padding FSM states */

  enum {
    PAD_IDLE,
    TOP_PAD_EXEC,
    LEFT_PAD_EXEC,
    RIGHT_PAD_EXEC,
    BOTTOM_PAD_EXEC,
    TOP_PAD_DONE,
    LEFT_PAD_DONE,
    RIGHT_PAD_DONE,
    BOTTOM_PAD_DONE
  }
      pad_state_q, pad_state_d, pad_state_x;

  enum logic {
    DMA_READ_FSM_IDLE,
    DMA_READ_FSM_ON
  }
      dma_read_fsm_state, dma_read_fsm_n_state, dma_read_addr_fsm_state, dma_read_addr_fsm_n_state;

  enum logic {
    DMA_WRITE_FSM_IDLE,
    DMA_WRITE_FSM_ON
  }
      dma_write_fsm_state, dma_write_fsm_n_state;

  logic [Addr_Fifo_Depth-1:0] outstanding_req, outstanding_addr_req;
  logic [31:0] window_counter;

  assign dma_read_ch0_req_o.req = data_in_req && ~pad_fifo_on;
  assign dma_read_ch0_req_o.we = data_in_we;
  assign dma_read_ch0_req_o.be = data_in_be;
  assign dma_read_ch0_req_o.addr = data_in_addr;
  assign dma_read_ch0_req_o.wdata = 32'h0;

  assign data_in_gnt = dma_read_ch0_resp_i.gnt || (data_in_gnt_virt & pad_fifo_on);
  assign data_in_rvalid = dma_read_ch0_resp_i.rvalid || (data_in_rvalid_virt & pad_fifo_on);
  assign data_in_rdata = dma_read_ch0_resp_i.rdata;

  assign dma_addr_ch0_req_o.req = data_addr_in_req;
  assign dma_addr_ch0_req_o.we = data_addr_in_we;
  assign dma_addr_ch0_req_o.be = data_addr_in_be;
  assign dma_addr_ch0_req_o.addr = data_addr_in_addr;
  assign dma_addr_ch0_req_o.wdata = 32'h0;

  assign data_addr_in_gnt = dma_addr_ch0_resp_i.gnt;
  assign data_addr_in_rvalid = dma_addr_ch0_resp_i.rvalid;
  assign data_addr_in_rdata = dma_addr_ch0_resp_i.rdata;

  assign dma_write_ch0_req_o.req = data_out_req;
  assign dma_write_ch0_req_o.we = data_out_we;
  assign dma_write_ch0_req_o.be = data_out_be;
  assign dma_write_ch0_req_o.addr = data_out_addr;
  assign dma_write_ch0_req_o.wdata = data_out_wdata;

  assign data_out_gnt = dma_write_ch0_resp_i.gnt;
  assign data_out_rvalid = dma_write_ch0_resp_i.rvalid;
  assign data_out_rdata = dma_write_ch0_resp_i.rdata;

  assign dma_done_intr = transaction_ifr;
  assign dma_window_intr = window_ifr;

  assign dma_done_intr_o = dma_done_intr_n;
  assign hw2reg.transaction_ifr.d = transaction_ifr;
  assign dma_window_intr_o = dma_window_intr_n;
  assign hw2reg.window_ifr.d = window_ifr;

  assign dst_data_type = dma_data_type_t'(reg2hw.dst_data_type.q);
  assign src_data_type = dma_data_type_t'(reg2hw.src_data_type.q);

  assign hw2reg.status.ready.d = (dma_state_q == DMA_READY);

  assign hw2reg.status.window_done.d = window_done_q;

  assign circular_mode = reg2hw.mode.q == 1;
  assign address_mode = reg2hw.mode.q == 2;

  /* DMA Dimensionality configuration flags */
  assign dma_conf_1d = reg2hw.dim_config.q == 0;
  assign dma_conf_2d = reg2hw.dim_config.q == 1;

  /* DMA read pointer source selection */
  assign read_ptr_update_sel = reg2hw.dim_inv.q;

  /* DMA 2D increment */
  assign dma_src_d2_inc = reg2hw.src_ptr_inc_d2.q;
  assign dma_src_d1_inc = reg2hw.src_ptr_inc_d1.q;
  assign dma_dst_d2_inc = reg2hw.dst_ptr_inc_d2.q;
  assign dma_dst_d1_inc = reg2hw.dst_ptr_inc_d1.q;

  /* Sign extend flag */

  assign sign_extend = reg2hw.sign_ext.q & ( (src_data_type[1] & ~dst_data_type[1]) | ((src_data_type[1] == dst_data_type[1]) & (src_data_type[0] & ~dst_data_type[0])));

  /* Padding FSM conditions assignments */

  assign idle_to_top_ex = {|reg2hw.pad_top.q == 1'b1 && dma_start == 1'b1};
  assign idle_to_left_ex = {
    |reg2hw.pad_top.q == 1'b0 && |reg2hw.pad_left.q == 1'b1 && dma_start == 1'b1
  };
  assign idle_to_right_ex = {
    |reg2hw.pad_top.q == 1'b0 && |reg2hw.pad_left.q == 1'b0 && |reg2hw.pad_right.q == 1'b1 
                      && dma_src_cnt_d1 == ({11'h0, reg2hw.pad_right.q} + {14'h0, dma_dst_cnt_du})
  };
  assign idle_to_bottom_ex = {
    |reg2hw.pad_top.q == 1'b0 && |reg2hw.pad_left.q == 1'b0 && |reg2hw.pad_right.q == 1'b0 && |reg2hw.pad_bottom.q == 1'b1 
                      && dma_src_cnt_d2 == ({11'h0, reg2hw.pad_bottom.q} + {14'h0, dma_dst_cnt_du}) && dma_src_cnt_d1 == ({14'h0, dma_dst_cnt_du})
  };
  assign top_ex_to_top_dn = {
    dma_src_cnt_d2 == ({1'h0, reg2hw.size_d2.q} + {11'h0, reg2hw.pad_bottom.q} + {14'h0, dma_dst_cnt_du}) && dma_src_cnt_d1 == ({14'h0, dma_dst_cnt_du}) && |reg2hw.pad_left.q == 1'b0
  };
  assign top_ex_to_left_ex = {
    dma_src_cnt_d2 == ({1'h0, reg2hw.size_d2.q} + {11'h0, reg2hw.pad_bottom.q} + {14'h0, dma_dst_cnt_du}) && dma_src_cnt_d1 == ({14'h0, dma_dst_cnt_du}) && |reg2hw.pad_left.q == 1'b1
  };
  assign top_dn_to_right_ex = {
    |reg2hw.pad_left.q == 1'b0 && |reg2hw.pad_right.q == 1'b1 && dma_src_cnt_d1 == ({11'h0, reg2hw.pad_right.q} + {14'h0, dma_dst_cnt_du})
  };
  assign top_dn_to_bottom_ex = {
    |reg2hw.pad_left.q == 1'b0 && |reg2hw.pad_right.q == 1'b0 && |reg2hw.pad_bottom.q == 1'b1 && dma_src_cnt_d2 == ({11'h0, reg2hw.pad_bottom.q} + {14'h0, dma_dst_cnt_du}) && dma_src_cnt_d1 == ({14'h0, dma_dst_cnt_du})
  };
  assign top_dn_to_idle = {
    |reg2hw.pad_left.q == 1'b0 && |reg2hw.pad_right.q == 1'b0 && |reg2hw.pad_bottom.q == 1'b0 && |dma_src_cnt_d2 == 1'b0
  };
  assign left_ex_to_left_dn = {
    dma_src_cnt_d1 == ({1'h0, reg2hw.size_d1.q} + {11'h0, reg2hw.pad_right.q} + {14'h0, dma_dst_cnt_du})
  };
  assign left_dn_to_left_ex = {
    dma_src_cnt_d1 == ({14'h0, dma_dst_cnt_du}) && dma_src_cnt_d2 != ({14'h0, dma_dst_cnt_du} + {11'h0, reg2hw.pad_bottom.q}) && |reg2hw.pad_right.q == 1'b0
  };
  assign left_dn_to_right_ex = {
    |reg2hw.pad_right.q == 1'b1 && dma_src_cnt_d1 == ({11'h0, reg2hw.pad_right.q} + {14'h0, dma_dst_cnt_du})
  };
  assign left_dn_to_bottom_ex = {
    |reg2hw.pad_right.q == 1'b0 && |reg2hw.pad_bottom.q == 1'b1 && dma_src_cnt_d2 == ({11'h0, reg2hw.pad_bottom.q} + {14'h0, dma_dst_cnt_du}) && dma_src_cnt_d1 == ({14'h0, dma_dst_cnt_du})
  };
  assign left_dn_to_idle = {
    |reg2hw.pad_right.q == 1'b0 && |reg2hw.pad_bottom.q == 1'b0 && |dma_src_cnt_d2 == 1'b0
  };
  assign right_ex_to_right_dn = {
    dma_src_cnt_d1 == ({14'h0, dma_dst_cnt_du}) && dma_src_cnt_d2 != ({11'h0, reg2hw.pad_bottom.q} + {14'h0, dma_dst_cnt_du}) && |reg2hw.pad_left.q == 1'b0
  };
  assign right_ex_to_left_ex = {
    dma_src_cnt_d1 == ({14'h0, dma_dst_cnt_du}) && dma_src_cnt_d2 != ({11'h0, reg2hw.pad_bottom.q} + {14'h0, dma_dst_cnt_du}) && |reg2hw.pad_left.q == 1'b1
  };
  assign right_ex_to_bottom_ex = {
    |reg2hw.pad_bottom.q == 1'b1 && dma_src_cnt_d2 == ({11'h0, reg2hw.pad_bottom.q} + {14'h0, dma_dst_cnt_du}) && dma_src_cnt_d1 == ({14'h0, dma_dst_cnt_du})
  };
  assign right_dn_to_right_ex = {
    dma_src_cnt_d1 == ({11'h0, reg2hw.pad_right.q} + {14'h0, dma_dst_cnt_du}) && |reg2hw.pad_left.q == 1'b0
  };
  assign right_dn_to_idle = {|reg2hw.pad_bottom.q == 1'b0 && |dma_src_cnt_d2 == 1'b0};
  assign bottom_ex_to_idle = {
    dma_src_cnt_d1 == {14'h0, dma_dst_cnt_du} && dma_src_cnt_d2 == {14'h0, dma_dst_cnt_du}
  };

  assign write_address = address_mode ? fifo_addr_output : write_ptr_reg;

  assign wait_for_rx = |(reg2hw.slot.rx_trigger_slot.q[SLOT_NUM-1:0] & (~trigger_slot_i));
  assign wait_for_tx = |(reg2hw.slot.tx_trigger_slot.q[SLOT_NUM-1:0] & (~trigger_slot_i));

  assign fifo_addr_empty_check = fifo_addr_empty && address_mode;

  assign fifo_alm_full = (fifo_usage == LastFifoUsage[Addr_Fifo_Depth-1:0]);
  assign fifo_addr_alm_full = (fifo_addr_usage == LastFifoUsage[Addr_Fifo_Depth-1:0]);

  assign dma_start = (dma_state_q == DMA_STARTING);

  //
  // Main DMA state machine
  //
  // READY   : idle, waiting for a write pulse to size registered in `dma_start_pending`
  // STARTING: load transaction data
  // RUNNING : waiting for transaction finish
  //           when `dma_done` rises either enter ready or restart in circular mode
  //

  always_comb begin
    dma_state_d = dma_state_q;
    case (dma_state_q)
      DMA_READY: begin
        if (dma_start_pending) begin
          dma_state_d = DMA_STARTING;
        end
      end
      DMA_STARTING: begin
        dma_state_d = DMA_RUNNING;
      end
      DMA_RUNNING: begin
        if (dma_done) begin
          if (circular_mode) dma_state_d = DMA_STARTING;
          else dma_state_d = DMA_READY;
        end
      end
    endcase
  end

  /* Update DMA state */
  always_ff @(posedge clk_i, negedge rst_ni) begin
    if (~rst_ni) begin
      dma_state_q <= DMA_READY;
    end else begin
      dma_state_q <= dma_state_d;
    end
  end

  /* DMA pulse start when dma_start register is written */
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_dma_start
    if (~rst_ni) begin
      dma_start_pending <= 1'b0;
    end else begin
      if (dma_start == 1'b1) begin
        dma_start_pending <= 1'b0;
      end else if ((reg2hw.size_d1.qe & |reg2hw.size_d1.q)) begin
        dma_start_pending <= 1'b1;
      end
    end
  end

  /*/ Store input data pointer and increment everytime read request is granted */
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_ptr_in_reg
    if (~rst_ni) begin
      read_ptr_reg <= '0;
    end else begin
      if (dma_start == 1'b1) begin
        read_ptr_reg <= reg2hw.src_ptr.q;
      end else if (data_in_gnt == 1'b1) begin
        if (dma_conf_1d == 1'b1) begin
          /* Increase the pointer by the amount written in ptr_inc */
          read_ptr_reg <= read_ptr_reg + {26'h0, dma_src_d1_inc};
        end else if (dma_conf_2d == 1'b1 && pad_cnt_on == 1'b0) begin
          if (read_ptr_update_sel == 1'b0) begin
            if (dma_src_cnt_d1 == {14'h0, dma_src_cnt_du} && |dma_src_cnt_d2 == 1'b1) begin
              /* In this case, the d1 is almost finished, so we need to increment the pointer by sizeof(d1)*data_unit */
              read_ptr_reg <= read_ptr_reg + {9'h0, dma_src_d2_inc};
            end else begin
              read_ptr_reg <= read_ptr_reg + {26'h0, dma_src_d1_inc}; /* Increment of the d1 increment (stride) */
            end
          end else begin
            if (dma_src_cnt_d1 == {14'h0, dma_src_cnt_du} && |dma_src_cnt_d2 == 1'b1) begin
              /* In this case, the d1 is almost finished, so we need to increment the pointer by sizeof(d2)*data_unit */
              read_ptr_reg <= src_ptr_reg;
            end else begin
              read_ptr_reg <= read_ptr_reg + {9'h0, dma_src_d2_inc}; /* Increment of the d1 increment (stride) */
            end
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
      src_ptr_reg <= '0;
    end else begin
      if (dma_start == 1'b1) begin
        src_ptr_reg <= reg2hw.src_ptr.q + {26'h0, dma_src_d1_inc};
      end else if (data_in_gnt == 1'b1 && dma_conf_2d == 1'b1 && pad_cnt_on == 1'b0 && read_ptr_update_sel == 1'b1 &&
                    (dma_src_cnt_d1 == {14'h0, dma_src_cnt_du} && |dma_src_cnt_d2 == 1'b1)) begin
        src_ptr_reg <= src_ptr_reg + {26'h0, dma_src_d1_inc};
      end
    end
  end

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

  // Store output data pointer and increment everytime write request is granted
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_ptr_out_reg
    if (~rst_ni) begin
      write_ptr_reg <= '0;
    end else begin
      if (dma_start == 1'b1) begin
        write_ptr_reg <= reg2hw.dst_ptr.q;
      end else if (data_out_gnt == 1'b1) begin
        if (dma_conf_1d == 1'b1) begin
          write_ptr_reg <= write_ptr_reg + {26'h0, dma_dst_d1_inc};
        end else if (dma_conf_2d == 1'b1) begin
          if (dma_dst_cnt_d1 == {14'h0, dma_dst_cnt_du}) begin
            // In this case, the d1 is finished, so we need to increment the pointer by sizeof(d1)*data_unit*strides
            write_ptr_reg <= write_ptr_reg + {9'h0, dma_dst_d2_inc};
          end else begin
            write_ptr_reg <= write_ptr_reg + {26'h0, dma_dst_d1_inc}; // Increment just of one du, since we need to increase the 1d
          end
        end
      end
    end
  end

  // Store dma transfer size and decrement it everytime input data rvalid is asserted.
  // Perform additional checks for 2D DMA
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_dma_src_cnt_reg
    if (~rst_ni) begin
      dma_src_cnt_d1 <= '0;
      dma_src_cnt_d2 <= '0;
    end else begin
      if (dma_start == 1'b1) begin
        dma_src_cnt_d1 <= {1'h0, reg2hw.size_d1.q} + {11'h0, reg2hw.pad_left.q} + {11'h0, reg2hw.pad_right.q};
        dma_src_cnt_d2 <= {1'h0, reg2hw.size_d2.q} + {11'h0, reg2hw.pad_top.q} + {11'h0, reg2hw.pad_bottom.q};
      end else if (data_in_gnt == 1'b1) begin
        if (dma_conf_1d == 1'b1) begin
          // 1D case
          dma_src_cnt_d1 <= dma_src_cnt_d1 - {14'h0, dma_src_cnt_du};
        end else if (dma_conf_2d == 1'b1) begin
          // 2D case
          if (dma_src_cnt_d1 == {14'h0, dma_src_cnt_du}) begin
            // In this case, the d1 is finished, so we need to decrement the d2 size and reset the d2 size
            dma_src_cnt_d2 <= dma_src_cnt_d2 - {14'h0, dma_src_cnt_du};
            dma_src_cnt_d1 <= {1'h0, reg2hw.size_d1.q} + {11'h0, reg2hw.pad_left.q} + {11'h0, reg2hw.pad_right.q};
          end else begin
            // In this case, the d1 isn't finished, so we need to decrement the d1 size
            dma_src_cnt_d1 <= dma_src_cnt_d1 - {14'h0, dma_src_cnt_du};
          end
        end
      end
    end
  end

  // Store dma transfer size and decrement it everytime input data write request is granted.
  // The need for two separate counters for reading and writing operations is due to the lack of synchronization between them.
  // Since the check on the read side is done on the rvalid signal, we need only an additional counter, for d1.
  // Performs additional checks for 2D DMA.

  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_dma_dst_cnt_reg
    if (~rst_ni) begin
      dma_dst_cnt_d1 <= '0;
    end else begin
      if (dma_start == 1'b1) begin
        dma_dst_cnt_d1 <= {1'h0, reg2hw.size_d1.q} + {11'h0, reg2hw.pad_left.q} + {11'h0, reg2hw.pad_right.q};
      end else if (data_out_gnt == 1'b1) begin
        if (dma_conf_1d == 1'b1) begin
          // 1D case
          dma_dst_cnt_d1 <= dma_dst_cnt_d1 - {14'h0, dma_dst_cnt_du};
        end else if (dma_conf_2d == 1'b1) begin
          // 2D case
          if (dma_dst_cnt_d1 == {14'h0, dma_dst_cnt_du}) begin
            // In this case, the d1 is finished, so we need to reset the d2 size
            dma_dst_cnt_d1 <= {1'h0, reg2hw.size_d1.q} + {11'h0, reg2hw.pad_left.q} + {11'h0, reg2hw.pad_right.q};
          end else begin
            // In this case, the d1 isn't finished, so we need to decrement the d1 size
            dma_dst_cnt_d1 <= dma_dst_cnt_d1 - {14'h0, dma_dst_cnt_du};
          end
        end
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
        dma_addr_cnt <= dma_addr_cnt - 32'h4;  //address always 32b
      end
    end
  end

  always_comb begin
    case (dst_data_type)
      DMA_DATA_TYPE_WORD: dma_dst_cnt_du = 3'h4;
      DMA_DATA_TYPE_HALF_WORD: dma_dst_cnt_du = 3'h2;
      DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_BYTE_: dma_dst_cnt_du = 3'h1;
    endcase
  end

  always_comb begin
    case (src_data_type)
      DMA_DATA_TYPE_WORD: dma_src_cnt_du = 3'h4;
      DMA_DATA_TYPE_HALF_WORD: dma_src_cnt_du = 3'h2;
      DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_BYTE_: dma_src_cnt_du = 3'h1;
    endcase
  end

  always_comb begin : proc_byte_enable_out
    case (dst_data_type)  // Data type 00 Word, 01 Half word, 11,10 byte
      DMA_DATA_TYPE_WORD: byte_enable_out = 4'b1111;  // Writing a word (32 bits)

      DMA_DATA_TYPE_HALF_WORD: begin  // Writing a half-word (16 bits)
        case (write_address[1])
          1'b0: byte_enable_out = 4'b0011;
          1'b1: byte_enable_out = 4'b1100;
        endcase
        ;  // case(write_address[1:0])
      end

      DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_BYTE_: begin  // Writing a byte (8 bits)
        case (write_address[1:0])
          2'b00: byte_enable_out = 4'b0001;
          2'b01: byte_enable_out = 4'b0010;
          2'b10: byte_enable_out = 4'b0100;
          2'b11: byte_enable_out = 4'b1000;
        endcase
        ;  // case(write_address[1:0])
      end
    endcase
    ;  // case (dst_data_type)
  end

  // Output data shift
  always_comb begin : proc_output_data

    data_out_wdata[7:0]   = fifo_output[7:0];
    data_out_wdata[15:8]  = fifo_output[15:8];
    data_out_wdata[23:16] = fifo_output[23:16];
    data_out_wdata[31:24] = fifo_output[31:24];

    case (write_address[1:0])
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
      2'b01: data_out_wdata[15:8] = fifo_output[7:0];  // Writing a byte, no need for sign extension
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


  assign fifo_addr_input = data_addr_in_rdata;  //never misaligned, always 32b

  // Input data shift: shift the input data to be on the LSB of the fifo
  always_comb begin : proc_input_data

    if (pad_fifo_on) begin
      fifo_input = 32'h0;
    end else begin
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
  end

  // FSM state update
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_fsm_state
    if (~rst_ni) begin
      dma_read_fsm_state <= DMA_READ_FSM_IDLE;
      dma_write_fsm_state <= DMA_WRITE_FSM_IDLE;
      dma_read_addr_fsm_state <= DMA_READ_FSM_IDLE;
      outstanding_req <= '0;
      outstanding_addr_req <= '0;
    end else begin
      dma_read_fsm_state <= dma_read_fsm_n_state;
      dma_write_fsm_state <= dma_write_fsm_n_state;
      dma_read_addr_fsm_state <= dma_read_addr_fsm_n_state;

      outstanding_req <= outstanding_req + (data_in_req && data_in_gnt) - data_in_rvalid;

      if (address_mode)
        outstanding_addr_req <= outstanding_addr_req + (data_addr_in_req && data_addr_in_gnt) - data_addr_in_rvalid;

    end
  end

  /* Padding synchronization signal generation 
   * When the pad_fifo_on is asserted, this logic mimics the behaviour of the data_in_rvalid and data_in_gnt signals
   * coming from the memory. This is done in order to keep the read/write operations working even without an
   * actual response from memory, reducing power consumptionnby avoiding unnecessary memory accesses.
   */
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_pad_sync_signal
    if (~rst_ni) begin
      data_in_rvalid_virt <= 1'b0;
      data_in_rvalid_virt_n <= 1'b1;
      data_in_rvalid_virt_n_n <= 1'b0;
      data_in_gnt_virt <= 1'b1;
      data_in_gnt_virt_n <= 1'b0;
      data_in_gnt_virt_n_n <= 1'b0;
    end else begin
      if (data_in_req == 1'b1 && pad_fifo_on == 1'b1) begin
        data_in_rvalid_virt <= data_in_rvalid_virt_n;
        data_in_rvalid_virt_n <= data_in_rvalid_virt_n_n;
        data_in_rvalid_virt_n_n <= data_in_rvalid;
        data_in_gnt_virt <= data_in_gnt_virt_n;
        data_in_gnt_virt_n <= data_in_gnt_virt_n_n;
        data_in_gnt_virt_n_n <= data_in_gnt;
      end else begin
        data_in_rvalid_virt <= 1'b0;
        data_in_rvalid_virt_n <= 1'b1;
        data_in_rvalid_virt_n_n <= 1'b0;
        data_in_gnt_virt <= 1'b1;
        data_in_gnt_virt_n <= 1'b0;
        data_in_gnt_virt_n_n <= 1'b0;
      end
    end
  end

  // Padding FSM state update
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_pad_state
    if (~rst_ni) begin
      pad_state_q <= PAD_IDLE;
      pad_state_x <= PAD_IDLE;
    end else if (dma_conf_2d == 1'b1) begin
      if (dma_start == 1'b1 && |reg2hw.pad_top.q == 1'b1) begin
        pad_state_q <= TOP_PAD_EXEC;
        pad_state_x <= TOP_PAD_EXEC;
      end else if (dma_start == 1'b1 && |reg2hw.pad_left.q == 1'b1) begin
        pad_state_q <= LEFT_PAD_EXEC;
        pad_state_x <= LEFT_PAD_EXEC;
      end else begin
        pad_state_x <= pad_state_d;
        if (data_in_rvalid == 1'b1) begin
          pad_state_q <= pad_state_x;
        end
      end
    end
  end

  // Pad fifo flag logic
  always_comb begin : proc_pad_fifo_on
    if (dma_conf_2d == 1'b1) begin
      case (pad_state_q)
        TOP_PAD_EXEC, LEFT_PAD_EXEC, RIGHT_PAD_EXEC, BOTTOM_PAD_EXEC: pad_fifo_on = 1'b1;

        default: pad_fifo_on = 1'b0;
      endcase
    end else begin
      pad_fifo_on = 1'b0;
    end
  end

  // Pad counter flag logic
  always_comb begin : proc_pad_cnt_on
    case (pad_state_q)
      PAD_IDLE: begin
        if (idle_to_right_ex || idle_to_bottom_ex || idle_to_left_ex || idle_to_top_ex) begin
          pad_cnt_on = 1'b1;
        end else begin
          pad_cnt_on = pad_fifo_on;
        end
      end

      TOP_PAD_DONE: begin
        if (top_dn_to_right_ex) begin
          pad_cnt_on = 1'b1;
        end else begin
          pad_cnt_on = pad_fifo_on;
        end
      end

      LEFT_PAD_DONE: begin
        if (left_dn_to_right_ex) begin
          pad_cnt_on = 1'b1;
        end else begin
          pad_cnt_on = pad_fifo_on;
        end
      end

      RIGHT_PAD_DONE: begin
        if (right_dn_to_right_ex) begin
          pad_cnt_on = 1'b1;
        end else begin
          pad_cnt_on = pad_fifo_on;
        end
      end

      RIGHT_PAD_EXEC: begin
        if (right_ex_to_right_dn || right_ex_to_left_ex) begin
          pad_cnt_on = 1'b0;
        end else begin
          pad_cnt_on = pad_fifo_on;
        end
      end

      default: pad_cnt_on = pad_fifo_on;
    endcase
  end

  // Padding FSM logic
  always_comb begin : proc_pad_fsm_logic
    if (dma_conf_1d == 1'b1) begin
      pad_state_d = PAD_IDLE;
    end else begin
      if (dma_start == 1'b1 && |reg2hw.pad_top.q == 1'b1) begin
        pad_state_d = TOP_PAD_EXEC;
      end else if (dma_start == 1'b1 && |reg2hw.pad_left.q == 1'b1) begin
        pad_state_d = LEFT_PAD_EXEC;
      end else begin
        pad_state_d = pad_state_x;
      end
    end

    unique case (pad_state_x)
      PAD_IDLE: begin
        if (idle_to_top_ex) begin
          pad_state_d = TOP_PAD_EXEC;
        end else if (idle_to_left_ex) begin
          pad_state_d = LEFT_PAD_EXEC;
        end else if (idle_to_right_ex) begin
          pad_state_d = RIGHT_PAD_EXEC;
        end else if (idle_to_bottom_ex) begin
          pad_state_d = BOTTOM_PAD_EXEC;
        end
      end

      TOP_PAD_EXEC: begin
        if (top_ex_to_left_ex) begin
          pad_state_d = LEFT_PAD_EXEC;
        end else if (top_ex_to_top_dn) begin
          pad_state_d = TOP_PAD_DONE;
        end
      end
      TOP_PAD_DONE: begin
        if (top_dn_to_right_ex) begin
          pad_state_d = RIGHT_PAD_EXEC;
        end else if (top_dn_to_bottom_ex) begin
          pad_state_d = BOTTOM_PAD_EXEC;
        end else if (top_dn_to_idle) begin
          pad_state_d = PAD_IDLE;
        end
      end
      LEFT_PAD_EXEC: begin
        if (left_ex_to_left_dn) begin
          pad_state_d = LEFT_PAD_DONE;
        end
      end
      LEFT_PAD_DONE: begin
        if (left_dn_to_right_ex) begin
          pad_state_d = RIGHT_PAD_EXEC;
        end else if (left_dn_to_bottom_ex) begin
          pad_state_d = BOTTOM_PAD_EXEC;
        end else if (left_dn_to_left_ex) begin
          pad_state_d = LEFT_PAD_EXEC;
        end else if (left_dn_to_idle) begin
          pad_state_d = PAD_IDLE;
        end
      end
      RIGHT_PAD_EXEC: begin
        if (right_ex_to_right_dn) begin
          pad_state_d = RIGHT_PAD_DONE;
        end else if (right_ex_to_left_ex) begin
          pad_state_d = LEFT_PAD_EXEC;
        end else if (right_ex_to_bottom_ex) begin
          pad_state_d = BOTTOM_PAD_EXEC;
        end
      end
      RIGHT_PAD_DONE: begin
        if (right_dn_to_idle) begin
          pad_state_d = PAD_IDLE;
        end else if (right_dn_to_right_ex) begin
          pad_state_d = RIGHT_PAD_EXEC;
        end
      end
      BOTTOM_PAD_EXEC: begin
        if (bottom_ex_to_idle) begin
          pad_state_d = PAD_IDLE;
        end
      end
    endcase
  end

  /* Transaction IFR update */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_transaction_ifr
    if (~rst_ni) begin
      transaction_ifr <= '0;
    end else if (reg2hw.interrupt_en.transaction_done.q == 1'b1) begin
      // Enter here only if the transaction_done interrupt is enabled
      if (dma_done == 1'b1) begin
        transaction_ifr <= 1'b1;
      end else if (reg2hw.transaction_ifr.re == 1'b1) begin
        // If the IFR bit is read, we must clear the transaction_ifr
        transaction_ifr <= 1'b0;
      end
    end
  end

  /* Delayed transaction interrupt signals */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_intr
    if (~rst_ni) begin
      dma_done_intr_n <= '0;
    end else begin
      dma_done_intr_n <= dma_done_intr;
    end
  end

  /* Window IFR update */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_window_ifr
    if (~rst_ni) begin
      window_ifr <= '0;
    end else if (reg2hw.interrupt_en.window_done.q == 1'b1) begin
      // Enter here only if the window_done interrupt is enabled
      if (dma_window_event == 1'b1) begin
        window_ifr <= 1'b1;
      end else if (reg2hw.window_ifr.re == 1'b1) begin
        // If the IFR bit is read, we must clear the window_ifr
        window_ifr <= 1'b0;
      end
    end
  end

  /* Delayed window interrupt signals */
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_ff_window_intr
    if (~rst_ni) begin
      dma_window_intr_n <= '0;
    end else begin
      dma_window_intr_n <= dma_window_intr;
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
          fifo_flush = 1'b1;
        end else begin
          dma_read_fsm_n_state = DMA_READ_FSM_IDLE;
        end
      end
      // Read one word
      DMA_READ_FSM_ON: begin
        // If all input data read exit
        if (ext_dma_stop_i == 1'b0) begin
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
            if (dma_src_cnt_d1 == {1'h0, reg2hw.size_d1.q} + {11'h0, reg2hw.pad_left.q} + {11'h0, reg2hw.pad_right.q} && |dma_src_cnt_d2 == 1'b0) begin
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

  // Read address master FSM
  always_comb begin : proc_dma_addr_read_fsm_logic

    dma_read_addr_fsm_n_state = DMA_READ_FSM_IDLE;

    data_addr_in_req = '0;
    data_addr_in_we = '0;
    data_addr_in_be = '0;
    data_addr_in_addr = '0;

    fifo_addr_flush = 1'b0;

    unique case (dma_read_addr_fsm_state)

      DMA_READ_FSM_IDLE: begin
        // Wait for start signal
        if (dma_start == 1'b1 && address_mode) begin
          dma_read_addr_fsm_n_state = DMA_READ_FSM_ON;
          fifo_addr_flush = 1'b1;
        end else begin
          dma_read_addr_fsm_n_state = DMA_READ_FSM_IDLE;
        end
      end
      // Read one word
      DMA_READ_FSM_ON: begin
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
      end
    endcase
  end

  // Write master FSM
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
        if (fifo_empty == 1'b1 && dma_read_fsm_state == DMA_READ_FSM_IDLE) begin
          dma_done = outstanding_req == '0 && outstanding_addr_req == '0;
          // If all input data has been read (dma_read_fsm_state == DMA_READ_FSM_IDLE, set when all data has been read) 
          // and all requests have been granted, (outstanding_req == 0) then we are done
          dma_write_fsm_n_state = dma_done ? DMA_WRITE_FSM_IDLE : DMA_WRITE_FSM_ON;
        end else begin
          dma_write_fsm_n_state = DMA_WRITE_FSM_ON;
          // Wait if fifo is empty or if the SPI TX is not ready for new data (only in SPI mode 2).
          if (fifo_empty == 1'b0 && wait_for_tx == 1'b0 && fifo_addr_empty_check == 1'b0) begin
            data_out_req  = 1'b1;
            data_out_we   = 1'b1;
            data_out_be   = byte_enable_out;
            data_out_addr = write_address;
          end
        end
      end
    endcase
  end

  fifo_v3 #(
      .DEPTH(FIFO_DEPTH)
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
      .push_i(data_in_rvalid),
      // as long as the queue is not empty we can pop new elements
      .data_o(fifo_output),
      .pop_i(data_out_gnt)
  );

  fifo_v3 #(
      .DEPTH(FIFO_DEPTH)
  ) dma_addr_fifo_i (
      .clk_i,
      .rst_ni,
      .flush_i(fifo_addr_flush),
      .testmode_i(1'b0),
      // status flags
      .full_o(fifo_addr_full),
      .empty_o(fifo_addr_empty),
      .usage_o(fifo_addr_usage),
      // as long as the queue is not full we can push new data
      .data_i(fifo_addr_input),
      .push_i(data_addr_in_rvalid),
      // as long as the queue is not empty we can pop new elements
      .data_o(fifo_addr_output),
      .pop_i(data_out_gnt && address_mode)
  );

  dma_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) dma_reg_top_i (
      .clk_i,
      .rst_ni,
      .reg_req_i,
      .reg_rsp_o,
      .reg2hw,
      .hw2reg,
      .devmode_i(1'b1)
  );

  // WINDOW EVENT
  // Count gnt write transaction and generate event pulse if WINDOW_SIZE is reached
  assign dma_window_event = |reg2hw.window_size.q &  data_out_gnt & (window_counter + 'h1 >= {19'h0, reg2hw.window_size.q});

  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_dma_window_cnt
    if (~rst_ni) begin
      window_counter <= 'h0;
    end else begin
      if (|reg2hw.window_size.q) begin
        if (dma_start | dma_done) begin
          window_counter <= 'h0;
        end else if (data_out_gnt) begin
          if (window_counter + 'h1 >= {19'h0, reg2hw.window_size.q}) begin
            window_counter <= 'h0;
          end else begin
            window_counter <= window_counter + 'h1;
          end
        end
      end
    end
  end

  // Update WINDOW_COUNT register
  always_comb begin : proc_dma_window_cnt_reg
    hw2reg.window_count.d  = reg2hw.window_count.q + 'h1;
    hw2reg.window_count.de = 1'b0;
    if (dma_start) begin
      hw2reg.window_count.d  = 'h0;
      hw2reg.window_count.de = 1'b1;
    end else if (dma_window_event) begin
      hw2reg.window_count.de = 1'b1;
    end
  end

  // update window_done flag
  // set on dma_window_event
  // reset on read
  always_ff @(posedge clk_i, negedge rst_ni) begin : proc_dma_window_done
    if (~rst_ni) begin
      window_done_q <= 1'b0;
    end else begin
      if (dma_window_event) window_done_q <= 1'b1;
      else if (reg2hw.status.window_done.re) window_done_q <= 1'b0;
    end
  end


endmodule : dma

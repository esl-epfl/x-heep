/*
 * Copyright 2024 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *  
 * Info: Direct Memory Access (DMA) channel module.
 */

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
    input logic clk_gate_en_ni,

    input logic ext_dma_stop_i,

    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    output obi_req_t  dma_read_req_o,
    input  obi_resp_t dma_read_resp_i,

    output obi_req_t  dma_write_req_o,
    input  obi_resp_t dma_write_resp_i,

    output obi_req_t  dma_addr_req_o,
    input  obi_resp_t dma_addr_resp_i,

    input logic [SLOT_NUM-1:0] trigger_slot_i,

    output dma_done_intr_o,
    output dma_window_intr_o,

    output dma_done_o
);

  import dma_reg_pkg::*;


  /*_________________________________________________________________________________________________________________________________ */

  /* Parameter definition */

  localparam int unsigned LastFifoUsage = FIFO_DEPTH - 1;
  localparam int unsigned Addr_Fifo_Depth = (FIFO_DEPTH > 1) ? $clog2(FIFO_DEPTH) : 1;

  /*_________________________________________________________________________________________________________________________________ */

  /* Signals declaration */

  /* Gated clock */
  logic clk_cg;

  /* Registers */
  dma_reg2hw_t reg2hw;
  dma_hw2reg_t hw2reg;

  /* General signals */
  logic dma_padding_fsm_on;
  logic padding_fsm_done;

  logic dma_start;
  logic dma_done;
  logic dma_window_event;

  logic window_done_q;

  logic data_in_req;
  logic data_in_we;
  logic [3:0] data_in_be;
  logic [31:0] data_in_addr;
  logic data_in_gnt;
  logic data_in_rvalid;
  logic [31:0] data_in_rdata;

  logic data_addr_in_req;
  logic data_addr_in_we;
  logic [3:0] data_addr_in_be;
  logic [31:0] data_addr_in_addr;
  logic data_addr_in_gnt;
  logic data_addr_in_rvalid;
  logic [31:0] data_addr_in_rdata;

  logic data_out_req;
  logic data_out_we;
  logic [3:0] data_out_be;
  logic [31:0] data_out_addr;
  logic [31:0] data_out_wdata;
  logic data_out_gnt;
  logic data_out_rvalid;
  logic [31:0] data_out_rdata;

  /* Interrupt Flag Register signals */
  logic transaction_ifr;
  logic dma_done_intr_n;
  logic dma_done_intr;
  logic window_ifr;
  logic dma_window_intr;
  logic dma_window_intr_n;

  /* FIFO signals */
  logic [Addr_Fifo_Depth-1:0] read_fifo_usage;
  logic [Addr_Fifo_Depth-1:0] read_addr_fifo_usage;
  logic [Addr_Fifo_Depth-1:0] write_fifo_usage;

  logic fifo_flush;
  logic read_fifo_full;
  logic read_fifo_empty;
  logic read_fifo_alm_full;
  logic read_fifo_pop;
  logic [31:0] read_fifo_input;
  logic [31:0] read_fifo_output;

  logic read_addr_fifo_full;
  logic read_addr_fifo_empty;
  logic read_addr_fifo_alm_full;
  logic [31:0] read_addr_fifo_output;

  logic write_fifo_full;
  logic write_fifo_empty;
  logic write_fifo_alm_full;
  logic write_fifo_push;
  logic write_fifo_pop;
  logic [31:0] write_fifo_input;
  logic [31:0] write_fifo_output;

  /* Trigger signals */
  logic wait_for_rx;
  logic wait_for_tx;

  /* Datatypes */
  typedef enum logic [1:0] {
    DMA_DATA_TYPE_WORD,
    DMA_DATA_TYPE_HALF_WORD,
    DMA_DATA_TYPE_BYTE,
    DMA_DATA_TYPE_BYTE_
  } dma_data_type_t;


  /* FSM states */
  enum {
    DMA_READY,
    DMA_STARTING,
    DMA_RUNNING
  }
      dma_state_q, dma_state_d;

  logic circular_mode;
  logic address_mode;

  logic dma_start_pending;

  logic [31:0] window_counter;

  /*_________________________________________________________________________________________________________________________________ */

  /* Module instantiation */

  /* Clock gating cell */

`ifndef FPGA_SYNTHESIS
`ifndef VERILATOR
  tc_clk_gating clk_gating_cell (
      .clk_i,
      .en_i(clk_gate_en_ni),
      .test_en_i(1'b0),
      .clk_o(clk_cg)
  );

`else
  assign clk_cg = clk_i & clk_gate_en_ni;
`endif

`else
  assign clk_cg = clk_i & clk_gate_en_ni;
`endif



  /* Read FIFO */
  fifo_v3 #(
      .DEPTH(FIFO_DEPTH),
      .FALL_THROUGH(1'b1)
  ) dma_read_fifo_i (
      .clk_i(clk_cg),
      .rst_ni,
      .flush_i(fifo_flush),
      .testmode_i(1'b0),
      // status flags
      .full_o(read_fifo_full),
      .empty_o(read_fifo_empty),
      .usage_o(read_fifo_usage),
      // as long as the queue is not full we can push new data
      .data_i(read_fifo_input),
      .push_i(data_in_rvalid),
      // as long as the queue is not empty we can pop new elements
      .data_o(read_fifo_output),
      .pop_i(read_fifo_pop)
  );

  /* Read address mode FIFO */
  fifo_v3 #(
      .DEPTH(FIFO_DEPTH),
      .FALL_THROUGH(1'b1)
  ) dma_read_addr_fifo_i (
      .clk_i(clk_cg),
      .rst_ni,
      .flush_i(fifo_flush),
      .testmode_i(1'b0),
      // status flags
      .full_o(read_addr_fifo_full),
      .empty_o(read_addr_fifo_empty),
      .usage_o(read_addr_fifo_usage),
      // as long as the queue is not full we can push new data
      .data_i(data_addr_in_rdata),
      .push_i(data_addr_in_rvalid),
      // as long as the queue is not empty we can pop new elements
      .data_o(read_addr_fifo_output),
      .pop_i(write_fifo_pop && address_mode)  // not an error!
  );

  /* Write FIFO */
  fifo_v3 #(
      .DEPTH(FIFO_DEPTH),
      .FALL_THROUGH(1'b1)
  ) dma_write_fifo_i (
      .clk_i(clk_cg),
      .rst_ni,
      .flush_i(fifo_flush),
      .testmode_i(1'b0),
      // status flags
      .full_o(write_fifo_full),
      .empty_o(write_fifo_empty),
      .usage_o(write_fifo_usage),
      // as long as the queue is not full we can push new data
      .data_i(write_fifo_input),
      .push_i(write_fifo_push),
      // as long as the queue is not empty we can pop new elements
      .data_o(write_fifo_output),
      .pop_i(write_fifo_pop)
  );

  dma_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) dma_reg_top_i (
      .clk_i(clk_cg),
      .rst_ni,
      .reg_req_i,
      .reg_rsp_o,
      .reg2hw,
      .hw2reg,
      .devmode_i(1'b1)
  );

  /* Read FSM */
  dma_obiread_fsm dma_obiread_fsm_i (
      .clk_i(clk_cg),
      .rst_ni,
      .reg2hw_i(reg2hw),
      .dma_start_i(dma_start),
      .dma_done_i(dma_done),
      .ext_dma_stop_i,
      .read_fifo_full_i(read_fifo_full),
      .read_fifo_alm_full_i(read_fifo_alm_full),
      .wait_for_rx_i(wait_for_rx),
      .data_in_gnt_i(data_in_gnt),
      .data_in_rvalid_i(data_in_rvalid),
      .data_in_rdata_i(data_in_rdata),
      .fifo_input_o(read_fifo_input),
      .data_in_req_o(data_in_req),
      .data_in_we_o(data_in_we),
      .data_in_be_o(data_in_be),
      .data_in_addr_o(data_in_addr),
      .read_fifo_flush_o(fifo_flush)
  );

  /* Read address FSM */
  dma_obiread_addr_fsm dma_obiread_addr_fsm_i (
      .clk_i(clk_cg),
      .rst_ni,
      .reg2hw_i(reg2hw),
      .dma_start_i(dma_start),
      .ext_dma_stop_i,
      .read_fifo_addr_full_i(read_addr_fifo_full),
      .read_fifo_addr_alm_full_i(read_addr_fifo_alm_full),
      .address_mode_i(address_mode),
      .data_in_gnt_i(data_addr_in_gnt),
      .data_addr_in_req_o(data_addr_in_req),
      .data_addr_in_we_o(data_addr_in_we),
      .data_addr_in_be_o(data_addr_in_be),
      .data_addr_in_addr_o(data_addr_in_addr)
  );

  /* DMA padding FSM */
  dma_padding_fsm dma_padding_fsm_i (
      .clk_i(clk_cg),
      .rst_ni,
      .reg2hw_i(reg2hw),
      .dma_padding_fsm_on_i(dma_padding_fsm_on),
      .dma_start_i(dma_start),
      .read_fifo_empty_i(read_fifo_empty),
      .write_fifo_full_i(write_fifo_full),
      .write_fifo_alm_full_i(write_fifo_alm_full),
      .data_read_i(read_fifo_output),
      .padding_fsm_done_o(padding_fsm_done),
      .write_fifo_push_o(write_fifo_push),
      .read_fifo_pop_o(read_fifo_pop),
      .data_write_o(write_fifo_input)
  );

  /* Write FSM */
  dma_obiwrite_fsm dma_obiwrite_fsm_i (
      .clk_i(clk_cg),
      .rst_ni,
      .reg2hw_i(reg2hw),
      .dma_start_i(dma_start),
      .write_fifo_empty_i(write_fifo_empty),
      .read_addr_fifo_empty_i(read_addr_fifo_empty),
      .fifo_output_i(write_fifo_output),
      .wait_for_tx_i(wait_for_tx),
      .address_mode_i(address_mode),
      .padding_fsm_done_i(padding_fsm_done),
      .fifo_addr_output_i(read_addr_fifo_output),
      .data_out_gnt_i(data_out_gnt),
      .data_out_req_o(data_out_req),
      .data_out_we_o(data_out_we),
      .data_out_be_o(data_out_be),
      .data_out_addr_o(data_out_addr),
      .data_out_wdata_o(data_out_wdata),
      .dma_done_o(dma_done)
  );

  /*_________________________________________________________________________________________________________________________________ */

  /* FSMs instantiation */

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
  always_ff @(posedge clk_cg, negedge rst_ni) begin
    if (~rst_ni) begin
      dma_state_q <= DMA_READY;
    end else begin
      dma_state_q <= dma_state_d;
    end
  end

  /* DMA pulse start when dma_start register is written */
  always_ff @(posedge clk_cg or negedge rst_ni) begin : proc_dma_start
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

  /* Transaction IFR update */
  always_ff @(posedge clk_cg, negedge rst_ni) begin : proc_ff_transaction_ifr
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
  always_ff @(posedge clk_cg, negedge rst_ni) begin : proc_ff_intr
    if (~rst_ni) begin
      dma_done_intr_n <= '0;
    end else begin
      dma_done_intr_n <= dma_done_intr;
    end
  end

  /* Window IFR update */
  always_ff @(posedge clk_cg, negedge rst_ni) begin : proc_ff_window_ifr
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
  always_ff @(posedge clk_cg, negedge rst_ni) begin : proc_ff_window_intr
    if (~rst_ni) begin
      dma_window_intr_n <= '0;
    end else begin
      dma_window_intr_n <= dma_window_intr;
    end
  end


  always_ff @(posedge clk_cg, negedge rst_ni) begin : proc_dma_window_cnt
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
  always_ff @(posedge clk_cg, negedge rst_ni) begin : proc_dma_window_done
    if (~rst_ni) begin
      window_done_q <= 1'b0;
    end else begin
      if (dma_window_event) window_done_q <= 1'b1;
      else if (reg2hw.status.window_done.re) window_done_q <= 1'b0;
    end
  end

  always_ff @(posedge clk_cg, negedge rst_ni) begin
    if (~rst_ni) begin
      dma_padding_fsm_on <= 1'b0;
    end else begin
      if (dma_start == 1'b1) begin
        dma_padding_fsm_on <= 1'b1;
      end else if (dma_done == 1'b1) begin
        dma_padding_fsm_on <= 1'b0;
      end
    end
  end


  /*_________________________________________________________________________________________________________________________________ */

  /* Signal assignments */

  assign dma_done_o = dma_done;
  assign dma_start = (dma_state_q == DMA_STARTING);

  /* OBI signals */
  assign dma_read_req_o.req = data_in_req;
  assign dma_read_req_o.we = data_in_we;
  assign dma_read_req_o.be = data_in_be;
  assign dma_read_req_o.addr = data_in_addr;
  assign dma_read_req_o.wdata = 32'h0;

  assign data_in_gnt = dma_read_resp_i.gnt;
  assign data_in_rvalid = dma_read_resp_i.rvalid;
  assign data_in_rdata = dma_read_resp_i.rdata;

  assign dma_addr_req_o.req = data_addr_in_req;
  assign dma_addr_req_o.we = data_addr_in_we;
  assign dma_addr_req_o.be = data_addr_in_be;
  assign dma_addr_req_o.addr = data_addr_in_addr;
  assign dma_addr_req_o.wdata = 32'h0;

  assign data_addr_in_gnt = dma_addr_resp_i.gnt;
  assign data_addr_in_rvalid = dma_addr_resp_i.rvalid;
  assign data_addr_in_rdata = dma_addr_resp_i.rdata;

  assign dma_write_req_o.req = data_out_req;
  assign dma_write_req_o.we = data_out_we;
  assign dma_write_req_o.be = data_out_be;
  assign dma_write_req_o.addr = data_out_addr;
  assign dma_write_req_o.wdata = data_out_wdata;

  assign data_out_gnt = dma_write_resp_i.gnt;
  assign data_out_rvalid = dma_write_resp_i.rvalid;
  assign data_out_rdata = dma_write_resp_i.rdata;

  /* FIFO signals */
  assign write_fifo_pop = (dma_state_q == DMA_RUNNING) & data_out_gnt; // @TOD0: check if this is correct

  assign dma_done_intr = transaction_ifr;
  assign dma_done_intr_o = dma_done_intr_n;
  assign dma_window_intr = window_ifr;
  assign dma_window_intr_o = dma_window_intr_n;

  assign hw2reg.transaction_ifr.d = transaction_ifr;
  assign hw2reg.window_ifr.d = window_ifr;

  assign hw2reg.status.ready.d = (dma_state_q == DMA_READY);
  assign hw2reg.status.window_done.d = window_done_q;

  assign circular_mode = reg2hw.mode.q == 1;
  assign address_mode = reg2hw.mode.q == 2;

  assign wait_for_rx = |(reg2hw.slot.rx_trigger_slot.q[SLOT_NUM-1:0] & (~trigger_slot_i));
  assign wait_for_tx = |(reg2hw.slot.tx_trigger_slot.q[SLOT_NUM-1:0] & (~trigger_slot_i));

  assign read_fifo_alm_full = (read_fifo_usage == LastFifoUsage[Addr_Fifo_Depth-1:0]);
  assign read_addr_fifo_alm_full = (read_addr_fifo_usage == LastFifoUsage[Addr_Fifo_Depth-1:0]);
  assign write_fifo_alm_full = (write_fifo_usage == LastFifoUsage[Addr_Fifo_Depth-1:0]);


  // WINDOW EVENT
  // Count gnt write transaction and generate event pulse if WINDOW_SIZE is reached
  assign dma_window_event = |reg2hw.window_size.q &  data_out_gnt & (window_counter + 'h1 >= {19'h0, reg2hw.window_size.q});

endmodule : dma

// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// DMA assume a read request is not granted before previous request rvalid is asserted

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

  logic        [               31:0] read_ptr_reg;
  logic        [               31:0] addr_ptr_reg;
  logic        [               31:0] read_ptr_valid_reg;
  logic        [               31:0] write_ptr_reg;
  logic        [               31:0] write_address;
  logic        [               31:0] dma_cnt;
  logic        [               31:0] dma_addr_cnt;
  logic        [                2:0] dma_cnt_dec;
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

  logic                              fifo_flush;
  logic                              fifo_full;
  logic                              fifo_empty;

  logic                              fifo_addr_flush;
  logic                              fifo_addr_full;
  logic fifo_addr_empty, fifo_addr_empty_check;

  logic        wait_for_rx;
  logic        wait_for_tx;

  logic [ 1:0] data_type;

  logic [31:0] fifo_input;
  logic [31:0] fifo_addr_input;
  logic [31:0] fifo_output;
  logic [31:0] fifo_addr_output;

  logic [ 3:0] byte_enable_out;

  logic        circular_mode;
  logic        address_mode;

  logic        dma_start_pending;

  enum {
    DMA_READY,
    DMA_STARTING,
    DMA_RUNNING
  }
      dma_state_q, dma_state_d;

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

  assign dma_read_ch0_req_o.req = data_in_req;
  assign dma_read_ch0_req_o.we = data_in_we;
  assign dma_read_ch0_req_o.be = data_in_be;
  assign dma_read_ch0_req_o.addr = data_in_addr;
  assign dma_read_ch0_req_o.wdata = 32'h0;

  assign data_in_gnt = dma_read_ch0_resp_i.gnt;
  assign data_in_rvalid = dma_read_ch0_resp_i.rvalid;
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

  assign dma_done_intr_o = dma_done & reg2hw.interrupt_en.transaction_done.q;
  assign dma_window_intr_o = dma_window_event & reg2hw.interrupt_en.window_done.q;


  logic [31:0] window_counter;


  assign data_type = reg2hw.data_type.q;

  assign hw2reg.status.ready.d = (dma_state_q == DMA_READY);

  assign hw2reg.status.window_done.d = window_done_q;

  assign circular_mode = reg2hw.mode.q == 1;
  assign address_mode = reg2hw.mode.q == 2;

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

  // update state
  always_ff @(posedge clk_i, negedge rst_ni) begin
    if (~rst_ni) begin
      dma_state_q <= DMA_READY;
    end else begin
      dma_state_q <= dma_state_d;
    end
  end


  // DMA pulse start when dma_start register is written
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_dma_start
    if (~rst_ni) begin
      dma_start_pending <= 1'b0;
    end else begin
      if (dma_start == 1'b1) begin
        dma_start_pending <= 1'b0;
      end else if (reg2hw.size.qe & |reg2hw.size.q) begin
        dma_start_pending <= 1'b1;
      end
    end
  end

  // Store input data pointer and increment everytime read request is granted
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_ptr_in_reg
    if (~rst_ni) begin
      read_ptr_reg <= '0;
    end else begin
      if (dma_start == 1'b1) begin
        read_ptr_reg <= reg2hw.src_ptr.q;
      end else if (data_in_gnt == 1'b1) begin
        read_ptr_reg <= read_ptr_reg + {24'h0, reg2hw.ptr_inc.src_ptr_inc.q};
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

  // Only update read_ptr_valid_reg when the data is stored in the fifo
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_ptr_valid_in_reg
    if (~rst_ni) begin
      read_ptr_valid_reg <= '0;
    end else begin
      if (dma_start == 1'b1) begin
        read_ptr_valid_reg <= reg2hw.src_ptr.q;
      end else if (data_in_rvalid == 1'b1) begin
        read_ptr_valid_reg <= read_ptr_valid_reg + {24'h0, reg2hw.ptr_inc.src_ptr_inc.q};
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
        write_ptr_reg <= write_ptr_reg + {24'h0, reg2hw.ptr_inc.dst_ptr_inc.q};
      end
    end
  end

  // Store dma transfer size and decrement it everytime input data rvalid is asserted
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_dma_cnt_reg
    if (~rst_ni) begin
      dma_cnt <= '0;
    end else begin
      if (dma_start == 1'b1) begin
        dma_cnt <= reg2hw.size.q;
      end else if (data_in_gnt == 1'b1) begin
        dma_cnt <= dma_cnt - {29'h0, dma_cnt_dec};
      end
    end
  end

  // Store dma transfer size for the address port
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_dma_addr_cnt_reg
    if (~rst_ni) begin
      dma_addr_cnt <= '0;
    end else begin
      if (dma_start == 1'b1 && address_mode) begin
        dma_addr_cnt <= reg2hw.size.q;
      end else if (data_addr_in_gnt == 1'b1 && address_mode) begin
        dma_addr_cnt <= dma_addr_cnt - 32'h4;  //address always 32b
      end
    end
  end

  always_comb begin
    case (data_type)
      2'b00: dma_cnt_dec = 3'h4;
      2'b01: dma_cnt_dec = 3'h2;
      2'b10, 2'b11: dma_cnt_dec = 3'h1;
    endcase
  end

  always_comb begin : proc_byte_enable_out
    case (data_type)  // Data type 00 Word, 01 Half word, 11,10 byte
      2'b00: byte_enable_out = 4'b1111;  // Writing a word (32 bits)

      2'b01: begin  // Writing a half-word (16 bits)
        case (write_address[1])
          1'b0: byte_enable_out = 4'b0011;
          1'b1: byte_enable_out = 4'b1100;
        endcase
        ;  // case(write_address[1:0])
      end

      2'b10, 2'b11: begin  // Writing a byte (8 bits)
        case (write_address[1:0])
          2'b00: byte_enable_out = 4'b0001;
          2'b01: byte_enable_out = 4'b0010;
          2'b10: byte_enable_out = 4'b0100;
          2'b11: byte_enable_out = 4'b1000;
        endcase
        ;  // case(write_address[1:0])
      end
    endcase
    ;  // case (data_type)
  end

  // Output data shift
  always_comb begin : proc_output_data

    data_out_wdata[7:0]   = fifo_output[7:0];
    data_out_wdata[15:8]  = fifo_output[15:8];
    data_out_wdata[23:16] = fifo_output[23:16];
    data_out_wdata[31:24] = fifo_output[31:24];

    case (write_address[1:0])
      2'b00: ;

      2'b01: data_out_wdata[15:8] = fifo_output[7:0];

      2'b10: begin
        data_out_wdata[23:16] = fifo_output[7:0];
        data_out_wdata[31:24] = fifo_output[15:8];
      end

      2'b11: data_out_wdata[31:24] = fifo_output[7:0];
    endcase
  end

  assign fifo_addr_input = data_addr_in_rdata;  //never misaligned, always 32b

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

  // FSM state update
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_fsm_state
    if (~rst_ni) begin
      dma_read_fsm_state <= DMA_READ_FSM_IDLE;
      dma_write_fsm_state <= DMA_WRITE_FSM_IDLE;
      dma_read_addr_fsm_state <= DMA_READ_FSM_IDLE;
    end else begin
      dma_read_fsm_state <= dma_read_fsm_n_state;
      dma_write_fsm_state <= dma_write_fsm_n_state;
      dma_read_addr_fsm_state <= dma_read_addr_fsm_n_state;
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
        if (|dma_cnt == 1'b0) begin
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
          dma_write_fsm_n_state = DMA_WRITE_FSM_IDLE;
          dma_done = 1'b1;
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
  assign dma_window_event = |reg2hw.window_size.q &  data_out_gnt & (window_counter + 'h1 >= reg2hw.window_size.q);

  always_ff @(posedge clk_i, negedge rst_ni) begin
    if (~rst_ni) begin
      window_counter <= 'h0;
    end else begin
      if (|reg2hw.window_size.q) begin
        if (dma_start | dma_done) begin
          window_counter <= 'h0;
        end else if (data_out_gnt) begin
          if (window_counter + 'h1 >= reg2hw.window_size.q) begin
            window_counter <= 'h0;
          end else begin
            window_counter <= window_counter + 'h1;
          end
        end
      end
    end
  end

  // Update WINDOW_COUNT register
  always_comb begin
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
  always_ff @(posedge clk_i, negedge rst_ni) begin
    if (~rst_ni) begin
      window_done_q <= 1'b0;
    end else begin
      if (dma_window_event) window_done_q <= 1'b1;
      else if (reg2hw.status.window_done.re) window_done_q <= 1'b0;
    end
  end


endmodule : dma

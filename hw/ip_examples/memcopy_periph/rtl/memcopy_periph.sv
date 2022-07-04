// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module memcopy_periph #(
    parameter type reg_req_t  = logic,
    parameter type reg_rsp_t  = logic,
    parameter type obi_req_t  = logic,
    parameter type obi_resp_t = logic
) (
    input logic clk_i,
    input logic rst_ni,

    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    output obi_req_t  master_req_o,
    input  obi_resp_t master_resp_i,

    output memcopy_intr_o
);

  import memcopy_periph_reg_pkg::*;

  memcopy_periph_reg2hw_t        reg2hw;
  memcopy_periph_hw2reg_t        hw2reg;

  logic                   [31:0] read_ptr_reg;
  logic                   [31:0] write_ptr_reg;
  logic                   [31:0] copy_cnt;
  logic                          read_success;
  logic                          write_success;
  logic                          memcopy_start;
  logic                          memcopy_done;

  logic                          data_req;
  logic                          data_we;
  logic                   [ 3:0] data_be;
  logic                   [31:0] data_addr;
  logic                   [31:0] data_wdata_reg;
  logic                          data_gnt;
  logic                          data_rvalid;
  logic                   [31:0] data_rdata;

  enum logic [2:0] {
    MEMCOPY_FSM_IDLE,
    MEMCOPY_FSM_READ,
    MEMCOPY_FSM_RVALID,
    MEMCOPY_FSM_WRITE,
    MEMCOPY_FSM_DONE
  }
      memcopy_fsm_state, memcopy_fsm_n_state;

  assign master_req_o.req = data_req;
  assign master_req_o.we = data_we;
  assign master_req_o.be = data_be;
  assign master_req_o.addr = data_addr;
  assign master_req_o.wdata = data_wdata_reg;

  assign data_gnt = master_resp_i.gnt;
  assign data_rvalid = master_resp_i.rvalid;
  assign data_rdata = master_resp_i.rdata;

  assign memcopy_intr_o = memcopy_done;

  assign hw2reg.done.de = memcopy_done | memcopy_start;
  assign hw2reg.done.d = memcopy_done == 1'b1 ? 1'b1 : 1'b0;

  assign hw2reg.cnt_start.de = memcopy_start;
  assign hw2reg.cnt_start.d = 32'h0;

  // Memcopy pulse start when cnt_start register is written
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_copy_start
    if (~rst_ni) begin
      memcopy_start <= 1'b0;
    end else begin
      if (memcopy_start == 1'b0 && reg2hw.done.q == 1'b1) begin
        memcopy_start <= |reg2hw.cnt_start;
      end else if (memcopy_start == 1'b1) begin
        memcopy_start <= 1'b0;
      end
    end
  end

  // Store memcopy parameters at start
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_memcopy_params
    if (~rst_ni) begin
      read_ptr_reg  <= '0;
      write_ptr_reg <= '0;
      copy_cnt      <= '0;
    end else begin
      if (memcopy_start == 1'b1) begin
        read_ptr_reg  <= reg2hw.ptr_read.q;
        write_ptr_reg <= reg2hw.ptr_write.q;
        copy_cnt      <= reg2hw.cnt_start.q;
      end else if (write_success == 1'b1) begin
        read_ptr_reg  <= read_ptr_reg + 32'h4;
        write_ptr_reg <= write_ptr_reg + 32'h4;
        copy_cnt      <= copy_cnt - 32'h1;
      end
    end
  end

  // Read data buffer before writing it to new location
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_data_buff
    if (~rst_ni) begin
      data_wdata_reg <= '0;
    end else begin
      if (data_rvalid == 1'b1 && read_success == 1'b1) begin
        data_wdata_reg <= data_rdata;
      end
    end
  end

  // Generate read success to control the data copy
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_read_success
    if (~rst_ni) begin
      read_success <= 1'b0;
    end else begin
      if (data_req == 1'b1 && data_we == 1'b0 && data_gnt == 1'b1) begin
        read_success <= 1'b1;
      end else if (read_success == 1'b1 && data_rvalid == 1'b1) begin
        read_success <= 1'b0;
      end
    end
  end

  // FSM state update
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_fsm_state
    if (~rst_ni) begin
      memcopy_fsm_state <= MEMCOPY_FSM_IDLE;
    end else begin
      memcopy_fsm_state <= memcopy_fsm_n_state;
    end
  end

  always_comb begin : proc_fsm_logic

    memcopy_fsm_n_state = MEMCOPY_FSM_IDLE;
    memcopy_done = 1'b0;
    write_success = 1'b0;

    data_req = '0;
    data_we = '0;
    data_be = '0;
    data_addr = '0;

    unique case (memcopy_fsm_state)

      MEMCOPY_FSM_IDLE: begin
        // Wait for start signal
        if (memcopy_start == 1'b1) begin
          memcopy_fsm_n_state = MEMCOPY_FSM_READ;
        end else begin
          memcopy_fsm_n_state = MEMCOPY_FSM_IDLE;
        end
      end
      // Read one word
      MEMCOPY_FSM_READ: begin
        // If all data transfered exit
        if (|copy_cnt == 32'h0) begin
          memcopy_fsm_n_state = MEMCOPY_FSM_DONE;
        end else begin
          data_req  = 1'b1;
          data_we   = 1'b0;
          data_be   = 4'b1111;
          data_addr = read_ptr_reg;

          // Wait for grant before next step
          if (data_gnt == 1'b1) begin
            memcopy_fsm_n_state = MEMCOPY_FSM_RVALID;
          end else begin
            memcopy_fsm_n_state = MEMCOPY_FSM_READ;
          end
        end
      end

      MEMCOPY_FSM_RVALID: begin
        // Wait for rvalid otherwise we might have a grant for write but we haven't buffer the next data yet
        if (data_rvalid == 1'b1) begin
          memcopy_fsm_n_state = MEMCOPY_FSM_WRITE;
        end else begin
          memcopy_fsm_n_state = MEMCOPY_FSM_RVALID;
        end
      end

      // Write one word
      MEMCOPY_FSM_WRITE: begin
        // Grant is not given before rvalid of previous read is given so we can already do the request
        data_req  = 1'b1;
        data_we   = 1'b1;
        data_be   = 4'b1111;
        data_addr = write_ptr_reg;

        // Wait for grant
        if (data_gnt == 1'b1) begin
          memcopy_fsm_n_state = MEMCOPY_FSM_READ;
          write_success = 1'b1;
        end else begin
          memcopy_fsm_n_state = MEMCOPY_FSM_WRITE;
        end
      end

      // Memcopy done
      MEMCOPY_FSM_DONE: begin
        memcopy_fsm_n_state = MEMCOPY_FSM_IDLE;
        memcopy_done = 1'b1;
      end

      default: begin
        memcopy_fsm_n_state = MEMCOPY_FSM_IDLE;
      end
    endcase
  end

  memcopy_periph_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) memcopy_periph_reg_top_i (
      .clk_i,
      .rst_ni,
      .reg_req_i,
      .reg_rsp_o,
      .reg2hw,
      .hw2reg,
      .devmode_i(1'b1)
  );

endmodule : memcopy_periph


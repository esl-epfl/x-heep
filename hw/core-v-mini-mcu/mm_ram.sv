// Copyright 2017 Embecosm Limited <www.embecosm.com>
// Copyright and related rights are licensed under the Solderpad Hardware
// License, Version 0.51 (the "License"); you may not use this file except in
// compliance with the License.  You may obtain a copy of the License at
// http://solderpad.org/licenses/SHL-0.51. Unless required by applicable law
// or agreed to in writing, software, hardware and materials distributed under
// this License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

// RAM and MM wrapper for RI5CY
// Contributor: Jeremy Bennett <jeremy.bennett@embecosm.com>
//              Robert Balas <balasr@student.ethz.ch>
//
// This maps the dp_ram module to the instruction and data ports of the RI5CY
// processor core and some pseudo peripherals

module mm_ram #(
    parameter RAM_ADDR_WIDTH = 16,
    parameter INSTR_RDATA_WIDTH = 128
) (
    input logic clk_i,
    input logic rst_ni,

    input  logic                         instr_req_i,
    input  logic [   RAM_ADDR_WIDTH-1:0] instr_addr_i,
    output logic [INSTR_RDATA_WIDTH-1:0] instr_rdata_o,
    output logic                         instr_rvalid_o,
    output logic                         instr_gnt_o,

    input  logic        data_req_i,
    input  logic [31:0] data_addr_i,
    input  logic        data_we_i,
    input  logic [ 3:0] data_be_i,
    input  logic [31:0] data_wdata_i,
    output logic [31:0] data_rdata_o,
    output logic        data_rvalid_o,
    output logic        data_gnt_o,
    input  logic [ 5:0] data_atop_i,

    input logic [4:0] irq_id_i,
    input logic       irq_ack_i,

    output logic        irq_software_o,
    output logic        irq_timer_o,
    output logic        irq_external_o,
    output logic [15:0] irq_fast_o,

    input logic [31:0] pc_core_id_i,

    output logic        tests_passed_o,
    output logic        tests_failed_o,
    output logic        exit_valid_o,
    output logic [31:0] exit_value_o
);

  localparam int TIMER_IRQ_ID = 7;
  localparam int RND_STALL_REGS = 16;
  localparam int IRQ_MAX_ID = 31;
  localparam int IRQ_MIN_ID = 26;

  typedef enum logic [1:0] {
    T_RAM,
    T_PER,
    T_RND_STALL,
    T_ERR
  } transaction_t;
  transaction_t transaction, transaction_q;

  class rand_default_gnt;
    rand logic gnt;
  endclass : rand_default_gnt

  logic [31:0] data_addr_aligned;

  // signals for handshake
  logic data_rvalid_q;
  logic instr_rvalid_q;
  logic [INSTR_RDATA_WIDTH-1:0] core_instr_rdata;
  logic [31:0] core_data_rdata;

  // signals from amo shim to ram
  logic ram_amoshimd_data_req;
  logic [31:0] ram_amoshimd_data_addr;
  logic ram_amoshimd_data_we;
  logic [63:0] ram_amoshimd_data_wdata;
  logic [7:0] ram_amoshimd_data_be;
  logic [63:0] ram_amoshimd_data_rdata;
  logic [31:0] tmp_ram_amoshimd_data_rdata;
  logic ram_amoshimd_data_gnt;

  // signals to ram (amo shim)
  logic ram_data_req;
  logic [RAM_ADDR_WIDTH-1:0] ram_data_addr;
  logic [31:0] ram_data_wdata;
  logic [31:0] ram_data_rdata;
  logic [63:0] tmp_ram_data_rdata;
  logic ram_data_we;
  logic [3:0] ram_data_be;
  logic ram_data_gnt;
  logic [5:0] ram_data_atop;
  logic [3:0] ram_data_atop_conv;  // convert ram_data_atop to amo_shim protocol

  logic data_req_dec;
  logic [31:0] data_wdata_dec;
  logic [RAM_ADDR_WIDTH-1:0] data_addr_dec;
  logic data_we_dec;
  logic [3:0] data_be_dec;
  logic [5:0] data_atop_dec;

  logic [INSTR_RDATA_WIDTH-1:0] ram_instr_rdata;
  logic ram_instr_req;
  logic [RAM_ADDR_WIDTH-1:0] ram_instr_addr;
  logic ram_instr_gnt;

  logic perip_gnt;

  // signals to print peripheral
  logic [31:0] print_wdata;
  logic print_valid;

  // signature data
  logic [31:0] sig_end_d, sig_end_q;
  logic [31:0] sig_begin_d, sig_begin_q;

  // signals to timer
  logic [31:0] timer_irq_mask_q;
  logic [31:0] timer_cnt_q;
  logic        irq_timer_q;
  logic        timer_reg_valid;
  logic        timer_val_valid;
  logic [31:0] timer_wdata;

  // signals to rnd_stall
  logic [31:0] rnd_stall_regs      [0:RND_STALL_REGS-1];

  logic        rnd_stall_req;
  logic [31:0] rnd_stall_addr;
  logic [31:0] rnd_stall_wdata;
  logic        rnd_stall_we;
  logic [31:0] rnd_stall_rdata;

  //signal delayed by random stall
  logic        rnd_stall_instr_req;
  logic        rnd_stall_instr_gnt;

  logic        rnd_stall_data_req;
  logic        rnd_stall_data_gnt;


  // sampled rnd_stall_addr
  logic [31:0] rnd_stall_addr_q;

  // sampled data_addr_i for coherent error reporting
  logic [31:0] error_addr_q;

  // IRQ related internal signals

  // struct irq_lines
  typedef struct packed {
    logic irq_software;
    logic irq_timer;
    logic irq_external;
    logic [15:0] irq_fast;
  } Interrupts_tb_t;

  Interrupts_tb_t irq_rnd_lines;

  // Atomic operations signaled through data_atop_i (TODO: use from RI5CY pkg)
  localparam AMO_LR = 5'b00010;
  localparam AMO_SC = 5'b00011;
  localparam AMO_SWAP = 5'b00001;
  localparam AMO_ADD = 5'b00000;
  localparam AMO_XOR = 5'b00100;
  localparam AMO_AND = 5'b01100;
  localparam AMO_OR = 5'b01000;
  localparam AMO_MIN = 5'b10000;
  localparam AMO_MAX = 5'b10100;
  localparam AMO_MINU = 5'b11000;
  localparam AMO_MAXU = 5'b11100;

  // uhh, align?
  always_comb data_addr_aligned = {data_addr_i[31:2], 2'b0};

  // handle the mapping of read and writes to either memory or pseudo
  // peripherals (currently just a redirection of writes to stdout)
  always_comb begin
    tests_passed_o  = '0;
    tests_failed_o  = '0;
    exit_value_o    = 0;
    exit_valid_o    = '0;
    perip_gnt       = '0;
    data_req_dec    = '0;
    data_addr_dec   = '0;
    data_wdata_dec  = '0;
    data_we_dec     = '0;
    data_be_dec     = '0;
    data_atop_dec   = '0;
    print_wdata     = '0;
    print_valid     = '0;
    timer_wdata     = '0;
    timer_reg_valid = '0;
    timer_val_valid = '0;
    sig_end_d       = sig_end_q;
    sig_begin_d     = sig_begin_q;
    rnd_stall_req   = '0;
    rnd_stall_addr  = '0;
    rnd_stall_wdata = '0;
    rnd_stall_we    = '0;
    transaction     = T_PER;

    if (data_req_i) begin
      if (data_we_i) begin  // handle writes
        if (data_addr_i < 2 ** RAM_ADDR_WIDTH) begin  // TODO: fail here if requesting atop or smth?
          data_req_dec   = data_req_i;
          data_addr_dec  = data_addr_i[RAM_ADDR_WIDTH-1:0];
          data_wdata_dec = data_wdata_i;
          data_we_dec    = data_we_i;
          data_be_dec    = data_be_i;
          data_atop_dec  = data_atop_i;
          transaction    = T_RAM;
        end else if (data_addr_i == 32'h1000_0000) begin
          print_wdata = data_wdata_i;
          print_valid = data_req_i;
          perip_gnt   = 1'b1;

        end else if (data_addr_i == 32'h2000_0000) begin
          if (data_wdata_i == 123456789) tests_passed_o = '1;
          else if (data_wdata_i == 1) tests_failed_o = '1;
          perip_gnt = 1'b1;

        end else if (data_addr_i == 32'h2000_0004) begin
          exit_valid_o = '1;
          exit_value_o = data_wdata_i;
          perip_gnt    = 1'b1;

        end else if (data_addr_i == 32'h2000_0008) begin
          // sets signature begin
          sig_begin_d = data_wdata_i;
          perip_gnt   = 1'b1;

        end else if (data_addr_i == 32'h2000_000C) begin
          // sets signature end
          sig_end_d = data_wdata_i;
          perip_gnt = 1'b1;

        end else if (data_addr_i == 32'h2000_0010) begin
          // halt and dump signature
          automatic string sig_file;
          automatic bit use_sig_file;
          automatic integer sig_fd;
          automatic integer errno;
          automatic string error_str;

          if ($value$plusargs("signature=%s", sig_file)) begin
            sig_fd = $fopen(sig_file, "w");
            if (sig_fd == 0) begin
`ifndef VERILATOR
              errno = $ferror(sig_fd, error_str);
              $error(error_str);
`else
              $error("can't open file");
`endif
              use_sig_file = 1'b0;
            end else begin
              use_sig_file = 1'b1;
            end
          end

          $display("Dumping signature");
          for (logic [31:0] addr = sig_begin_q; addr < sig_end_q; addr += 4) begin
            $display("%x%x%x%x", dp_ram_i.mem[addr+3], dp_ram_i.mem[addr+2], dp_ram_i.mem[addr+1],
                     dp_ram_i.mem[addr+0]);
            if (use_sig_file) begin
              $fdisplay(sig_fd, "%x%x%x%x", dp_ram_i.mem[addr+3], dp_ram_i.mem[addr+2],
                        dp_ram_i.mem[addr+1], dp_ram_i.mem[addr+0]);
            end
          end
          // end simulation
          exit_valid_o = '1;
          exit_value_o = '0;
          perip_gnt    = 1'b1;

        end else if (data_addr_i == 32'h1500_0000) begin
          timer_wdata = data_wdata_i;
          timer_reg_valid = '1;
          perip_gnt    = 1'b1;

        end else if (data_addr_i == 32'h1500_0004) begin
          timer_wdata = data_wdata_i;
          timer_val_valid = '1;
          perip_gnt = 1'b1;

          // write to rnd stall regs
        end else if (data_addr_i[31:16] == 16'h1600) begin
          rnd_stall_req   = data_req_i;
          rnd_stall_wdata = data_wdata_i;
          rnd_stall_addr  = data_addr_i;
          rnd_stall_we    = data_we_i;
          perip_gnt       = 1'b1;
        end else begin
          // out of bounds write
        end

      end else begin  // handle reads
        if (data_addr_i < 2 ** RAM_ADDR_WIDTH) begin
          data_req_dec   = data_req_i;
          data_addr_dec  = data_addr_i[RAM_ADDR_WIDTH-1:0];
          data_wdata_dec = data_wdata_i;
          data_we_dec    = data_we_i;
          data_be_dec    = data_be_i;
          data_atop_dec  = data_atop_i;
          transaction    = T_RAM;
        end else if (data_addr_i[31:16] == 16'h1600) begin
          rnd_stall_req   = data_req_i;
          rnd_stall_wdata = data_wdata_i;
          rnd_stall_addr  = data_addr_i;
          rnd_stall_we    = data_we_i;
          transaction     = T_RND_STALL;
        end else if (data_addr_i[31:00] == 32'h1500_1000) begin
          transaction = T_PER;
          perip_gnt   = 1'b1;
        end else transaction = T_ERR;
      end
    end
  end

  // Store rnd_stall_addr when the processor wants to read a rnd_stall_reg
  // This is useful if the request is delayed because of a memory stall
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      rnd_stall_addr_q <= '0;
    end else begin
      if (transaction == T_RND_STALL) begin
        rnd_stall_addr_q <= rnd_stall_addr;
      end
    end
  end

  // Store data_addr_i when the processor wants to read a rnd_stall_reg
  // This is useful if the request is delayed because of a memory stall
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      error_addr_q <= '0;
    end else begin
      if (transaction != T_RAM) begin
        error_addr_q <= data_addr_i;
      end
    end
  end

`ifndef VERILATOR
  // signal out of bound writes
  out_of_bounds_write :
  assert property
    (@(posedge clk_i) disable iff (~rst_ni)
     (data_req_i && data_we_i |-> data_addr_i < 2 ** RAM_ADDR_WIDTH
      || data_addr_i == 32'h1000_0000
      || data_addr_i == 32'h1500_0000
      || data_addr_i == 32'h1500_0004
      || data_addr_i == 32'h2000_0000
      || data_addr_i == 32'h2000_0004
      || data_addr_i == 32'h2000_0008
      || data_addr_i == 32'h2000_000c
      || data_addr_i == 32'h2000_0010
      || data_addr_i[31:16] == 16'h1600))
  else $fatal("out of bounds write to %08x with %08x", data_addr_i, data_wdata_i);
`endif

  logic [31:0] data_rdata_mux;

  // make sure we select the proper read data
  always_comb begin : read_mux
    data_rdata_mux = '0;

    if (transaction_q == T_RAM) begin
      data_rdata_mux = core_data_rdata;
    end else if (transaction_q == T_RND_STALL) begin
`ifndef VERILATOR
      data_rdata_mux = rnd_stall_rdata;
`else
      $display(
          "out of bounds read from %08x\nRandom stall generator is not supported with Verilator",
          data_addr_i);
      $fatal(2);
`endif
    end else if (transaction_q == T_ERR) begin
      $display("out of bounds read from %08x", data_addr_i);
      $fatal(2);
    end
  end

  // print to stdout pseudo peripheral
  always_ff @(posedge clk_i, negedge rst_ni) begin : print_peripheral
    if (print_valid) begin
      if ($test$plusargs("verbose")) begin
        if (32 <= print_wdata && print_wdata < 128) $display("OUT: '%c'", print_wdata[7:0]);
        else $display("OUT: %3d", print_wdata);

      end else begin
        $write("%c", print_wdata[7:0]);
`ifndef VERILATOR
        $fflush();
`endif
      end
    end
  end



  // Control timer. We need one to have some kind of timeout for tests that
  // get stuck in some loop. The riscv-tests also mandate that. Enable timer
  // interrupt by writing 1 to timer_irq_mask_q. Write initial value to
  // timer_cnt_q which gets counted down each cycle. When it transitions from
  // 1 to 0, and interrupt request (irq_q) is made (masked by timer_irq_mask_q).
  always_ff @(posedge clk_i, negedge rst_ni) begin : tb_timer
    if (~rst_ni) begin
      timer_irq_mask_q <= '0;
      timer_cnt_q      <= '0;
      irq_timer_q      <= '0;
      for (int i = 0; i < RND_STALL_REGS; i++) begin
        rnd_stall_regs[i] <= '0;
      end
      rnd_stall_rdata <= '0;
    end else begin
      // set timer irq mask
      if (timer_reg_valid) begin
        timer_irq_mask_q <= timer_wdata;

        // write timer value
      end else if (timer_val_valid) begin
        timer_cnt_q <= timer_wdata;

      end else if (rnd_stall_req) begin
        if (rnd_stall_we) rnd_stall_regs[rnd_stall_addr[5:2]] <= rnd_stall_wdata;
        else rnd_stall_rdata <= rnd_stall_regs[rnd_stall_addr_q[5:2]];
      end else begin
        if (timer_cnt_q > 0) timer_cnt_q <= timer_cnt_q - 1;

        if (timer_cnt_q == 1) irq_timer_q <= 1'b1 && timer_irq_mask_q[TIMER_IRQ_ID];

        if (irq_ack_i == 1'b1 && irq_id_i == TIMER_IRQ_ID) irq_timer_q <= '0;

      end
    end
  end

  // show writes if requested
  always_ff @(posedge clk_i, negedge rst_ni) begin : verbose_writes
    if ($test$plusargs("verbose") && data_req_i && data_we_i)
      $display("write addr=0x%08x: data=0x%08x", data_addr_i, data_wdata_i);
  end

  // instantiate the ram
  dp_ram #(
      .ADDR_WIDTH(RAM_ADDR_WIDTH),
      .INSTR_RDATA_WIDTH(INSTR_RDATA_WIDTH)
  ) dp_ram_i (
      .clk_i(clk_i),

      .en_a_i  (instr_req_i),
      .addr_a_i(instr_addr_i),

      .wdata_a_i('0),  // Not writing so ignored
      .rdata_a_o(ram_instr_rdata),
      .we_a_i   ('0),
      .be_a_i   (4'b1111),  // Always want 32-bits

      .en_b_i   (data_req_dec),
      .addr_b_i (data_addr_dec),
      .wdata_b_i(ram_amoshimd_data_wdata[31:0]),
      .rdata_b_o(tmp_ram_amoshimd_data_rdata),
      .we_b_i   (ram_amoshimd_data_we),
      .be_b_i   (ram_amoshimd_data_be[3:0])
  );

    assign instr_gnt_o   = instr_req_i;
    assign data_gnt_o    = data_req_dec | perip_gnt;

    assign instr_rdata_o = ram_instr_rdata;

    assign ram_data_req     = data_req_dec;
    assign ram_data_addr    = data_addr_dec;
    assign ram_data_gnt     = ram_amoshimd_data_gnt;
    assign core_data_rdata  = data_req_dec;
    assign ram_data_wdata   = data_wdata_dec;
    assign ram_data_we      = data_we_dec;
    assign ram_data_be      = data_be_dec;
  // signature range
  always_ff @(posedge clk_i, negedge rst_ni) begin
    if (~rst_ni) begin
      sig_end_q   <= '0;
      sig_begin_q <= '0;
    end else begin
      sig_end_q   <= sig_end_d;
      sig_begin_q <= sig_begin_d;
    end


  end

  always_ff @(posedge clk_i, negedge rst_ni) begin
    if (~rst_ni) begin
      data_rvalid_q  <= '0;
      instr_rvalid_q <= '0;
    end else begin
      data_rvalid_q  <= ram_data_req;
      instr_rvalid_q <= ram_instr_req;
    end
  end


  always_ff @(posedge clk_i, negedge rst_ni) begin
    if (~rst_ni) begin
      transaction_q <= T_RAM;
    end else begin
      transaction_q <= transaction;
    end
  end


  // IRQ SIGNALS ROUTING
  assign irq_software_o = '0;
  assign irq_timer_o    = '0;
  assign irq_external_o = '0;
  assign irq_fast_o     = '0;


endmodule  // ram

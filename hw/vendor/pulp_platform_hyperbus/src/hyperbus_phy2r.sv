// Copyright 2023 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Luca Valente <luca.valente@unibo.it>

module hyperbus_phy2r #(
  parameter int unsigned AxiDataWidth = -1,
  parameter int unsigned NumPhys = -1,
  parameter type T = logic,
  parameter int unsigned BurstLength = -1,
  parameter int unsigned AddrWidth = $clog2(AxiDataWidth/8)
) (
  input logic                   clk_i,
  input logic                   rst_ni,
  input logic [2:0]             size,
  input logic                   is_a_read,
  input logic                   trans_handshake,
  input logic [AddrWidth-1:0]   start_addr,
  input logic [BurstLength-1:0] burst_len,
  output logic                  axi_valid_o,
  input logic                   axi_ready_i,
  output                        T data_o,
  input logic                   phy_valid_i,
  output logic                  phy_ready_o,
  input logic [16*NumPhys-1:0]  data_i,
  input logic                   last_i,
  input logic                   error_i
);

   // Cutting the combinatorial path between cdc fifo and AXI Master
   typedef enum logic [2:0] {
       Idle,
       WaitData,
       Sample,
       CntReady
   } hyper_splitter_state_t;

   hyper_splitter_state_t state_d, state_q;

   localparam  int unsigned NumAxiBytes = AxiDataWidth/8;
   localparam  int unsigned NumPhyBytes = NumPhys*2;
   localparam  int unsigned AxiBytesInPhyBeat = NumAxiBytes/NumPhyBytes;
   localparam  int unsigned WordCntWidth = (AxiBytesInPhyBeat==1) ? 1 : $clog2(AxiBytesInPhyBeat);

   logic [BurstLength-1:0] byte_axi_addr_d, byte_axi_addr_q;
   logic [BurstLength-1:0] byte_phy_cnt_d, byte_phy_cnt_q;
   logic [BurstLength-1:0] last_addr_d, last_addr_q;

   logic [3:0]           size_d, size_q;
   T data_buffer_d, data_buffer_q;

   logic                        is_16_bw, is_8_bw;
   logic [WordCntWidth-1:0]     word_cnt;
   logic                        enough_data;
   logic                        sent_available_data;
   logic [BurstLength-1:0]      next_axi_addr;

   assign word_cnt = (AxiBytesInPhyBeat==1) ? '0 : byte_phy_cnt_q[($clog2(NumPhys)+1) +:WordCntWidth];
   assign next_axi_addr = ((byte_axi_addr_q>>size_d)<< size_d) + (1<<size_d);
   assign enough_data = byte_phy_cnt_d >= next_axi_addr;
   assign sent_available_data = byte_axi_addr_d >= byte_phy_cnt_q;
   assign data_o.data = data_buffer_q.data;
   assign data_o.error = data_buffer_q.error;
   assign data_o.valid = '0;
   assign data_o.last = data_buffer_q.last && (last_addr_q==byte_axi_addr_d);

   always_comb begin : counter
      byte_axi_addr_d = byte_axi_addr_q;
      size_d = size_q;
      byte_phy_cnt_d = byte_phy_cnt_q;
      last_addr_d = last_addr_q;
      if (trans_handshake & is_a_read) begin
         byte_axi_addr_d[BurstLength-1:AddrWidth] = '0;
         byte_axi_addr_d[AddrWidth-1:0] = start_addr;
         size_d = size;
         byte_phy_cnt_d[BurstLength-1:AddrWidth] = '0;
         byte_phy_cnt_d[AddrWidth-1:0] = (start_addr>>NumPhys)<<NumPhys;
         last_addr_d = ((start_addr>>size)<<size) + (burst_len<<size);
      end
      if ( axi_valid_o & axi_ready_i ) begin
         byte_axi_addr_d = ((byte_axi_addr_q>>size_q)<< size_q) + (1<<size_q);
      end
      if ( phy_valid_i & phy_ready_o ) begin
         byte_phy_cnt_d = byte_phy_cnt_q + NumPhys*2;
      end
   end

   always_comb begin : sampler
      data_buffer_d = data_buffer_q;
      if (state_d==Idle) begin
         data_buffer_d.last = 1'b0;
         data_buffer_d.data = '0; // for debug
      end else if (phy_ready_o && phy_valid_i) begin
         data_buffer_d.data[word_cnt*(16*NumPhys) +: (16*NumPhys)] = data_i;
         data_buffer_d.error = error_i;
         data_buffer_d.last = last_i;
      end
   end

   always_comb begin : fsm
      state_d = state_q;
      axi_valid_o = 1'b0;
      phy_ready_o = 1'b0;
      case (state_q)
        Idle: begin
           if (trans_handshake & is_a_read) begin
              state_d = WaitData;
           end
        end
        WaitData: begin
           phy_ready_o = 1'b1;
           if (phy_valid_i) begin
              state_d = Sample;
           end
        end
        Sample: begin
           phy_ready_o = 1'b1;
           if(enough_data) begin
              state_d = CntReady;
           end
        end
        CntReady: begin
           phy_ready_o = 1'b0;
           axi_valid_o = 1'b1;
           if(sent_available_data) begin
                phy_ready_o = 1'b1;
                state_d = (data_o.last) ? Idle : ( phy_valid_i ? Sample : WaitData );
           end else if (last_addr_q==byte_axi_addr_d) begin
              state_d = Idle;
           end
        end
      endcase
   end

   always_ff @(posedge clk_i or negedge rst_ni) begin : proc_ff_phy
       if (~rst_ni) begin
          data_buffer_q <= '0;
          state_q <= Idle;
          byte_axi_addr_q <= '0;
          byte_phy_cnt_q <= '0;
          size_q <= '0;
          last_addr_q <= '0;
       end else begin
          state_q <= state_d;
          data_buffer_q <= data_buffer_d;
          byte_axi_addr_q <= byte_axi_addr_d;
          byte_phy_cnt_q <= byte_phy_cnt_d;
          size_q <= size_d;
          last_addr_q <= last_addr_d;
       end
   end


endmodule

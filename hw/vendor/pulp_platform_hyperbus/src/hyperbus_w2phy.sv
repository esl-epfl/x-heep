// Copyright 2023 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Thomas Benz <paulsc@iis.ee.ethz.ch>
// Paul Scheffler <paulsc@iis.ee.ethz.ch>
// Luca Valente <luca.valente@unibo.it>

module hyperbus_w2phy #(
  parameter int unsigned AxiDataWidth = -1,
  parameter int unsigned NumPhys = -1,
  parameter int unsigned BurstLength = -1,
  parameter type T = logic,
  parameter int unsigned AddrWidth = $clog2(AxiDataWidth/8)
) (
  input logic                   clk_i,
  input logic                   rst_ni,
  input logic [2:0]             size,
  input logic [AddrWidth-1:0]   start_addr,
  input logic [BurstLength-1:0] len,
  input logic                   is_a_write,
  input logic                   trans_handshake,
  input logic                   axi_valid_i,
  output logic                  axi_ready_o,
  input                         T data_i,
  output logic                  phy_valid_o,
  input logic                   phy_ready_i,
  output logic [16*NumPhys-1:0] data_o,
  output logic                  last_o,
  output logic [2*NumPhys-1:0]  strb_o
);

   localparam  int unsigned NumAxiBytes = AxiDataWidth/8;
   localparam  int unsigned NumPhyBytes = NumPhys*2;
   localparam  int unsigned AxiBytesInPhyBeat = NumAxiBytes/NumPhyBytes;
   localparam  int unsigned WordCntWidth = (AxiBytesInPhyBeat==1) ? 1 : $clog2(AxiBytesInPhyBeat);
   // Cutting the combinatorial path between AXI master and cdc fifo
   typedef enum logic [2:0] {
       Idle,
       Sample,
       CntReady
   } hyper_upsizer_state_t;

   hyper_upsizer_state_t state_d,    state_q;

   typedef struct packed {
       logic [AxiDataWidth/8-1:0] strb;
       logic [AxiDataWidth-1:0]   data;
       logic                      last;
   } w2phy_chan_t;

   w2phy_chan_t data_buffer_d, data_buffer_q;

   logic        is_16_bw, is_8_bw;
   logic        upsize;
   logic        enough_data;
   logic        first_tx_d, first_tx_q;


   logic [NumPhys*2-1:0]         mask_strobe_d, mask_strobe_q;
   logic [WordCntWidth-1:0]      word_cnt;
   logic [AddrWidth-1:0]         byte_idx_d, byte_idx_q;
   logic [3:0]                   size_d, size_q;
   logic [AddrWidth-1:0]         cnt_data_phy_d, cnt_data_phy_q;
   logic                         keep_sampling, keep_sending;
   logic                         upsize_q;

   assign is_8_bw = (size_d == 0);
   assign is_16_bw = (size_d == 1) ;
   assign upsize = (is_16_bw && (NumPhys==2)) | is_8_bw ;
   assign upsize_q = ( (size_q==1) && (NumPhys==2)) | (size_q==0) ;
   assign enough_data = !upsize;
   assign keep_sampling = (size_d<($clog2(NumPhys)+1)) && (byte_idx_d[NumPhys-1:0]!='0);
   assign keep_sending =  (size_d>($clog2(NumPhys)+1)) && (cnt_data_phy_d != byte_idx_q);
   assign word_cnt = cnt_data_phy_q>>($clog2(NumPhys)+1);


   assign data_o = data_buffer_q.data[(16*NumPhys)*word_cnt +:(16*NumPhys)];
   assign strb_o = data_buffer_q.strb[ (2*NumPhys)*word_cnt +: (2*NumPhys)] & mask_strobe_q;
   assign last_o = data_buffer_q.last && (!keep_sending || upsize_q);

   always_comb begin : counter
      byte_idx_d = byte_idx_q;
      size_d = size_q;
      cnt_data_phy_d = cnt_data_phy_q;
      first_tx_d = first_tx_q;
      if (trans_handshake & is_a_write) begin
         byte_idx_d = start_addr;
         size_d = size;
         cnt_data_phy_d = (start_addr>>NumPhys)<<NumPhys;
         first_tx_d = 1'b1;
      end
      if ( axi_valid_i & axi_ready_o ) begin
         byte_idx_d = ((byte_idx_q>>size_d)<< size_d) + (1<<size_d);
         first_tx_d = 1'b0;
      end
      if ( phy_valid_o & phy_ready_i ) begin
         cnt_data_phy_d = cnt_data_phy_q + NumPhys*2;
      end
   end

   always_comb begin : sampler
      data_buffer_d.data = data_buffer_q.data;
      data_buffer_d.strb = data_buffer_q.strb;
      data_buffer_d.last = data_buffer_q.last;
      if (state_d==Idle) begin
         data_buffer_d.last = 1'b0;
         data_buffer_d.data = '0; // for debug
      end else if (axi_ready_o && axi_valid_i) begin
         if(!upsize) begin
            // If we sample enough data in a single beat, we can sample the whole
            // AXI DATA WIDTH as we'll send all the data to the phy before sampling again.
            data_buffer_d.data = data_i.data;
            data_buffer_d.strb = data_i.strb;
            data_buffer_d.last = data_i.last;
            if(first_tx_q) begin
               for (int i=0; i<byte_idx_q; i++)
                 data_buffer_d.strb[i]='0;
            end
         end else begin
            data_buffer_d.strb[byte_idx_q +: (2*NumPhys)] = data_i.strb[byte_idx_q +: (2*NumPhys)];
            data_buffer_d.data[byte_idx_q*8 +: (8*NumPhys)] = data_i.data[byte_idx_q*8 +: (8*NumPhys)];
            data_buffer_d.last = data_i.last;
            if(first_tx_q) begin
               for (int j=0; j<byte_idx_q; j++)
                 data_buffer_d.strb[j]='0;
            end
         end
      end
   end

   always_comb begin : fsm
      state_d = state_q;
      phy_valid_o = 1'b0;
      axi_ready_o = 1'b0;
      mask_strobe_d = mask_strobe_q;
      case (state_q)
        Idle: begin
           mask_strobe_d = '1;
           if (trans_handshake & is_a_write) begin
              state_d = Sample;
           end
        end
        Sample: begin
           axi_ready_o = 1'b1;
           if(axi_valid_i && enough_data) begin
              state_d = CntReady;
           end else if (axi_valid_i && !enough_data) begin
              if (byte_idx_d[NumPhys-1:0]!='0) begin
                 if (data_i.last) begin
                    state_d = CntReady;
                    for (int k=0; k<NumPhys*2 ; k++)
                      mask_strobe_d[k] = (k<byte_idx_d[NumPhys-1:0]) ? 1'b1 : 1'b0;
                 end else begin
                    state_d = Sample;
                 end
              end else begin
                 state_d = CntReady;
              end
           end
        end
        CntReady: begin
           axi_ready_o = 1'b0;
           phy_valid_o = 1'b1;
           if(phy_ready_i) begin
              if(last_o) begin
                 if(trans_handshake) begin
                    state_d = Sample;
                 end else begin
                    state_d = Idle;
                 end
              end else if (size_d>=NumPhys) begin
                 if (cnt_data_phy_d != byte_idx_q) begin
                   state_d = CntReady;
                 end else if (axi_valid_i) begin
                    axi_ready_o = 1'b1;
                    state_d = enough_data ? CntReady : Sample;
                 end else begin
                   state_d = Sample;
                 end
              end else if (size_d<NumPhys) begin
                 if (cnt_data_phy_d[NumPhys-1:0]=='0) begin
                    axi_ready_o = !upsize;
                    state_d = Sample;
                 end
              end
           end
        end
      endcase
   end


   always_ff @(posedge clk_i or negedge rst_ni) begin : proc_ff_phy
       if (~rst_ni) begin
           data_buffer_q <= '0;
           state_q <= Idle;
           byte_idx_q <= '0;
           size_q <= '0;
           cnt_data_phy_q <= '0;
           first_tx_q <= '0;
           mask_strobe_q <= '0;
       end else begin
           state_q <= state_d;
           data_buffer_q <= data_buffer_d;
           byte_idx_q <= byte_idx_d;
           size_q <= size_d;
           cnt_data_phy_q <= cnt_data_phy_d;
           first_tx_q <= first_tx_d;
           mask_strobe_q <= mask_strobe_d;
       end
   end


endmodule

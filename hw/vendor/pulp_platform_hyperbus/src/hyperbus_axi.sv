// Copyright 2023 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Thomas Benz <tbenz@iis.ee.ethz.ch>
// Paul Scheffler <paulsc@iis.ee.ethz.ch>

`include "axi/typedef.svh"
`include "common_cells/registers.svh"

module hyperbus_axi #(
    parameter int unsigned AxiDataWidth  = -1,
    parameter int unsigned AxiAddrWidth  = -1,
    parameter int unsigned AxiIdWidth    = -1,
    parameter int unsigned AxiUserWidth  = -1,
    parameter type         axi_req_t     = logic,
    parameter type         axi_rsp_t     = logic,
    parameter int unsigned NumChips    	 = -1,
    parameter int unsigned NumPhys       = -1,
    parameter type         hyper_tx_t    = logic,
    parameter type         hyper_rx_t    = logic,
    parameter type         rule_t        = logic
) (
    input  logic                    clk_i,
    input  logic                    rst_ni,
    // AXI port
    input  axi_req_t                axi_req_i,
    output axi_rsp_t                axi_rsp_o,
    // PHI port
    input  hyper_rx_t               rx_i,
    input  logic                    rx_valid_i,
    output logic                    rx_ready_o,

    output hyper_tx_t               tx_o,
    output logic                    tx_valid_o,
    input  logic                    tx_ready_i,

    input  logic                    b_error_i,
    input  logic                    b_valid_i,
    output logic                    b_ready_o,

    output hyperbus_pkg::hyper_tf_t trans_o,
    output logic [NumChips-1:0]     trans_cs_o,
    output logic                    trans_valid_o,
    input  logic                    trans_ready_i,

    input  rule_t [NumChips-1:0]    chip_rules_i,
    input  logic                    phys_in_use_i,
    input  logic                    which_phy_i,
    input  logic [4:0]              addr_mask_msb_i,
    input  logic                    addr_space_i,
    output logic                    trans_active_o
);

    localparam AxiDataBytes    = AxiDataWidth/8;
    localparam AxiBusAddrWidth = $clog2(AxiDataBytes);
    localparam PhyDataWidth    = NumPhys*16;
    localparam PhyDataBytes    = PhyDataWidth/8;
    localparam ChipSelWidth    = cf_math_pkg::idx_width(NumChips);
    localparam ByteCntWidth    = cf_math_pkg::idx_width(AxiDataBytes);

    typedef logic [AxiAddrWidth-1:0] axi_addr_t;
    typedef logic [ByteCntWidth-1:0] byte_cnt_t;
    typedef logic [ByteCntWidth-3:0] word_cnt_t;
    typedef logic [AxiDataWidth-1:0] axi_data_t;
    typedef logic [ChipSelWidth-1:0] chip_sel_idx_t;

    `AXI_TYPEDEF_ALL_CT(axi_fifo,axi_fifo_req,axi_fifo_rsp,axi_addr_t,logic[AxiIdWidth-1:0],axi_data_t,logic[AxiDataBytes-1:0],logic[AxiUserWidth-1:0])

    // No need to track ID: serializer buffers it for us
    typedef struct packed {
        axi_addr_t          addr;
        axi_pkg::len_t      len;
        axi_pkg::burst_t    burst;
        axi_pkg::size_t     size;
    } axi_ax_t;

    typedef struct packed {
       axi_ax_t ax_data;
       logic    write;
    } ax_channel_spill_t;

    typedef struct packed {
        logic               valid;
        axi_data_t          data;
        logic               error;
        logic               last;
    } axi_r_t;

    typedef struct packed {
        logic [AxiDataWidth/8-1:0] strb;
        axi_data_t                 data;
        logic [AxiUserWidth-1:0]   user;
        logic                      last;
    } axi_w_chan_t;

    typedef struct packed {
        logic [7:0]         data;
        logic               strb;
    } axi_wbyte_t;

    // AXI FIFO downstream
    axi_req_t       fifo_out_req;
    axi_rsp_t       fifo_out_rsp;

    // Atomics Filter downstream
    axi_req_t       atop_out_req;
    axi_rsp_t       atop_out_rsp;

    // ID serializer downstream
    axi_req_t       ser_out_req;
    axi_rsp_t       ser_out_rsp;
    axi_ax_t        ser_out_req_aw;
    axi_ax_t        ser_out_req_ar;

    // AX arbiter downstream
    axi_ax_t           rr_out_req_ax;
    logic              rr_out_req_write;
    logic              spill_ax_valid, spill_ax_ready;
    axi_ax_t           spill_rr_out_req_ax;
    logic              spill_rr_out_req_write;
    ax_channel_spill_t spill_ax_channel_in, spill_ax_channel_out;

    // AX handling
    logic           trans_handshake;
    logic           ax_valid, ax_ready;
    axi_pkg::size_t ax_size_d, ax_size_q;
    chip_sel_idx_t  ax_chip_sel_idx;
    hyperbus_pkg::hyper_blen_t ax_blen_postinc;
    logic           ax_blen_inc;

    // R channel
    axi_r_t         s_r_split;

    // R channel merge when phys_in_use != NumPhys
    logic [PhyDataWidth-1:0]   s_rx_data;
    logic [PhyDataWidth/2-1:0] s_rx_data_lower_d, s_rx_data_lower_q;
    logic                      s_rx_error;
    logic                      s_rx_last;
    logic                      s_rx_valid;
    logic                      s_rx_ready;
    logic                      merge_r_d, merge_r_q;

    // W channel
    logic w_data_valid;
    logic w_data_ready;
    axi_w_chan_t w_data_fifo;
    axi_w_chan_t w_data_fifo_in;

    // W channel split when phys_in_use != NumPhys
    logic [PhyDataWidth-1:0]  s_tx_data;
    logic [PhyDataBytes-1:0]  s_tx_strb;
    logic                     s_tx_last;
    logic                     s_tx_valid;
    logic                     s_tx_ready;
    logic                     split_w_d, split_w_q;

    // Whether a transfer is currently active
    logic           trans_active_d, trans_active_q;
    logic           trans_active_set, trans_active_reset;
    logic           trans_wready_d, trans_wready_q;
    logic           trans_wready_set, trans_wready_reset;

    logic [1:0]      phys_in_use;

    assign phys_in_use = (NumPhys==2) ? (phys_in_use_i + 1) : 1;

    // ============================
    //    Serialize requests
    // ============================

    axi_fifo #(
        .Depth       ( 4                  ),
        .FallThrough ( 1'b0               ),
        .aw_chan_t   ( axi_fifo_aw_chan_t ),
        .w_chan_t    ( axi_fifo_w_chan_t  ),
        .b_chan_t    ( axi_fifo_b_chan_t  ),
        .ar_chan_t   ( axi_fifo_ar_chan_t ),
        .r_chan_t    ( axi_fifo_r_chan_t  ),
        .axi_req_t   ( axi_req_t          ),
        .axi_resp_t  ( axi_rsp_t          )
    ) i_axi_fifo (
        .clk_i,
        .rst_ni,
        .test_i     ( 1'b0          ),
        .slv_req_i  ( axi_req_i     ),
        .slv_resp_o ( axi_rsp_o     ),
        .mst_req_o  ( fifo_out_req  ),
        .mst_resp_i ( fifo_out_rsp  )
   );


    // Block unsupported atomics
    axi_atop_filter #(
        .AxiIdWidth         ( AxiIdWidth    ),
        .AxiMaxWriteTxns    ( 1             ),
        .axi_req_t          ( axi_req_t     ),
        .axi_resp_t         ( axi_rsp_t     )
    ) i_axi_atop_filter (
        .clk_i,
        .rst_ni,
        .slv_req_i  ( fifo_out_req  ),
        .slv_resp_o ( fifo_out_rsp  ),
        .mst_req_o  ( atop_out_req  ),
        .mst_resp_i ( atop_out_rsp  )
    );

    // Ensure we only handle one ID (master) at a time
    axi_serializer #(
        .MaxReadTxns    ( 1             ),
        .MaxWriteTxns   ( 1             ),
        .AxiIdWidth     ( AxiIdWidth    ),
        .axi_req_t      ( axi_req_t     ),
        .axi_resp_t     ( axi_rsp_t     )
    ) i_axi_serializer (
        .clk_i,
        .rst_ni,
        .slv_req_i  ( atop_out_req  ),
        .slv_resp_o ( atop_out_rsp  ),
        .mst_req_o  ( ser_out_req   ),
        .mst_resp_i ( ser_out_rsp   )
    );

    // Round-robin-arbitrate between AR and AW channels (HyperBus is simplex)
    assign ser_out_req_ar.addr  = ser_out_req.ar.addr;
    assign ser_out_req_ar.len   = ser_out_req.ar.len;
    assign ser_out_req_ar.burst = ser_out_req.ar.burst;
    assign ser_out_req_ar.size  = ser_out_req.ar.size;

    assign ser_out_req_aw.addr  = ser_out_req.aw.addr;
    assign ser_out_req_aw.len   = ser_out_req.aw.len;
    assign ser_out_req_aw.burst = ser_out_req.aw.burst;
    assign ser_out_req_aw.size  = ser_out_req.aw.size;

    rr_arb_tree #(
        .NumIn      ( 2         ),
        .DataType   ( axi_ax_t  ),
        .AxiVldRdy  ( 1         ),
        .ExtPrio    ( 1'b1      )
    ) i_rr_arb_tree_ax (
        .clk_i,
        .rst_ni,
        .flush_i    ( 1'b0              ),
        .rr_i       ( '0                ),
        .req_i      ( { ser_out_req.aw_valid, ser_out_req.ar_valid } ),
        .gnt_o      ( { ser_out_rsp.aw_ready, ser_out_rsp.ar_ready } ),
        .data_i     ( { ser_out_req_aw,       ser_out_req_ar       } ),
        .req_o      ( spill_ax_valid          ),
        .gnt_i      ( spill_ax_ready          ),
        .data_o     ( spill_rr_out_req_ax     ),
        .idx_o      ( spill_rr_out_req_write  )
    );

    // Cut paths between serializer and rr arb tree
    assign spill_ax_channel_in.ax_data = spill_rr_out_req_ax;
    assign spill_ax_channel_in.write = spill_rr_out_req_write;

    spill_register #(
         .T ( ax_channel_spill_t )
         ) ax_spill_register (
         .clk_i,
         .rst_ni,
         .valid_i (spill_ax_valid),
         .ready_o (spill_ax_ready),
         .data_i  (spill_ax_channel_in),
         .valid_o (ax_valid),
         .ready_i (ax_ready),
         .data_o  (spill_ax_channel_out)
         );

    assign rr_out_req_ax = spill_ax_channel_out.ax_data;
    assign rr_out_req_write = spill_ax_channel_out.write;

    assign trans_valid_o    = ax_valid & ~trans_active_q;
    assign ax_ready         = trans_ready_i & ~trans_active_q;

    assign trans_handshake = trans_valid_o & trans_ready_i;

    // ============================
    //    AX channel: handle
    // ============================

    // Handle address mapping to chip select
    addr_decode #(
        .NoIndices  ( NumChips      ),
        .NoRules    ( NumChips      ),
        .addr_t     ( axi_addr_t    ),
        .rule_t     ( rule_t        )
    ) i_addr_decode_chip_sel (
        .addr_i             ( rr_out_req_ax.addr    ),
        .addr_map_i         ( chip_rules_i          ),
        .idx_o              ( ax_chip_sel_idx       ),
        .dec_valid_o        (                       ),
        .dec_error_o        (                       ),
        .en_default_idx_i   ( 1'b1                  ),
        .default_idx_i      ( '0                    )
    );

    // Chip select binary to one hot decoding
    always_comb begin : proc_comb_trans_cs
        trans_cs_o = '0;
        trans_cs_o[ax_chip_sel_idx] = 1'b1;
    end

    // AX channel: forward, converting unmasked byte to masked word addresses
    assign trans_o.write            = rr_out_req_write;
    assign trans_o.burst_type       = 1'b1;             // Wrapping bursts not (yet) supported
    assign trans_o.address_space    = addr_space_i;
    assign trans_o.address          = ( (rr_out_req_ax.addr & ~32'(32'hFFFF_FFFF << addr_mask_msb_i)) >> ( NumPhys ) ) << ( (NumPhys==2) & ~phys_in_use_i );

    // Convert burst length from decremented, unaligned beats to non-decremented, aligned 16-bit words
    always_comb begin
        trans_o.burst= NumPhys;
        ax_blen_inc   = 1'b1;
        if (rr_out_req_ax.size > NumPhys) begin
           ax_blen_inc   = 1'b1;
           if( ((rr_out_req_ax.addr>>rr_out_req_ax.size)<<rr_out_req_ax.size) != rr_out_req_ax.addr) begin
              if (rr_out_req_ax.size==2) begin
                 trans_o.burst = (ax_blen_postinc << (rr_out_req_ax.size-1)) - 1;
              end else if (rr_out_req_ax.size==3) begin
                 trans_o.burst = (ax_blen_postinc << (rr_out_req_ax.size-1)) - (rr_out_req_ax.addr[2]<<1) - (rr_out_req_ax.addr[1] && (NumPhys==1));
              end else if (rr_out_req_ax.size==4) begin
                   trans_o.burst = (ax_blen_postinc << (rr_out_req_ax.size-1)) - (rr_out_req_ax.addr[3:2]<<1) - (rr_out_req_ax.addr[1] && (NumPhys==1));
              end
           end else begin
              trans_o.burst = (ax_blen_postinc << (rr_out_req_ax.size-1));
           end
        end else if (rr_out_req_ax.size == NumPhys) begin
           trans_o.burst = (ax_blen_postinc << (rr_out_req_ax.size-1));
        end else begin
           ax_blen_inc = 1'b1;
           if (ax_blen_postinc==1) begin
              trans_o.burst= NumPhys;
           end else begin
              if ( ((rr_out_req_ax.addr>>rr_out_req_ax.size)<<rr_out_req_ax.size) != rr_out_req_ax.addr) begin
                 trans_o.burst = ( ( ( (ax_blen_postinc<<rr_out_req_ax.size) - 1 ) >> NumPhys ) + 1 ) << (NumPhys-1);
              end else begin
                 trans_o.burst = ( ( ( rr_out_req_ax.addr[NumPhys-1:0] + (ax_blen_postinc<<rr_out_req_ax.size) - 1 ) >> NumPhys ) + 1 ) << (NumPhys-1);
              end
           end
        end
    end

    assign ax_blen_postinc = rr_out_req_ax.len + hyperbus_pkg::hyper_blen_t'(ax_blen_inc) ;

    // ============================
    //    R channel
    // ============================

    assign ser_out_rsp.r.data   = s_r_split.data;
    assign ser_out_rsp.r.last   = s_r_split.last;
    assign ser_out_rsp.r.resp   = s_r_split.error ? axi_pkg::RESP_SLVERR : axi_pkg::RESP_OKAY;
    assign ser_out_rsp.r.id     = '0;
    assign ser_out_rsp.r.user   = '0;

    always_comb begin
       s_rx_valid = rx_valid_i;
       rx_ready_o = s_rx_ready;
       s_rx_data = rx_i.data;
       s_rx_last = rx_i.last;
       s_rx_error = rx_i.error;
       s_rx_data_lower_d = s_rx_data_lower_q;
       merge_r_d = merge_r_q;
       if( (NumPhys==2) & (~phys_in_use_i) ) begin
          if(rx_valid_i & s_rx_ready) begin
             merge_r_d = merge_r_q + 1;
          end
          if(~which_phy_i)
            s_rx_data = { rx_i.data[PhyDataWidth/2-1:0] , s_rx_data_lower_q };
          else
            s_rx_data = { rx_i.data[PhyDataWidth-1:PhyDataWidth/2] , s_rx_data_lower_q };
          s_rx_valid = rx_valid_i & merge_r_q;
          rx_ready_o = s_rx_ready;
          s_rx_last = rx_i.last & merge_r_q;
          s_rx_error = rx_i.error;
          if(~merge_r_q) begin
             if(~which_phy_i)
               s_rx_data_lower_d = rx_i.data[PhyDataWidth/2-1:0];
             else
               s_rx_data_lower_d = rx_i.data[PhyDataWidth-1:PhyDataWidth/2];
          end
       end
    end

    hyperbus_phy2r #(
        .AxiDataWidth  ( AxiDataWidth                  ),
        .BurstLength   ( hyperbus_pkg::HyperBurstWidth ),
        .T             ( axi_r_t                       ),
        .NumPhys       ( NumPhys                       )
    ) i_hyperbus_phy2r (
        .clk_i,
        .rst_ni,
        .size            ( rr_out_req_ax.size                      ),
        .trans_handshake ( trans_handshake                         ),
        .start_addr      ( rr_out_req_ax.addr[AxiBusAddrWidth-1:0] ),
        .burst_len       ( ax_blen_postinc                         ),
        .is_a_read       ( !rr_out_req_write                       ),
        .phy_valid_i     ( s_rx_valid                              ),
        .phy_ready_o     ( s_rx_ready                              ),
        .data_i          ( s_rx_data                               ),
        .last_i          ( s_rx_last                               ),
        .error_i         ( s_rx_error                              ),
        .axi_valid_o     ( ser_out_rsp.r_valid                     ),
        .axi_ready_i     ( ser_out_req.r_ready                     ),
        .data_o          ( s_r_split                               )
    );

    `FFARN(merge_r_q,merge_r_d,1'b0,clk_i,rst_ni)
    `FFARN(s_rx_data_lower_q,s_rx_data_lower_d,'0,clk_i,rst_ni)

    // =========================================================
    //    W channel: Buffer. Cuts path and upsamples when needed
    // =========================================================

    assign w_data_fifo_in.data = ser_out_req.w.data;
    assign w_data_fifo_in.strb = ser_out_req.w.strb;
    assign w_data_fifo_in.last = ser_out_req.w.last;
    assign w_data_fifo_in.user = ser_out_req.w.user;

    stream_fifo #(
        .FALL_THROUGH ( 1'b0         ),
        .T            ( axi_w_chan_t ),
        .DEPTH        ( 8            )
        ) wchan_stream_fifo (
        .clk_i,
        .rst_ni,
        .flush_i    ( 1'b0                ),
        .testmode_i ( 1'b0                ),
        .usage_o    (                     ),
        .data_i     ( w_data_fifo_in      ),
        .valid_i    ( ser_out_req.w_valid ),
        .ready_o    ( ser_out_rsp.w_ready ),
        .data_o     ( w_data_fifo         ),
        .valid_o    ( w_data_valid        ),
        .ready_i    ( w_data_ready        )
        );

    hyperbus_w2phy #(
        .AxiDataWidth ( AxiDataWidth                  ),
        .BurstLength  ( hyperbus_pkg::HyperBurstWidth ),
        .T            ( axi_w_chan_t                  ),
        .NumPhys      ( NumPhys                       )
        ) i_hyperbus_w2phy (
        .clk_i,
        .rst_ni,
        .size            ( rr_out_req_ax.size                      ),
        .len             ( ax_blen_postinc                         ),
        .is_a_write      ( rr_out_req_write                        ),
        .trans_handshake ( trans_handshake                         ),
        .start_addr      ( rr_out_req_ax.addr[AxiBusAddrWidth-1:0] ),
        .data_i          ( w_data_fifo                             ),
        .axi_valid_i     ( w_data_valid                            ),
        .axi_ready_o     ( w_data_ready                            ),
        .data_o          ( s_tx_data                               ),
        .last_o          ( s_tx_last                               ),
        .strb_o          ( s_tx_strb                               ),
        .phy_valid_o     ( s_tx_valid                              ),
        .phy_ready_i     ( s_tx_ready                              )
    );

    always_comb begin
       tx_o.data = s_tx_data;
       tx_o.strb = s_tx_strb;
       tx_o.last = s_tx_last;
       tx_valid_o = s_tx_valid;
       s_tx_ready = tx_ready_i;
       split_w_d = split_w_q;
       if( (NumPhys==2) & (~phys_in_use_i) ) begin
          if(s_tx_valid & tx_ready_i) begin
             split_w_d = split_w_q+1;
          end
          tx_o.last = s_tx_last & split_w_q;
          tx_valid_o = s_tx_valid;
          s_tx_ready = tx_ready_i & split_w_q;
          tx_o.data = { s_tx_data[PhyDataWidth/2-1:0] , s_tx_data[PhyDataWidth/2-1:0] };
          tx_o.strb = { s_tx_strb[NumPhys-1:0] , s_tx_strb[NumPhys-1:0] };
          if(split_w_q) begin
             tx_o.data = { s_tx_data[PhyDataWidth-1:PhyDataWidth/2] , s_tx_data[PhyDataWidth-1:PhyDataWidth/2] };
             tx_o.strb = { s_tx_strb[NumPhys*2-1:NumPhys] , s_tx_strb[NumPhys*2-1:NumPhys] };
          end
       end
    end

    `FFARN(split_w_q,split_w_d,1'b0,clk_i,rst_ni)

    // ============================
    //    B channel: passthrough
    // ============================

    assign ser_out_rsp.b.resp   = b_error_i ? axi_pkg::RESP_SLVERR : axi_pkg::RESP_OKAY;
    assign ser_out_rsp.b.user   = '0;
    assign ser_out_rsp.b.id     = '0;
    assign ser_out_rsp.b_valid  = b_valid_i;
    assign b_ready_o            = ser_out_req.b_ready;

    // ============================
    //    Transfer status
    // ============================

    assign trans_active_o = trans_active_q;

    assign trans_active_set     = trans_handshake;
    assign trans_active_reset   = (rx_valid_i & rx_ready_o & rx_i.last) | (b_valid_i & b_ready_o);

    // Allow W transfers iff currently engaged in write (AW already received)
    assign trans_wready_set     = trans_active_set & rr_out_req_write;
    assign trans_wready_reset   = (tx_valid_o & tx_ready_i & tx_o.last);

    // Set overrules reset as a transfer must start before it finishes
    always_comb begin : proc_comb_trans_active
        trans_active_d = trans_active_q;
        trans_wready_d = trans_wready_q;
        if (trans_active_reset) trans_active_d = 1'b0;
        if (trans_active_set)   trans_active_d = 1'b1;
        if (trans_wready_reset) trans_wready_d = 1'b0;
        if (trans_wready_set)   trans_wready_d = 1'b1;
    end

    // =========================
    //    Registers
    // =========================

    always_ff @(posedge clk_i or negedge rst_ni) begin : proc_ff
        if(~rst_ni) begin
            trans_active_q      <= '0;
            trans_wready_q      <= '0;
        end else begin
            trans_active_q      <= trans_active_d;
            trans_wready_q      <= trans_wready_d;
        end
    end

    // =========================
    //    Assertions
    // =========================

    // pragma translate_off
    `ifndef VERILATOR
    initial assert (AxiDataWidth >= 16 && AxiDataWidth <= 1024)
            else $error("AxiDatawidth must be a power of two within [16, 1024].");

//    access_16b_align : assert property(
//      @(posedge clk_i) trans_handshake & (rr_out_req_ax.size != '0) |-> (rr_out_req_ax.addr[0] == 1'b0))
//        else $fatal (1, "The address of a non-byte-size access must be 2-byte aligned.");

    burst_type : assert property(
      @(posedge clk_i) trans_handshake |-> ( (rr_out_req_ax.burst == axi_pkg::BURST_INCR) || ((rr_out_req_ax.burst == axi_pkg::BURST_FIXED) &&  (rr_out_req_ax.len == '0)) ) )
        else $fatal (1, "Non-incremental burst passed; this is currently not supported.");
    `endif
    // pragma translate_on

endmodule

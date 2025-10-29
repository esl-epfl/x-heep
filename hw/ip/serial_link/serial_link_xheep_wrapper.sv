module serial_link_xheep_wrapper
  //import obi_pkg::*;
  //import reg_pkg::*;
  //import axi_pkg::*;
 #(
  parameter type axi_req_t  = logic,
  parameter type axi_rsp_t  = logic,
  parameter type aw_chan_t  = logic,
  parameter type ar_chan_t  = logic,
  parameter type r_chan_t   = logic,
  parameter type w_chan_t   = logic,
  parameter type b_chan_t   = logic,
  parameter type cfg_req_t  = logic,
  parameter type cfg_rsp_t  = logic,
  parameter type obi_req_t  = logic,
  parameter type obi_resp_t = logic,
  parameter int NumChannels = 1,
  //parameter int NumChannels = 32,
  parameter int NumLanes = 4,//8,
  parameter int MaxClkDiv = 32,
  parameter int AddrWidth = 32,
  parameter int DataWidth = 32,
  parameter int AW_CH_SIZE = 1,
  parameter int W_CH_SIZE = 1,
  parameter int B_CH_SIZE = 1,
  parameter int AR_CH_SIZE = 1,
  parameter int R_CH_SIZE = 1
) (
  input  logic                      clk_i,
  input  logic                      fast_clock,
  input  logic                      rst_ni,
  input  logic                      clk_reg_i,
  input  logic                      rst_reg_ni,
  input  logic                      testmode_i,

  input  obi_req_t                  obi_req_i,
  output obi_resp_t                 obi_rsp_i,
  // output obi_req_t                  obi_req_o, //fifo writing
  // input  obi_resp_t                 obi_rsp_o,
  input  obi_req_t                  reader_req_i,
  output obi_resp_t                 reader_resp_o,

  input  cfg_req_t                  cfg_req_i,
  output cfg_rsp_t                  cfg_rsp_o,

  output logic fifo_empty_o,
  output logic fifo_full_o,

  input  logic [NumChannels-1:0]    ddr_rcv_clk_i,
  output logic [NumChannels-1:0]    ddr_rcv_clk_o,
  input  logic [NumChannels-1:0][NumLanes-1:0] ddr_i,
  output logic [NumChannels-1:0][NumLanes-1:0] ddr_o

);

  //logic clk_serial_link;
  logic rst_serial_link_n;

  logic clk_ena;
  logic reset_n;

  axi_req_t                  fast_sl_req_i,fast_sl_req_O,axi_in_req_i,axi_out_req_o,  axi_lite_req;
  axi_rsp_t                  fast_sl_rsp_i,fast_sl_rsp_O,axi_in_rsp_o,axi_out_rsp_i,  axi_lite_rsp;
  cfg_req_t                  fast_cfg_req_i;
  cfg_rsp_t                  fast_cfg_rsp_o;
  
  obi_req_t                  obi_req_o; //fifo writing
  obi_resp_t                 obi_rsp_o;


axi_lite_from_mem #(
  .MemAddrWidth    ( AddrWidth ),
  .AxiAddrWidth    ( AddrWidth ),
  .DataWidth       ( DataWidth ),
  .MaxRequests     ( DataWidth ),  // fifo size
  //.AxiProt         ( AxiProt  ),
  .axi_req_t       ( axi_req_t  ),
  .axi_rsp_t       ( axi_rsp_t )
) i_obi2axi (
  .clk_i,
  .rst_ni,
  .mem_req_i                (obi_req_i.req),
  .mem_addr_i               (obi_req_i.addr),
  .mem_we_i                 (obi_req_i.we),
  .mem_wdata_i              (obi_req_i.wdata),
  .mem_be_i                 (obi_req_i.be),
  .mem_gnt_o                (obi_rsp_i.gnt),
  .mem_rsp_valid_o          (obi_rsp_i.rvalid),
  .mem_rsp_rdata_o          (obi_rsp_i.rdata),
  .mem_rsp_error_o          (),
  .axi_req_o                (axi_lite_req),
  .axi_rsp_i                (axi_lite_rsp)
);

axi_lite_to_axi #(
  .AxiDataWidth(32'd32),
  
  .req_lite_t(axi_req_t),
  .resp_lite_t(axi_rsp_t),
  
  .axi_req_t(axi_req_t),
  .axi_resp_t(axi_rsp_t)
) i_axi_lite_to_axi(
  // Slave AXI LITE port
  .slv_req_lite_i(axi_lite_req),
  .slv_resp_lite_o(axi_lite_rsp),
  .slv_aw_cache_i(),
  .slv_ar_cache_i(),
  .mst_req_o(axi_in_req_i),
  .mst_resp_i(axi_in_rsp_o)
);



fifo_serial_link_wrapper #(
  .FIFO_DEPTH(8)
) fifo_serial_link_wrapper_i(
    .testmode_i(0),
  
    .reader_gnt_o             (reader_resp_o.gnt),
    .reader_req_i             (reader_req_i.req),
    .reader_rvalid_o          (reader_resp_o.rvalid),
    .reader_addr_i            (reader_req_i.addr),
    .reader_we_i              (reader_req_i.we),
    .reader_be_i              (reader_req_i.be),
    .reader_rdata_o           (reader_resp_o.rdata),
    .reader_wdata_i           (reader_req_i.wdata),
    
    // .writer_req_i             (obi_req_o.req),
    // .writer_gnt_o             (obi_rsp_o.gnt),
    // .writer_rvalid_o          (obi_rsp_o.rvalid),
    // .writer_addr_i            (obi_req_o.addr),
    // .writer_we_i              (obi_req_o.we),
    // .writer_be_i              (obi_req_o.be),
    // .writer_rdata_o           (obi_rsp_o.rdata),
    // .writer_wdata_i           (obi_req_o.wdata),
    .writer_req_i             (fast_sl_req_O.w_valid),
    .writer_gnt_o             (),
    .writer_rvalid_o          (fast_sl_rsp_O.b_valid),
    .writer_addr_i            (fast_sl_req_O.ar.addr),
    .writer_we_i              (1),
    .writer_be_i              ('0),
    .writer_rdata_o           (fast_sl_rsp_O.r.data),
    .writer_wdata_i           (fast_sl_req_O.w.data),

    .fifo_empty_o,
    .fifo_full_o,

    .clk_i (clk_i),
    .rst_ni(rst_ni)
);

assign fast_sl_rsp_O.ar_ready = 1;
assign fast_sl_rsp_O.aw_ready = 1;
assign fast_sl_rsp_O.w_ready = 1;

// axi_to_mem #(
//   .axi_req_t(axi_req_t),
//   .axi_resp_t(axi_rsp_t),
//   .AddrWidth(AddrWidth),
//   .DataWidth(DataWidth),
//   //.IdWidth(),
//   .NumBanks(1),
//   //.BufDepth(),
//   //.HideStrb(),
//   .OutFifoDepth(1)
//   ) axi_to_mem_i(
//   .clk_i,
//   .rst_ni,
//   .busy_o(),
//   .axi_req_i(axi_out_req_o),
//   .axi_resp_o(axi_out_rsp_i),
//   .mem_req_o(obi_req_o.req),
//   .mem_gnt_i(obi_rsp_o.gnt),
//   .mem_addr_o(obi_req_o.addr),
//   .mem_wdata_o(obi_req_o.wdata),
//   .mem_strb_o(),
//   .mem_atop_o(),
//   .mem_we_o(obi_req_o.we),
//   .mem_rvalid_i(obi_rsp_o.rvalid),
//   .mem_rdata_i(obi_rsp_o.rdata)
// );



  tc_clk_mux2 i_tc_reset_mux (
    .clk0_i (reset_n),
    .clk1_i (rst_ni),
    .clk_sel_i (testmode_i),
    .clk_o (rst_serial_link_n)
  );

//  axi_cdc #(
//      .axi_req_t        ( axi_req_t   ),
//      .axi_resp_t       ( axi_rsp_t   ),
//      .aw_chan_t        ( aw_chan_t   ),
//      .w_chan_t         ( w_chan_t    ),
//      .b_chan_t         ( b_chan_t    ),
//      .ar_chan_t        ( ar_chan_t   ),
//      .r_chan_t         ( r_chan_t    ),
//  /// Depth of the FIFO crossing the clock domain, given as 2**LOG_DEPTH.
//      .LogDepth(2)
//)axi_cdc_i (
//  // slave side - clocked by `src_clk_i`
//  .src_clk_i(clk_i),
//  .src_rst_ni(rst_ni),
//  .src_req_i(axi_in_req_i),
//  .src_resp_o(axi_in_rsp_o),
//  // master side - clocked by `dst_clk_i`
//  .dst_clk_i(fast_clock),
//  .dst_rst_ni(rst_ni),
//  .dst_req_o(fast_sl_req_i),
//  .dst_resp_i(fast_sl_rsp_i)
//);
//


//   axi_cdc #(
//       .axi_req_t        ( axi_req_t   ),
//       .axi_resp_t       ( axi_rsp_t   ),
//       .aw_chan_t        ( aw_chan_t   ),
//       .w_chan_t         ( w_chan_t    ),
//       .b_chan_t         ( b_chan_t    ),
//       .ar_chan_t        ( ar_chan_t   ),
//       .r_chan_t         ( r_chan_t    ),
//   /// Depth of the FIFO crossing the clock domain, given as 2**LOG_DEPTH.
//       .LogDepth(2)
// )axi_cdc_O (
//   // slave side - clocked by `src_clk_i`
//   .src_clk_i(fast_clock),
//   .src_rst_ni(rst_ni),
//   .src_req_i(fast_sl_req_O),
//   .src_resp_o(fast_sl_rsp_O),
//   // master side - clocked by `dst_clk_i`
//   .dst_clk_i(clk_i),
//   .dst_rst_ni(rst_ni),
//   .dst_req_o(axi_out_req_o),
//   .dst_resp_i(axi_out_rsp_i)
// );


    function automatic int max(int a, int b);
        return (a > b) ? a : b;
    endfunction

    function automatic int calc_max_axi_channel_bit(
        int aw_width,
        int w_width,
        int b_width,
        int ar_width,
        int r_width
    );
        return max(max(max(aw_width, w_width), max(b_width, ar_width)), r_width);
    endfunction
  
  parameter int MaxAxiChannelBits = calc_max_axi_channel_bit(AW_CH_SIZE, W_CH_SIZE, B_CH_SIZE, AR_CH_SIZE, R_CH_SIZE);

  if (NumChannels > 1) begin : gen_multi_channel_serial_link
    serial_link #(
      .axi_req_t        ( axi_req_t   ),
      .axi_rsp_t        ( axi_rsp_t   ),
      .aw_chan_t        ( aw_chan_t   ),
      .w_chan_t         ( w_chan_t    ),
      .b_chan_t         ( b_chan_t    ),
      .ar_chan_t        ( ar_chan_t   ),
      .r_chan_t         ( r_chan_t    ),
      .cfg_req_t        ( cfg_req_t   ),
      .cfg_rsp_t        ( cfg_rsp_t   ),
      .hw2reg_t         ( serial_link_reg_pkg::serial_link_hw2reg_t ),
      .reg2hw_t         ( serial_link_reg_pkg::serial_link_reg2hw_t ),
      .NumChannels      ( NumChannels ),
      .NumLanes         ( NumLanes    ),
      .MaxClkDiv        ( MaxClkDiv   ),
      .MaxAxiChannelBits(MaxAxiChannelBits)
    ) i_serial_link (
      .clk_i          ( clk_i             ),
      .rst_ni         ( rst_ni            ),
      .clk_sl_i       ( clk_i             ),
      .rst_sl_ni      ( rst_serial_link_n ),
      .clk_reg_i      ( clk_i             ),
      .rst_reg_ni     ( rst_reg_ni        ),
      .testmode_i     ( 1'b0              ),
      .axi_in_req_i   ( axi_in_req_i      ),
      .axi_in_rsp_o   ( axi_in_rsp_o      ),//axi_in_rsp_o      ),
      .axi_out_req_o  ( fast_sl_req_O     ),
      .axi_out_rsp_i  ( fast_sl_rsp_O     ),
      // .axi_out_req_o  ( axi_out_req_o     ),
      // .axi_out_rsp_i  ( axi_out_rsp_i     ),

      .cfg_req_i      ( cfg_req_i         ),
      .cfg_rsp_o      ( cfg_rsp_o         ),//cfg_rsp_o         ),
      .ddr_rcv_clk_i  ( ddr_rcv_clk_i     ),
      .ddr_rcv_clk_o  ( ddr_rcv_clk_o     ),
      .ddr_i          ( ddr_i             ),
      .ddr_o          ( ddr_o             ),
      .isolated_i     (   2'b0            ), //2'b0
      .isolate_o      (                   ), //2'b0
      .clk_ena_o      ( clk_ena           ),
      .reset_no       ( reset_n           )

      

    );
  end else begin : gen_single_channel_serial_link
    serial_link #(
      .axi_req_t        ( axi_req_t   ),
      .axi_rsp_t        ( axi_rsp_t   ),
      .aw_chan_t        ( aw_chan_t   ),
      .w_chan_t         ( w_chan_t    ),
      .b_chan_t         ( b_chan_t    ),
      .ar_chan_t        ( ar_chan_t   ),
      .r_chan_t         ( r_chan_t    ),
      .cfg_req_t        ( cfg_req_t   ),
      .cfg_rsp_t        ( cfg_rsp_t   ),
      .hw2reg_t         ( serial_link_single_channel_reg_pkg::serial_link_single_channel_hw2reg_t ),
      .reg2hw_t         ( serial_link_single_channel_reg_pkg::serial_link_single_channel_reg2hw_t ),
      .NumChannels      ( NumChannels ),
      .NumLanes         ( NumLanes    ),
      .MaxClkDiv        ( MaxClkDiv   ),
      .MaxAxiChannelBits(MaxAxiChannelBits)
    ) i_serial_link (
      .clk_i          ( clk_i             ),
      .rst_ni         ( rst_ni            ),
      .clk_sl_i       ( clk_i             ),
      .rst_sl_ni      ( rst_serial_link_n ),
      .clk_reg_i      ( clk_i             ),
      .rst_reg_ni     ( rst_reg_ni        ),
      .testmode_i     ( 1'b0              ),
      .axi_in_req_i   ( axi_in_req_i      ),
      .axi_in_rsp_o   ( axi_in_rsp_o      ),//axi_in_rsp_o      ),
      .axi_out_req_o  ( fast_sl_req_O     ),
      .axi_out_rsp_i  ( fast_sl_rsp_O     ),
      // .axi_out_req_o  ( axi_out_req_o     ),
      // .axi_out_rsp_i  ( axi_out_rsp_i     ),
      .cfg_req_i      ( cfg_req_i         ),
      .cfg_rsp_o      ( cfg_rsp_o         ),//cfg_rsp_o         ),
      .ddr_rcv_clk_i  ( ddr_rcv_clk_i     ),
      .ddr_rcv_clk_o  ( ddr_rcv_clk_o     ),
      .ddr_i          ( ddr_i             ),
      .ddr_o          ( ddr_o             ),
      .isolated_i     ( 2'b0              ),
      .isolate_o      (                   ),
      .clk_ena_o      ( clk_ena           ),
      .reset_no       ( reset_n           )
    );
  end

endmodule



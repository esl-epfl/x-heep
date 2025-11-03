


package serial_link_minimum_axi_pkg;
  `include "axi/typedef.svh"
  parameter int unsigned AXI_ADDR_WIDTH = 1;  //original 32
  parameter int unsigned AXI_DATA_WIDTH = 32;
  parameter int unsigned AXI_USER_WIDTH = 1;  //8
  parameter int unsigned AXI_ID_WIDTH = 1;  //8

  parameter int unsigned AxiDataWidth = 32;
  // parameter int unsigned RegDataWidth = 32;
  // parameter int unsigned StreamDataBytes = 32;
  localparam int unsigned AxiStrbWidth = AxiDataWidth / 8;
  // localparam int unsigned RegStrbWidth = RegDataWidth / 8;
  // typedef logic [StreamDataBytes*8-1:0] tdata_t;
  // typedef logic [StreamDataBytes-1:0] tstrb_t;
  // typedef logic [StreamDataBytes-1:0] tkeep_t;
  // typedef logic tlast_t;
  // typedef logic id_t;
  // typedef logic tdest_t;
  // typedef logic tuser_t;
  // typedef logic tready_t;
  typedef logic [AXI_ADDR_WIDTH-1:0] axi_addr_t;
  typedef logic [AXI_DATA_WIDTH-1:0] axi_data_t;
  typedef logic [AxiStrbWidth-1:0] axi_strb_t;
  typedef logic [AXI_USER_WIDTH-1:0] axi_user_t;
  typedef logic [AXI_ID_WIDTH-1:0] axi_id_t;

  `AXI_TYPEDEF_AW_CHAN_T(axi_aw_t, axi_addr_t, axi_id_t, axi_user_t)  //
  `AXI_TYPEDEF_W_CHAN_T(axi_w_t, axi_data_t, axi_strb_t, axi_user_t)
  `AXI_TYPEDEF_B_CHAN_T(axi_b_t, axi_id_t, axi_user_t)
  `AXI_TYPEDEF_AR_CHAN_T(axi_ar_t, axi_addr_t, axi_id_t, axi_user_t)  //
  `AXI_TYPEDEF_R_CHAN_T(axi_r_t, axi_data_t, axi_id_t, axi_user_t)

  `AXI_TYPEDEF_REQ_T(axi_req_t, axi_aw_t, axi_w_t, axi_ar_t)  //
  `AXI_TYPEDEF_RESP_T(axi_resp_t, axi_b_t, axi_r_t)

  parameter int unsigned AW_CH_SIZE = 35 + AXI_ADDR_WIDTH + AXI_ID_WIDTH + AXI_USER_WIDTH;  // 35 + addr_t + id_t + user_t
  parameter int unsigned W_CH_SIZE = 1 + AXI_DATA_WIDTH + AxiStrbWidth + AXI_USER_WIDTH;    // 1 + data_t + strb_t + user_t
  parameter int unsigned B_CH_SIZE = 2 + AXI_ID_WIDTH + AXI_USER_WIDTH;  // 2 + id_t + user_t
  parameter int unsigned AR_CH_SIZE = 29 + AXI_ADDR_WIDTH + AXI_ID_WIDTH + AXI_USER_WIDTH;  // 29 + addr_t + id_t + user_t
  parameter int unsigned R_CH_SIZE = 3 + AXI_DATA_WIDTH + AXI_ID_WIDTH + AXI_USER_WIDTH;    // 3 + data_t + id_t + user_t

endpackage

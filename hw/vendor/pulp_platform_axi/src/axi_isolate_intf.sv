`include "axi/assign.svh"

/// Interface variant of [`axi_isolate`](module.axi_isolate).
///
/// See the documentation of the main module for the definition of ports and parameters.
module axi_isolate_intf #(
  parameter int unsigned NUM_PENDING    = 32'd16,
  parameter bit TERMINATE_TRANSACTION   = 1'b0,
  parameter bit ATOP_SUPPORT            = 1'b1,
  parameter int unsigned AXI_ID_WIDTH   = 32'd0,
  parameter int unsigned AXI_ADDR_WIDTH = 32'd0,
  parameter int unsigned AXI_DATA_WIDTH = 32'd0,
  parameter int unsigned AXI_USER_WIDTH = 32'd0
) (
  input  logic   clk_i,
  input  logic   rst_ni,
  AXI_BUS.Slave  slv,
  AXI_BUS.Master mst,
  input  logic   isolate_i,
  output logic   isolated_o
);
  typedef logic [AXI_ID_WIDTH-1:0]     id_t;
  typedef logic [AXI_ADDR_WIDTH-1:0]   addr_t;
  typedef logic [AXI_DATA_WIDTH-1:0]   data_t;
  typedef logic [AXI_DATA_WIDTH/8-1:0] strb_t;
  typedef logic [AXI_USER_WIDTH-1:0]   user_t;

  `AXI_TYPEDEF_AW_CHAN_T(aw_chan_t, addr_t, id_t, user_t)
  `AXI_TYPEDEF_W_CHAN_T(w_chan_t, data_t, strb_t, user_t)
  `AXI_TYPEDEF_B_CHAN_T(b_chan_t, id_t, user_t)

  `AXI_TYPEDEF_AR_CHAN_T(ar_chan_t, addr_t, id_t, user_t)
  `AXI_TYPEDEF_R_CHAN_T(r_chan_t, data_t, id_t, user_t)

  `AXI_TYPEDEF_REQ_T(axi_req_t, aw_chan_t, w_chan_t, ar_chan_t)
  `AXI_TYPEDEF_RESP_T(axi_resp_t, b_chan_t, r_chan_t)

  axi_req_t  slv_req,  mst_req;
  axi_resp_t slv_resp, mst_resp;

  `AXI_ASSIGN_TO_REQ(slv_req, slv)
  `AXI_ASSIGN_FROM_RESP(slv, slv_resp)

  `AXI_ASSIGN_FROM_REQ(mst, mst_req)
  `AXI_ASSIGN_TO_RESP(mst_resp, mst)

  axi_isolate #(
    .NumPending           ( NUM_PENDING           ),
    .TerminateTransaction ( TERMINATE_TRANSACTION ),
    .AtopSupport          ( ATOP_SUPPORT          ),
    .AxiAddrWidth         ( AXI_ADDR_WIDTH        ),
    .AxiDataWidth         ( AXI_DATA_WIDTH        ),
    .AxiIdWidth           ( AXI_ID_WIDTH          ),
    .AxiUserWidth         ( AXI_USER_WIDTH        ),
    .axi_req_t            ( axi_req_t             ),
    .axi_resp_t           ( axi_resp_t            )
  ) i_axi_isolate (
    .clk_i,
    .rst_ni,
    .slv_req_i  ( slv_req  ),
    .slv_resp_o ( slv_resp ),
    .mst_req_o  ( mst_req  ),
    .mst_resp_i ( mst_resp ),
    .isolate_i,
    .isolated_o
  );

  // pragma translate_off
  `ifndef VERILATOR
  initial begin
    assume (AXI_ID_WIDTH   > 0) else $fatal(1, "AXI_ID_WIDTH   has to be > 0.");
    assume (AXI_ADDR_WIDTH > 0) else $fatal(1, "AXI_ADDR_WIDTH has to be > 0.");
    assume (AXI_DATA_WIDTH > 0) else $fatal(1, "AXI_DATA_WIDTH has to be > 0.");
    assume (AXI_USER_WIDTH > 0) else $fatal(1, "AXI_USER_WIDTH has to be > 0.");
  end
  `endif
  // pragma translate_on
endmodule


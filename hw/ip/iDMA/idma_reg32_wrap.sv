
`include "./include/axi/assign.svh"
`include "./include/axi/typedef.svh"
`include "./include/idma/typedef.svh"
// `include "register_interface/typedef.svh"

module dma_reg32_wrap #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic,
    parameter type obi_req_t = logic,
    parameter type obi_resp_t = logic,
    parameter int unsigned OBI_DATA_WIDTH = -1,
    parameter int unsigned OBI_ADDR_WIDTH = -1,
    parameter int unsigned OBI_USER_WIDTH = -1,
    parameter int unsigned OBI_ID_WIDTH = -1

) (
    input logic clk_i,  // Clock
    input logic rst_ni, // Asynchronous reset active low

    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    output obi_req_t  dma_master0_ch0_req_o,
    input  obi_resp_t dma_master0_ch0_resp_i,

    output obi_req_t  dma_master1_ch0_req_o,
    input  obi_resp_t dma_master1_ch0_resp_i,

    input logic spi_rx_valid_i,
    input logic spi_tx_ready_i,
    input logic spi_flash_rx_valid_i,
    input logic spi_flash_tx_ready_i,

    output logic dma_intr_o

);
  localparam int unsigned OBI_SLV_ID_WIDTH = OBI_ID_WIDTH;
  typedef logic [OBI_ADDR_WIDTH-1:0] addr_t;
  typedef logic [OBI_DATA_WIDTH-1:0] data_t;
  typedef logic [(OBI_DATA_WIDTH/8)-1:0] strb_t;
  typedef logic [OBI_USER_WIDTH-1:0] user_t;
  typedef logic [OBI_ID_WIDTH-1:0] axi_id_t;
  typedef logic [OBI_SLV_ID_WIDTH-1:0] axi_slv_id_t;

  `AXI_TYPEDEF_ALL(axi_mst, addr_t, axi_id_t, data_t, strb_t, user_t)
  axi_mst_req_t  axi_mst_req;
  axi_mst_resp_t axi_mst_resp;

  localparam int unsigned TFLenWidth = OBI_ADDR_WIDTH;
  typedef logic [TFLenWidth-1:0] tf_len_t;

  `IDMA_TYPEDEF_FULL_REQ_T(idma_req_t, axi_slv_id_t, addr_t, tf_len_t)
  `IDMA_TYPEDEF_FULL_RSP_T(idma_rsp_t, addr_t)
  idma_req_t idma_req;

  logic valid, ready, be_trans_complete;
  idma_pkg::idma_busy_t idma_busy;


  // Frontend
  idma_reg32_frontend #(
      .DMAAddrWidth  (OBI_ADDR_WIDTH),
      .dma_regs_req_t(reg_req_t),
      .dma_regs_rsp_t(reg_rsp_t),
      .idma_req_t    (idma_req_t)
  ) i_idma_reg32_frontend (
      .clk_i,
      .rst_ni,
      .dma_ctrl_req_i  (reg_req_i),
      .dma_ctrl_rsp_o  (reg_rsp_o),
      .idma_req_o      (idma_req),
      .valid_o         (valid),
      .ready_i         (ready),
      .backend_idle_i  (~|idma_busy),
      .trans_complete_i(be_trans_complete)
  );
  assign dma_intr_o = be_trans_complete;
  // Backend
  idma_backend #(
      .DataWidth          (OBI_DATA_WIDTH),
      .AddrWidth          (OBI_ADDR_WIDTH),
      .UserWidth          (OBI_USER_WIDTH),
      .AxiIdWidth         (OBI_ID_WIDTH),
      .NumAxInFlight      (4),
      .BufferDepth        (3),
      .TFLenWidth         (TFLenWidth),
      .RAWCouplingAvail   (1'b1),
      .MaskInvalidData    (1'b1),
      .HardwareLegalizer  (1'b1),
      .RejectZeroTransfers(1'b1),
      .MemSysDepth        (32'd1),
      .ErrorCap           (idma_pkg::NO_ERROR_HANDLING),
      .idma_req_t         (idma_req_t),
      .idma_rsp_t         (idma_rsp_t),
      .idma_eh_req_t      (idma_pkg::idma_eh_req_t),
      .idma_busy_t        (idma_pkg::idma_busy_t),
      .axi_req_t          (axi_mst_req_t),
      .axi_rsp_t          (axi_mst_resp_t)
  ) i_idma_backend (
      .clk_i,
      .rst_ni,
      .testmode_i('0),

      .idma_req_i (idma_req),
      .req_valid_i(valid),
      .req_ready_o(ready),

      .idma_rsp_o (),
      .rsp_valid_o(be_trans_complete),
      .rsp_ready_i(1'b1),

      .idma_eh_req_i ('0),
      .eh_req_valid_i(1'b1),
      .eh_req_ready_o(),

      .axi_req_o(axi_mst_req),
      .axi_rsp_i(axi_mst_resp),
      .busy_o   (idma_busy)
  );

  // AXI to OBI 
  axi_to_mem_split #(
      .axi_req_t   (axi_mst_req_t),
      .axi_resp_t  (axi_mst_resp_t),
      .AddrWidth   (OBI_ADDR_WIDTH),
      .AxiDataWidth(OBI_DATA_WIDTH),
      .IdWidth     (OBI_ID_WIDTH),
      .MemDataWidth(OBI_DATA_WIDTH),
      .BufDepth    (2),
      .HideStrb    (1'b1)
  ) i_axi_to_mem_1 (
      .clk_i,
      .rst_ni,

      .mem_req_o   ({dma_master1_ch0_req_o.req, dma_master0_ch0_req_o.req}),
      .mem_gnt_i   ({dma_master1_ch0_resp_i.gnt, dma_master0_ch0_resp_i.gnt}),
      .mem_addr_o  ({dma_master1_ch0_req_o.addr, dma_master0_ch0_req_o.addr}),
      .mem_wdata_o ({dma_master1_ch0_req_o.wdata, dma_master0_ch0_req_o.wdata}),
      .mem_strb_o  ({dma_master1_ch0_req_o.be, dma_master0_ch0_req_o.be}),
      .mem_atop_o  (),
      .mem_we_o    ({dma_master1_ch0_req_o.we, dma_master0_ch0_req_o.we}),
      .mem_rvalid_i({dma_master1_ch0_resp_i.rvalid, dma_master0_ch0_resp_i.rvalid}),
      .mem_rdata_i ({dma_master1_ch0_resp_i.rdata, dma_master0_ch0_resp_i.rdata}),


      .axi_req_i (axi_mst_req),
      .axi_resp_o(axi_mst_resp),
      .busy_o    ()
  );

endmodule : dma_reg32_wrap

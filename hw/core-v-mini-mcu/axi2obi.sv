module axi2obi #(
    // RISC-V interface parameters
    parameter WordSize = 32,
    parameter AddrSize = 32,

    // Parameters of Axi Slave Bus Interface S00_AXI
    parameter C_S00_AXI_DATA_WIDTH = 32,
    parameter C_S00_AXI_ADDR_WIDTH = 32

) (

    // RISC-V interface ports
    input logic gnt_i,  // granted that serial link can access the bus 
    //input  logic                             rvalid_i,
    //output logic                             we_o,
    //output logic [                      3:0] be_o,
    //output logic [C_S00_AXI_ADDR_WIDTH -1:0] addr_o,
    //output logic [C_S00_AXI_DATA_WIDTH -1:0] wdata_o,   //output logic [WordSize-1:0] wdata_i,
    //input  logic [C_S00_AXI_DATA_WIDTH -1:0] rdata_i,
    //output logic                             req_o,


    // Clock and Reset
    //input logic                           clk_i,
    //input logic                           rst_ni,

    input  logic                            data_req_i,
    output logic                            data_gnt_o,
    output logic                            data_rvalid_o,
    input  logic [C_S00_AXI_ADDR_WIDTH-1:0] data_addr_i,
    input  logic                            data_we_i,
    input  logic [                     3:0] data_be_i,
    output logic [                    31:0] data_rdata_o,
    input  logic [                    31:0] data_wdata_i,


    // Ports of Axi Slave Bus Interface S00_AXI

    // Clk and rst signals

    input logic s00_axi_aclk,
    input logic s00_axi_aresetn,

    // Read address channel signals

    input  logic [C_S00_AXI_ADDR_WIDTH-1:0] s00_axi_araddr,
    input  logic                            s00_axi_arvalid,
    output logic                            s00_axi_arready,
    input  logic [                     2:0] s00_axi_arprot,


    // Read data channel signals

    output logic [C_S00_AXI_DATA_WIDTH-1:0] s00_axi_rdata,
    output logic [                     1:0] s00_axi_rresp,
    output logic                            s00_axi_rvalid,
    input  logic                            s00_axi_rready,

    // Write address channel signals

    input  logic [C_S00_AXI_ADDR_WIDTH-1:0] s00_axi_awaddr,
    input  logic                            s00_axi_awvalid,
    output logic                            s00_axi_awready,
    input  logic [                     2:0] s00_axi_awprot,


    // Write data channel signals

    input  logic [  C_S00_AXI_DATA_WIDTH-1:0] s00_axi_wdata,
    input  logic                              s00_axi_wvalid,
    output logic                              s00_axi_wready,
    input  logic [(C_S00_AXI_DATA_WIDTH/8):0] s00_axi_wstrb,

    // Write response channel signals

    output logic [1:0] s00_axi_bresp,
    output logic       s00_axi_bvalid,
    input  logic       s00_axi_bready

);
  enum logic [2:0] {
    IDLE,
    WRITE_REQ_SL,
    WRITE_REQ_BUFFER,     // address phase OBI
    WRITE_REQ_HANDSHAKE,  // data phase OBI
    READ_REQ_SL,
    READ_REQ_BUFFER,      // address phase OBI
    READ_REQ_HANDSHAKE    // data phase OBI
  }
      CS, NS;

  logic [C_S00_AXI_ADDR_WIDTH-1:0] curr_addr;
  logic [C_S00_AXI_ADDR_WIDTH-1:0] next_addr;

  logic [C_S00_AXI_DATA_WIDTH-1:0] curr_wdata;
  logic [C_S00_AXI_DATA_WIDTH-1:0] next_wdata;

  logic [C_S00_AXI_DATA_WIDTH-1:0] curr_rdata;
  logic [C_S00_AXI_DATA_WIDTH-1:0] next_rdata;








  assign s00_axi_rresp = '0;
  assign s00_axi_bresp = '0;

  //(clk) begin
  always @(posedge s00_axi_aclk or negedge s00_axi_aresetn) begin : FSM_SEQ
    if (!s00_axi_aresetn) begin
      CS <= IDLE;
    end else begin
      CS <= NS;
    end
  end


  //always @(clk) begin
  always @(posedge s00_axi_aclk or negedge s00_axi_aresetn) begin
    if (!s00_axi_aresetn) begin
      curr_addr  = '0;
      curr_wdata = '0;
      curr_rdata = '0;
    end else begin
      curr_addr  = next_addr;
      curr_wdata = next_wdata;
      curr_rdata = next_rdata;
    end
  end

  //end
  //assign addr_o = curr_addr;

  assign s00_axi_rdata = curr_rdata;

  always_comb begin
    NS = CS;
    next_addr = curr_addr;
    next_wdata = curr_wdata;
    next_rdata = curr_rdata;

    //we_o = '0;
    //be_o = '0;
    //req_o = '0;
    data_gnt_o = '0;
    data_rvalid_o = '0;
    data_rdata_o = '0;
    s00_axi_arready = '0;
    s00_axi_rvalid = '0;
    s00_axi_awready = '0;
    s00_axi_wready = '0;
    s00_axi_bvalid = '0;

    case (CS)
      // wait for a request to come in from the serial link
      IDLE: begin
        if (s00_axi_arvalid) begin
          NS = READ_REQ_SL;
          next_addr = s00_axi_araddr;
          s00_axi_arready = '1;
          data_gnt_o = '1;
        end else if (s00_axi_awvalid == '1 && s00_axi_wvalid == '0) begin
          next_addr = s00_axi_awaddr;
          s00_axi_awready = '1;
          NS = WRITE_REQ_SL;
        end else begin
          NS = IDLE;
        end
      end
      WRITE_REQ_SL: begin
        if (s00_axi_wvalid == '1) begin
          NS = WRITE_REQ_HANDSHAKE;
          next_rdata = s00_axi_wdata;



        end else begin
          NS = WRITE_REQ_SL;
        end
      end
      WRITE_REQ_HANDSHAKE: begin
        if (s00_axi_wvalid == '1 && data_req_i == '1) begin
          NS = WRITE_REQ_BUFFER;
          data_gnt_o = '1;
        end else begin
          NS = WRITE_REQ_HANDSHAKE;
        end
      end
      WRITE_REQ_BUFFER: begin
        if (s00_axi_wvalid == '1 && data_we_i == '0) begin
          data_rvalid_o = '1;
          s00_axi_wready = '1;
          data_rdata_o = curr_rdata;
          NS = IDLE;
        end else begin
          NS = WRITE_REQ_BUFFER;
        end
      end
      READ_REQ_SL: begin
        //if (data_req_i == '1 && data_we_i == '1) begin //&& data_req_i == '1 && data_we_i == '1 in  case we have OBI request

        NS = READ_REQ_BUFFER;
        next_rdata = 00000001;
        s00_axi_rvalid = '1;
        //end
      end
      READ_REQ_BUFFER: begin
        //if (data_req_i == '1 && data_we_i == '1) begin //&& data_req_i == '1 && data_we_i == '1 in  case we have OBI request

        NS = IDLE;
        next_rdata = 00000001;
        s00_axi_rvalid = '1;
        //end
      end

    endcase
  end



endmodule

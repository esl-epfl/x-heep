module axi2obi #(
    // RISC-V interface parameters
    parameter WordSize = 32,
    parameter AddrSize = 32,

    // Parameters of Axi Slave Bus Interface S00_AXI
    parameter C_S00_AXI_DATA_WIDTH = 32,
    parameter C_S00_AXI_ADDR_WIDTH = 32

) (

    // RISC-V interface ports
    input  logic                             gnt_i,
    input  logic                             rvalid_i,
    output logic                             we_o,
    output logic [                      3:0] be_o,
    output logic [C_S00_AXI_ADDR_WIDTH -1:0] addr_o,
    output logic [C_S00_AXI_DATA_WIDTH -1:0] wdata_o,   //output logic [WordSize-1:0] wdata_i,
    input  logic [C_S00_AXI_DATA_WIDTH -1:0] rdata_i,
    output logic                             req_o,


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
    READ1,
    READ2,
    READ3,
    WRITE1,
    WRITE2,
    WRITE3
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
  always @(posedge s00_axi_aclk) begin
    if (!s00_axi_aresetn) begin
      assign curr_addr = '0;
      assign curr_wdata = '0;
      assign curr_rdata = '0;
    end else begin
      assign curr_addr = next_addr;
      assign curr_wdata = next_wdata;
      assign curr_rdata = next_rdata;
    end

  end
  //end
  assign addr_o = curr_addr;
  assign wdata_o = curr_wdata;
  assign s00_axi_rdata = curr_rdata;

  always_comb begin
    NS = CS;
    next_addr = curr_addr;
    next_wdata = curr_wdata;
    next_rdata = curr_rdata;

    we_o = '0;
    be_o = '0;
    req_o = '0;

    s00_axi_arready = '0;
    s00_axi_rvalid = '0;
    s00_axi_awready = '0;
    s00_axi_wready = '0;
    s00_axi_bvalid = '0;

    case (CS)
      // wait for a request to come in from the serial link
      IDLE: begin
        if (s00_axi_arvalid) begin
          assign NS = READ1;
          assign next_addr = s00_axi_araddr;
          assign s00_axi_arready = '1;
        end else if (s00_axi_awvalid == '1 & s00_axi_wvalid == '1) begin
          assign next_addr = s00_axi_awaddr;
          assign next_wdata = s00_axi_wdata;
          assign s00_axi_awready = '1;
          assign s00_axi_wready = '1;
          assign NS = WRITE1;
        end else begin
          assign NS = IDLE;
        end
      end
      READ1: begin
        req_o = '1;
        we_o  = '0;
        be_o  = '1;
        if (gnt_i) begin
          assign NS = READ2;
        end else begin
          assign NS = READ1;
        end

      end
      READ2: begin
        req_o = '0;
        we_o  = '0;
        be_o  = '1;

        if (rvalid_i) begin
          assign next_rdata = rdata_i;
          assign NS = READ3;
        end else begin
          assign NS = READ2;
        end
      end
      READ3: begin
        s00_axi_rvalid = '1;

        if (s00_axi_rready) begin
          assign NS = IDLE;
        end else begin
          assign NS = READ3;
        end
      end
      WRITE1: begin
        req_o = '1;
        we_o  = '1;
        be_o  = '1;

        if (gnt_i) begin
          assign NS = WRITE2;
        end else begin
          assign NS = WRITE1;
        end
      end
      WRITE2: begin
        req_o = '0;
        we_o  = '1;
        be_o  = '1;

        if (rvalid_i) begin
          assign NS = WRITE3;
        end else begin
          assign NS = WRITE2;
        end
      end
      WRITE3: begin
        assign s00_axi_bvalid = '1;
        if (s00_axi_bready) begin  //if(s00_axi_bready='1')
          assign NS = IDLE;
        end else begin
          assign NS = WRITE3;
        end
      end


    endcase
  end


endmodule

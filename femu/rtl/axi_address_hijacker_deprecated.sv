module axi_address_hijacker #(
    parameter AXI_ADDR_WIDTH = 32,
    parameter AXI_ADDR_WIDTH_SLAVE = 5,
    parameter AXI_DATA_WIDTH = 32,
    parameter C_NUM_REGISTERS = 5
) (
    // input write and read address coming from the axi_spi_slave
    input logic [AXI_ADDR_WIDTH-1:0] axi_master_awaddr_in,
    input logic [AXI_ADDR_WIDTH-1:0] axi_master_araddr_in,

    // output write and read address by adding fixed offset 
    output logic [AXI_ADDR_WIDTH-1:0] axi_master_araddr_out,
    output logic [AXI_ADDR_WIDTH-1:0] axi_master_awaddr_out,

    input logic s00_axi_aclk,
    input logic s00_axi_aresetn,
    input logic [AXI_ADDR_WIDTH_SLAVE - 1 : 0] s00_axi_awaddr,
    input logic s00_axi_awvalid,
    output logic s00_axi_awready,
    input logic [AXI_DATA_WIDTH - 1 : 0] s00_axi_wdata,
    input logic s00_axi_wvalid,
    output logic s00_axi_wready,
    output logic s00_axi_bvalid,
    input logic s00_axi_bready,
    input logic [AXI_ADDR_WIDTH_SLAVE - 1 : 0] s00_axi_araddr,
    input logic s00_axi_arvalid,
    output logic s00_axi_arready,
    output logic [AXI_DATA_WIDTH - 1 : 0] s00_axi_rdata,
    output logic s00_axi_rvalid,
    input logic s00_axi_rready,
    output logic [1:0] s00_axi_rresp,
    output logic [1:0] s00_axi_bresp
);

  enum logic [2:0] {
    Idle,
    Read,
    Write,
    Wait_Ready,
    Wait_BReady,
    Check,
    Finish
  } current_state;
  logic [AXI_ADDR_WIDTH_SLAVE - 1 - 2 : 0] aread, awrite;
  logic [31 : 0] register_file_internal[C_NUM_REGISTERS - 1 : 0];

  assign aread = s00_axi_araddr[AXI_ADDR_WIDTH_SLAVE-1 : 2];
  assign awrite = s00_axi_awaddr[AXI_ADDR_WIDTH_SLAVE-1 : 2];

  assign s00_axi_rresp = '0;
  assign s00_axi_bresp = '0;

  integer i;

  always @(posedge s00_axi_aclk or negedge s00_axi_aresetn) begin
    s00_axi_awready <= 0;
    s00_axi_arready <= 0;
    s00_axi_wready  <= 0;
    s00_axi_rvalid  <= 0;
    s00_axi_bvalid  <= 0;
    if (!s00_axi_aresetn) begin
      for (i = 0; i < C_NUM_REGISTERS; i = i + 1) register_file_internal[i] <= '0;
      s00_axi_rdata <= '0;
      current_state <= Idle;
    end else begin
      case (current_state)
        Idle: begin
          s00_axi_bvalid <= 0;
          if (s00_axi_awvalid == 1) begin
            current_state   <= Write;
            s00_axi_awready <= 1;
          end else if (s00_axi_arvalid == 1) begin  // add all the required conditionstion
            current_state   <= Read;
            s00_axi_arready <= 1;
          end
        end
        Write: begin
          s00_axi_wready <= 1;
          if (s00_axi_wvalid == 1) begin
            current_state <= Finish;
            register_file_internal[awrite] <= s00_axi_wdata;
          end
        end
        Finish: begin
          s00_axi_wready <= 0;
          s00_axi_bvalid <= 1;
          if (s00_axi_bready == 1) begin
            current_state <= Idle;
          end
        end
        Read: begin
          s00_axi_rvalid <= 1;
          s00_axi_rdata  <= register_file_internal[aread];
          if (s00_axi_rready == 1) begin
            current_state <= Idle;
          end
        end
        default: begin

        end
      endcase
    end
  end

  assign axi_master_araddr_out = axi_master_araddr_in + register_file_internal[0];
  assign axi_master_awaddr_out = axi_master_awaddr_in + register_file_internal[0];

endmodule

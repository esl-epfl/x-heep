`timescale 1ns / 100ps


module tb_axi_address_hijacker;

  parameter AXI_ADDR_WIDTH = 32;
  parameter AXI_ADDR_WIDTH_SLAVE = 5;
  parameter AXI_DATA_WIDTH = 32;
  parameter C_NUM_REGISTERS = 5;

  logic [AXI_ADDR_WIDTH-1:0] axi_master_awaddr_in;
  logic [AXI_ADDR_WIDTH-1:0] axi_master_araddr_in;

  // output write and read address by adding fixed offset 
  logic [AXI_ADDR_WIDTH-1:0] axi_master_araddr_out;
  logic [AXI_ADDR_WIDTH-1:0] axi_master_awaddr_out;

  logic s00_axi_aclk = 0;
  logic s00_axi_aresetn;
  logic [AXI_ADDR_WIDTH_SLAVE - 1 : 0] s00_axi_awaddr;
  logic s00_axi_awvalid;
  logic s00_axi_awready;
  logic [AXI_DATA_WIDTH - 1 : 0] s00_axi_wdata;
  logic s00_axi_wvalid;
  logic s00_axi_wready;
  logic s00_axi_bvalid;
  logic s00_axi_bready;
  logic [AXI_ADDR_WIDTH_SLAVE - 1 : 0] s00_axi_araddr;
  logic s00_axi_arvalid;
  logic s00_axi_arready;
  logic [AXI_DATA_WIDTH - 1 : 0] s00_axi_rdata;
  logic s00_axi_rvalid;
  logic s00_axi_rready;

  parameter AXI_ACLK_HALF_PERIOD = 5;

  always #AXI_ACLK_HALF_PERIOD s00_axi_aclk <= ~s00_axi_aclk;

  axi_address_hijacker #(
      .AXI_ADDR_WIDTH(AXI_ADDR_WIDTH),
      .AXI_ADDR_WIDTH_SLAVE(AXI_ADDR_WIDTH_SLAVE),
      .AXI_DATA_WIDTH(AXI_DATA_WIDTH),
      .C_NUM_REGISTERS(C_NUM_REGISTERS)
  ) uut (
      .axi_master_awaddr_in(axi_master_awaddr_in),
      .axi_master_araddr_in(axi_master_araddr_in),

      // output write and read address by adding fixed offset 
      .axi_master_araddr_out(axi_master_araddr_out),
      .axi_master_awaddr_out(axi_master_awaddr_out),

      .s00_axi_aclk(s00_axi_aclk),
      .s00_axi_aresetn(s00_axi_aresetn),
      .s00_axi_awaddr(s00_axi_awaddr),
      .s00_axi_awvalid(s00_axi_awvalid),
      .s00_axi_awready(s00_axi_awready),
      .s00_axi_wdata(s00_axi_wdata),
      .s00_axi_wvalid(s00_axi_wvalid),
      .s00_axi_wready(s00_axi_wready),
      .s00_axi_bvalid(s00_axi_bvalid),
      .s00_axi_bready(s00_axi_bready),
      .s00_axi_araddr(s00_axi_araddr),
      .s00_axi_arvalid(s00_axi_arvalid),
      .s00_axi_arready(s00_axi_arready),
      .s00_axi_rdata(s00_axi_rdata),
      .s00_axi_rvalid(s00_axi_rvalid),
      .s00_axi_rready(s00_axi_rready)
  );

  task write_reg;
    input [31:0] data;
    begin
      s00_axi_awaddr  = 5'b00000;
      s00_axi_wdata   = 32'h00001000;
      s00_axi_awvalid = 1;
      s00_axi_wvalid  = 1;
      wait (s00_axi_awready == 1);
      s00_axi_awvalid = 0;
      wait (s00_axi_wready == 1);
      s00_axi_wvalid = 0;
      s00_axi_bready = 1;
      wait (s00_axi_bvalid == 1);
      #AXI_ACLK_HALF_PERIOD s00_axi_bready = 0;
    end
  endtask
  ;

  task read_reg;
    output [31:0] data;
    begin
      s00_axi_araddr  = 5'b00000;
      s00_axi_arvalid = 1;
      wait (s00_axi_arready == 1);
      s00_axi_arvalid = 0;
      s00_axi_rready  = 1;
      wait (s00_axi_rvalid == 1) data = s00_axi_rdata;
      s00_axi_rready = 0;
      #(4 * AXI_ACLK_HALF_PERIOD);
    end
  endtask
  ;

  logic [31:0] res;

  initial begin
    s00_axi_aresetn = 0;
    #20 s00_axi_aresetn = 1;
    write_reg(32'h1c1c1c1c);
    read_reg(res);
    #(10 * AXI_ACLK_HALF_PERIOD);
    $finish;
  end


endmodule
;

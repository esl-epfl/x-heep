`timescale 1ns / 100ps


module tb_axi_address_hijacker_and_spi_slave;

  parameter AXI_ADDR_WIDTH = 32;
  parameter AXI_ADDR_WIDTH_SLAVE = 5;
  parameter AXI_DATA_WIDTH = 32;
  parameter C_NUM_REGISTERS = 5;

  logic [AXI_ADDR_WIDTH-1:0] axi_master_awaddr_in;
  logic [AXI_ADDR_WIDTH-1:0] axi_master_araddr_in;

  // output write and read address by adding fixed offset 
  logic [AXI_ADDR_WIDTH-1:0] axi_master_araddr_out;
  logic [AXI_ADDR_WIDTH-1:0] axi_master_awaddr_out;

  logic s00_axi_aclk;
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


  parameter AXI_USER_WIDTH = 6;
  parameter AXI_ID_WIDTH = 3;
  parameter DUMMY_CYCLES = 3;

  logic                        test_mode;
  logic                        spi_sclk = 0;
  logic                        spi_cs = 1;
  logic                        spi_oen0_o;
  logic                        spi_oen1_o;
  logic                        spi_oen2_o;
  logic                        spi_oen3_o;
  logic                        spi_sdi0;
  logic                        spi_sdi1;
  logic                        spi_sdi2;
  logic                        spi_sdi3;
  logic                        spi_sdo0;
  logic                        spi_sdo1;
  logic                        spi_sdo2;
  logic                        spi_sdo3;

  // AXI4 MASTER
  //***************************************
  logic                        axi_aclk = 0;
  logic                        axi_aresetn = 1;
  // WRITE ADDRESS CHANNEL
  logic                        axi_master_aw_valid;
  logic [  AXI_ADDR_WIDTH-1:0] axi_master_aw_addr;
  logic [                 2:0] axi_master_aw_prot;
  logic [                 3:0] axi_master_aw_region;
  logic [                 7:0] axi_master_aw_len;
  logic [                 2:0] axi_master_aw_size;
  logic [                 1:0] axi_master_aw_burst;
  logic                        axi_master_aw_lock;
  logic [                 3:0] axi_master_aw_cache;
  logic [                 3:0] axi_master_aw_qos;
  logic [    AXI_ID_WIDTH-1:0] axi_master_aw_id;
  logic [  AXI_USER_WIDTH-1:0] axi_master_aw_user;
  logic                        axi_master_aw_ready;

  // READ ADDRESS CHANNEL
  logic                        axi_master_ar_valid;
  logic [  AXI_ADDR_WIDTH-1:0] axi_master_ar_addr;
  logic [                 2:0] axi_master_ar_prot;
  logic [                 3:0] axi_master_ar_region;
  logic [                 7:0] axi_master_ar_len;
  logic [                 2:0] axi_master_ar_size;
  logic [                 1:0] axi_master_ar_burst;
  logic                        axi_master_ar_lock;
  logic [                 3:0] axi_master_ar_cache;
  logic [                 3:0] axi_master_ar_qos;
  logic [    AXI_ID_WIDTH-1:0] axi_master_ar_id;
  logic [  AXI_USER_WIDTH-1:0] axi_master_ar_user;
  logic                        axi_master_ar_ready;

  // WRITE DATA CHANNEL
  logic                        axi_master_w_valid;
  logic [  AXI_DATA_WIDTH-1:0] axi_master_w_data;
  logic [AXI_DATA_WIDTH/8-1:0] axi_master_w_strb;
  logic [  AXI_USER_WIDTH-1:0] axi_master_w_user;
  logic                        axi_master_w_last;
  logic                        axi_master_w_ready;

  // READ DATA CHANNEL
  logic                        axi_master_r_valid;
  logic [  AXI_DATA_WIDTH-1:0] axi_master_r_data;
  logic [                 1:0] axi_master_r_resp;
  logic                        axi_master_r_last;
  logic [    AXI_ID_WIDTH-1:0] axi_master_r_id;
  logic [  AXI_USER_WIDTH-1:0] axi_master_r_user;
  logic                        axi_master_r_ready;

  // WRITE RESPONSE CHANNEL
  logic                        axi_master_b_valid;
  logic [                 1:0] axi_master_b_resp;
  logic [    AXI_ID_WIDTH-1:0] axi_master_b_id;
  logic [  AXI_USER_WIDTH-1:0] axi_master_b_user;
  logic                        axi_master_b_ready;

  axi_spi_slave #(
      .AXI_ADDR_WIDTH(AXI_ADDR_WIDTH),
      .AXI_DATA_WIDTH(AXI_DATA_WIDTH),
      .AXI_USER_WIDTH(AXI_USER_WIDTH),
      .AXI_ID_WIDTH  (AXI_ID_WIDTH),
      .DUMMY_CYCLES  (DUMMY_CYCLES)
  ) uut2 (
      .test_mode(test_mode),
      .spi_sclk(spi_sclk),
      .spi_cs(spi_cs),
      .spi_oen0_o(spi_oen0_o),
      .spi_oen1_o(spi_oen1_o),
      .spi_oen2_o(spi_oen2_o),
      .spi_oen3_o(spi_oen3_o),
      .spi_sdi0(spi_sdi0),
      .spi_sdi1(spi_sdi1),
      .spi_sdi2(spi_sdi2),
      .spi_sdi3(spi_sdi3),
      .spi_sdo0(spi_sdo0),
      .spi_sdo1(spi_sdo1),
      .spi_sdo2(spi_sdo2),
      .spi_sdo3(spi_sdo3),

      // AXI4 MASTER
      //***************************************
      .axi_aclk(axi_aclk),
      .axi_aresetn(axi_aresetn),
      // WRITE ADDRESS CHANNEL
      .axi_master_aw_valid(axi_master_aw_valid),
      .axi_master_aw_addr(axi_master_aw_addr),
      .axi_master_aw_prot(axi_master_aw_prot),
      .axi_master_aw_region(axi_master_aw_region),
      .axi_master_aw_len(axi_master_aw_len),
      .axi_master_aw_size(axi_master_aw_size),
      .axi_master_aw_burst(axi_master_aw_burst),
      .axi_master_aw_lock(axi_master_aw_lock),
      .axi_master_aw_cache(axi_master_aw_cache),
      .axi_master_aw_qos(axi_master_aw_qos),
      .axi_master_aw_id(axi_master_aw_id),
      .axi_master_aw_user(axi_master_aw_user),
      .axi_master_aw_ready(axi_master_aw_ready),

      // READ ADDRESS CHANNEL
      .axi_master_ar_valid(axi_master_ar_valid),
      .axi_master_ar_addr(axi_master_ar_addr),
      .axi_master_ar_prot(axi_master_ar_prot),
      .axi_master_ar_region(axi_master_ar_region),
      .axi_master_ar_len(axi_master_ar_len),
      .axi_master_ar_size(axi_master_ar_size),
      .axi_master_ar_burst(axi_master_ar_burst),
      .axi_master_ar_lock(axi_master_ar_lock),
      .axi_master_ar_cache(axi_master_ar_cache),
      .axi_master_ar_qos(axi_master_ar_qos),
      .axi_master_ar_id(axi_master_ar_id),
      .axi_master_ar_user(axi_master_ar_user),
      .axi_master_ar_ready(axi_master_ar_ready),

      // WRITE DATA CHANNEL
      .axi_master_w_valid(axi_master_w_valid),
      .axi_master_w_data (axi_master_w_data),
      .axi_master_w_strb (axi_master_w_strb),
      .axi_master_w_user (axi_master_w_user),
      .axi_master_w_last (axi_master_w_last),
      .axi_master_w_ready(axi_master_w_ready),

      // READ DATA CHANNEL
      .axi_master_r_valid(axi_master_r_valid),
      .axi_master_r_data(axi_master_r_data),
      .axi_master_r_resp(axi_master_r_resp),
      .axi_master_r_last(axi_master_r_last),
      .axi_master_r_id(axi_master_r_id),
      .axi_master_r_user(axi_master_r_user),
      .axi_master_r_ready(axi_master_r_ready),

      // WRITE RESPONSE CHANNEL
      .axi_master_b_valid(axi_master_b_valid),
      .axi_master_b_resp(axi_master_b_resp),
      .axi_master_b_id(axi_master_b_id),
      .axi_master_b_user(axi_master_b_user),
      .axi_master_b_ready(axi_master_b_ready)
  );

  logic enable_spi_clk = 0;
  parameter SPI_CLK_HALF_PERIOD = 10;
  parameter AXI_ACLK_HALF_PERIOD = 5;

  always #AXI_ACLK_HALF_PERIOD axi_aclk <= ~axi_aclk;

  assign s00_axi_aclk = axi_aclk;
  assign s00_axi_aresetn = axi_aresetn;

  always #SPI_CLK_HALF_PERIOD spi_sclk <= (~spi_sclk) & enable_spi_clk;


  always @(posedge axi_aclk) begin
    axi_master_ar_ready <= 0;
    axi_master_aw_ready <= 0;
    axi_master_w_ready  <= 0;
    axi_master_b_valid  <= 1;
    if (axi_master_ar_valid == 1) begin
      axi_master_ar_ready <= 1;
      axi_master_r_data   <= axi_master_araddr_out;
      axi_master_r_valid  <= 1;
    end
    if (axi_master_aw_valid == 1) axi_master_aw_ready <= 1;
    axi_master_w_ready <= 1;
    if (axi_master_r_ready == 1) begin
      axi_master_r_valid <= 0;
      axi_master_r_data  <= 32'h0;
    end
  end


  task write_test;
    begin
      spi_send8(8'h02, 1);
      spi_send32(32'h1f1f1f1f, 1);
      spi_send32(32'h00000000, 1);
      spi_send32(32'h11111111, 1);
      spi_send32(32'h22222222, 1);
      spi_send32(32'h33333333, 1);
      spi_send32(32'h44444444, 1);
      spi_send32(32'h55555555, 1);
      spi_send32(32'h66666666, 1);
      spi_send32(32'h77777777, 1);
      spi_send32(32'h88888888, 1);
      spi_send32(32'h99999999, 1);
      spi_send32(32'haaaaaaaa, 0);
    end
  endtask
  ;

  logic [31:0] res;


  task read_test;
    begin
      spi_send8(8'h0b, 1);
      spi_send32(32'h1f1f1f1f, 1);
      spi_send32(32'h00000000, 1);
      spi_read32(res, 1);
      spi_read32(res, 1);
      spi_read32(res, 1);
      spi_read32(res, 1);
      spi_read32(res, 1);
      spi_read32(res, 1);
      spi_read32(res, 1);
      spi_read32(res, 1);
      spi_read32(res, 1);
      spi_read32(res, 1);
      spi_read32(res, 1);
      spi_read32(res, 1);
      spi_read32(res, 0);
    end
  endtask
  ;

  task spi_read32;
    output [31:0] data_out;
    input keep_cs_low;
    begin
      integer k;
      spi_cs = 0;
      #(SPI_CLK_HALF_PERIOD) enable_spi_clk <= 1;
      #(SPI_CLK_HALF_PERIOD * 2);
      for (k = 31; k >= 0; k = k - 1) begin
        data_out[k] = spi_sdo1;
        #(SPI_CLK_HALF_PERIOD * 2);
        @(posedge spi_sclk);
      end
      enable_spi_clk = 0;
      if (keep_cs_low == 0) begin
        #(SPI_CLK_HALF_PERIOD * 2) spi_cs = 1;
      end
      #(SPI_CLK_HALF_PERIOD * 1);
    end
  endtask
  ;

  task spi_send8;
    input [7:0] data;
    input keep_cs_low;
    begin
      integer k;
      spi_cs = 0;
      #(SPI_CLK_HALF_PERIOD) enable_spi_clk <= 1;
      for (k = 7; k >= 0; k = k - 1) begin
        spi_sdi0 = data[k];
        #(SPI_CLK_HALF_PERIOD * 2);
        @(negedge spi_sclk);
      end
      enable_spi_clk = 0;
      if (keep_cs_low == 0) begin
        #(SPI_CLK_HALF_PERIOD * 2) spi_cs = 1;
      end
      #(SPI_CLK_HALF_PERIOD * 1);
    end
  endtask
  ;

  task spi_send16;
    input [15:0] data;
    input keep_cs_low;

    begin
      integer k;
      spi_cs = 0;
      #(SPI_CLK_HALF_PERIOD) enable_spi_clk <= 1;
      for (k = 15; k >= 0; k = k - 1) begin
        spi_sdi0 = data[k];
        #(SPI_CLK_HALF_PERIOD * 2);
        @(negedge spi_sclk);
      end
      enable_spi_clk = 0;
      if (keep_cs_low == 0) begin
        #(SPI_CLK_HALF_PERIOD * 2) spi_cs = 1;
      end
      #(SPI_CLK_HALF_PERIOD * 1);
    end
  endtask
  ;

  task spi_send32;
    input [31:0] data;
    input keep_cs_low;
    begin
      integer k;
      spi_cs = 0;
      #(SPI_CLK_HALF_PERIOD) enable_spi_clk <= 1;
      for (k = 31; k >= 0; k = k - 1) begin
        spi_sdi0 = data[k];
        #(SPI_CLK_HALF_PERIOD * 2);
        @(negedge spi_sclk);
      end
      enable_spi_clk = 0;
      if (keep_cs_low == 0) begin
        #(SPI_CLK_HALF_PERIOD * 2) spi_cs = 1;
      end
      #(SPI_CLK_HALF_PERIOD * 1);
    end
  endtask
  ;


  axi_address_hijacker #(
      .AXI_ADDR_WIDTH(AXI_ADDR_WIDTH),
      .AXI_ADDR_WIDTH_SLAVE(AXI_ADDR_WIDTH_SLAVE),
      .AXI_DATA_WIDTH(AXI_DATA_WIDTH),
      .C_NUM_REGISTERS(C_NUM_REGISTERS)
  ) uut (
      .axi_master_awaddr_in(axi_master_aw_addr),
      .axi_master_araddr_in(axi_master_ar_addr),

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


  logic [31:0] res2;
  integer i;

  initial begin
    axi_aresetn = 0;
    test_mode = 0;
    i = 0;
    #20 axi_aresetn = 1;
    write_reg(32'h1c1c1c1c);
    read_reg(res2);

    read_test();
    write_test();
    #(10 * AXI_ACLK_HALF_PERIOD);
    $finish;
  end


endmodule

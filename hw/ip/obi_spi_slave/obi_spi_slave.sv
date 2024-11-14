// Copyright 2017 ETH Zurich and University of Bologna.
// Copyright and related rights are licensed under the Solderpad Hardware
// License, Version 0.51 (the “License”); you may not use this file except in
// compliance with the License.  You may obtain a copy of the License at
// http://solderpad.org/licenses/SHL-0.51. Unless required by applicable law
// or agreed to in writing, software, hardware and materials distributed under
// this License is distributed on an “AS IS” BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

module obi_spi_slave #(

    parameter OBI_ADDR_WIDTH = 32,
    parameter OBI_DATA_WIDTH = 32,
    parameter DUMMY_CYCLES   = 32
) (
    //input  logic test_mode,
    input  logic spi_sclk,
    input  logic spi_cs,
    input  logic spi_mosi,
    output logic spi_miso,

    // OBI MASTER
    //***************************************
    input logic obi_aclk,
    input logic obi_aresetn,

    // ADDRESS CHANNEL
    output logic                      obi_master_req,
    input  logic                      obi_master_gnt,
    output logic [OBI_ADDR_WIDTH-1:0] obi_master_addr,
    output logic                      obi_master_we,
    output logic [OBI_DATA_WIDTH-1:0] obi_master_w_data,
    output logic [               3:0] obi_master_be,

    // RESPONSE CHANNEL
    input logic obi_master_r_valid,
    input logic [OBI_DATA_WIDTH-1:0] obi_master_r_data
);

  logic [               7:0] rx_counter;
  logic                      rx_counter_upd;
  logic [              31:0] rx_data;
  logic                      rx_data_valid;

  logic [               7:0] tx_counter;
  logic                      tx_counter_upd;
  logic [              31:0] tx_data;
  logic                      tx_data_valid;

  logic                      ctrl_rd_wr;

  logic [              31:0] ctrl_addr;
  logic                      ctrl_addr_valid;

  logic [              31:0] ctrl_data_rx;
  logic                      ctrl_data_rx_valid;
  logic                      ctrl_data_rx_ready;
  logic [              31:0] ctrl_data_tx;
  logic                      ctrl_data_tx_valid;
  logic                      ctrl_data_tx_ready;

  logic [              31:0] fifo_data_rx;
  logic                      fifo_data_rx_valid;
  logic                      fifo_data_rx_ready;
  logic [              31:0] fifo_data_tx;
  logic                      fifo_data_tx_valid;
  logic                      fifo_data_tx_ready;

  logic [OBI_ADDR_WIDTH-1:0] addr_sync;
  logic                      addr_valid_sync;
  logic                      cs_sync;

  logic                      tx_done;
  logic                      rd_wr_sync;

  logic [              15:0] wrap_length;
  logic                      test_mode;

  spi_slave_rx u_rxreg (
      .sclk          (spi_sclk),
      .cs            (spi_cs),
      .mosi          (spi_mosi),
      .counter_in    (rx_counter),
      .counter_in_upd(rx_counter_upd),
      .data          (rx_data),
      .data_ready    (rx_data_valid)
  );

  spi_slave_tx u_txreg (
      .test_mode     (test_mode),
      .sclk          (spi_sclk),
      .cs            (spi_cs),
      .miso          (spi_miso),
      .counter_in    (tx_counter),
      .counter_in_upd(tx_counter_upd),
      .data          (tx_data),
      .data_valid    (tx_data_valid),
      .done          (tx_done)
  );

  spi_slave_controller #(
      .DUMMY_CYCLES(DUMMY_CYCLES)
  ) u_slave_sm (
      .sclk              (spi_sclk),
      .sys_rstn          (obi_aresetn),
      .cs                (spi_cs),
      .rx_counter        (rx_counter),
      .rx_counter_upd    (rx_counter_upd),
      .rx_data           (rx_data),
      .rx_data_valid     (rx_data_valid),
      .tx_counter        (tx_counter),
      .tx_counter_upd    (tx_counter_upd),
      .tx_data           (tx_data),
      .tx_data_valid     (tx_data_valid),
      .tx_done           (tx_done),
      .ctrl_rd_wr        (ctrl_rd_wr),
      .ctrl_addr         (ctrl_addr),
      .ctrl_addr_valid   (ctrl_addr_valid),
      .ctrl_data_rx      (ctrl_data_rx),
      .ctrl_data_rx_valid(ctrl_data_rx_valid),
      .ctrl_data_tx      (ctrl_data_tx),
      .ctrl_data_tx_ready(ctrl_data_tx_ready),
      .wrap_length       (wrap_length)
  );

  spi_slave_dc_fifo #(
      .DATA_WIDTH  (32),
      .BUFFER_DEPTH(8)
  ) u_dcfifo_rx (
      .clk_a  (spi_sclk),
      .rstn_a (obi_aresetn),
      .data_a (ctrl_data_rx),
      .valid_a(ctrl_data_rx_valid),
      .ready_a(ctrl_data_rx_ready),
      .clk_b  (obi_aclk),
      .rstn_b (obi_aresetn),
      .data_b (fifo_data_rx),
      .valid_b(fifo_data_rx_valid),
      .ready_b(fifo_data_rx_ready)
  );

  spi_slave_dc_fifo #(
      .DATA_WIDTH  (32),
      .BUFFER_DEPTH(8)
  ) u_dcfifo_tx (
      .clk_a  (obi_aclk),
      .rstn_a (obi_aresetn),
      .data_a (fifo_data_tx),
      .valid_a(fifo_data_tx_valid),
      .ready_a(fifo_data_tx_ready),
      .clk_b  (spi_sclk),
      .rstn_b (obi_aresetn),
      .data_b (ctrl_data_tx),
      .valid_b(ctrl_data_tx_valid),
      .ready_b(ctrl_data_tx_ready)
  );

  spi_slave_obi_plug #(
      .OBI_ADDR_WIDTH(OBI_ADDR_WIDTH),
      .OBI_DATA_WIDTH(OBI_DATA_WIDTH)
  ) u_obiplug (
      .obi_aclk          (obi_aclk),
      .obi_aresetn       (obi_aresetn),
      .obi_master_req    (obi_master_req),
      .obi_master_gnt    (obi_master_gnt),
      .obi_master_addr   (obi_master_addr),
      .obi_master_we     (obi_master_we),
      .obi_master_w_data (obi_master_w_data),
      .obi_master_be     (obi_master_be),
      .obi_master_r_valid(obi_master_r_valid),
      .obi_master_r_data (obi_master_r_data),
      .rxtx_addr         (addr_sync),
      .rxtx_addr_valid   (addr_valid_sync),
      .start_tx          (rd_wr_sync & addr_valid_sync),
      .cs                (cs_sync),
      .tx_data           (fifo_data_tx),
      .tx_valid          (fifo_data_tx_valid),
      .tx_ready          (fifo_data_tx_ready),
      .rx_data           (fifo_data_rx),
      .rx_valid          (fifo_data_rx_valid),
      .rx_ready          (fifo_data_rx_ready),
      .wrap_length       (wrap_length)
  );

  spi_slave_syncro #(
      .AXI_ADDR_WIDTH(OBI_ADDR_WIDTH)
  ) u_syncro (
      .sys_clk           (obi_aclk),
      .rstn              (obi_aresetn),
      .cs                (spi_cs),
      .address           (ctrl_addr),
      .address_valid     (ctrl_addr_valid),
      .rd_wr             (ctrl_rd_wr),
      .cs_sync           (cs_sync),
      .address_sync      (addr_sync),
      .address_valid_sync(addr_valid_sync),
      .rd_wr_sync        (rd_wr_sync)
  );

  assign test_mode = 1'b0;
endmodule

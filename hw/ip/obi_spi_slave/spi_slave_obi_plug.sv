// Copyright 2017 ETH Zurich and University of Bologna.
// Copyright and related rights are licensed under the Solderpad Hardware
// License, Version 0.51 (the “License”); you may not use this file except in
// compliance with the License.  You may obtain a copy of the License at
// http://solderpad.org/licenses/SHL-0.51. Unless required by applicable law
// or agreed to in writing, software, hardware and materials distributed under
// this License is distributed on an “AS IS” BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

module spi_slave_obi_plug #(
    parameter OBI_ADDR_WIDTH = 32,
    parameter OBI_DATA_WIDTH = 32
) (
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
    output logic [3:0]                obi_master_be,

    // RESPONSE CHANNEL
    input logic obi_master_r_valid,
    input logic [OBI_DATA_WIDTH-1:0] obi_master_r_data,

    //SPI/BUFFER
    input  logic [OBI_ADDR_WIDTH-1:0]   rxtx_addr,
    input logic rxtx_addr_valid,
    input logic start_tx,
    input logic cs,
    output logic [31:0] tx_data,
    output logic tx_valid,
    input logic tx_ready,
    input logic [31:0] rx_data,
    input logic rx_valid,
    output logic rx_ready
);

  logic [OBI_ADDR_WIDTH-1:0] curr_addr;
  logic [              31:0] curr_data_rx;
  logic [OBI_DATA_WIDTH-1:0] curr_data_tx;
  logic                      sample_fifo;
  logic                      sample_obidata;
  logic                      sample_rxtx_state;
  logic                      rxtx_state;
  logic [0:0]                curr_rxtx_state; //low for reading, high for writing



  enum logic [1:0] {
    IDLE,
    OBIADDR,
    OBIRESP,
    SEND_TX_DATA
  }
      OBI_CS, OBI_NS; 

  always_ff @(posedge obi_aclk or negedge obi_aresetn) begin
    if (obi_aresetn == 0) begin
      OBI_CS       <= IDLE;
      curr_data_rx <= 'h0;
      curr_data_tx <= 'h0;
      curr_addr    <= 'h0;
      curr_rxtx_state <= 'h0;
    end else begin
      OBI_CS <= OBI_NS;
      if (sample_fifo) curr_data_rx <= rx_data;
      if (sample_obidata) curr_data_tx <= obi_master_r_data;
      if (rxtx_addr_valid) curr_addr <= rxtx_addr;
      if (sample_rxtx_state) curr_rxtx_state <= rxtx_state; 
    end
  end

  always_comb begin
    OBI_NS         = IDLE;
    sample_fifo    = 1'b0;
    rx_ready       = 1'b0;
    tx_valid       = 1'b0;
    obi_master_req = 1'b0;
    obi_master_we  = 1'b0;
    sample_obidata = 1'b0;
    sample_rxtx_state = 1'b0;
    rxtx_state = 1'b0;
    case (OBI_CS)
      IDLE: begin
        if (rx_valid) begin
          sample_fifo   = 1'b1;
          rx_ready      = 1'b1;
          rxtx_state = 1'b1;
          sample_rxtx_state = 1'b1;
          OBI_NS        = OBIADDR;
        end else if (start_tx && !cs) begin
          rxtx_state = 1'b0;
          sample_rxtx_state = 1'b1;
          OBI_NS = OBIADDR;
        end else begin
          OBI_NS = IDLE;
        end
      end
      OBIADDR: begin
        if(curr_rxtx_state)begin 
          obi_master_we = 1'b1;               
        end
        
        obi_master_req = 1'b1;

        if (obi_master_gnt && ((tx_ready && !curr_rxtx_state) || curr_rxtx_state)) OBI_NS = OBIRESP;
        else OBI_NS = OBIADDR;
      end
      OBIRESP: begin
        if (obi_master_r_valid) begin
          OBI_NS = IDLE;
          if (!curr_rxtx_state) begin
            sample_obidata = 1'b1;
            OBI_NS = SEND_TX_DATA;
          end
        end else OBI_NS = OBIRESP;
      end
      SEND_TX_DATA: begin
        tx_valid = 1'b1;
        OBI_NS = IDLE;
      end
    endcase
  end

  assign tx_data = curr_data_tx;
  assign obi_master_addr   =  curr_addr;
  assign obi_master_w_data    = curr_data_rx;
  assign obi_master_be = 4'b1111;

endmodule

/* Copyright 2018 ETH Zurich and University of Bologna.
 * Copyright and related rights are licensed under the Solderpad Hardware
 * License, Version 0.51 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://solderpad.org/licenses/SHL-0.51. Unless required by applicable law
 * or agreed to in writing, software, hardware and materials distributed under
 * this License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * File: $filename.v
 *
 * Description: Auto-generated bootrom
 */

// Auto-generated code
module boot_rom
  import reg_pkg::*;
(
    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o
);

  localparam int unsigned RomSize = 71;

  logic [RomSize-1:0][31:0] mem;
  assign mem = {
    32'h0000b5e5,
    32'h00019602,
    32'h49902000,
    32'h05b7fad1,
    32'hf0068693,
    32'hfb749ae3,
    32'h02048493,
    32'h01f4ae23,
    32'h01e4ac23,
    32'h01d4aa23,
    32'h01c4a823,
    32'h0074a623,
    32'h0064a423,
    32'h0054a223,
    32'h0114a023,
    32'h0285af83,
    32'h0285af03,
    32'h0285ae83,
    32'h0285ae03,
    32'h0285a383,
    32'h0285a303,
    32'h0285a283,
    32'h0285a883,
    32'hdfed8b85,
    32'h83d149dc,
    32'h10048b93,
    32'hfe07dee3,
    32'h49dc0001,
    32'h0355a223,
    32'h0ff40a93,
    32'h00db4463,
    32'h2ff40a93,
    32'h64051000,
    32'h0b134481,
    32'h40000693,
    32'h4601fe07,
    32'hdee349dc,
    32'h0001d1d8,
    32'h20370713,
    32'h6709fe07,
    32'h5ee349d8,
    32'h0001d598,
    32'h470dfe07,
    32'h5fe349d8,
    32'hd1d8070d,
    32'h6709d598,
    32'h0ab00713,
    32'hc9980087,
    32'h6713f007,
    32'h77134998,
    32'hd1884501,
    32'hcd980705,
    32'h0fff0737,
    32'hc9988f49,
    32'h49988000,
    32'h05372002,
    32'h05b79582,
    32'h18058593,
    32'h400005b7,
    32'hc1884505,
    32'h200285b7,
    32'hc9110145,
    32'hc5039582,
    32'h498cd175,
    32'h00c5c503,
    32'he5110085,
    32'hc5032000,
    32'h05b79502,
    32'h4588c119,
    32'h0045c503,
    32'h200305b7
  };

  logic [$clog2(RomSize)-1:0] addr;

  assign addr = reg_req_i.addr[$clog2(RomSize)-1+2:2];

  assign reg_rsp_o.error = addr > RomSize - 1 && reg_req_i.valid;
  assign reg_rsp_o.ready = 1'b1;
  assign reg_rsp_o.rdata = mem[addr];

endmodule

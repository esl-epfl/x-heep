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

  localparam int unsigned RomSize = 131;

  logic [RomSize-1:0][31:0] mem;
  assign mem = {
    32'h00010001,
    32'hbff90491,
    32'h14710114,
    32'ha0230285,
    32'ha8839602,
    32'hc1882002,
    32'h85b74990,
    32'hc9c8cd88,
    32'h45052000,
    32'h05b7e819,
    32'hdfed8b85,
    32'h83d149dc,
    32'hc99c8fd9,
    32'h00245793,
    32'hf0077713,
    32'h4998fa8a,
    32'h69e30204,
    32'h84931401,
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
    32'hb741f004,
    32'h0413fb74,
    32'h9ae30204,
    32'h849301f4,
    32'hae2301e4,
    32'hac2301d4,
    32'haa2301c4,
    32'ha8230074,
    32'ha6230064,
    32'ha4230054,
    32'ha2230114,
    32'ha0230285,
    32'haf830285,
    32'haf030285,
    32'hae830285,
    32'hae030285,
    32'ha3830285,
    32'ha3030285,
    32'ha2830285,
    32'ha883dfed,
    32'h8b8583d1,
    32'h49dc1004,
    32'h8b93fe07,
    32'hdee349dc,
    32'h00010355,
    32'ha223a85d,
    32'h02000a13,
    32'hfe07dee3,
    32'h49dc0001,
    32'hd1dc8fd9,
    32'hfff40713,
    32'h6785008b,
    32'h4e632ffa,
    32'h8a936a85,
    32'h10000b13,
    32'h8436fe07,
    32'h5ee349d8,
    32'h0001d1d8,
    32'h20370713,
    32'h6709fe07,
    32'hdee349dc,
    32'h0001d590,
    32'h00366613,
    32'h00f76633,
    32'h8f4107c2,
    32'h83e10106,
    32'h17938041,
    32'h04620106,
    32'h54130186,
    32'h1713c99c,
    32'h0087e793,
    32'hf007f793,
    32'h499c5584,
    32'h55945590,
    32'hdf6d8b05,
    32'h835149d8,
    32'hfe075ee3,
    32'h49d80001,
    32'hd1d8072d,
    32'h6705fe07,
    32'h5ee349d8,
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
    32'hc9980037,
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

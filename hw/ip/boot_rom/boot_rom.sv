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
  input  reg_req_t     reg_req_i,
  output reg_rsp_t     reg_rsp_o
);

  localparam int unsigned RomSize = 152;

  logic [RomSize-1:0][31:0] mem;
  assign mem = {
    32'h00000013,
    32'h00000013,
    32'hfe1ff06f,
    32'h00448493,
    32'hffc40413,
    32'h0114a023,
    32'h0285a883,
    32'h000580e7,
    32'h0105a583,
    32'h200005b7,
    32'h00041863,
    32'hfe078ae3,
    32'h0017f793,
    32'h0147d793,
    32'h0145a783,
    32'h00f5a823,
    32'h00e7e7b3,
    32'h00245793,
    32'hf0077713,
    32'h0105a703,
    32'hfa8a64e3,
    32'h02048493,
    32'hfe040413,
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
    32'hfe078ae3,
    32'h0017f793,
    32'h0147d793,
    32'h0145a783,
    32'hf71ff06f,
    32'hf0040413,
    32'hfb7496e3,
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
    32'hfe078ae3,
    32'h0017f793,
    32'h0147d793,
    32'h0145a783,
    32'h40048b93,
    32'hfe07dee3,
    32'h0145a783,
    32'h0355a223,
    32'h0c80006f,
    32'h02000a13,
    32'h00000413,
    32'hfe07dee3,
    32'h0145a783,
    32'h02f5a223,
    32'h1ff7e793,
    32'h000017b7,
    32'h028b4263,
    32'h2ffa8a93,
    32'h00001ab7,
    32'h10000b13,
    32'h00068413,
    32'hfe07dee3,
    32'h0145a783,
    32'h02c5a423,
    32'h00360613,
    32'h00861613,
    32'h00f5a823,
    32'h0087e793,
    32'hf007f793,
    32'h0105a783,
    32'h0285a483,
    32'h0285a683,
    32'h0285a603,
    32'hfe070ae3,
    32'h00177713,
    32'h01475713,
    32'h0145a703,
    32'hfe075ee3,
    32'h0145a703,
    32'h02e5a223,
    32'h00270713,
    32'h00002737,
    32'hfe075ee3,
    32'h0145a703,
    32'h02e5a423,
    32'h00300713,
    32'hfe075ee3,
    32'h0145a703,
    32'h02e5a223,
    32'h00370713,
    32'h00002737,
    32'h02e5a423,
    32'h0ab00713,
    32'h00e5a823,
    32'h00376713,
    32'hf0077713,
    32'h0105a703,
    32'h02a5a023,
    32'h00000513,
    32'h00e5ac23,
    32'h00170713,
    32'h0fff0737,
    32'h00e5a823,
    32'h00a76733,
    32'h0105a703,
    32'h80000537,
    32'h200205b7,
    32'h00000013,
    32'h00000013,
    32'h000580e7,
    32'h18058593,
    32'h400005b7,
    32'h00a5a023,
    32'h00100513,
    32'h200285b7,
    32'h02050263,
    32'h0145c503,
    32'h000580e7,
    32'h0105a583,
    32'hfc050ee3,
    32'h00c5c503,
    32'h00051a63,
    32'h0085c503,
    32'h200005b7,
    32'h000500e7,
    32'h0085a503,
    32'h00050663,
    32'h0045c503,
    32'h200305b7
  };

  logic [$clog2(RomSize)-1:0] addr;

  assign addr = reg_req_i.addr[$clog2(RomSize)-1+2:2];

  assign reg_rsp_o.error = addr > RomSize-1 && reg_req_i.valid;
  assign reg_rsp_o.ready = 1'b1;
  assign reg_rsp_o.rdata = mem[addr];

endmodule

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

  localparam int unsigned RomSize = 20;

  logic [RomSize-1:0][31:0] mem;
  assign mem = {
    32'h00000013,
    32'h00000013,
    32'h000580e7,
    32'h18058593,
    32'hf00005b7,
    32'h00a5a023,
    32'h00100513,
    32'h200285b7,
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

  assign reg_rsp_o.error = addr > RomSize - 1 && reg_req_i.valid;
  assign reg_rsp_o.ready = 1'b1;
  assign reg_rsp_o.rdata = mem[addr];

endmodule

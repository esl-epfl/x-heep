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
  import core_v_mini_mcu_pkg::*;

  localparam int unsigned RomSize = 59;

  logic [RomSize-1:0][31:0] mem;
  assign mem = {
    32'h96024990,
    32'h200005b7,
    32'hf2e9f006,
    32'h8693ff74,
    32'h93e3fec4,
    32'h9be30491,
    32'h0114a023,
    32'h0285a883,
    32'h02048613,
    32'hdfed8b85,
    32'h83d149dc,
    32'h10048b93,
    32'hfe07dfe3,
    32'h49dc0001,
    32'h0355a223,
    32'h0ff40a93,
    32'h08000437,
    32'h00db4663,
    32'h0ff40a93,
    32'h09000437,
    32'h10000b13,
    32'h44818006,
    32'h86936685,
    32'hfe07dfe3,
    32'h49dc0001,
    32'hd1d8070d,
    32'h11000737,
    32'hfe075fe3,
    32'h49d80001,
    32'hd5d8470d,
    32'hfe075fe3,
    32'h49d8d1d8,
    32'h070d1000,
    32'h0737d5d8,
    32'h0ab00713,
    32'hc9980087,
    32'h6713f007,
    32'h77134998,
    32'hd1884501,
    32'hcd980705,
    32'h0fff0737,
    32'hc9988f49,
    32'h4998a000,
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
    32'h41c8c119,
    32'h0005c503,
    32'h200405b7
  };

  logic [$clog2(core_v_mini_mcu_pkg::BOOTROM_SIZE)-1-2:0] word_addr;
  logic [$clog2(RomSize)-1:0] rom_addr;

  assign word_addr = reg_req_i.addr[$clog2(core_v_mini_mcu_pkg::BOOTROM_SIZE)-1:2];
  assign rom_addr = word_addr[$clog2(RomSize)-1:0];

  assign reg_rsp_o.error = 1'b0;
  assign reg_rsp_o.ready = 1'b1;

  always_comb begin
    if (word_addr > (RomSize - 1)) begin
      reg_rsp_o.rdata = '0;
    end else begin
      reg_rsp_o.rdata = mem[rom_addr];
    end
  end

endmodule

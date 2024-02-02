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
    32'h00009602,
    32'h49902000,
    32'h05b7f2e9,
    32'hf0068693,
    32'hff7493e3,
    32'hfec49be3,
    32'h04910114,
    32'ha0230285,
    32'ha8830204,
    32'h8613dfed,
    32'h8b8583d1,
    32'h49dc1004,
    32'h8b93fe07,
    32'hdfe349dc,
    32'h00010355,
    32'ha2230ff4,
    32'h0a930800,
    32'h043700db,
    32'h46630ff4,
    32'h0a930900,
    32'h04371000,
    32'h0b134481,
    32'h40000693,
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

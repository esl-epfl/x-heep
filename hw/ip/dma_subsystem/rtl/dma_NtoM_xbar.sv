/*
 * Copyright 2024 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *
 * Author: Tommaso Terzano <tommaso.terzano@epfl.ch> 
 *                         <tommaso.terzano@gmail.com>
 *  
 * Info: Generated N-to-M crossbar.
 *       This module has been developed to be used in the DMA subsystem, in which it is not possible to use an NtoM crossbar.
 *       The problem is that each slave port of the DMA xbar is actually a master port of the system bus. The DMA xbar slave ports 
 *       cannot be indexed by the master's request address because they all perform the same function, they all can read/write 
 *       to/from the same memory spaces.
 *       This module is a workaround to this problem, as it is composed by a number of N-to-1 crossbars, each for a different slave
 *       (which would be a system bus master port).
 *       The amount of masters per crossbar is defined by the parameter NUM_MASTERS_PER_XBAR.
 *       It is thus possible, from an application poin of view, to use channels that are guaranteed to have a dedicated master port,
 *       reducing the need for additional arbitration logic.
 */

module dma_NtoM_xbar #(
    parameter int unsigned XBAR_NMASTER = 4,
    parameter int unsigned XBAR_MSLAVE = 2,
    parameter int unsigned NUM_MASTERS_PER_XBAR = 2
) (
    input logic clk_i,
    input logic rst_ni,

    // Master ports
    input  obi_pkg::obi_req_t  [XBAR_NMASTER-1:0] master_req_i,
    output obi_pkg::obi_resp_t [XBAR_NMASTER-1:0] master_resp_o,

    // slave ports
    output obi_pkg::obi_req_t  slave_req_o [XBAR_MSLAVE-1:0],
    input  obi_pkg::obi_resp_t slave_resp_i[XBAR_MSLAVE-1:0]
);
  import obi_pkg::*;
  import core_v_mini_mcu_pkg::*;

  generate
    for (genvar i = 0; i < XBAR_MSLAVE; i++) begin : gen_xbar

      localparam int NUM_MASTERS = (XBAR_NMASTER > (i+1) * NUM_MASTERS_PER_XBAR) ? NUM_MASTERS_PER_XBAR : (XBAR_NMASTER - i * NUM_MASTERS_PER_XBAR);
      xbar_varlat_n_to_one #(
          .XBAR_NMASTER(NUM_MASTERS)
      ) xbar_i (
          .clk_i(clk_i),
          .rst_ni(rst_ni),
          .master_req_i(master_req_i[i*NUM_MASTERS_PER_XBAR+:NUM_MASTERS]),
          .master_resp_o(master_resp_o[i*NUM_MASTERS_PER_XBAR+:NUM_MASTERS]),
          .slave_req_o(slave_req_o[i]),
          .slave_resp_i(slave_resp_i[i])
      );
    end
  endgenerate

endmodule

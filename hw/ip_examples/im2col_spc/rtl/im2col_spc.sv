/*
 * Copyright 2024 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *
 * Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *  
 * Info: Im2col accelerator implemented as a Smart Peripheral Controller. It accesses the DMA CH0 to perform
 *       the matrix manipulation operation known as "image to column" (im2col), which enables efficient
 *       CNN inference by preparing the input tensor to use the GEMM library.
 */

module im2col_spc
  import obi_pkg::*;
  import reg_pkg::*; 
#()(
  input logic clk_i,
  input logic rst_ni,

  input obi_resp_t aopx2im2col_resp_i,
  output obi_req_t im2col2aopx_req_o,

  input  reg_req_t reg_req_i,
  output reg_rsp_t reg_rsp_o,
);



endmodule
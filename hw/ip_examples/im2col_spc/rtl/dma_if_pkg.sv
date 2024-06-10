/*
 * Copyright 2024 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *
 * Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *  
 * Info: This module defines the interface structure for the DMA registers.
 */

package reg_pkg;

  typedef struct packed {
    logic [31:0] input_ptr_i,
    logic [31:0] output_ptr_i,
    logic [31:0] in_inc_d1_i,
    logic [31:0] in_inc_d2_i,
    logic [31:0] out_inc_d1_i,
    logic [31:0] out_inc_d2_i,
    logic [31:0] n_zeros_top_i,
    logic [31:0] n_zeros_bottom_i,
    logic [31:0] n_zeros_left_i,
    logic [31:0] n_zeros_right_i,
    logic [31:0] in_size_du_d1_i,
    logic [31:0] in_size_du_d2_i,
    logic [31:0] out_size_du_d1_i,
    logic [31:0] out_size_du_d2_i,
  } dma_if_t;

endpackage
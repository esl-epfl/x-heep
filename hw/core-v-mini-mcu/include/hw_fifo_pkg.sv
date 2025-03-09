// Copyright 2025 EPFL and Politecnico di Torino.
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: hw_fifo_pkg.sv
// Author: Alessio Naclerio
// Date: 17/02/2025
// Description: Package for HW FIFO MODE dma interface.

package hw_fifo_pkg;

  typedef struct packed {
    logic pop;
    logic push;
    logic [31:0] data;
  } hw_fifo_req_t;

  typedef struct packed {
    logic empty;
    logic full;
    logic push;
    logic [31:0] data;
  } hw_fifo_resp_t;

endpackage

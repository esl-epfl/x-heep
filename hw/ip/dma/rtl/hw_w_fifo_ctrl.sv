// Copyright 2025 EPFL and Politecnico di Torino.
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: hw_w_fifo_ctrl.sv
// Author: Alessio Naclerio
// Date: 17/02/2025
// Description: This module is responsible for controlling the popping of data from the external write hardware FIFO
// located inside the streaming accelerator tightly-coupled with the DMA to be used in the HW_FIFO_MODE.

module hw_w_fifo_ctrl (
    input logic hw_fifo_mode_i,
    output logic [31:0] data_o,
    input logic [31:0] data_i,
    input logic data_out_gnt_i,
    output logic pop_o
);

  always_comb begin
    pop_o = 1'b0;
    if (hw_fifo_mode_i && data_out_gnt_i == 1'b1) begin
      pop_o = 1'b1;
    end
    data_o = data_i;
  end

endmodule

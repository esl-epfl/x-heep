// Copyright 2025 EPFL and Politecnico di Torino.
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: hw_r_fifo_ctrl.sv
// Author: Alessio Naclerio
// Date: 17/02/2025
// Description: This module is responsible for controlling the pushing of data into the external hardware read FIFO
//              located inside the streaming accelerator tightly-coupled with the DMA to be used in the HW_FIFO_MODE.


module hw_r_fifo_ctrl (
    input logic hw_fifo_mode_i,
    input logic [31:0] data_i,
    input logic data_valid_i,
    input logic hw_r_fifo_push_padding_i,
    output logic push_o,
    output logic [31:0] data_o
);

  always_comb begin
    push_o = 1'b0;
    data_o = '0;
    if (hw_fifo_mode_i && (data_valid_i == 1'b1 || hw_r_fifo_push_padding_i == 1'b1)) begin
      push_o = 1'b1;
      data_o = data_i;
    end
  end

endmodule

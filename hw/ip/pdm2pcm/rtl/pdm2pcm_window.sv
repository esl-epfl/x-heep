// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// Module to manage TX FIFO window for Serial Peripheral Interface (SPI) host IP.
//

module pdm2pcm_window #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic
) (
    input  reg_req_t        rx_win_i,
    output reg_rsp_t        rx_win_o,
    input            [31:0] rx_data_i,
    output logic            rx_ready_o
);

  import pdm2pcm_reg_pkg::*;

  logic [BlockAw-1:0] rx_addr;
  logic rx_win_error;

  assign rx_win_error = (rx_win_i.write == 1'b1) && (rx_addr != pdm2pcm_reg_pkg::PDM2PCM_RXDATA_OFFSET);
  assign rx_ready_o = rx_win_i.valid & ~rx_win_i.write;
  assign rx_win_o.rdata = rx_data_i;
  assign rx_win_o.error = rx_win_error;
  assign rx_win_o.ready = 1'b1;
  assign rx_addr = rx_win_i.addr;

endmodule : pdm2pcm_window

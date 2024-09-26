// Copyright 2023 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Thomas Benz <paulsc@iis.ee.ethz.ch>
// Paul Scheffler <paulsc@iis.ee.ethz.ch>

module hyperbus_delay (
    input  logic        in_i,
    input  logic [3:0]  delay_i,
    output logic        out_o
);

    configurable_delay #(
      .NUM_STEPS(16)
    ) i_delay (
        .clk_i      ( in_i      ),
        `ifndef TARGET_ASIC
        .enable_i   ( 1'b1      ),
        `endif
        .delay_i    ( delay_i   ),
        .clk_o      ( out_o     )
    );

endmodule : hyperbus_delay

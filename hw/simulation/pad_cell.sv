// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module pad_cell #(
) (
    input  logic gpio_i,
    input  logic gpio_en_i,
    output logic gpio_o,

    inout logic pad_io
);

  logic pad;

  assign gpio_o = pad_io;
  assign pad_io = pad;

  always_comb begin
    if (gpio_en_i == 1'b1) begin
      pad = gpio_i;
    end else begin
      pad = 1'bz;
    end
  end

endmodule

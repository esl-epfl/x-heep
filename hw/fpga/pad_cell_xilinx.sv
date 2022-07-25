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

  IOBUF qspi_iobuf (
      .T (~gpio_en_i),
      .I (gpio_i),
      .O (gpio_o),
      .IO(pad_io)
  );

endmodule

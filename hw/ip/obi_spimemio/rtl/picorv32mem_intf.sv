/*
*  Copyright 2022 EPFL Christoph Müller <christoph.mueller@epfl.ch>
*  Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
*  SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
*/

interface PICORV32_BUS (
    input logic clk, nrst
);

    logic valid;
    logic ready;
    logic [  23:0] addr;
    logic [  31:0] rdata; 
    logic [  31:0] wdata;
    logic [3:0] wstrb;

  modport core(input ready, rdata, output valid, addr, wstrb, wdata);
  modport mem(output ready, rdata, input valid, addr, wstrb, wdata);

endinterface



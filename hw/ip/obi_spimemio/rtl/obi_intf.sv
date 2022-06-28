/*
*  Copyright 2022 EPFL Christoph Müller <christoph.mueller@epfl.ch>
*  Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
*  SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
*/

interface SIMPLE_OBI_BUS #(
    /// The width of the address.
    parameter int ADDR_WIDTH = 32,
    /// The width of the data.
    parameter int DATA_WIDTH = 32
) (
    input logic clk
);

  logic                    req;
  logic                    we;
  logic [DATA_WIDTH/8-1:0] be;
  logic [  ADDR_WIDTH-1:0] addr;
  logic [  DATA_WIDTH-1:0] wdata;
  logic                    gnt;
  logic                    rvalid;
  logic [  DATA_WIDTH-1:0] rdata;

  modport master(input gnt, rvalid, rdata, output req, we, be, addr, wdata);
  modport slave(output gnt, rvalid, rdata, input req, we, be, addr, wdata);

endinterface

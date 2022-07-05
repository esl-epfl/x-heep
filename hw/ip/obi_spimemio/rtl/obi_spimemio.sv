/*
*  Copyright 2022 EPFL Christoph Müller <christoph.mueller@epfl.ch>
*  Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
*  SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
*/

module obi_spimemio
  import obi_pkg::*;
(
    input logic clk_i,
    input logic rst_ni,

    output logic flash_csb_o,
    output logic flash_clk_o,

    output logic flash_io0_oe_o,
    output logic flash_io1_oe_o,
    output logic flash_io2_oe_o,
    output logic flash_io3_oe_o,

    output logic flash_io0_do_o,
    output logic flash_io1_do_o,
    output logic flash_io2_do_o,
    output logic flash_io3_do_o,

    input logic flash_io0_di_i,
    input logic flash_io1_di_i,
    input logic flash_io2_di_i,
    input logic flash_io3_di_i,

    input  logic [ 3:0] cfgreg_we,
    input  logic [31:0] cfgreg_di,
    output logic [31:0] cfgreg_do,

    input  obi_req_t  spimemio_req_i,
    output obi_resp_t spimemio_resp_o
);

  import picorv32_pkg::*;

  picorv32_req_t  picorv32_req;
  picorv32_resp_t picorv32_resp;
  
  obi_to_picorv32 obi_to_picorv32_i (
      .clk_i(clk_i),
      .rst_ni(rst_ni),
      .picorv32_req_o(picorv32_req),
      .picorv32_resp_i(picorv32_resp),
      .obi_req_i(spimemio_req_i),
      .obi_resp_o(spimemio_resp_o)
  );

  

  spimemio spimemio_i (
      .clk(clk_i),
      .resetn(rst_ni),

      .valid(picorv32_req.valid),
      .ready(picorv32_resp.ready),
      .addr (picorv32_req.addr[23:0]),
      .rdata(picorv32_resp.rdata),

      .flash_csb(flash_csb_o),
      .flash_clk(flash_clk_o),

      .flash_io0_oe(flash_io0_oe_o),
      .flash_io1_oe(flash_io1_oe_o),
      .flash_io2_oe(flash_io2_oe_o),
      .flash_io3_oe(flash_io3_oe_o),

      .flash_io0_do(flash_io0_do_o),
      .flash_io1_do(flash_io1_do_o),
      .flash_io2_do(flash_io2_do_o),
      .flash_io3_do(flash_io3_do_o),

      .flash_io0_di(flash_io0_di_i),
      .flash_io1_di(flash_io1_di_i),
      .flash_io2_di(flash_io2_di_i),
      .flash_io3_di(flash_io3_di_i),

      .cfgreg_we(cfgreg_we),
      .cfgreg_di(cfgreg_di),
      .cfgreg_do(cfgreg_do)
  );
endmodule

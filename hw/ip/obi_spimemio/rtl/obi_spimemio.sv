/*
*  Copyright 2022 EPFL Christoph Müller <christoph.mueller@epfl.ch>
*  Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
*  SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
*/

module obi_spimemio
//import obi_intf::*;
import obi_pkg::*;
//import Picorv32mem_intf::*;
( 
    input clk, nrst,

	output flash_csb,
	output flash_clk,

	output flash_io0_oe,
	output flash_io1_oe,
	output flash_io2_oe,
	output flash_io3_oe,

	output flash_io0_do,
	output flash_io1_do,
	output flash_io2_do,
	output flash_io3_do,

	input  flash_io0_di,
	input  flash_io1_di,
	input  flash_io2_di,
	input  flash_io3_di,

    input   [3:0] cfgreg_we,
	input  [31:0] cfgreg_di,
	output [31:0] cfgreg_do,

    input  obi_req_t  spimemio_req_i,
    output obi_resp_t spimemio_resp_o
);

// interface instances
PICORV32_BUS(clk, nrst) P32; 
SIMPLE_OBI_BUS(clk,nrst) OBI;

always_comb begin : 
    OBI.req <= spimemio_req_i.req;
    OBI.we  <= spimemio_req_i.we;
    OBI.be  <= spimemio_req_i.be;
    OBI.addr <= spimemio_req_i.addr;
    OBI.wdata  <= spimemio_req_i.wdata;

    spimemio_resp_o.gnt <= OBI.gnt;
    spimemio_resp_o.rvalid <= OBI.rvalid;
    spimemio_resp_o.rdata <= OBI.rdata;
end

obi2picorv32mem o2p32 (
    .P32 (P32),
    .OBI (OBI),
    .clk(clk),
    .nrst(nrst)
);

spimemio memio (
    .clk (P32.clk),
    .resetn (P32.nrst),

    .valid (P32.valid),
    .ready (P32.ready),
    .addr (P32.addr),
    .rdata (P32.rdata),

    .flash_csb (flash_csb),
	.flash_clk (flash_clk),

	.flash_io0_oe (flash_io0_oe),
	.flash_io1_oe (flash_io1_oe),
	.flash_io2_oe (flash_io2_oe),
	.flash_io3_oe (flash_io3_oe),

	.flash_io0_do (flash_io0_do),
	.flash_io1_do (flash_io1_do),
	.flash_io2_do (flash_io2_do),
	.flash_io3_do (flash_io3_do),

	.flash_io0_di (flash_io0_di),
	.flash_io1_di (flash_io1_di),
	.flash_io2_di (flash_io2_di),
	.flash_io3_di (flash_io3_di),

    .cfgreg_we (cfgreg_we),
    .cfgreg_di (cfgreg_di),
    .cfgreg_do (cfgreg_do)
);
endmodule
// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module spi_subsystem
  import obi_pkg::*;
  import reg_pkg::*;
(

    input logic clk_i,
    input logic rst_ni,

    input logic use_spimemio_i,

    //memory mapped spi
    input  obi_req_t  spimemio_req_i,
    output obi_resp_t spimemio_resp_o,

    //opentitan spi configuration
    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    // SPI Interface
    output logic                               spi_sck_o,
    output logic                               spi_sck_en_o,
    output logic [spi_host_reg_pkg::NumCS-1:0] spi_csb_o,
    output logic [spi_host_reg_pkg::NumCS-1:0] spi_csb_en_o,
    output logic [                        3:0] spi_sd_o,
    output logic [                        3:0] spi_sd_en_o,
    input  logic [                        3:0] spi_sd_i

);

  // OpenTitan SPI Interface
  logic                               ot_spi_sck;
  logic                               ot_spi_sck_en;
  logic [spi_host_reg_pkg::NumCS-1:0] ot_spi_csb;
  logic [spi_host_reg_pkg::NumCS-1:0] ot_spi_csb_en;
  logic [                        3:0] ot_spi_sd_out;
  logic [                        3:0] ot_spi_sd_en;
  logic [                        3:0] ot_spi_sd_in;

  // YosysHW SPI Interface
  logic                               yo_spi_sck;
  logic                               yo_spi_sck_en;
  logic [spi_host_reg_pkg::NumCS-1:0] yo_spi_csb;
  logic [spi_host_reg_pkg::NumCS-1:0] yo_spi_csb_en;
  logic [                        3:0] yo_spi_sd_out;
  logic [                        3:0] yo_spi_sd_en;
  logic [                        3:0] yo_spi_sd_in;

  //Multiplexer
  always_comb begin
    if (!use_spimemio_i) begin
      spi_sck_o = ot_spi_sck;
      spi_sck_en_o = ot_spi_sck_en;
      spi_csb_o = ot_spi_csb;
      spi_csb_en_o = ot_spi_csb_en;
      spi_sd_o = ot_spi_sd_out;
      spi_sd_en_o = ot_spi_sd_en;
      ot_spi_sd_in = spi_sd_i;
      yo_spi_sd_in = '0;
    end else begin
      spi_sck_o = yo_spi_sck;
      spi_sck_en_o = yo_spi_sck_en;
      spi_csb_o = yo_spi_csb;
      spi_csb_en_o = yo_spi_csb_en;
      spi_sd_o = yo_spi_sd_out;
      spi_sd_en_o = yo_spi_sd_en;
      ot_spi_sd_in = '0;
      yo_spi_sd_in = spi_sd_i;
    end
  end


  //YosysHQ SPI
  assign yo_spi_sck_en = 1'b1;
  assign yo_spi_csb_en = 2'b01;
  assign yo_spi_csb[1] = 1'b1;

  obi_spimemio obi_spimemio_i (
      .clk_i,
      .rst_ni,
      .flash_csb_o(yo_spi_csb[0]),
      .flash_clk_o(yo_spi_sck),
      .flash_io0_oe_o(yo_spi_sd_en[0]),
      .flash_io1_oe_o(yo_spi_sd_en[1]),
      .flash_io2_oe_o(yo_spi_sd_en[2]),
      .flash_io3_oe_o(yo_spi_sd_en[3]),
      .flash_io0_do_o(yo_spi_sd_out[0]),
      .flash_io1_do_o(yo_spi_sd_out[1]),
      .flash_io2_do_o(yo_spi_sd_out[2]),
      .flash_io3_do_o(yo_spi_sd_out[3]),
      .flash_io0_di_i(yo_spi_sd_in[0]),
      .flash_io1_di_i(yo_spi_sd_in[1]),
      .flash_io2_di_i(yo_spi_sd_in[2]),
      .flash_io3_di_i(yo_spi_sd_in[3]),
      .cfgreg_we('0),  //TODO: attach to register
      .cfgreg_di('0),
      .cfgreg_do(),
      .spimemio_req_i(spimemio_req_i),
      .spimemio_resp_o(spimemio_resp_o)
  );

  //OpenTitan SPI Snitch Version
  spi_host #(
      .reg_req_t(reg_pkg::reg_req_t),
      .reg_rsp_t(reg_pkg::reg_rsp_t)
  ) spi_host_i (
      .clk_i,
      .rst_ni,
      .clk_core_i(clk_i),
      .rst_core_ni(rst_ni),
      .reg_req_i,
      .reg_rsp_o,
      .cio_sck_o(ot_spi_sck),
      .cio_sck_en_o(ot_spi_sck_en),
      .cio_csb_o(ot_spi_csb),
      .cio_csb_en_o(ot_spi_csb_en),
      .cio_sd_o(ot_spi_sd_out),
      .cio_sd_en_o(ot_spi_sd_en),
      .cio_sd_i(ot_spi_sd_in),
      .intr_error_o(),
      .intr_spi_event_o()
  );

endmodule  // spi_subsystem

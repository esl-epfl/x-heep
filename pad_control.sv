// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module pad_control #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic,
    parameter NUM_PAD = 1
) (

    input logic clk_i,
    input logic rst_ni,

    // Bus Interface
    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o
      ,
    output logic [NUM_PAD-1:0][0:0] pad_muxes_o
);


  import core_v_mini_mcu_pkg::*;

  import pad_control_reg_pkg::*;

  pad_control_reg2hw_t reg2hw;

  pad_control_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) pad_control_reg_top_i (
      .clk_i,
      .rst_ni,
      .reg_req_i,
      .reg_rsp_o,
      .reg2hw,
      .devmode_i(1'b1)
  );



  assign pad_muxes_o[PAD_SPI_SLAVE_SCK] = $unsigned(reg2hw.pad_mux_spi_slave_sck.q);
  assign pad_muxes_o[PAD_SPI_SLAVE_CS] = $unsigned(reg2hw.pad_mux_spi_slave_cs.q);
  assign pad_muxes_o[PAD_SPI_SLAVE_MISO] = $unsigned(reg2hw.pad_mux_spi_slave_miso.q);
  assign pad_muxes_o[PAD_SPI_SLAVE_MOSI] = $unsigned(reg2hw.pad_mux_spi_slave_mosi.q);
  assign pad_muxes_o[PAD_PDM2PCM_PDM] = $unsigned(reg2hw.pad_mux_pdm2pcm_pdm.q);
  assign pad_muxes_o[PAD_PDM2PCM_CLK] = $unsigned(reg2hw.pad_mux_pdm2pcm_clk.q);
  assign pad_muxes_o[PAD_I2S_SCK] = $unsigned(reg2hw.pad_mux_i2s_sck.q);
  assign pad_muxes_o[PAD_I2S_WS] = $unsigned(reg2hw.pad_mux_i2s_ws.q);
  assign pad_muxes_o[PAD_I2S_SD] = $unsigned(reg2hw.pad_mux_i2s_sd.q);
  assign pad_muxes_o[PAD_SPI2_CS_0] = $unsigned(reg2hw.pad_mux_spi2_cs_0.q);
  assign pad_muxes_o[PAD_SPI2_CS_1] = $unsigned(reg2hw.pad_mux_spi2_cs_1.q);
  assign pad_muxes_o[PAD_SPI2_SCK] = $unsigned(reg2hw.pad_mux_spi2_sck.q);
  assign pad_muxes_o[PAD_SPI2_SD_0] = $unsigned(reg2hw.pad_mux_spi2_sd_0.q);
  assign pad_muxes_o[PAD_SPI2_SD_1] = $unsigned(reg2hw.pad_mux_spi2_sd_1.q);
  assign pad_muxes_o[PAD_SPI2_SD_2] = $unsigned(reg2hw.pad_mux_spi2_sd_2.q);
  assign pad_muxes_o[PAD_SPI2_SD_3] = $unsigned(reg2hw.pad_mux_spi2_sd_3.q);
  assign pad_muxes_o[PAD_I2C_SCL] = $unsigned(reg2hw.pad_mux_i2c_scl.q);
  assign pad_muxes_o[PAD_I2C_SDA] = $unsigned(reg2hw.pad_mux_i2c_sda.q);

endmodule : pad_control

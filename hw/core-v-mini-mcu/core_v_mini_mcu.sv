// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module core_v_mini_mcu
  import obi_pkg::*;
  import reg_pkg::*;
#(
    parameter PULP_XPULP = 0,
    parameter FPU = 0,
    parameter PULP_ZFINX = 0,
    parameter EXT_XBAR_NMASTER = 0
) (
    input logic clk_i,
    input logic rst_ni,

    input  logic jtag_tck_i,
    input  logic jtag_tms_i,
    input  logic jtag_trst_ni,
    input  logic jtag_tdi_i,
    output logic jtag_tdo_o,

    input obi_req_t [EXT_XBAR_NMASTER-1:0] ext_xbar_master_req_i,
    output obi_resp_t [EXT_XBAR_NMASTER-1:0] ext_xbar_master_resp_o,
    output obi_req_t ext_xbar_slave_req_o,
    input obi_resp_t ext_xbar_slave_resp_i,

    input  logic        boot_select_i,
    input  logic        execute_from_flash_i,
    output logic [31:0] exit_value_o,
    output logic        exit_valid_o,

    inout logic [3:0] spi_sd_io,
    inout logic [spi_host_reg_pkg::NumCS-1:0] spi_csb_o,
    inout logic spi_sck_o,

    input logic [core_v_mini_mcu_pkg::NEXT_INT-1:0] intr_vector_ext_i,

    input  logic uart_rx_i,
    output logic uart_tx_o,

    inout logic [31:0] gpio_io,

    inout logic i2c_scl_io,
    inout logic i2c_sda_io,

    output reg_req_t ext_peripheral_slave_req_o,
    input  reg_rsp_t ext_peripheral_slave_resp_i
);

  import core_v_mini_mcu_pkg::*;
  import cv32e40p_apu_core_pkg::*;

  localparam NUM_BYTES = core_v_mini_mcu_pkg::MEM_SIZE;
  localparam DM_HALTADDRESS = core_v_mini_mcu_pkg::DEBUG_START_ADDRESS + 32'h00000800; //debug rom code (section .text in linker) starts at 0x800

  localparam JTAG_IDCODE = 32'h10001c05;
  localparam BOOT_ADDR = core_v_mini_mcu_pkg::BOOTROM_START_ADDRESS;
  localparam NUM_MHPMCOUNTERS = 1;

  // masters signals
  obi_req_t                                core_instr_req;
  obi_resp_t                               core_instr_resp;
  obi_req_t                                core_data_req;
  obi_resp_t                               core_data_resp;
  obi_req_t                                debug_master_req;
  obi_resp_t                               debug_master_resp;
  obi_req_t                                dma_master0_ch0_req;
  obi_resp_t                               dma_master0_ch0_resp;
  obi_req_t                                dma_master1_ch0_req;
  obi_resp_t                               dma_master1_ch0_resp;

  // ram signals
  obi_req_t                                ram0_slave_req;
  obi_resp_t                               ram0_slave_resp;
  obi_req_t                                ram1_slave_req;
  obi_resp_t                               ram1_slave_resp;
  obi_req_t                                ram2_slave_req;
  obi_resp_t                               ram2_slave_resp;
  obi_req_t                                ram3_slave_req;
  obi_resp_t                               ram3_slave_resp;
  obi_req_t                                ram4_slave_req;
  obi_resp_t                               ram4_slave_resp;
  obi_req_t                                ram5_slave_req;
  obi_resp_t                               ram5_slave_resp;
  obi_req_t                                ram6_slave_req;
  obi_resp_t                               ram6_slave_resp;
  obi_req_t                                ram7_slave_req;
  obi_resp_t                               ram7_slave_resp;

  // debug signals
  obi_req_t                                debug_slave_req;
  obi_resp_t                               debug_slave_resp;

  // peripherals signals
  obi_req_t                                ao_peripheral_slave_req;
  obi_resp_t                               ao_peripheral_slave_resp;
  obi_req_t                                peripheral_slave_req;
  obi_resp_t                               peripheral_slave_resp;

  // spi flash signals
  obi_req_t                                spi_flash_slave_req;
  obi_resp_t                               spi_flash_slave_resp;

  // signals to debug unit
  logic                                    debug_core_req;

  // core
  logic                                    core_sleep;

  // irq signals
  logic                                    irq_ack;
  logic      [                        4:0] irq_id_out;
  logic                                    irq_software;
  logic                                    irq_timer;
  logic                                    irq_external;
  logic      [                       15:0] irq_fast;

  // gpio signals
  logic      [                       31:0] gpio_in;
  logic      [                       31:0] gpio_out;
  logic      [                       31:0] gpio_en;

  // spi signals
  logic                                    spi_sck;
  logic                                    spi_sck_en;
  logic      [spi_host_reg_pkg::NumCS-1:0] spi_csb;
  logic      [spi_host_reg_pkg::NumCS-1:0] spi_csb_en;
  logic      [                        3:0] spi_sd_out;
  logic      [                        3:0] spi_sd_en;
  logic      [                        3:0] spi_sd_in;

  // power manager
  logic                                    power_gate_core;
  logic                                    cpu_subsystem_rst_n;
  logic                                    tmp_cpu_subsystem_rst_n;

  // I2C
  logic                                    i2c_scl_in;
  logic                                    i2c_scl_out;
  logic                                    i2c_scl_en;
  logic                                    i2c_sda_in;
  logic                                    i2c_sda_out;
  logic                                    i2c_sda_en;

  // DMA
  logic                                    dma_intr;

  cpu_subsystem #(
      .BOOT_ADDR(BOOT_ADDR),
      .PULP_XPULP(PULP_XPULP),
      .FPU(FPU),
      .PULP_ZFINX(PULP_ZFINX),
      .NUM_MHPMCOUNTERS(NUM_MHPMCOUNTERS),
      .DM_HALTADDRESS(DM_HALTADDRESS)
  ) cpu_subsystem_i (
      .clk_i,
      .rst_ni(cpu_subsystem_rst_n),
      .core_instr_req_o(core_instr_req),
      .core_instr_resp_i(core_instr_resp),
      .core_data_req_o(core_data_req),
      .core_data_resp_i(core_data_resp),
      .irq_i({irq_fast, 4'b0, irq_external, 3'b0, irq_timer, 3'b0, irq_software, 3'b0}),
      .irq_ack_o(irq_ack),
      .irq_id_o(irq_id_out),
      .debug_req_i(debug_core_req),
      .core_sleep_o(core_sleep)
  );

  assign irq_fast = '0;

  debug_subsystem #(
      .JTAG_IDCODE(JTAG_IDCODE)
  ) debug_subsystem_i (
      .clk_i,
      .rst_ni,
      .jtag_tck_i(jtag_tck_i),
      .jtag_tms_i(jtag_tms_i),
      .jtag_trst_ni(jtag_trst_ni),
      .jtag_tdi_i(jtag_tdi_i),
      .jtag_tdo_o(jtag_tdo_o),
      .debug_core_req_o(debug_core_req),
      .debug_slave_req_i(debug_slave_req),
      .debug_slave_resp_o(debug_slave_resp),
      .debug_master_req_o(debug_master_req),
      .debug_master_resp_i(debug_master_resp)
  );

  system_bus #(
      .EXT_XBAR_NMASTER(EXT_XBAR_NMASTER)
  ) system_bus_i (
      .clk_i,
      .rst_ni,
      .core_instr_req_i          (core_instr_req),
      .core_instr_resp_o         (core_instr_resp),
      .core_data_req_i           (core_data_req),
      .core_data_resp_o          (core_data_resp),
      .debug_master_req_i        (debug_master_req),
      .debug_master_resp_o       (debug_master_resp),
      .dma_master0_ch0_req_i     (dma_master0_ch0_req),
      .dma_master0_ch0_resp_o    (dma_master0_ch0_resp),
      .dma_master1_ch0_req_i     (dma_master1_ch0_req),
      .dma_master1_ch0_resp_o    (dma_master1_ch0_resp),
      .ext_xbar_master_req_i     (ext_xbar_master_req_i),
      .ext_xbar_master_resp_o    (ext_xbar_master_resp_o),
      .ram0_req_o                (ram0_slave_req),
      .ram0_resp_i               (ram0_slave_resp),
      .ram1_req_o                (ram1_slave_req),
      .ram1_resp_i               (ram1_slave_resp),
      .ram2_req_o                (ram2_slave_req),
      .ram2_resp_i               (ram2_slave_resp),
      .ram3_req_o                (ram3_slave_req),
      .ram3_resp_i               (ram3_slave_resp),
      .ram4_req_o                (ram4_slave_req),
      .ram4_resp_i               (ram4_slave_resp),
      .ram5_req_o                (ram5_slave_req),
      .ram5_resp_i               (ram5_slave_resp),
      .ram6_req_o                (ram6_slave_req),
      .ram6_resp_i               (ram6_slave_resp),
      .ram7_req_o                (ram7_slave_req),
      .ram7_resp_i               (ram7_slave_resp),
      .debug_slave_req_o         (debug_slave_req),
      .debug_slave_resp_i        (debug_slave_resp),
      .ao_peripheral_slave_req_o (ao_peripheral_slave_req),
      .ao_peripheral_slave_resp_i(ao_peripheral_slave_resp),
      .peripheral_slave_req_o    (peripheral_slave_req),
      .peripheral_slave_resp_i   (peripheral_slave_resp),
      .spi_flash_slave_req_o     (spi_flash_slave_req),
      .spi_flash_slave_resp_i    (spi_flash_slave_resp),
      .ext_xbar_slave_req_o      (ext_xbar_slave_req_o),
      .ext_xbar_slave_resp_i     (ext_xbar_slave_resp_i)
  );

  memory_subsystem #(
      .NUM_BYTES(NUM_BYTES)
  ) memory_subsystem_i (
      .clk_i,
      .rst_ni,
      .ram0_req_i (ram0_slave_req),
      .ram0_resp_o(ram0_slave_resp),
      .ram1_req_i (ram1_slave_req),
      .ram1_resp_o(ram1_slave_resp),
      .ram2_req_i (ram2_slave_req),
      .ram2_resp_o(ram2_slave_resp),
      .ram3_req_i (ram3_slave_req),
      .ram3_resp_o(ram3_slave_resp),
      .ram4_req_i (ram4_slave_req),
      .ram4_resp_o(ram4_slave_resp),
      .ram5_req_i (ram5_slave_req),
      .ram5_resp_o(ram5_slave_resp),
      .ram6_req_i (ram6_slave_req),
      .ram6_resp_o(ram6_slave_resp),
      .ram7_req_i (ram7_slave_req),
      .ram7_resp_o(ram7_slave_resp)
  );

  ao_peripheral_subsystem ao_peripheral_subsystem_i (
      .clk_i,
      .rst_ni,
      .slave_req_i(ao_peripheral_slave_req),
      .slave_resp_o(ao_peripheral_slave_resp),
      .boot_select_i,
      .execute_from_flash_i,
      .exit_valid_o,
      .exit_value_o,
      .spimemio_req_i(spi_flash_slave_req),
      .spimemio_resp_o(spi_flash_slave_resp),
      .spi_sck_o(spi_sck),
      .spi_sck_en_o(spi_sck_en),
      .spi_csb_o(spi_csb),
      .spi_csb_en_o(spi_csb_en),
      .spi_sd_o(spi_sd_out),
      .spi_sd_en_o(spi_sd_en),
      .spi_sd_i(spi_sd_in),
      .core_sleep_i(core_sleep),
      .power_gate_core_o(power_gate_core),
      .cpu_subsystem_rst_no(tmp_cpu_subsystem_rst_n),
      .rv_timer_irq_timer_o(irq_timer),
      .dma_master0_ch0_req_o(dma_master0_ch0_req),
      .dma_master0_ch0_resp_i(dma_master0_ch0_resp),
      .dma_master1_ch0_req_o(dma_master1_ch0_req),
      .dma_master1_ch0_resp_i(dma_master1_ch0_resp),
      .dma_intr_o(dma_intr)
  );

  assign cpu_subsystem_rst_n = rst_ni & tmp_cpu_subsystem_rst_n;

  peripheral_subsystem #(
      .NEXT_INT(NEXT_INT)
  ) peripheral_subsystem_i (
      .clk_i,
      .rst_ni,
      .slave_req_i(peripheral_slave_req),
      .slave_resp_o(peripheral_slave_resp),
      .intr_vector_ext_i(intr_vector_ext_i),
      .irq_plic_o(irq_external),
      .msip_o(irq_software),
      .uart_rx_i(uart_rx_i),
      .uart_tx_o(uart_tx_o),
      .uart_tx_en_o(),
      .cio_gpio_i(gpio_in),
      .cio_gpio_o(gpio_out),
      .cio_gpio_en_o(gpio_en),
      .cio_scl_i(i2c_scl_in),
      .cio_scl_o(i2c_scl_out),
      .cio_scl_en_o(i2c_scl_en),
      .cio_sda_i(i2c_sda_in),
      .cio_sda_o(i2c_sda_out),
      .cio_sda_en_o(i2c_sda_en),
      .ext_peripheral_slave_req_o(ext_peripheral_slave_req_o),
      .ext_peripheral_slave_resp_i(ext_peripheral_slave_resp_i),
      .dma_intr_i(dma_intr)
  );

  // Pad cells GPIOs
  genvar i;
  generate
    for (i = 0; i < 32; i++) begin
      pad_cell #() pad_cell_gpio_i (
          .gpio_i(gpio_out[i]),
          .gpio_en_i(gpio_en[i]),
          .gpio_o(gpio_in[i]),
          .pad_io(gpio_io[i])
      );
    end
  endgenerate

  // Pad cells SPI
  pad_cell pad_cell_spi_sck_i (
      .gpio_i(spi_sck),
      .gpio_en_i(spi_sck_en),
      .gpio_o(),
      .pad_io(spi_sck_o)
  );

  genvar k;
  generate
    for (k = 0; k < spi_host_reg_pkg::NumCS; k++) begin
      pad_cell pad_cell_spi_csb_i (
          .gpio_i(spi_csb[k]),
          .gpio_en_i(spi_csb_en[k]),
          .gpio_o(),
          .pad_io(spi_csb_o[k])
      );
    end
  endgenerate

  genvar j;
  generate
    for (j = 0; j < 4; j++) begin
      pad_cell #() pad_cell_sd_i (
          .gpio_i(spi_sd_out[j]),
          .gpio_en_i(spi_sd_en[j]),
          .gpio_o(spi_sd_in[j]),
          .pad_io(spi_sd_io[j])
      );
    end
  endgenerate

  // Pad cells I2C
  pad_cell pad_cell_i2c_scl_i (
      .gpio_i(i2c_scl_out),
      .gpio_en_i(i2c_scl_en),
      .gpio_o(i2c_scl_in),
      .pad_io(i2c_scl_io)
  );

  pad_cell pad_cell_i2c_sda_i (
      .gpio_i(i2c_sda_out),
      .gpio_en_i(i2c_sda_en),
      .gpio_o(i2c_sda_in),
      .pad_io(i2c_sda_io)
  );

endmodule  // core_v_mini_mcu

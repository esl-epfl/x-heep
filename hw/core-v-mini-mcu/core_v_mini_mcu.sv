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
    inout logic clk_i,
    inout logic rst_ni,

    inout logic boot_select_i,
    inout logic execute_from_flash_i,

    inout logic jtag_tck_i,
    inout logic jtag_tms_i,
    inout logic jtag_trst_ni,
    inout logic jtag_tdi_i,
    inout logic jtag_tdo_o,

    input  obi_req_t  [EXT_XBAR_NMASTER-1:0] ext_xbar_master_req_i,
    output obi_resp_t [EXT_XBAR_NMASTER-1:0] ext_xbar_master_resp_o,

    output obi_req_t  ext_xbar_slave_req_o,
    input  obi_resp_t ext_xbar_slave_resp_i,

    output reg_req_t ext_peripheral_slave_req_o,
    input  reg_rsp_t ext_peripheral_slave_resp_i,

    inout logic uart_rx_i,
    inout logic uart_tx_o,

    input logic [core_v_mini_mcu_pkg::NEXT_INT-1:0] intr_vector_ext_i,

    inout logic [31:0] gpio_io,

    output logic [31:0] exit_value_o,
    inout  logic        exit_valid_o,

    inout logic [3:0] spi_flash_sd_io,
    inout logic [spi_host_reg_pkg::NumCS-1:0] spi_flash_csb_o,
    inout logic spi_flash_sck_o,

    inout logic [3:0] spi_sd_io,
    inout logic [spi_host_reg_pkg::NumCS-1:0] spi_csb_o,
    inout logic spi_sck_o,

    inout logic i2c_scl_io,
    inout logic i2c_sda_io
);

  import core_v_mini_mcu_pkg::*;
  import cv32e40p_apu_core_pkg::*;

  localparam NUM_BYTES = core_v_mini_mcu_pkg::MEM_SIZE;
  localparam DM_HALTADDRESS = core_v_mini_mcu_pkg::DEBUG_START_ADDRESS + 32'h00000800; //debug rom code (section .text in linker) starts at 0x800

  localparam JTAG_IDCODE = 32'h10001c05;
  localparam BOOT_ADDR = core_v_mini_mcu_pkg::BOOTROM_START_ADDRESS;
  localparam NUM_MHPMCOUNTERS = 1;

  // Log top level parameter values
`ifndef SYNTHESIS
  initial begin
    $display("[X-HEEP]: NUM_BYTES = %dKB", NUM_BYTES / 1024);
  end
`endif

  logic clk, rst_n, boot_select, execute_from_flash, exit_valid;

  // masters signals
  obi_req_t core_instr_req;
  obi_resp_t core_instr_resp;
  obi_req_t core_data_req;
  obi_resp_t core_data_resp;
  obi_req_t debug_master_req;
  obi_resp_t debug_master_resp;
  obi_req_t dma_master0_ch0_req;
  obi_resp_t dma_master0_ch0_resp;
  obi_req_t dma_master1_ch0_req;
  obi_resp_t dma_master1_ch0_resp;

  // ram signals
  obi_req_t [core_v_mini_mcu_pkg::NUM_BANKS-1:0] ram_slave_req;
  obi_resp_t [core_v_mini_mcu_pkg::NUM_BANKS-1:0] ram_slave_resp;

  // debug signals
  obi_req_t debug_slave_req;
  obi_resp_t debug_slave_resp;

  // peripherals signals
  obi_req_t ao_peripheral_slave_req;
  obi_resp_t ao_peripheral_slave_resp;
  obi_req_t peripheral_slave_req;
  obi_resp_t peripheral_slave_resp;

  // signals to debug unit
  logic debug_core_req;

  // jtag
  logic jtag_tck;
  logic jtag_tms;
  logic jtag_trst_n;
  logic jtag_tdi;
  logic jtag_tdo;

  // core
  logic core_sleep;

  // irq signals
  logic irq_ack;
  logic [4:0] irq_id_out;
  logic irq_software;
  logic irq_external;
  logic [14:0] irq_fast;

  // SPI Interface (YosysHW SPI and OpenTitan SPI multiplexed)
  logic spi_flash_sck;
  logic spi_flash_sck_en;
  logic [spi_host_reg_pkg::NumCS-1:0] spi_flash_csb;
  logic [spi_host_reg_pkg::NumCS-1:0] spi_flash_csb_en;
  logic [3:0] spi_flash_sd_out;
  logic [3:0] spi_flash_sd_en;
  logic [3:0] spi_flash_sd_in;
  logic spi_flash_intr;

  // Memory Map SPI Region
  obi_req_t flash_mem_slave_req;
  obi_resp_t flash_mem_slave_resp;

  // OpenTitan SPI interface (connected to DMA)
  logic spi_sck;
  logic spi_sck_en;
  logic [spi_host_reg_pkg::NumCS-1:0] spi_csb;
  logic [spi_host_reg_pkg::NumCS-1:0] spi_csb_en;
  logic [3:0] spi_sd_out;
  logic [3:0] spi_sd_en;
  logic [3:0] spi_sd_in;
  logic spi_intr;

  // power manager
  logic cpu_subsystem_powergate_switch;
  logic peripheral_subsystem_powergate_switch;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_switches;
  logic cpu_subsystem_rst_n;
  logic peripheral_subsystem_rst_n;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_rst_n;

  // rv_timer
  logic [3:0] rv_timer_intr;

  // dma
  logic dma_intr;

  // fast intr ctrl
  logic [14:0] fast_intr;

  // uart
  logic uart_rx, uart_tx;

  // gpio signals
  logic [31:0] gpio_in;
  logic [31:0] gpio_out;
  logic [31:0] gpio_en;
  logic [7:0] gpio_intr;

  // i2c
  logic cio_scl_in;
  logic cio_scl_out;
  logic cio_scl_en;
  logic cio_sda_in;
  logic cio_sda_out;
  logic cio_sda_en;

  // pads
  logic [core_v_mini_mcu_pkg::NUM_PAD-1:0][15:0] pad_attributes;

  // interrupt array
  logic [31:0] intr;

  cpu_subsystem #(
      .BOOT_ADDR(BOOT_ADDR),
      .PULP_XPULP(PULP_XPULP),
      .FPU(FPU),
      .PULP_ZFINX(PULP_ZFINX),
      .NUM_MHPMCOUNTERS(NUM_MHPMCOUNTERS),
      .DM_HALTADDRESS(DM_HALTADDRESS)
  ) cpu_subsystem_i (
      .clk_i(clk),
      .rst_ni(cpu_subsystem_rst_n),
      .core_instr_req_o(core_instr_req),
      .core_instr_resp_i(core_instr_resp),
      .core_data_req_o(core_data_req),
      .core_data_resp_i(core_data_resp),
      .irq_i(intr),
      .irq_ack_o(irq_ack),
      .irq_id_o(irq_id_out),
      .debug_req_i(debug_core_req),
      .core_sleep_o(core_sleep)
  );

  assign intr = {
    1'b0, irq_fast, 4'b0, irq_external, 3'b0, rv_timer_intr[0], 3'b0, irq_software, 3'b0
  };

  assign fast_intr = {
    1'b0,
    gpio_intr,
    spi_flash_intr,
    spi_intr,
    dma_intr,
    rv_timer_intr[3],
    rv_timer_intr[2],
    rv_timer_intr[1]
  };

  debug_subsystem #(
      .JTAG_IDCODE(JTAG_IDCODE)
  ) debug_subsystem_i (
      .clk_i(clk),
      .rst_ni(rst_n),
      .jtag_tck_i(jtag_tck),
      .jtag_tms_i(jtag_tms),
      .jtag_trst_ni(jtag_trst_n),
      .jtag_tdi_i(jtag_tdi),
      .jtag_tdo_o(jtag_tdo),
      .debug_core_req_o(debug_core_req),
      .debug_slave_req_i(debug_slave_req),
      .debug_slave_resp_o(debug_slave_resp),
      .debug_master_req_o(debug_master_req),
      .debug_master_resp_i(debug_master_resp)
  );

  system_bus #(
      .NUM_BANKS(core_v_mini_mcu_pkg::NUM_BANKS),
      .EXT_XBAR_NMASTER(EXT_XBAR_NMASTER)
  ) system_bus_i (
      .clk_i(clk),
      .rst_ni(rst_n),
      .core_instr_req_i(core_instr_req),
      .core_instr_resp_o(core_instr_resp),
      .core_data_req_i(core_data_req),
      .core_data_resp_o(core_data_resp),
      .debug_master_req_i(debug_master_req),
      .debug_master_resp_o(debug_master_resp),
      .dma_master0_ch0_req_i(dma_master0_ch0_req),
      .dma_master0_ch0_resp_o(dma_master0_ch0_resp),
      .dma_master1_ch0_req_i(dma_master1_ch0_req),
      .dma_master1_ch0_resp_o(dma_master1_ch0_resp),
      .ext_xbar_master_req_i(ext_xbar_master_req_i),
      .ext_xbar_master_resp_o(ext_xbar_master_resp_o),
      .ram_req_o(ram_slave_req),
      .ram_resp_i(ram_slave_resp),
      .debug_slave_req_o(debug_slave_req),
      .debug_slave_resp_i(debug_slave_resp),
      .ao_peripheral_slave_req_o(ao_peripheral_slave_req),
      .ao_peripheral_slave_resp_i(ao_peripheral_slave_resp),
      .peripheral_slave_req_o(peripheral_slave_req),
      .peripheral_slave_resp_i(peripheral_slave_resp),
      .flash_mem_slave_req_o(flash_mem_slave_req),
      .flash_mem_slave_resp_i(flash_mem_slave_resp),
      .ext_xbar_slave_req_o(ext_xbar_slave_req_o),
      .ext_xbar_slave_resp_i(ext_xbar_slave_resp_i)
  );

  memory_subsystem #(
      .NUM_BANKS(core_v_mini_mcu_pkg::NUM_BANKS)
  ) memory_subsystem_i (
      .clk_i(clk),
      .rst_ni(memory_subsystem_rst_n),
      .ram_req_i(ram_slave_req),
      .ram_resp_o(ram_slave_resp)
  );

  ao_peripheral_subsystem ao_peripheral_subsystem_i (
      .clk_i(clk),
      .rst_ni(rst_n),
      .slave_req_i(ao_peripheral_slave_req),
      .slave_resp_o(ao_peripheral_slave_resp),
      .boot_select_i(boot_select),
      .execute_from_flash_i(execute_from_flash),
      .exit_valid_o(exit_valid),
      .exit_value_o(exit_value_o),
      .spimemio_req_i(flash_mem_slave_req),
      .spimemio_resp_o(flash_mem_slave_resp),
      .spi_flash_sck_o(spi_flash_sck),
      .spi_flash_sck_en_o(spi_flash_sck_en),
      .spi_flash_csb_o(spi_flash_csb),
      .spi_flash_csb_en_o(spi_flash_csb_en),
      .spi_flash_sd_o(spi_flash_sd_out),
      .spi_flash_sd_en_o(spi_flash_sd_en),
      .spi_flash_sd_i(spi_flash_sd_in),
      .spi_sck_o(spi_sck),
      .spi_sck_en_o(spi_sck_en),
      .spi_csb_o(spi_csb),
      .spi_csb_en_o(spi_csb_en),
      .spi_sd_o(spi_sd_out),
      .spi_sd_en_o(spi_sd_en),
      .spi_sd_i(spi_sd_in),
      .spi_intr_event_o(spi_intr),
      .spi_flash_intr_event_o(spi_flash_intr),
      .intr_i(intr),
      .ext_intr_i(intr_vector_ext_i),
      .core_sleep_i(core_sleep),
      .cpu_subsystem_powergate_switch_o(cpu_subsystem_powergate_switch),
      .peripheral_subsystem_powergate_switch_o(peripheral_subsystem_powergate_switch),
      .memory_subsystem_banks_powergate_switches_o(memory_subsystem_banks_powergate_switches),
      .cpu_subsystem_rst_no(cpu_subsystem_rst_n),
      .peripheral_subsystem_rst_no(peripheral_subsystem_rst_n),
      .memory_subsystem_rst_no(memory_subsystem_rst_n),
      .rv_timer_0_intr_o(rv_timer_intr[0]),
      .rv_timer_1_intr_o(rv_timer_intr[1]),
      .dma_master0_ch0_req_o(dma_master0_ch0_req),
      .dma_master0_ch0_resp_i(dma_master0_ch0_resp),
      .dma_master1_ch0_req_o(dma_master1_ch0_req),
      .dma_master1_ch0_resp_i(dma_master1_ch0_resp),
      .dma_intr_o(dma_intr),
      .fast_intr_i(fast_intr),
      .fast_intr_o(irq_fast),
      .pad_attributes_o(pad_attributes)
  );

  peripheral_subsystem #(
      .NEXT_INT(NEXT_INT)
  ) peripheral_subsystem_i (
      .clk_i(clk),
      .rst_ni(peripheral_subsystem_rst_n),
      .slave_req_i(peripheral_slave_req),
      .slave_resp_o(peripheral_slave_resp),
      .intr_vector_ext_i,
      .irq_plic_o(irq_external),
      .msip_o(irq_software),
      .uart_rx_i(uart_rx),
      .uart_tx_o(uart_tx),
      .uart_tx_en_o(),
      .cio_gpio_i(gpio_in),
      .cio_gpio_o(gpio_out),
      .cio_gpio_en_o(gpio_en),
      .cio_gpio_intr_o(gpio_intr),
      .cio_scl_i(cio_scl_in),
      .cio_scl_o(cio_scl_out),
      .cio_scl_en_o(cio_scl_en),
      .cio_sda_i(cio_sda_in),
      .cio_sda_o(cio_sda_out),
      .cio_sda_en_o(cio_sda_en),
      .rv_timer_2_intr_o(rv_timer_intr[2]),
      .rv_timer_3_intr_o(rv_timer_intr[3]),
      .ext_peripheral_slave_req_o(ext_peripheral_slave_req_o),
      .ext_peripheral_slave_resp_i(ext_peripheral_slave_resp_i)
  );

  pad_ring pad_ring_i (
      .clk_io(clk_i),
      .clk_o(clk),
      .rst_io(rst_ni),
      .rst_o(rst_n),
      .boot_select_io(boot_select_i),
      .boot_select_o(boot_select),
      .execute_from_flash_io(execute_from_flash_i),
      .execute_from_flash_o(execute_from_flash),
      .jtag_tck_io(jtag_tck_i),
      .jtag_tck_o(jtag_tck),
      .jtag_tms_io(jtag_tms_i),
      .jtag_tms_o(jtag_tms),
      .jtag_trst_io(jtag_trst_ni),
      .jtag_trst_o(jtag_trst_n),
      .jtag_tdi_io(jtag_tdi_i),
      .jtag_tdi_o(jtag_tdi),
      .jtag_tdo_io(jtag_tdo_o),
      .jtag_tdo_i(jtag_tdo),
      .uart_rx_io(uart_rx_i),
      .uart_rx_o(uart_rx),
      .uart_tx_io(uart_tx_o),
      .uart_tx_i(uart_tx),
      .exit_valid_io(exit_valid_o),
      .exit_valid_i(exit_valid),
      .gpio_0_io(gpio_io[0]),
      .gpio_0_i(gpio_out[0]),
      .gpio_0_o(gpio_in[0]),
      .gpio_0_oe_i(gpio_en[0]),
      .gpio_1_io(gpio_io[1]),
      .gpio_1_i(gpio_out[1]),
      .gpio_1_o(gpio_in[1]),
      .gpio_1_oe_i(gpio_en[1]),
      .gpio_2_io(gpio_io[2]),
      .gpio_2_i(gpio_out[2]),
      .gpio_2_o(gpio_in[2]),
      .gpio_2_oe_i(gpio_en[2]),
      .gpio_3_io(gpio_io[3]),
      .gpio_3_i(gpio_out[3]),
      .gpio_3_o(gpio_in[3]),
      .gpio_3_oe_i(gpio_en[3]),
      .gpio_4_io(gpio_io[4]),
      .gpio_4_i(gpio_out[4]),
      .gpio_4_o(gpio_in[4]),
      .gpio_4_oe_i(gpio_en[4]),
      .gpio_5_io(gpio_io[5]),
      .gpio_5_i(gpio_out[5]),
      .gpio_5_o(gpio_in[5]),
      .gpio_5_oe_i(gpio_en[5]),
      .gpio_6_io(gpio_io[6]),
      .gpio_6_i(gpio_out[6]),
      .gpio_6_o(gpio_in[6]),
      .gpio_6_oe_i(gpio_en[6]),
      .gpio_7_io(gpio_io[7]),
      .gpio_7_i(gpio_out[7]),
      .gpio_7_o(gpio_in[7]),
      .gpio_7_oe_i(gpio_en[7]),
      .gpio_8_io(gpio_io[8]),
      .gpio_8_i(gpio_out[8]),
      .gpio_8_o(gpio_in[8]),
      .gpio_8_oe_i(gpio_en[8]),
      .gpio_9_io(gpio_io[9]),
      .gpio_9_i(gpio_out[9]),
      .gpio_9_o(gpio_in[9]),
      .gpio_9_oe_i(gpio_en[9]),
      .gpio_10_io(gpio_io[10]),
      .gpio_10_i(gpio_out[10]),
      .gpio_10_o(gpio_in[10]),
      .gpio_10_oe_i(gpio_en[10]),
      .gpio_11_io(gpio_io[11]),
      .gpio_11_i(gpio_out[11]),
      .gpio_11_o(gpio_in[11]),
      .gpio_11_oe_i(gpio_en[11]),
      .gpio_12_io(gpio_io[12]),
      .gpio_12_i(gpio_out[12]),
      .gpio_12_o(gpio_in[12]),
      .gpio_12_oe_i(gpio_en[12]),
      .gpio_13_io(gpio_io[13]),
      .gpio_13_i(gpio_out[13]),
      .gpio_13_o(gpio_in[13]),
      .gpio_13_oe_i(gpio_en[13]),
      .gpio_14_io(gpio_io[14]),
      .gpio_14_i(gpio_out[14]),
      .gpio_14_o(gpio_in[14]),
      .gpio_14_oe_i(gpio_en[14]),
      .gpio_15_io(gpio_io[15]),
      .gpio_15_i(gpio_out[15]),
      .gpio_15_o(gpio_in[15]),
      .gpio_15_oe_i(gpio_en[15]),
      .gpio_16_io(gpio_io[16]),
      .gpio_16_i(gpio_out[16]),
      .gpio_16_o(gpio_in[16]),
      .gpio_16_oe_i(gpio_en[16]),
      .gpio_17_io(gpio_io[17]),
      .gpio_17_i(gpio_out[17]),
      .gpio_17_o(gpio_in[17]),
      .gpio_17_oe_i(gpio_en[17]),
      .gpio_18_io(gpio_io[18]),
      .gpio_18_i(gpio_out[18]),
      .gpio_18_o(gpio_in[18]),
      .gpio_18_oe_i(gpio_en[18]),
      .gpio_19_io(gpio_io[19]),
      .gpio_19_i(gpio_out[19]),
      .gpio_19_o(gpio_in[19]),
      .gpio_19_oe_i(gpio_en[19]),
      .gpio_20_io(gpio_io[20]),
      .gpio_20_i(gpio_out[20]),
      .gpio_20_o(gpio_in[20]),
      .gpio_20_oe_i(gpio_en[20]),
      .gpio_21_io(gpio_io[21]),
      .gpio_21_i(gpio_out[21]),
      .gpio_21_o(gpio_in[21]),
      .gpio_21_oe_i(gpio_en[21]),
      .gpio_22_io(gpio_io[22]),
      .gpio_22_i(gpio_out[22]),
      .gpio_22_o(gpio_in[22]),
      .gpio_22_oe_i(gpio_en[22]),
      .gpio_23_io(gpio_io[23]),
      .gpio_23_i(gpio_out[23]),
      .gpio_23_o(gpio_in[23]),
      .gpio_23_oe_i(gpio_en[23]),
      .gpio_24_io(gpio_io[24]),
      .gpio_24_i(gpio_out[24]),
      .gpio_24_o(gpio_in[24]),
      .gpio_24_oe_i(gpio_en[24]),
      .gpio_25_io(gpio_io[25]),
      .gpio_25_i(gpio_out[25]),
      .gpio_25_o(gpio_in[25]),
      .gpio_25_oe_i(gpio_en[25]),
      .gpio_26_io(gpio_io[26]),
      .gpio_26_i(gpio_out[26]),
      .gpio_26_o(gpio_in[26]),
      .gpio_26_oe_i(gpio_en[26]),
      .gpio_27_io(gpio_io[27]),
      .gpio_27_i(gpio_out[27]),
      .gpio_27_o(gpio_in[27]),
      .gpio_27_oe_i(gpio_en[27]),
      .gpio_28_io(gpio_io[28]),
      .gpio_28_i(gpio_out[28]),
      .gpio_28_o(gpio_in[28]),
      .gpio_28_oe_i(gpio_en[28]),
      .gpio_29_io(gpio_io[29]),
      .gpio_29_i(gpio_out[29]),
      .gpio_29_o(gpio_in[29]),
      .gpio_29_oe_i(gpio_en[29]),
      .gpio_30_io(gpio_io[30]),
      .gpio_30_i(gpio_out[30]),
      .gpio_30_o(gpio_in[30]),
      .gpio_30_oe_i(gpio_en[30]),
      .gpio_31_io(gpio_io[31]),
      .gpio_31_i(gpio_out[31]),
      .gpio_31_o(gpio_in[31]),
      .gpio_31_oe_i(gpio_en[31]),
      .spi_flash_sck_io(spi_flash_sck_o),
      .spi_flash_sck_i(spi_flash_sck),
      .spi_flash_sck_o(),
      .spi_flash_sck_oe_i(spi_flash_sck_en),
      .spi_flash_cs_0_io(spi_flash_csb_o[0]),
      .spi_flash_cs_0_i(spi_flash_csb[0]),
      .spi_flash_cs_0_o(),
      .spi_flash_cs_0_oe_i(spi_flash_csb_en[0]),
      .spi_flash_cs_1_io(spi_flash_csb_o[1]),
      .spi_flash_cs_1_i(spi_flash_csb[1]),
      .spi_flash_cs_1_o(),
      .spi_flash_cs_1_oe_i(spi_flash_csb_en[1]),
      .spi_flash_sd_0_io(spi_flash_sd_io[0]),
      .spi_flash_sd_0_i(spi_flash_sd_out[0]),
      .spi_flash_sd_0_o(spi_flash_sd_in[0]),
      .spi_flash_sd_0_oe_i(spi_flash_sd_en[0]),
      .spi_flash_sd_1_io(spi_flash_sd_io[1]),
      .spi_flash_sd_1_i(spi_flash_sd_out[1]),
      .spi_flash_sd_1_o(spi_flash_sd_in[1]),
      .spi_flash_sd_1_oe_i(spi_flash_sd_en[1]),
      .spi_flash_sd_2_io(spi_flash_sd_io[2]),
      .spi_flash_sd_2_i(spi_flash_sd_out[2]),
      .spi_flash_sd_2_o(spi_flash_sd_in[2]),
      .spi_flash_sd_2_oe_i(spi_flash_sd_en[2]),
      .spi_flash_sd_3_io(spi_flash_sd_io[3]),
      .spi_flash_sd_3_i(spi_flash_sd_out[3]),
      .spi_flash_sd_3_o(spi_flash_sd_in[3]),
      .spi_flash_sd_3_oe_i(spi_flash_sd_en[3]),
      .spi_sck_io(spi_sck_o),
      .spi_sck_i(spi_sck),
      .spi_sck_o(),
      .spi_sck_oe_i(spi_sck_en),
      .spi_cs_0_io(spi_csb_o[0]),
      .spi_cs_0_i(spi_csb[0]),
      .spi_cs_0_o(),
      .spi_cs_0_oe_i(spi_csb_en[0]),
      .spi_cs_1_io(spi_csb_o[1]),
      .spi_cs_1_i(spi_csb[1]),
      .spi_cs_1_o(),
      .spi_cs_1_oe_i(spi_csb_en[1]),
      .spi_sd_0_io(spi_sd_io[0]),
      .spi_sd_0_i(spi_sd_out[0]),
      .spi_sd_0_o(spi_sd_in[0]),
      .spi_sd_0_oe_i(spi_sd_en[0]),
      .spi_sd_1_io(spi_sd_io[1]),
      .spi_sd_1_i(spi_sd_out[1]),
      .spi_sd_1_o(spi_sd_in[1]),
      .spi_sd_1_oe_i(spi_sd_en[1]),
      .spi_sd_2_io(spi_sd_io[2]),
      .spi_sd_2_i(spi_sd_out[2]),
      .spi_sd_2_o(spi_sd_in[2]),
      .spi_sd_2_oe_i(spi_sd_en[2]),
      .spi_sd_3_io(spi_sd_io[3]),
      .spi_sd_3_i(spi_sd_out[3]),
      .spi_sd_3_o(spi_sd_in[3]),
      .spi_sd_3_oe_i(spi_sd_en[3]),
      .i2c_scl_io(i2c_scl_io),
      .i2c_scl_i(cio_scl_out),
      .i2c_scl_o(cio_scl_in),
      .i2c_scl_oe_i(cio_scl_en),
      .i2c_sda_io(i2c_sda_io),
      .i2c_sda_i(cio_sda_out),
      .i2c_sda_o(cio_sda_in),
      .i2c_sda_oe_i(cio_sda_en),
      .pad_attributes_i(pad_attributes)
  );

endmodule  // core_v_mini_mcu

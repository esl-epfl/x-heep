// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module ao_peripheral_subsystem
  import obi_pkg::*;
  import reg_pkg::*;
(
    input logic clk_i,
    input logic rst_ni,

    input  obi_req_t  slave_req_i,
    output obi_resp_t slave_resp_o,

    // SOC CTRL
    input  logic        boot_select_i,
    input  logic        execute_from_flash_i,
    output logic        exit_valid_o,
    output logic [31:0] exit_value_o,

    // Memory Map SPI Region
    input  obi_req_t  spimemio_req_i,
    output obi_resp_t spimemio_resp_o,

    // SPI Interface
    output logic                               spi_sck_o,
    output logic                               spi_sck_en_o,
    output logic [spi_host_reg_pkg::NumCS-1:0] spi_csb_o,
    output logic [spi_host_reg_pkg::NumCS-1:0] spi_csb_en_o,
    output logic [                        3:0] spi_sd_o,
    output logic [                        3:0] spi_sd_en_o,
    input  logic [                        3:0] spi_sd_i,
    output logic                               spi_intr_o,

    // POWER MANAGER
    input  logic                                      core_sleep_i,
    output logic                                      cpu_subsystem_powergate_switch_o,
    output logic                                      peripheral_subsystem_powergate_switch_o,
    output logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_switches_o,
    output logic                                      cpu_subsystem_rst_no,
    output logic                                      peripheral_subsystem_rst_no,
    output logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_rst_no,

    //RV TIMER
    output logic rv_timer_0_intr_o,
    output logic rv_timer_1_intr_o,
    input  logic rv_timer_2_intr_i,
    input  logic rv_timer_3_intr_i,

    // DMA
    output obi_req_t  dma_master0_ch0_req_o,
    input  obi_resp_t dma_master0_ch0_resp_i,
    output obi_req_t  dma_master1_ch0_req_o,
    input  obi_resp_t dma_master1_ch0_resp_i,
    output logic      dma_intr_o,

    // GPIO
    input logic [7:0] gpio_intr_i,

    // External interrupts
    input logic [core_v_mini_mcu_pkg::NEXT_INT-1:0] ext_intr_i
);

  import core_v_mini_mcu_pkg::*;
  import tlul_pkg::*;
  import rv_plic_reg_pkg::*;

  reg_pkg::reg_req_t peripheral_req;
  reg_pkg::reg_rsp_t peripheral_rsp;

  reg_pkg::reg_req_t [core_v_mini_mcu_pkg::AO_PERIPHERALS-1:0] peripheral_slv_req;
  reg_pkg::reg_rsp_t [core_v_mini_mcu_pkg::AO_PERIPHERALS-1:0] peripheral_slv_rsp;

  tlul_pkg::tl_h2d_t rv_timer_tl_h2d;
  tlul_pkg::tl_d2h_t rv_timer_tl_d2h;

  logic [AO_PERIPHERALS_PORT_SEL_WIDTH-1:0] peripheral_select;

  logic use_spimemio;
  logic rv_timer_0_intr;
  logic rv_timer_1_intr;
  logic spi_intr;
  logic dma_intr;

  periph_to_reg #(
      .req_t(reg_pkg::reg_req_t),
      .rsp_t(reg_pkg::reg_rsp_t),
      .IW(1)
  ) periph_to_reg_i (
      .clk_i,
      .rst_ni,
      .req_i(slave_req_i.req),
      .add_i(slave_req_i.addr),
      .wen_i(~slave_req_i.we),
      .wdata_i(slave_req_i.wdata),
      .be_i(slave_req_i.be),
      .id_i('0),
      .gnt_o(slave_resp_o.gnt),
      .r_rdata_o(slave_resp_o.rdata),
      .r_opc_o(),
      .r_id_o(),
      .r_valid_o(slave_resp_o.rvalid),
      .reg_req_o(peripheral_req),
      .reg_rsp_i(peripheral_rsp)
  );

  addr_decode #(
      .NoIndices(core_v_mini_mcu_pkg::AO_PERIPHERALS),
      .NoRules(core_v_mini_mcu_pkg::AO_PERIPHERALS),
      .addr_t(logic [31:0]),
      .rule_t(addr_map_rule_pkg::addr_map_rule_t)
  ) i_addr_decode_soc_regbus_periph_xbar (
      .addr_i(peripheral_req.addr),
      .addr_map_i(core_v_mini_mcu_pkg::AO_PERIPHERALS_ADDR_RULES),
      .idx_o(peripheral_select),
      .dec_valid_o(),
      .dec_error_o(),
      .en_default_idx_i(1'b0),
      .default_idx_i('0)
  );

  reg_demux #(
      .NoPorts(core_v_mini_mcu_pkg::AO_PERIPHERALS),
      .req_t  (reg_pkg::reg_req_t),
      .rsp_t  (reg_pkg::reg_rsp_t)
  ) reg_demux_i (
      .clk_i,
      .rst_ni,
      .in_select_i(peripheral_select),
      .in_req_i(peripheral_req),
      .in_rsp_o(peripheral_rsp),
      .out_req_o(peripheral_slv_req),
      .out_rsp_i(peripheral_slv_rsp)
  );

  soc_ctrl #(
      .reg_req_t(reg_pkg::reg_req_t),
      .reg_rsp_t(reg_pkg::reg_rsp_t)
  ) soc_ctrl_i (
      .clk_i,
      .rst_ni,
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::SOC_CTRL_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::SOC_CTRL_IDX]),
      .boot_select_i,
      .execute_from_flash_i,
      .use_spimemio_o(use_spimemio),
      .exit_valid_o,
      .exit_value_o
  );

  boot_rom boot_rom_i (
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::BOOTROM_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::BOOTROM_IDX])
  );

  spi_subsystem spi_subsystem_i (
      .clk_i,
      .rst_ni,
      .use_spimemio_i(use_spimemio),
      .spimemio_req_i,
      .spimemio_resp_o,
      .yo_reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::SPI_MEMIO_IDX]),
      .yo_reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::SPI_MEMIO_IDX]),
      .ot_reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::SPI_HOST_IDX]),
      .ot_reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::SPI_HOST_IDX]),
      .spi_sck_o,
      .spi_sck_en_o,
      .spi_csb_o,
      .spi_csb_en_o,
      .spi_sd_o,
      .spi_sd_en_o,
      .spi_sd_i,
      .spi_intr_o(spi_intr)
  );

  assign spi_intr_o = spi_intr;

  power_manager #(
      .reg_req_t(reg_pkg::reg_req_t),
      .reg_rsp_t(reg_pkg::reg_rsp_t)
  ) power_manager_i (
      .clk_i,
      .rst_ni,
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::POWER_MANAGER_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::POWER_MANAGER_IDX]),
      .spi_intr_i(spi_intr),
      .rv_timer_0_irq_i(rv_timer_0_intr),
      .rv_timer_1_irq_i(rv_timer_1_intr),
      .rv_timer_2_irq_i(rv_timer_2_intr_i),
      .rv_timer_3_irq_i(rv_timer_3_intr_i),
      .dma_irq_i(dma_intr),
      .gpio_irq_i(gpio_intr_i),
      .ext_irq_i(ext_intr_i),
      .core_sleep_i,
      .cpu_subsystem_powergate_switch_o,
      .peripheral_subsystem_powergate_switch_o,
      .memory_subsystem_banks_powergate_switches_o,
      .cpu_subsystem_rst_no,
      .peripheral_subsystem_rst_no,
      .memory_subsystem_rst_no
  );

  reg_to_tlul rv_timer_reg_to_tlul_i (
      .tl_o(rv_timer_tl_h2d),
      .tl_i(rv_timer_tl_d2h),
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::RV_TIMER_AO_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::RV_TIMER_AO_IDX])
  );

  rv_timer rv_timer_0_1_i (
      .clk_i,
      .rst_ni,
      .tl_i(rv_timer_tl_h2d),
      .tl_o(rv_timer_tl_d2h),
      .intr_timer_expired_0_0_o(rv_timer_0_intr),
      .intr_timer_expired_1_0_o(rv_timer_1_intr)
  );

  assign rv_timer_0_intr_o = rv_timer_0_intr;
  assign rv_timer_1_intr_o = rv_timer_1_intr;

  dma #(
      .reg_req_t (reg_pkg::reg_req_t),
      .reg_rsp_t (reg_pkg::reg_rsp_t),
      .obi_req_t (obi_pkg::obi_req_t),
      .obi_resp_t(obi_pkg::obi_resp_t)
  ) dma_i (
      .clk_i,
      .rst_ni,
      .reg_req_i (peripheral_slv_req[core_v_mini_mcu_pkg::DMA_IDX]),
      .reg_rsp_o (peripheral_slv_rsp[core_v_mini_mcu_pkg::DMA_IDX]),
      .dma_master0_ch0_req_o,
      .dma_master0_ch0_resp_i,
      .dma_master1_ch0_req_o,
      .dma_master1_ch0_resp_i,
      .dma_intr_o(dma_intr)
  );

  assign dma_intr_o = dma_intr;

endmodule : ao_peripheral_subsystem

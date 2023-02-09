// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module linux_femu
  import obi_pkg::*;
  import reg_pkg::*;
#(
  parameter PULP_XPULP           = 0,
  parameter FPU                  = 0,
  parameter PULP_ZFINX           = 0,
  parameter EXT_XBAR_NMASTER     = 0,
  parameter CLK_LED_COUNT_LENGTH = 27
) (
  inout logic clk_in,
  inout logic rst_i,

  output logic rst_led,
  output logic clk_led,
  output logic clk_out,

  inout logic boot_select_i,
  inout logic execute_from_flash_i,

  inout logic [29:0] gpio_io,

  output logic exit_value_o,
  inout  logic exit_valid_o,

  inout logic spi_sd_0_io,
  inout logic spi_sd_1_io,
  inout logic spi_sd_2_io,
  inout logic spi_sd_3_io,
  inout logic spi_csb_io,
  inout logic spi_sck_io,

  inout logic spi2_sd_0_io,
  inout logic spi2_sd_1_io,
  inout logic spi2_sd_2_io,
  inout logic spi2_sd_3_io,
  inout logic [1:0] spi2_csb_io,
  inout logic spi2_sck_io,

  inout logic i2c_scl_io,
  inout logic i2c_sda_io,

  inout wire [14:0] DDR_addr,
  inout wire [2:0] DDR_ba,
  inout wire DDR_cas_n,
  inout wire DDR_ck_n,
  inout wire DDR_ck_p,
  inout wire DDR_cke,
  inout wire DDR_cs_n,
  inout wire [3:0] DDR_dm,
  inout wire [31:0] DDR_dq,
  inout wire [3:0] DDR_dqs_n,
  inout wire [3:0] DDR_dqs_p,
  inout wire DDR_odt,
  inout wire DDR_ras_n,
  inout wire DDR_reset_n,
  inout wire DDR_we_n,
  inout wire FIXED_IO_ddr_vrn,
  inout wire FIXED_IO_ddr_vrp,
  inout wire [53:0] FIXED_IO_mio,
  inout wire FIXED_IO_ps_clk,
  inout wire FIXED_IO_ps_porb,
  inout wire FIXED_IO_ps_srstb
);

  import core_v_mini_mcu_pkg::*;

  parameter AXI_ADDR_WIDTH = 32;
  parameter AXI_ADDR_WIDTH_SLAVE = 4;
  parameter AXI_DATA_WIDTH = 32;

  // PM signals
  logic cpu_subsystem_powergate_switch;
  logic cpu_subsystem_powergate_switch_ack;
  logic cpu_subsystem_powergate_iso;
  logic cpu_subsystem_rst_n;
  logic peripheral_subsystem_powergate_switch;
  logic peripheral_subsystem_powergate_switch_ack;
  logic peripheral_subsystem_powergate_iso;
  logic peripheral_subsystem_rst_n;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_switch;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_switch_ack;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_iso;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_set_retentive;

  // PS SIDE PORTS
  logic AXI_HP_ACLK;
  logic AXI_HP_ARESETN;
  logic [AXI_ADDR_WIDTH - 1:0] AXI_HP_araddr_sig;
  logic [1:0] AXI_HP_arburst_sig;
  logic [3:0] AXI_HP_arcache_sig;
  logic [5:0] AXI_HP_arid_sig;
  logic [3:0] AXI_HP_arlen_sig;
  logic [1:0] AXI_HP_arlock_sig;
  logic [2:0] AXI_HP_arprot_sig;
  logic [3:0] AXI_HP_arqos_sig;
  logic AXI_HP_arready_sig;
  logic [2:0] AXI_HP_arsize_sig;
  logic AXI_HP_arvalid_sig;
  logic [AXI_ADDR_WIDTH - 1:0] AXI_HP_awaddr_sig;
  logic [1:0] AXI_HP_awburst_sig;
  logic [3:0] AXI_HP_awcache_sig;
  logic [5:0] AXI_HP_awid_sig;
  logic [3:0] AXI_HP_awlen_sig;
  logic [1:0] AXI_HP_awlock_sig;
  logic [2:0] AXI_HP_awprot_sig;
  logic [3:0] AXI_HP_awqos_sig;
  logic AXI_HP_awready_sig;
  logic [2:0] AXI_HP_awsize_sig;
  logic AXI_HP_awvalid_sig;
  logic [5:0] AXI_HP_bid_sig;
  logic AXI_HP_bready_sig;
  logic [1:0] AXI_HP_bresp_sig;
  logic AXI_HP_bvalid_sig;
  logic [AXI_DATA_WIDTH - 1:0] AXI_HP_rdata_sig;
  logic [5:0] AXI_HP_rid_sig;
  logic AXI_HP_rlast_sig;
  logic AXI_HP_rready_sig;
  logic [1:0] AXI_HP_rresp_sig;
  logic AXI_HP_rvalid_sig;
  logic [AXI_DATA_WIDTH - 1:0] AXI_HP_wdata_sig;
  logic [5:0] AXI_HP_wid_sig;
  logic AXI_HP_wlast_sig;
  logic AXI_HP_wready_sig;
  logic [3:0] AXI_HP_wstrb_sig;
  logic AXI_HP_wvalid_sig;

  logic spi_test_clk_sig;
  logic spi_test_cs_sig;
  logic [3:0] spi_test_data_sig;

  // ADDRESS HIJACKER PORTS
  logic [AXI_ADDR_WIDTH-1:0] axi_master_awaddr_in_sig;
  logic [AXI_ADDR_WIDTH-1:0] axi_master_araddr_in_sig;

  logic [AXI_ADDR_WIDTH_SLAVE - 1 : 0] s00_axi_awaddr_sig;
  logic s00_axi_awvalid_sig;
  logic s00_axi_awready_sig;
  logic [AXI_DATA_WIDTH - 1 : 0] s00_axi_wdata_sig;
  logic s00_axi_wvalid_sig;
  logic s00_axi_wready_sig;
  logic s00_axi_bvalid_sig;
  logic s00_axi_bready_sig;
  logic [(AXI_DATA_WIDTH / 8)-1 : 0] s00_axi_wstrb_sig;
  logic [2 : 0] s00_axi_arprot_sig;
  logic [2 : 0] s00_axi_awprot_sig;
  logic [AXI_ADDR_WIDTH_SLAVE - 1 : 0] s00_axi_araddr_sig;
  logic s00_axi_arvalid_sig;
  logic s00_axi_arready_sig;
  logic [AXI_DATA_WIDTH - 1 : 0] s00_axi_rdata_sig;
  logic s00_axi_rvalid_sig;
  logic s00_axi_rready_sig;
  logic [1:0] s00_axi_rresp_sig;
  logic [1:0] s00_axi_bresp_sig;

  // PAD controller
  reg_req_t pad_req;
  reg_rsp_t pad_resp;
  logic [core_v_mini_mcu_pkg::NUM_PAD-1:0][7:0] pad_attributes;
  logic [core_v_mini_mcu_pkg::NUM_PAD-1:0][3:0] pad_muxes;

  logic rst_ngen;

  // input, output pins from core_v_mini_mcu
% for pad in total_pad_list:
${pad.internal_signals}
% endfor

  wire                               clk_gen;
  logic [                      31:0] exit_value;
  wire                               rst_n;
  logic [CLK_LED_COUNT_LENGTH - 1:0] clk_count;

  // low active reset
  assign rst_n   = !rst_i;

  // reset LED for debugging
  assign rst_led = rst_n;

  // counter to blink an LED
  assign clk_led = clk_count[CLK_LED_COUNT_LENGTH-1];

  always_ff @(posedge clk_gen or negedge rst_n) begin : clk_count_process
    if (!rst_n) begin
      clk_count <= '0;
    end else begin
      clk_count <= clk_count + 1;
    end
  end

  // clock output for debugging
  assign clk_out = clk_gen;

  xilinx_clk_wizard_wrapper xilinx_clk_wizard_wrapper_i (
    .clk_125MHz(clk_in),
    .clk_out1_0(clk_gen)
  );

  logic clk_i;
  assign clk_i = clk_gen;

  core_v_mini_mcu #(
    .PULP_XPULP(PULP_XPULP),
    .FPU(FPU),
    .PULP_ZFINX(PULP_ZFINX),
    .EXT_XBAR_NMASTER(EXT_XBAR_NMASTER)
  ) core_v_mini_mcu_i (

    .rst_ni(rst_ngen),
% for pad in pad_list:
${pad.core_v_mini_mcu_bonding}
% endfor
    .intr_vector_ext_i('0),
    .ext_xbar_master_req_i('0),
    .ext_xbar_master_resp_o(),
    .ext_xbar_slave_req_o(),
    .ext_xbar_slave_resp_i('0),
    .ext_peripheral_slave_req_o(),
    .ext_peripheral_slave_resp_i('0),
    .external_subsystem_powergate_switch_o(),
    .external_subsystem_powergate_switch_ack_i(),
    .external_subsystem_powergate_iso_o(),
    .external_subsystem_rst_no(),
    .external_ram_banks_set_retentive_o(),
    .exit_value_o(exit_value),
    .pad_req_o(pad_req),
    .pad_resp_i(pad_resp),
    .cpu_subsystem_powergate_switch_o(cpu_subsystem_powergate_switch),
    .cpu_subsystem_powergate_switch_ack_i(cpu_subsystem_powergate_switch_ack),
    .peripheral_subsystem_powergate_switch_o(peripheral_subsystem_powergate_switch),
    .peripheral_subsystem_powergate_switch_ack_i(peripheral_subsystem_powergate_switch_ack),
    .memory_subsystem_banks_powergate_switch_o(memory_subsystem_banks_powergate_switch),
    .memory_subsystem_banks_powergate_switch_ack_i(memory_subsystem_banks_powergate_switch_ack)
  );

  logic gpio_0_io;
  logic gpio_1_io;
  logic gpio_2_io;
  logic gpio_3_io;
  logic gpio_4_io;
  logic gpio_5_io;
  logic gpio_6_io;
  logic gpio_7_io;
  logic gpio_8_io;
  logic gpio_9_io;
  logic gpio_10_io;
  logic gpio_11_io;
  logic gpio_12_io;
  logic gpio_13_io;
  logic gpio_14_io;
  logic gpio_15_io;
  logic gpio_16_io;
  logic gpio_17_io;
  logic gpio_18_io;
  logic gpio_19_io;
  logic gpio_20_io;
  logic gpio_21_io;
  logic gpio_22_io;

  assign spi_flash_sck_o_sig = spi_flash_sck_out_x;
  assign spi_flash_csb_o_sig = spi_flash_cs_0_out_x;
  assign spi_sdi1_sig = spi_flash_sd_1_in_x;
  assign spi_sdo0_sig = spi_flash_sd_0_out_x;
  assign spi_sdo2_sig = spi_sd_2_out_x;
  assign spi_sdo3_sig = spi_sd_3_out_x;

  assign spi2_csb_io[0] = spi2_cs_0_io;
  assign spi2_csb_io[1] = spi2_cs_1_io;

  assign gpio_io[0] = gpio_0_io;
  assign gpio_io[1] = gpio_1_io;
  assign gpio_io[2] = gpio_2_io;
  assign gpio_io[3] = gpio_3_io;
  assign gpio_io[4] = gpio_4_io;
  assign gpio_io[5] = gpio_5_io;
  assign gpio_io[6] = gpio_6_io;
  assign gpio_io[7] = gpio_7_io;
  assign gpio_io[8] = gpio_8_io;
  assign gpio_io[9] = gpio_9_io;
  assign gpio_io[10] = gpio_10_io;
  assign gpio_io[11] = gpio_11_io;
  assign gpio_io[12] = gpio_12_io;
  assign gpio_io[13] = gpio_13_io;
  assign gpio_io[14] = gpio_14_io;
  assign gpio_io[15] = gpio_15_io;
  assign gpio_io[16] = gpio_16_io;
  assign gpio_io[17] = gpio_17_io;
  assign gpio_io[18] = gpio_18_io;
  assign gpio_io[19] = gpio_19_io;
  assign gpio_io[20] = gpio_20_io;
  assign gpio_io[21] = gpio_21_io;
  assign gpio_io[22] = gpio_22_io;

  processing_system_wrapper processing_system_wrapper_i (
    .DDR_addr(DDR_addr),
    .DDR_ba(DDR_ba),
    .DDR_cas_n(DDR_cas_n),
    .DDR_ck_n(DDR_ck_n),
    .DDR_ck_p(DDR_ck_p),
    .DDR_cke(DDR_cke),
    .DDR_cs_n(DDR_cs_n),
    .DDR_dm(DDR_dm),
    .DDR_dq(DDR_dq),
    .DDR_dqs_n(DDR_dqs_n),
    .DDR_dqs_p(DDR_dqs_p),
    .DDR_odt(DDR_odt),
    .DDR_ras_n(DDR_ras_n),
    .DDR_reset_n(DDR_reset_n),
    .DDR_we_n(DDR_we_n),
    .FIXED_IO_ddr_vrn(FIXED_IO_ddr_vrn),
    .FIXED_IO_ddr_vrp(FIXED_IO_ddr_vrp),
    .FIXED_IO_mio(FIXED_IO_mio),
    .FIXED_IO_ps_clk(FIXED_IO_ps_clk),
    .FIXED_IO_ps_porb(FIXED_IO_ps_porb),
    .FIXED_IO_ps_srstb(FIXED_IO_ps_srstb),
    .UART_rxd(uart_tx_out_x),
    .UART_txd(uart_rx_in_x),
    .gpio_jtag_tck_i(jtag_tck_in_x),
    .gpio_jtag_tms_i(jtag_tms_in_x),
    .gpio_jtag_trst_ni(jtag_trst_nin_x),
    .gpio_jtag_tdi_i(jtag_tdi_in_x),
    .gpio_jtag_tdo_o(jtag_tdo_out_x),
    .AXI_HP_ACLK(AXI_HP_ACLK),
    .AXI_HP_ARESETN(AXI_HP_ARESETN),
    .AXI_HP_araddr(AXI_HP_araddr_sig),
    .AXI_HP_arburst(AXI_HP_arburst_sig),
    .AXI_HP_arcache(AXI_HP_arcache_sig),
    .AXI_HP_arlen(AXI_HP_arlen_sig),
    .AXI_HP_arlock(AXI_HP_arlock_sig),
    .AXI_HP_arprot(AXI_HP_arprot_sig),
    .AXI_HP_arqos(AXI_HP_arqos_sig),
    .AXI_HP_arready(AXI_HP_arready_sig),
    .AXI_HP_arsize(AXI_HP_arsize_sig),
    .AXI_HP_arvalid(AXI_HP_arvalid_sig),
    .AXI_HP_awaddr(AXI_HP_awaddr_sig),
    .AXI_HP_awburst(AXI_HP_awburst_sig),
    .AXI_HP_awcache(AXI_HP_awcache_sig),
    .AXI_HP_awlen(AXI_HP_awlen_sig),
    .AXI_HP_awlock(AXI_HP_awlock_sig),
    .AXI_HP_awprot(AXI_HP_awprot_sig),
    .AXI_HP_awqos(AXI_HP_awqos_sig),
    .AXI_HP_awready(AXI_HP_awready_sig),
    .AXI_HP_awsize(AXI_HP_awsize_sig),
    .AXI_HP_awvalid(AXI_HP_awvalid_sig),
    .AXI_HP_bready(AXI_HP_bready_sig),
    .AXI_HP_bresp(AXI_HP_bresp_sig),
    .AXI_HP_bvalid(AXI_HP_bvalid_sig),
    .AXI_HP_rdata(AXI_HP_rdata_sig),
    .AXI_HP_rlast(AXI_HP_rlast_sig),
    .AXI_HP_rready(AXI_HP_rready_sig),
    .AXI_HP_rresp(AXI_HP_rresp_sig),
    .AXI_HP_rvalid(AXI_HP_rvalid_sig),
    .AXI_HP_wdata(AXI_HP_wdata_sig),
    .AXI_HP_wlast(AXI_HP_wlast_sig),
    .AXI_HP_wready(AXI_HP_wready_sig),
    .AXI_HP_wstrb(AXI_HP_wstrb_sig),
    .AXI_HP_wvalid(AXI_HP_wvalid_sig),
    .M_AXI_araddr(s00_axi_araddr_sig),
    .M_AXI_arready(s00_axi_arready_sig),
    .M_AXI_arvalid(s00_axi_arvalid_sig),
    .M_AXI_awaddr(s00_axi_awaddr_sig),
    .M_AXI_awready(s00_axi_awready_sig),
    .M_AXI_awvalid(s00_axi_awvalid_sig),
    .M_AXI_bready(s00_axi_bready_sig),
    .M_AXI_bresp(s00_axi_bresp_sig),
    .M_AXI_bvalid(s00_axi_bvalid_sig),
    .M_AXI_rdata(s00_axi_rdata_sig),
    .M_AXI_rready(s00_axi_rready_sig),
    .M_AXI_rresp(s00_axi_rresp_sig),
    .M_AXI_rvalid(s00_axi_rvalid_sig),
    .M_AXI_wdata(s00_axi_wdata_sig),
    .M_AXI_wready(s00_axi_wready_sig),
    .M_AXI_wvalid(s00_axi_wvalid_sig),
    .M_AXI_awprot(s00_axi_awprot_sig),
    .M_AXI_arprot(s00_axi_arprot_sig),
    .M_AXI_wstrb(s00_axi_wstrb_sig)
  );

  axi_address_hijacker #(
    .AXI_ADDR_WIDTH(AXI_ADDR_WIDTH),
    .C_S_AXI_DATA_WIDTH(AXI_DATA_WIDTH)
  ) add_hij (
    .axi_master_awaddr_in(axi_master_awaddr_in_sig),
    .axi_master_araddr_in(axi_master_araddr_in_sig),

    // output write and read address by adding fixed offset
    .axi_master_araddr_out(AXI_HP_araddr_sig),
    .axi_master_awaddr_out(AXI_HP_awaddr_sig),

    .S_AXI_ACLK(AXI_HP_ACLK),
    .S_AXI_ARESETN(AXI_HP_ARESETN),

    .S_AXI_AWADDR (s00_axi_awaddr_sig),
    .S_AXI_AWPROT (s00_axi_awprot_sig),
    .S_AXI_AWVALID(s00_axi_awvalid_sig),
    .S_AXI_AWREADY(s00_axi_awready_sig),
    .S_AXI_WDATA  (s00_axi_wdata_sig),
    .S_AXI_WSTRB  (s00_axi_wstrb_sig),
    .S_AXI_WVALID (s00_axi_wvalid_sig),
    .S_AXI_WREADY (s00_axi_wready_sig),
    .S_AXI_BRESP  (s00_axi_bresp_sig),
    .S_AXI_BVALID (s00_axi_bvalid_sig),
    .S_AXI_BREADY (s00_axi_bready_sig),
    .S_AXI_ARADDR (s00_axi_araddr_sig),
    .S_AXI_ARPROT (s00_axi_arprot_sig),
    .S_AXI_ARVALID(s00_axi_arvalid_sig),
    .S_AXI_ARREADY(s00_axi_arready_sig),
    .S_AXI_RDATA  (s00_axi_rdata_sig),
    .S_AXI_RRESP  (s00_axi_rresp_sig),
    .S_AXI_RVALID (s00_axi_rvalid_sig),
    .S_AXI_RREADY (s00_axi_rready_sig)
  );

  axi_spi_slave #(
    .AXI_DATA_WIDTH(AXI_DATA_WIDTH)
  ) fake_flash (
    .axi_aclk(AXI_HP_ACLK),
    .axi_aresetn(AXI_HP_ARESETN),

    .test_mode('0),

    .axi_master_aw_valid(AXI_HP_awvalid_sig),
    .axi_master_aw_id(AXI_HP_awid_sig),
    .axi_master_aw_prot(AXI_HP_awprot_sig),
    .axi_master_aw_qos(AXI_HP_awqos_sig),
    .axi_master_aw_cache(AXI_HP_awcache_sig),
    .axi_master_aw_lock(AXI_HP_awlock_sig),
    .axi_master_aw_burst(AXI_HP_awburst_sig),
    .axi_master_aw_size(AXI_HP_awsize_sig),
    .axi_master_aw_len(AXI_HP_awlen_sig),
    .axi_master_aw_addr(axi_master_awaddr_in_sig),
    .axi_master_aw_ready(AXI_HP_awready_sig),

    .axi_master_w_valid(AXI_HP_wvalid_sig),
    .axi_master_w_data (AXI_HP_wdata_sig),
    .axi_master_w_strb (AXI_HP_wstrb_sig),
    .axi_master_w_last (AXI_HP_wlast_sig),
    .axi_master_w_ready(AXI_HP_wready_sig),

    .axi_master_b_valid(AXI_HP_bvalid_sig),
    .axi_master_b_id(AXI_HP_bid_sig),
    .axi_master_b_resp(AXI_HP_bresp_sig),
    .axi_master_b_ready(AXI_HP_bready_sig),

    .axi_master_ar_valid(AXI_HP_arvalid_sig),
    .axi_master_ar_id(AXI_HP_arid_sig),
    .axi_master_ar_prot(AXI_HP_arprot_sig),
    .axi_master_ar_qos(AXI_HP_arqos_sig),
    .axi_master_ar_cache(AXI_HP_arcache_sig),
    .axi_master_ar_lock(AXI_HP_arlock_sig),
    .axi_master_ar_burst(AXI_HP_arburst_sig),
    .axi_master_ar_size(AXI_HP_arsize_sig),
    .axi_master_ar_len(AXI_HP_arlen_sig),
    .axi_master_ar_addr(axi_master_araddr_in_sig),
    .axi_master_ar_ready(AXI_HP_arready_sig),

    .axi_master_r_valid(AXI_HP_rvalid_sig),
    .axi_master_r_id(AXI_HP_rid_sig),
    .axi_master_r_data(AXI_HP_rdata_sig),
    .axi_master_r_resp(AXI_HP_rresp_sig),
    .axi_master_r_last(AXI_HP_rlast_sig),
    .axi_master_r_ready(AXI_HP_rready_sig),

    .spi_sclk(spi_flash_sck_o_sig),
    .spi_cs  (spi_flash_csb_o_sig),
    .spi_sdo1(spi_sdi1_sig),
    .spi_sdi0(spi_sdo0_sig),
    .spi_sdi2(spi_sdo2_sig),
    .spi_sdi3(spi_sdo3_sig)
  );

  // TESTING PURPOSES -> THEY WILL BE INPUT TO PS AND READ BY SYSTEM ILA
  assign spi_test_clk_sig  = spi_flash_sck_o_sig;
  assign spi_test_cs_sig   = spi_flash_csb_o_sig;
  assign spi_test_data_sig = {spi_sdo0_sig, spi_sdi1_sig, spi_sdo2_sig, spi_sdo3_sig};

  pad_ring pad_ring_i (
% for pad in total_pad_list:
${pad.pad_ring_bonding_bonding}
% endfor
    .pad_attributes_i(pad_attributes)
  );

${pad_constant_driver_assign}

${pad_mux_process}

  pad_control #(
    .reg_req_t(reg_pkg::reg_req_t),
    .reg_rsp_t(reg_pkg::reg_rsp_t),
    .NUM_PAD  (core_v_mini_mcu_pkg::NUM_PAD)
  ) pad_control_i (
    .clk_i(clk_in_x),
    .rst_ni(rst_ngen),
    .reg_req_i(pad_req),
    .reg_rsp_o(pad_resp),
    .pad_attributes_o(pad_attributes),
    .pad_muxes_o(pad_muxes)
  );

  rstgen rstgen_i (
    .clk_i(clk_in_x),
    .rst_ni(rst_n),
    .test_mode_i(1'b0),
    .rst_no(rst_ngen),
    .init_no()
  );

  assign exit_value_o = exit_value[0];
endmodule

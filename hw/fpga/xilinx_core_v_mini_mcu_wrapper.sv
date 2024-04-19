// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module xilinx_core_v_mini_mcu_wrapper
  import obi_pkg::*;
  import reg_pkg::*;
#(
    parameter COREV_PULP           = 0,
    parameter FPU                  = 0,
    parameter ZFINX                = 0,
    parameter X_EXT                = 0,  // eXtension interface in cv32e40x
    parameter CLK_LED_COUNT_LENGTH = 27
) 
(

`ifdef FPGA_ZCU104
    inout logic clk_300mhz_n,
    inout logic clk_300mhz_p,
`else
    inout logic clk_i,
`endif
    inout logic rst_i,

    output logic rst_led_o,
    output logic clk_led_o,

    inout logic boot_select_i,
    inout logic execute_from_flash_i,

    inout logic jtag_tck_i,
    inout logic jtag_tms_i,
    inout logic jtag_trst_ni,
    inout logic jtag_tdi_i,
    inout logic jtag_tdo_o,

    inout logic uart_rx_i,
    inout logic uart_tx_o,

    inout logic [17:0] gpio_io,

    output logic exit_value_o,
    inout  logic exit_valid_o,

    inout logic [3:0] spi_flash_sd_io,
    inout logic spi_flash_csb_o,
    inout logic spi_flash_sck_o,

    inout logic [3:0] spi_sd_io,
    inout logic spi_csb_o,
    inout logic spi_sck_o,

    inout logic [3:0] spi2_sd_io,
    inout logic [1:0] spi2_csb_o,
    inout logic spi2_sck_o,

    inout logic i2c_scl_io,
    inout logic i2c_sda_io,

    inout logic pdm2pcm_clk_io,
    inout logic pdm2pcm_pdm_io,

    inout logic i2s_sck_io,
    inout logic i2s_ws_io,
    inout logic i2s_sd_io

);
  // ...................
  // OBI INTERFACE signals
  obi_req_t obi_req;
  obi_resp_t obi_resp;

  obi_req_t r_obi_req;
  obi_resp_t r_obi_resp;
  // ...................
  // AXI OBI Master-slave 
  
  logic [AXI_ADDR_WIDTH - 1:0] AXI_M_OBI_araddr_sig;
  logic [1:0] AXI_M_OBI_arburst_sig;
  logic [3:0] AXI_M_OBI_arcache_sig;
  logic [1:0] AXI_M_OBI_arid_sig;
  logic [7:0] AXI_M_OBI_arlen_sig;
  logic [0:0] AXI_M_OBI_arlock_sig;
  logic [2:0] AXI_M_OBI_arprot_sig;
  logic [3:0] AXI_M_OBI_arqos_sig;
  logic AXI_M_OBI_arready_sig;
  logic [3:0] AXI_M_OBI_arregion;
  logic [2:0] AXI_M_OBI_arsize_sig;
  logic AXI_M_OBI_arvalid_sig;
  logic [AXI_ADDR_WIDTH - 1:0] AXI_M_OBI_awaddr_sig;
  logic [1:0] AXI_M_OBI_awburst_sig;
  logic [3:0] AXI_M_OBI_awcache_sig;
  logic [1:0] AXI_M_OBI_awid_sig;
  logic [7:0] AXI_M_OBI_awlen_sig;
  logic [0:0] AXI_M_OBI_awlock_sig;
  logic [2:0] AXI_M_OBI_awprot_sig;
  logic [3:0] AXI_M_OBI_awqos_sig;
  logic AXI_M_OBI_awready_sig;
  logic [3:0] AXI_M_OBI_awregion;
  logic [2:0] AXI_M_OBI_awsize_sig;
  logic AXI_M_OBI_awvalid_sig;
  logic [1:0] AXI_M_OBI_bid_sig;
  logic AXI_M_OBI_bready_sig;
  logic [1:0] AXI_M_OBI_bresp_sig;
  logic AXI_M_OBI_bvalid_sig;
  logic [AXI_DATA_WIDTH - 1:0] AXI_M_OBI_rdata_sig;
  logic [1:0] AXI_M_OBI_rid_sig;
  logic AXI_M_OBI_rlast_sig;
  logic AXI_M_OBI_rready_sig;
  logic [1:0] AXI_M_OBI_rresp_sig;
  logic AXI_M_OBI_rvalid_sig;
  logic [AXI_DATA_WIDTH - 1:0] AXI_M_OBI_wdata_sig;
  logic AXI_M_OBI_wlast_sig;
  logic AXI_M_OBI_wready_sig;
  logic [3:0] AXI_M_OBI_wstrb_sig;
  logic AXI_M_OBI_wvalid_sig;

  logic [3 : 0] AXI_S_OBI_awaddr_sig;
  logic [2:0] AXI_S_OBI_awprot_sig;
  logic AXI_S_OBI_awready_sig;
  logic AXI_S_OBI_awvalid_sig;
  logic [AXI_DATA_WIDTH - 1 : 0] AXI_S_OBI_wdata_sig;
  logic AXI_S_OBI_wready_sig;
  logic [(AXI_DATA_WIDTH / 8)-1 : 0] AXI_S_OBI_wstrb_sig;
  logic AXI_S_OBI_wvalid_sig;
  logic AXI_S_OBI_bready_sig;
  logic [1:0] AXI_S_OBI_bresp_sig;
  logic AXI_S_OBI_bvalid_sig;
  logic [3 : 0] AXI_S_OBI_araddr_sig;
  logic [2 : 0] AXI_S_OBI_arprot_sig;
  logic AXI_S_OBI_arready_sig;
  logic AXI_S_OBI_arvalid_sig;
  logic [AXI_DATA_WIDTH - 1 : 0] AXI_S_OBI_rdata_sig;
  logic AXI_S_OBI_rready_sig;
  logic [1:0] AXI_S_OBI_rresp_sig;
  logic AXI_S_OBI_rvalid_sig;


  logic [AXI_ADDR_WIDTH-1:0] AXI_M_OBI_awaddr_in_sig;
  logic [AXI_ADDR_WIDTH-1:0] AXI_M_OBI_araddr_in_sig;

  // ...................



  wire                               clk_gen;
  logic [                      31:0] exit_value;
  wire                               rst_n;
  logic [CLK_LED_COUNT_LENGTH - 1:0] clk_count;

  // low active reset
`ifdef FPGA_NEXYS
  assign rst_n = rst_i;
`else
  assign rst_n = !rst_i;
`endif

  // reset LED for debugging
  assign rst_led_o = rst_n;

  // counter to blink an LED
  assign clk_led_o = clk_count[CLK_LED_COUNT_LENGTH-1];

  always_ff @(posedge clk_gen or negedge rst_n) begin : clk_count_process
    if (!rst_n) begin
      clk_count <= '0;
    end else begin
      clk_count <= clk_count + 1;
    end
  end

  // eXtension Interface
  if_xif #() ext_if ();

`ifdef FPGA_ZCU104
  xilinx_clk_wizard_wrapper xilinx_clk_wizard_wrapper_i (
      .CLK_IN1_D_0_clk_n(clk_300mhz_n),
      .CLK_IN1_D_0_clk_p(clk_300mhz_p),
      .clk_out1_0(clk_gen)
  );
`elsif FPGA_NEXYS
  xilinx_clk_wizard_wrapper xilinx_clk_wizard_wrapper_i (
      .clk_100MHz(clk_i),
      .clk_out1_0(clk_gen)
  );
`else  // FPGA PYNQ-Z2
  xilinx_clk_wizard_wrapper xilinx_clk_wizard_wrapper_i (
      .clk_125MHz(clk_i),
      .clk_out1_0(clk_gen)
  );
`endif








  x_heep_system #(
      .X_EXT(X_EXT),
      .COREV_PULP(COREV_PULP),
      .FPU(FPU),
      .ZFINX(ZFINX)
  ) x_heep_system_i 
  (
      .intr_vector_ext_i('0),
      .xif_compressed_if(ext_if),
      .xif_issue_if(ext_if),
      .xif_commit_if(ext_if),
      .xif_mem_if(ext_if),
      .xif_mem_result_if(ext_if),
      .xif_result_if(ext_if),
      .ext_xbar_master_req_i(r_obi_req),            // obi req 
      .ext_xbar_master_resp_o(r_obi_resp),          // obi resp for core2axi

      .ext_xbar_slave_req_o(obi_req),               // INSERTED FROM FEMU EXAMPLE OBI SLAVE INTERFACE
      .ext_xbar_slave_resp_i(obi_resp),             // INSERTED ..



      .ext_core_instr_req_o(),
      .ext_core_instr_resp_i('0),
      .ext_core_data_req_o(),
      .ext_core_data_resp_i('0),
      .ext_debug_master_req_o(),
      .ext_debug_master_resp_i('0),
      .ext_dma_read_ch0_req_o(),
      .ext_dma_read_ch0_resp_i('0),
      .ext_dma_write_ch0_req_o(),
      .ext_dma_write_ch0_resp_i('0),
      .ext_dma_addr_ch0_req_o(),
      .ext_dma_addr_ch0_resp_i('0),
      .ext_peripheral_slave_req_o(),
      .ext_peripheral_slave_resp_i('0),
      .external_subsystem_powergate_switch_no(),
      .external_subsystem_powergate_switch_ack_ni(),
      .external_subsystem_powergate_iso_no(),
      .external_subsystem_rst_no(),
      .external_ram_banks_set_retentive_no(),
      .external_subsystem_clkgate_en_no(),
      .exit_value_o(exit_value),
      .clk_i(clk_gen),
      .rst_ni(rst_n),
      .boot_select_i(boot_select_i),
      .execute_from_flash_i(execute_from_flash_i),
      .jtag_tck_i(jtag_tck_i),
      .jtag_tms_i(jtag_tms_i),
      .jtag_trst_ni(jtag_trst_ni),
      .jtag_tdi_i(jtag_tdi_i),
      .jtag_tdo_o(jtag_tdo_o),
      .uart_rx_i(uart_rx_i),
      .uart_tx_o(uart_tx_o),
      .exit_valid_o(exit_valid_o),
      .gpio_0_io(gpio_io[0]),
      .gpio_1_io(gpio_io[1]),
      .gpio_2_io(gpio_io[2]),
      .gpio_3_io(gpio_io[3]),
      .gpio_13_io(gpio_io[13]),
      .gpio_14_io(gpio_io[14]),
      .gpio_15_io(gpio_io[15]),
      .gpio_16_io(gpio_io[16]),
      .gpio_17_io(gpio_io[17]),
      .gpio_4_io(gpio_io[4]),
      .gpio_5_io(gpio_io[5]),
      .gpio_6_io(gpio_io[6]),
      .gpio_7_io(gpio_io[7]),
      .gpio_8_io(gpio_io[8]),
      .gpio_9_io(gpio_io[9]),
      .gpio_10_io(gpio_io[10]),
      .gpio_11_io(gpio_io[11]),
      .gpio_12_io(gpio_io[12]),
      .gpio_13_io(gpio_io[13]),
      .gpio_14_io(gpio_io[14]),
      .gpio_15_io(gpio_io[15]),
      .gpio_16_io(gpio_io[16]),
      .gpio_17_io(gpio_io[17]),ext_core_instr_req_o_io[2]),
      .spi_flash_sd_3_io(spi_flash_sd_io[3]),
      .spi_flash_cs_0_io(spi_flash_csb_o),
      .spi_flash_cs_1_io(),
      .spi_flash_sck_io(spi_flash_sck_o),
      .spi_sd_0_io(spi_sd_io[0]),
      .spi_sd_1_io(spi_sd_io[1]),
      .spi_sd_2_io(spi_sd_io[2]),
      .spi_sd_3_io(spi_sd_io[3]),
      .spi_cs_0_io(spi_csb_o),
      .spi_cs_1_io(),
      .spi_sck_io(spi_sck_o),
      .i2c_scl_io,
      .i2c_sda_io,
      .spi2_sd_0_io(spi2_sd_io[0]),
      .spi2_sd_1_io(spi2_sd_io[1]),
      .spi2_sd_2_io(spi2_sd_io[2]),
      .spi2_sd_3_io(spi2_sd_io[3]),
      .spi2_cs_0_io(spi2_csb_o[0]),
      .spi2_cs_1_io(spi2_csb_o[1]),
      .spi2_sck_io(spi2_sck_o),
      .pdm2pcm_clk_io,
      .pdm2pcm_pdm_io,
      .i2s_sck_io(i2s_sck_io),
      .i2s_ws_io(i2s_ws_io),
      .i2s_sd_io(i2s_sd_io),
      .ext_dma_slot_tx_i('0),
      .ext_dma_slot_rx_i('0)  
  );

  // Core2axi
  core2axi #(
    .AXI4_WDATA_WIDTH(AXI_DATA_WIDTH),
    .AXI4_RDATA_WIDTH(AXI_DATA_WIDTH)
  ) obi2axi_bridge_virtual_obi_i 
  (
    .clk_i(clk_gen), //done
    .rst_ni(rst_n), // done

    .data_req_i(obi_req.req),
    .data_gnt_o(obi_resp.gnt),
    .data_rvalid_o(obi_resp.rvalid),
    .data_addr_i(obi_req.addr),
    .data_we_i(obi_req.we),
    .data_be_i(obi_req.be),
    .data_rdata_o(obi_resp.rdata),
    .data_wdata_i(obi_req.wdata),

    .aw_id_o(AXI_M_OBI_awid_sig),
    .aw_addr_o(AXI_M_OBI_awaddr_in_sig),
    .aw_len_o(AXI_M_OBI_awlen_sig),
    .aw_size_o(AXI_M_OBI_awsize_sig),
    .aw_burst_o(AXI_M_OBI_awburst_sig),data_req_i
    .aw_lock_o(AXI_M_OBI_awlock_sig),
    .aw_cache_o(AXI_M_OBI_awcache_sig),
    .aw_prot_o(AXI_M_OBI_awprot_sig),
    .aw_region_o(AXI_M_OBI_awregion_sig),
    .aw_user_o(),
    .aw_qos_o(AXI_M_OBI_awqos_sig),
    .aw_valid_o(AXI_M_OBI_awvalid_sig),
    .aw_ready_i(AXI_M_OBI_awready_sig),

    .w_data_o(AXI_M_OBI_wdata_sig),
    .w_strb_o(AXI_M_OBI_wstrb_sig),
    .w_last_o(AXI_M_OBI_wlast_sig),
    .w_user_o(),
    .w_valid_o(AXI_M_OBI_wvalid_sig),
    .w_ready_i(AXI_M_OBI_wready_sig),

    .b_id_i(AXI_M_OBI_bid_sig),
    .b_resp_i(AXI_M_OBI_bresp_sig),
    .b_valid_i(AXI_M_OBI_bvalid_sig),
    .b_user_i('0),
    .b_ready_o(AXI_M_OBI_bready_sig),

    .ar_id_o(AXI_M_OBI_arid_sig),
    .ar_addr_o(AXI_M_OBI_araddr_in_sig),
    .ar_len_o(AXI_M_OBI_arlen_sig),
    .ar_size_o(AXI_M_OBI_arsize_sig),
    .ar_burst_o(AXI_M_OBI_arburst_sig),
    .ar_lock_o(AXI_M_OBI_arlock_sig),
    .ar_cache_o(AXI_M_OBI_arcache_sig),data_req_i
    .ar_prot_o(AXI_M_OBI_arprot_sig),
    .ar_region_o(AXI_M_OBI_arregion_sig),
    .ar_user_o(),
    .ar_qos_o(AXI_M_OBI_arqos_sig),
    .ar_valid_o(AXI_M_OBI_arvalid_sig),
    .ar_ready_i(AXI_M_OBI_arready_sig),

    .r_id_i(AXI_M_OBI_rid_sig),
    .r_data_i(AXI_M_OBI_rdata_sig),
    .r_resp_i(AXI_M_OBI_rresp_sig),
    .r_last_i(AXI_M_OBI_rlast_sig),
    .r_user_i('0),
    .r_valid_i(AXI_M_OBI_rvalid_sig),
    .r_ready_o(AXI_M_OBI_rready_sig)
  );




  

  // seial link input signals


  assign exit_value_o = exit_value[0];


endmodule

// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

`ifdef USE_UPF
import UPF::*;
`endif

module testharness #(
    parameter COREV_PULP                  = 0,
    parameter FPU                         = 0,
    parameter ZFINX                       = 0,
    parameter X_EXT                       = 0,         // eXtension interface in cv32e40x
    parameter JTAG_DPI                    = 0,
    parameter USE_EXTERNAL_DEVICE_EXAMPLE = 1,
    parameter CLK_FREQUENCY               = 'd100_000  //KHz
) (
    inout wire  clk_i,
    input logic fast_clock,
    inout wire  rst_ni,

    inout wire boot_select_i,
    inout wire execute_from_flash_i,

`ifdef SIM_SYSTEMC
    output logic        ext_systemc_req_req_o,
    output logic        ext_systemc_req_we_o,
    output logic [ 3:0] ext_systemc_req_be_o,
    output logic [31:0] ext_systemc_req_addr_o,
    output logic [31:0] ext_systemc_req_wdata_o,

    input  logic        ext_systemc_resp_gnt_i,
    input  logic        ext_systemc_resp_rvalid_i,
    input  logic [31:0] ext_systemc_resp_rdata_i,
`endif
    input  wire         jtag_tck_i,
    input  wire         jtag_tms_i,
    input  wire         jtag_trst_ni,
    input  wire         jtag_tdi_i,
    output wire         jtag_tdo_o,
    output logic [31:0] exit_value_o,
    inout  wire         exit_valid_o


);


  `include "tb_util.svh"

  import obi_pkg::*;
  import reg_pkg::*;
  import testharness_pkg::*;
  import addr_map_rule_pkg::*;

  localparam SWITCH_ACK_LATENCY = 15;
  localparam EXT_XBAR_NMASTER_RND = USE_EXTERNAL_DEVICE_EXAMPLE ? testharness_pkg::EXT_XBAR_NMASTER : 1;
  localparam HEEP_EXT_XBAR_NMASTER = USE_EXTERNAL_DEVICE_EXAMPLE ? testharness_pkg::EXT_XBAR_NMASTER : 0;

  localparam int unsigned LOG_EXT_XBAR_NSLAVE = EXT_XBAR_NSLAVE > 32'd1 ? $clog2(
      EXT_XBAR_NSLAVE
  ) : 32'd1;

  localparam EXT_DOMAINS_RND = core_v_mini_mcu_pkg::EXTERNAL_DOMAINS == 0 ? 1 : core_v_mini_mcu_pkg::EXTERNAL_DOMAINS;
  localparam NEXT_INT_RND = core_v_mini_mcu_pkg::NEXT_INT == 0 ? 1 : core_v_mini_mcu_pkg::NEXT_INT;

  wire uart_rx;
  wire uart_tx;
  logic sim_jtag_enable = (JTAG_DPI == 1);
  wire sim_jtag_tck;
  wire sim_jtag_tms;
  wire sim_jtag_tdi;
  wire sim_jtag_tdo;
  wire sim_jtag_trstn;
  wire mux_jtag_tck;
  wire mux_jtag_tms;
  wire mux_jtag_tdi;
  wire mux_jtag_tdo;
  wire mux_jtag_trstn;
  wire [31:0] gpio;

  wire [3:0] spi_flash_sd_io;
  wire [1:0] spi_flash_csb;
  wire spi_flash_sck;

  wire [3:0] spi_sd_io;
  wire [1:0] spi_csb;
  wire spi_sck;

  logic [EXT_PERIPHERALS_PORT_SEL_WIDTH-1:0] ext_periph_select;

  logic iffifo_in_ready, iffifo_out_valid;
  logic iffifo_int_o;

  // External xbar master/slave and peripheral ports
  obi_req_t [EXT_XBAR_NMASTER_RND-1:0] ext_master_req;
  obi_req_t [EXT_XBAR_NMASTER_RND-1:0] heep_slave_req;
  obi_resp_t [EXT_XBAR_NMASTER_RND-1:0] ext_master_resp;
  obi_resp_t [EXT_XBAR_NMASTER_RND-1:0] heep_slave_resp;
  obi_req_t heep_core_instr_req;
  obi_resp_t heep_core_instr_resp;
  obi_req_t heep_core_data_req;
  obi_resp_t heep_core_data_resp;
  obi_req_t heep_debug_master_req;
  obi_resp_t heep_debug_master_resp;
  obi_req_t heep_dma_read_ch0_req;
  obi_resp_t heep_dma_read_ch0_resp;
  obi_req_t heep_dma_write_ch0_req;
  obi_resp_t heep_dma_write_ch0_resp;
  obi_req_t heep_dma_addr_ch0_req;
  obi_resp_t heep_dma_addr_ch0_resp;
  obi_req_t [EXT_XBAR_NSLAVE-1:0] ext_slave_req;
  obi_resp_t [EXT_XBAR_NSLAVE-1:0] ext_slave_resp;
  reg_req_t periph_slave_req;
  reg_rsp_t periph_slave_rsp;

  reg_pkg::reg_req_t [testharness_pkg::EXT_NPERIPHERALS-1:0] ext_periph_slv_req;
  reg_pkg::reg_rsp_t [testharness_pkg::EXT_NPERIPHERALS-1:0] ext_periph_slv_rsp;

  // External interrupts
  logic [NEXT_INT_RND-1:0] intr_vector_ext;
  logic memcopy_intr;

  // External subsystems
  logic [EXT_DOMAINS_RND-1:0] external_subsystem_powergate_switch_n;
  logic [EXT_DOMAINS_RND-1:0] external_subsystem_powergate_switch_ack_n;
  logic [EXT_DOMAINS_RND-1:0] external_subsystem_powergate_iso_n;
  logic [EXT_DOMAINS_RND-1:0] external_subsystem_rst_n;
  logic [EXT_DOMAINS_RND-1:0] external_ram_banks_set_retentive_n;
  logic [EXT_DOMAINS_RND-1:0] external_subsystem_clkgate_en_n;

  //serial link check ddr
  import serial_link_pkg::*;
  logic [NumChannels-1:0][NumLanes-1:0] ddr_i_xheep;  // check NumLanes parameter 
  logic [NumChannels-1:0][NumLanes-1:0] ddr_o_xheep;
  logic [NumChannels-1:0]               clk_sl_int2ext;
  logic [NumChannels-1:0]               clk_sl_ext2int;




  // eXtension Interface
  if_xif #(
      .X_NUM_RS(fpu_ss_pkg::X_NUM_RS),
      .X_ID_WIDTH(fpu_ss_pkg::X_ID_WIDTH),
      .X_MEM_WIDTH(fpu_ss_pkg::X_MEM_WIDTH),
      .X_RFR_WIDTH(fpu_ss_pkg::X_RFR_WIDTH),
      .X_RFW_WIDTH(fpu_ss_pkg::X_RFW_WIDTH),
      .X_MISA(fpu_ss_pkg::X_MISA)
  ) ext_if ();

  always_comb begin
    // All interrupt lines set to zero by default
    for (int i = 0; i < core_v_mini_mcu_pkg::NEXT_INT; i++) begin
      intr_vector_ext[i] = 1'b0;
    end
    // Re-assign the interrupt lines used here
    intr_vector_ext[0] = memcopy_intr;
    intr_vector_ext[1] = iffifo_int_o;
  end

  //log parameters
  initial begin
    $display("%t: the parameter COREV_PULP is %x", $time, COREV_PULP);
    $display("%t: the parameter FPU is %x", $time, FPU);
    $display("%t: the parameter ZFINX is %x", $time, ZFINX);
    $display("%t: the parameter X_EXT is %x", $time, X_EXT);
    $display("%t: the parameter ZFINX is %x", $time, ZFINX);
    $display("%t: the parameter JTAG_DPI is %x", $time, JTAG_DPI);
    $display("%t: the parameter USE_EXTERNAL_DEVICE_EXAMPLE is %x", $time,
             USE_EXTERNAL_DEVICE_EXAMPLE);
    $display("%t: the parameter CLK_FREQUENCY is %d KHz", $time, CLK_FREQUENCY);
  end

`ifdef USE_UPF
  initial begin
    $display("%t: All Power Supply ON", $time);
    supply_on("VDD", 1.2);
    supply_on("VSS", 0);
  end
`endif

  x_heep_system #(
      .COREV_PULP(COREV_PULP),
      .FPU(FPU),
      .ZFINX(ZFINX),
      .X_EXT(X_EXT),
      .EXT_XBAR_NMASTER(HEEP_EXT_XBAR_NMASTER)
  ) x_heep_system_i (
      .clk_i,
      .rst_ni,
      .jtag_tck_i(mux_jtag_tck),
      .jtag_tms_i(mux_jtag_tms),
      .jtag_trst_ni(mux_jtag_trstn),
      .jtag_tdi_i(mux_jtag_tdi),
      .jtag_tdo_o(mux_jtag_tdo),
      .boot_select_i,
      .execute_from_flash_i,
      .exit_valid_o,
      .uart_rx_i(uart_rx),
      .uart_tx_o(uart_tx),
      .gpio_0_io(gpio[0]),
      .gpio_1_io(gpio[1]),
      .gpio_2_io(gpio[2]),
      .gpio_3_io(gpio[3]),
      .gpio_4_io(gpio[4]),
      .gpio_5_io(gpio[5]),
      .gpio_6_io(gpio[6]),
      .gpio_7_io(gpio[7]),
      .gpio_8_io(gpio[8]),
      .gpio_9_io(gpio[9]),
      .gpio_10_io(gpio[10]),
      .gpio_11_io(gpio[11]),
      .gpio_12_io(gpio[12]),
      .gpio_13_io(gpio[13]),
      .gpio_14_io(gpio[14]),
      .gpio_15_io(gpio[15]),
      .gpio_16_io(gpio[16]),
      .spi_flash_sck_io(spi_flash_sck),
      .spi_flash_cs_0_io(spi_flash_csb[0]),
      .spi_flash_cs_1_io(spi_flash_csb[1]),
      .spi_flash_sd_0_io(spi_flash_sd_io[0]),
      .spi_flash_sd_1_io(spi_flash_sd_io[1]),
      .spi_flash_sd_2_io(spi_flash_sd_io[2]),
      .spi_flash_sd_3_io(spi_flash_sd_io[3]),
      .spi_sck_io(spi_sck),
      .spi_cs_0_io(spi_csb[0]),
      .spi_cs_1_io(spi_csb[1]),
      .spi_sd_0_io(spi_sd_io[0]),
      .spi_sd_1_io(spi_sd_io[1]),
      .spi_sd_2_io(spi_sd_io[2]),
      .spi_sd_3_io(spi_sd_io[3]),
      .pdm2pcm_pdm_io(gpio[18]),
      .pdm2pcm_clk_io(gpio[19]),
      .i2s_sck_io(gpio[20]),
      .i2s_ws_io(gpio[21]),
      .i2s_sd_io(gpio[22]),
      .spi2_cs_0_io(gpio[23]),
      .spi2_cs_1_io(gpio[24]),
      .spi2_sck_io(gpio[25]),
      .spi2_sd_0_io(gpio[26]),
      .spi2_sd_1_io(gpio[27]),
      .spi2_sd_2_io(gpio[28]),
      .spi2_sd_3_io(gpio[29]),
      .i2c_scl_io(gpio[31]),
      .i2c_sda_io(gpio[30]),
      .exit_value_o,
      .intr_vector_ext_i(intr_vector_ext),
      .xif_compressed_if(ext_if),
      .xif_issue_if(ext_if),
      .xif_commit_if(ext_if),
      .xif_mem_if(ext_if),
      .xif_mem_result_if(ext_if),
      .xif_result_if(ext_if),
      .ext_xbar_master_req_i(heep_slave_req),
      .ext_xbar_master_resp_o(heep_slave_resp),
      .ext_core_instr_req_o(heep_core_instr_req),
      .ext_core_instr_resp_i(heep_core_instr_resp),
      .ext_core_data_req_o(heep_core_data_req),
      .ext_core_data_resp_i(heep_core_data_resp),
      .ext_debug_master_req_o(heep_debug_master_req),
      .ext_debug_master_resp_i(heep_debug_master_resp),
      .ext_dma_read_ch0_req_o(heep_dma_read_ch0_req),
      .ext_dma_read_ch0_resp_i(heep_dma_read_ch0_resp),
      .ext_dma_write_ch0_req_o(heep_dma_write_ch0_req),
      .ext_dma_write_ch0_resp_i(heep_dma_write_ch0_resp),
      .ext_dma_addr_ch0_req_o(heep_dma_addr_ch0_req),
      .ext_dma_addr_ch0_resp_i(heep_dma_addr_ch0_resp),
      .ext_peripheral_slave_req_o(periph_slave_req),
      .ext_peripheral_slave_resp_i(periph_slave_rsp),
      .external_subsystem_powergate_switch_no(external_subsystem_powergate_switch_n),
      .external_subsystem_powergate_switch_ack_ni(external_subsystem_powergate_switch_ack_n),
      .external_subsystem_powergate_iso_no(external_subsystem_powergate_iso_n),
      .external_subsystem_rst_no(external_subsystem_rst_n),
      .external_ram_banks_set_retentive_no(external_ram_banks_set_retentive_n),
      .external_subsystem_clkgate_en_no(external_subsystem_clkgate_en_n),
      .ext_dma_slot_tx_i(iffifo_in_ready),
      .ext_dma_slot_rx_i(iffifo_out_valid),
      .ddr_i(ddr_i_xheep),
      .ddr_o(ddr_o_xheep),
      .ddr_rcv_clk_i(clk_sl_ext2int),
      .ddr_rcv_clk_o(clk_sl_int2ext),
      .fast_clock,
      .result()
  );

  // Testbench external bus
  // ----------------------
  // The external bus connects the external peripherals among them and to
  // the corresponding X-HEEP slave port (to the internal system bus).
  ext_bus #(
      .EXT_XBAR_NMASTER(EXT_XBAR_NMASTER),
      .EXT_XBAR_NSLAVE (EXT_XBAR_NSLAVE)
  ) ext_bus_i (
      .clk_i                    (clk_i),
      .rst_ni                   (rst_ni),
      .addr_map_i               (EXT_XBAR_ADDR_RULES),
      .default_idx_i            (SLOW_MEMORY_IDX[LOG_EXT_XBAR_NSLAVE-1:0]),
      .heep_core_instr_req_i    (heep_core_instr_req),
      .heep_core_instr_resp_o   (heep_core_instr_resp),
      .heep_core_data_req_i     (heep_core_data_req),
      .heep_core_data_resp_o    (heep_core_data_resp),
      .heep_debug_master_req_i  (heep_debug_master_req),
      .heep_debug_master_resp_o (heep_debug_master_resp),
      .heep_dma_read_ch0_req_i  (heep_dma_read_ch0_req),
      .heep_dma_read_ch0_resp_o (heep_dma_read_ch0_resp),
      .heep_dma_write_ch0_req_i (heep_dma_write_ch0_req),
      .heep_dma_write_ch0_resp_o(heep_dma_write_ch0_resp),
      .heep_dma_addr_ch0_req_i  (heep_dma_addr_ch0_req),
      .heep_dma_addr_ch0_resp_o (heep_dma_addr_ch0_resp),
      .ext_master_req_i         (ext_master_req),
      .ext_master_resp_o        (ext_master_resp),
      .heep_slave_req_o         (heep_slave_req),
      .heep_slave_resp_i        (heep_slave_resp),
      .ext_slave_req_o          (ext_slave_req),
      .ext_slave_resp_i         (ext_slave_resp)
  );

  logic pdm;

  //pretending to be SWITCH CELLs that delay by SWITCH_ACK_LATENCY cycles the ACK signal
  logic
      tb_cpu_subsystem_powergate_switch_ack_n[SWITCH_ACK_LATENCY+1],
      tb_peripheral_subsystem_powergate_switch_ack_n[SWITCH_ACK_LATENCY+1];
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] tb_memory_subsystem_banks_powergate_switch_ack_n[SWITCH_ACK_LATENCY+1];
  logic [EXT_DOMAINS_RND-1:0] tb_external_subsystem_powergate_switch_ack_n[SWITCH_ACK_LATENCY+1];

  logic delayed_tb_cpu_subsystem_powergate_switch_ack_n;
  logic delayed_tb_peripheral_subsystem_powergate_switch_ack_n;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] delayed_tb_memory_subsystem_banks_powergate_switch_ack_n;
  logic [EXT_DOMAINS_RND-1:0] delayed_tb_external_subsystem_powergate_switch_ack_n;

  always_ff @(negedge clk_i) begin
    tb_cpu_subsystem_powergate_switch_ack_n[0] <= x_heep_system_i.cpu_subsystem_powergate_switch_n;
    tb_peripheral_subsystem_powergate_switch_ack_n[0] <= x_heep_system_i.peripheral_subsystem_powergate_switch_n;
    tb_memory_subsystem_banks_powergate_switch_ack_n[0] <= x_heep_system_i.memory_subsystem_banks_powergate_switch_n;
    tb_external_subsystem_powergate_switch_ack_n[0] <= external_subsystem_powergate_switch_n;
    for (int i = 0; i < SWITCH_ACK_LATENCY; i++) begin
      tb_memory_subsystem_banks_powergate_switch_ack_n[i+1] <= tb_memory_subsystem_banks_powergate_switch_ack_n[i];
      tb_cpu_subsystem_powergate_switch_ack_n[i+1] <= tb_cpu_subsystem_powergate_switch_ack_n[i];
      tb_peripheral_subsystem_powergate_switch_ack_n[i+1] <= tb_peripheral_subsystem_powergate_switch_ack_n[i];
      tb_external_subsystem_powergate_switch_ack_n[i+1] <= tb_external_subsystem_powergate_switch_ack_n[i];
    end
  end

  assign delayed_tb_cpu_subsystem_powergate_switch_ack_n = tb_cpu_subsystem_powergate_switch_ack_n[SWITCH_ACK_LATENCY];
  assign delayed_tb_peripheral_subsystem_powergate_switch_ack_n = tb_peripheral_subsystem_powergate_switch_ack_n[SWITCH_ACK_LATENCY];
  assign delayed_tb_memory_subsystem_banks_powergate_switch_ack_n = tb_memory_subsystem_banks_powergate_switch_ack_n[SWITCH_ACK_LATENCY];
  assign delayed_tb_external_subsystem_powergate_switch_ack_n = tb_external_subsystem_powergate_switch_ack_n[SWITCH_ACK_LATENCY];

  always_comb begin
`ifndef VERILATOR
    force x_heep_system_i.core_v_mini_mcu_i.cpu_subsystem_powergate_switch_ack_ni = delayed_tb_cpu_subsystem_powergate_switch_ack_n;
    force x_heep_system_i.core_v_mini_mcu_i.peripheral_subsystem_powergate_switch_ack_ni = delayed_tb_peripheral_subsystem_powergate_switch_ack_n;
    force x_heep_system_i.core_v_mini_mcu_i.memory_subsystem_banks_powergate_switch_ack_ni = delayed_tb_memory_subsystem_banks_powergate_switch_ack_n;
    force external_subsystem_powergate_switch_ack_n = delayed_tb_external_subsystem_powergate_switch_ack_n;
`else
    x_heep_system_i.cpu_subsystem_powergate_switch_ack_n = delayed_tb_cpu_subsystem_powergate_switch_ack_n;
    x_heep_system_i.peripheral_subsystem_powergate_switch_ack_n = delayed_tb_peripheral_subsystem_powergate_switch_ack_n;
    x_heep_system_i.memory_subsystem_banks_powergate_switch_ack_n = delayed_tb_memory_subsystem_banks_powergate_switch_ack_n;
    external_subsystem_powergate_switch_ack_n = delayed_tb_external_subsystem_powergate_switch_ack_n;
`endif
  end


  uartdpi #(
      .BAUD('d256000),
      .FREQ(CLK_FREQUENCY * 1000),  //Hz
      .NAME("uart0")
  ) i_uart0 (
      .clk_i,
      .rst_ni,
      .tx_o(uart_rx),
      .rx_i(uart_tx)
  );

  // jtag calls from dpi
  SimJTAG #(
      .TICK_DELAY(1),
      .PORT      (4567)
  ) i_sim_jtag (
      .clock(clk_i),
      .reset(~rst_ni),
      .enable(sim_jtag_enable),
      .init_done(rst_ni),
      .jtag_TCK(sim_jtag_tck),
      .jtag_TMS(sim_jtag_tms),
      .jtag_TDI(sim_jtag_tdi),
      .jtag_TRSTn(sim_jtag_trstn),
      .jtag_TDO_data(sim_jtag_tdo),
      .jtag_TDO_driven(1'b1),
      .exit()
  );

  assign mux_jtag_tck   = JTAG_DPI ? sim_jtag_tck : jtag_tck_i;
  assign mux_jtag_tms   = JTAG_DPI ? sim_jtag_tms : jtag_tms_i;
  assign mux_jtag_tdi   = JTAG_DPI ? sim_jtag_tdi : jtag_tdi_i;
  assign mux_jtag_trstn = JTAG_DPI ? sim_jtag_trstn : jtag_trst_ni;

  assign sim_jtag_tdo   = JTAG_DPI ? mux_jtag_tdo : '0;
  assign jtag_tdo_o     = !JTAG_DPI ? mux_jtag_tdo : '0;

  // External xbar slave example port
  obi_req_t  slow_ram_slave_req;
  obi_resp_t slow_ram_slave_resp;

`ifndef SIM_SYSTEMC

  assign slow_ram_slave_req              = ext_slave_req[SLOW_MEMORY_IDX];
  assign ext_slave_resp[SLOW_MEMORY_IDX] = slow_ram_slave_resp;
`else

  obi_req_t  ext_systemc_req;
  obi_resp_t ext_systemc_resp;

  assign ext_systemc_req_req_o           = ext_systemc_req.req;
  assign ext_systemc_req_we_o            = ext_systemc_req.we;
  assign ext_systemc_req_be_o            = ext_systemc_req.be;
  assign ext_systemc_req_addr_o          = ext_systemc_req.addr;
  assign ext_systemc_req_wdata_o         = ext_systemc_req.wdata;

  assign ext_systemc_resp.gnt            = ext_systemc_resp_gnt_i;
  assign ext_systemc_resp.rvalid         = ext_systemc_resp_rvalid_i;
  assign ext_systemc_resp.rdata          = ext_systemc_resp_rdata_i;

  assign ext_systemc_req                 = ext_slave_req[SLOW_MEMORY_IDX];
  assign ext_slave_resp[SLOW_MEMORY_IDX] = ext_systemc_resp;
`endif

  generate
    if (USE_EXTERNAL_DEVICE_EXAMPLE) begin : gen_USE_EXTERNAL_DEVICE_EXAMPLE

`ifndef SIM_SYSTEMC
      obi_pkg::obi_req_t  slave_fifoout_req;
      obi_pkg::obi_resp_t slave_fifoout_resp;

      //this FIFO makes the slow memory even more slower in terms of latency
      obi_fifo obi_fifo_i (
          .clk_i,
          .rst_ni,
          .producer_req_i (slow_ram_slave_req),
          .producer_resp_o(slow_ram_slave_resp),
          .consumer_req_o (slave_fifoout_req),
          .consumer_resp_i(slave_fifoout_resp)
      );



      // External xbar slave memory example
      slow_memory #(
          .NumWords (8192),
          .DataWidth(32'd32)
      ) slow_ram_i (
          .clk_i,
          .rst_ni,
          .req_i(slave_fifoout_req.req),
          .we_i(slave_fifoout_req.we),
          .addr_i(slave_fifoout_req.addr[15:2]),
          .wdata_i(slave_fifoout_req.wdata),
          .be_i(slave_fifoout_req.be),
          // output ports
          .gnt_o(slave_fifoout_resp.gnt),
          .rdata_o(slave_fifoout_resp.rdata),
          .rvalid_o(slave_fifoout_resp.rvalid)
      );
`endif

      parameter DMA_TRIGGER_SLOT_NUM = 4;

      // External peripheral example with master port to access memory
      dma #(
          .reg_req_t (reg_pkg::reg_req_t),
          .reg_rsp_t (reg_pkg::reg_rsp_t),
          .obi_req_t (obi_pkg::obi_req_t),
          .obi_resp_t(obi_pkg::obi_resp_t),
          .SLOT_NUM  (DMA_TRIGGER_SLOT_NUM)
      ) dma_i (
          .clk_i,
          .rst_ni,
          .reg_req_i(ext_periph_slv_req[testharness_pkg::MEMCOPY_CTRL_IDX]),
          .reg_rsp_o(ext_periph_slv_rsp[testharness_pkg::MEMCOPY_CTRL_IDX]),
          .dma_read_ch0_req_o(ext_master_req[testharness_pkg::EXT_MASTER0_IDX]),
          .dma_read_ch0_resp_i(ext_master_resp[testharness_pkg::EXT_MASTER0_IDX]),
          .dma_write_ch0_req_o(ext_master_req[testharness_pkg::EXT_MASTER1_IDX]),
          .dma_write_ch0_resp_i(ext_master_resp[testharness_pkg::EXT_MASTER1_IDX]),
          .dma_addr_ch0_req_o(),
          .dma_addr_ch0_resp_i('0),
          .trigger_slot_i('0),
          .dma_done_intr_o(memcopy_intr),
          .dma_window_intr_o()
      );

      simple_accelerator #(
          .reg_req_t (reg_pkg::reg_req_t),
          .reg_rsp_t (reg_pkg::reg_rsp_t),
          .obi_req_t (obi_pkg::obi_req_t),
          .obi_resp_t(obi_pkg::obi_resp_t)
      ) simple_accelerator_i (
          .clk_i,
          .rst_ni,
          .reg_req_i(ext_periph_slv_req[testharness_pkg::SIMPLE_ACC_IDX]),
          .reg_rsp_o(ext_periph_slv_rsp[testharness_pkg::SIMPLE_ACC_IDX]),
          .acc_read_ch0_req_o(ext_master_req[testharness_pkg::EXT_MASTER2_IDX]),
          .acc_read_ch0_resp_i(ext_master_resp[testharness_pkg::EXT_MASTER2_IDX]),
          .acc_write_ch0_req_o(ext_master_req[testharness_pkg::EXT_MASTER3_IDX]),
          .acc_write_ch0_resp_i(ext_master_resp[testharness_pkg::EXT_MASTER3_IDX])
      );

      // AMS external peripheral
      ams #(
          .reg_req_t(reg_pkg::reg_req_t),
          .reg_rsp_t(reg_pkg::reg_rsp_t)
      ) ams_i (
          .clk_i,
          .rst_ni,
          .reg_req_i(ext_periph_slv_req[testharness_pkg::AMS_IDX]),
          .reg_rsp_o(ext_periph_slv_rsp[testharness_pkg::AMS_IDX])
      );

      // InterFaced FIFO (IFFIFO) external peripheral
      iffifo #(
          .reg_req_t(reg_pkg::reg_req_t),
          .reg_rsp_t(reg_pkg::reg_rsp_t)
      ) iffifo_i (
          .clk_i,
          .rst_ni,
          .reg_req_i(ext_periph_slv_req[testharness_pkg::IFFIFO_IDX]),
          .reg_rsp_o(ext_periph_slv_rsp[testharness_pkg::IFFIFO_IDX]),
          // DMA slots
          .iffifo_in_ready_o(iffifo_in_ready),
          .iffifo_out_valid_o(iffifo_out_valid),
          // Interrupt lines
          .iffifo_int_o(iffifo_int_o)
      );

      addr_decode #(
          .NoIndices(testharness_pkg::EXT_NPERIPHERALS),
          .NoRules(testharness_pkg::EXT_NPERIPHERALS),
          .addr_t(logic [31:0]),
          .rule_t(addr_map_rule_pkg::addr_map_rule_t)
      ) i_addr_decode_soc_regbus_ext_periphs (
          .addr_i(periph_slave_req.addr),
          .addr_map_i(testharness_pkg::EXT_PERIPHERALS_ADDR_RULES),
          .idx_o(ext_periph_select),
          .dec_valid_o(),
          .dec_error_o(),
          .en_default_idx_i(1'b0),
          .default_idx_i('0)
      );

      reg_demux #(
          .NoPorts(testharness_pkg::EXT_NPERIPHERALS),
          .req_t  (reg_pkg::reg_req_t),
          .rsp_t  (reg_pkg::reg_rsp_t)
      ) reg_demux_i (
          .clk_i,
          .rst_ni,
          .in_select_i(ext_periph_select),
          .in_req_i(periph_slave_req),
          .in_rsp_o(periph_slave_rsp),
          .out_req_o(ext_periph_slv_req),
          .out_rsp_i(ext_periph_slv_rsp)
      );

      // GPIO counter example
      gpio_cnt #(
          .CntMax(32'd2048)
      ) gpio_cnt_i (
          .clk_i,
          .rst_ni,
          .gpio_i(gpio[30]),
          .gpio_o(gpio[31])
      );

      pdm2pcm_dummy pdm2pcm_dummy_i (
          .clk_i,
          .rst_ni,
          .pdm_data_o(gpio[18]),
          .pdm_clk_i (gpio[19])
      );

      // I2s "microphone"/rx example
      i2s_microphone i2s_microphone_i (
          .rst_ni(rst_ni),
          .i2s_sck_i(gpio[20]),
          .i2s_ws_i(gpio[21]),
          .i2s_sd_o(gpio[22])
      );
      //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

      //    .reg_req_i(ext_periph_slv_req[testharness_pkg::MEMCOPY_CTRL_IDX]),
      //  .reg_rsp_o(ext_periph_slv_rsp[testharness_pkg::MEMCOPY_CTRL_IDX]),
      //    .dma_read_ch0_req_o(ext_master_req[testharness_pkg::EXT_MASTER0_IDX]),
      //  .dma_read_ch0_resp_i(ext_master_resp[testharness_pkg::EXT_MASTER0_IDX]),
      //  .dma_write_ch0_req_o(ext_master_req[testharness_pkg::EXT_MASTER1_IDX]),
      //  .dma_write_ch0_resp_i(ext_master_resp[testharness_pkg::EXT_MASTER1_IDX]),




      obi_req_t sl_obi2axi_req;
      assign sl_obi2axi_req = ext_master_req[testharness_pkg::EXT_MASTER4_IDX];//ext_master_req[testharness_pkg::EXT_MASTER4_IDX];
      obi_resp_t sl_obi2axi_resp;
      assign ext_master_resp[testharness_pkg::EXT_MASTER4_IDX] = sl_obi2axi_resp ;//ext_master_resp[testharness_pkg::EXT_MASTER4_IDX];
      obi_req_t sl_axi2obi_req;
      assign sl_axi2obi_req =ext_slave_req[testharness_pkg::SL_EXT_IDX];// ext_master_req[testharness_pkg::SL_EXT_IDX];
      obi_resp_t sl_axi2obi_resp;
      assign ext_slave_resp[testharness_pkg::SL_EXT_IDX]=sl_axi2obi_resp ;// ext_master_resp[testharness_pkg::SL_EXT_IDX];
      core_v_mini_mcu_pkg::axi_req_t axi_in_req_i, axi_out_req_o;
      core_v_mini_mcu_pkg::axi_resp_t axi_in_rsp_o, axi_out_rsp_i;

      reg_req_t cfg_req_ext;
      assign cfg_req_ext = ext_periph_slv_req[testharness_pkg::SL_REG_IDX];
      reg_rsp_t cfg_rsp_ext;
      assign ext_periph_slv_rsp[testharness_pkg::SL_REG_IDX] = cfg_rsp_ext;

      // assign cfg_req_sl = ao_peripheral_slv_req[core_v_mini_mcu_pkg::SERIAL_LINK_IDX];
      // assign ao_peripheral_slv_rsp[core_v_mini_mcu_pkg::SERIAL_LINK_IDX] = cfg_rsp_sl;

      //.reg_req_i(ext_periph_slv_req[testharness_pkg::MEMCOPY_CTRL_IDX]),
      //.reg_rsp_o(ext_periph_slv_rsp[testharness_pkg::MEMCOPY_CTRL_IDX]),
      //.dma_read_ch0_req_o(ext_master_req[testharness_pkg::EXT_MASTER0_IDX]),
      //.dma_read_ch0_resp_i(ext_master_resp[testharness_pkg::EXT_MASTER0_IDX]),

      // test serial link 
      // CORE(OBI)2AXI 

      core2axi #(
      //.AXI4_WDATA_WIDTH(AXI_DATA_WIDTH),
      //.AXI4_RDATA_WIDTH(AXI_DATA_WIDTH)
      ) obi2axi_bridge_virtual_obi_i (
          .clk_i,
          .rst_ni,

          .data_req_i(sl_obi2axi_req.req),
          //.data_req_i('1),
          .data_gnt_o(sl_obi2axi_resp.gnt),
          .data_rvalid_o(sl_obi2axi_resp.rvalid),
          .data_addr_i(sl_obi2axi_req.addr),
          .data_we_i(sl_obi2axi_req.we),
          .data_be_i(sl_obi2axi_req.be),
          .data_rdata_o(sl_obi2axi_resp.rdata),
          .data_wdata_i(sl_obi2axi_req.wdata),

          .aw_id_o(axi_out_req_o.aw.id),
          .aw_addr_o(axi_out_req_o.aw.addr),
          .aw_len_o(axi_out_req_o.aw.len),
          .aw_size_o(axi_out_req_o.aw.size),
          .aw_burst_o(axi_out_req_o.aw.burst),
          .aw_lock_o(axi_out_req_o.aw.lock),
          .aw_cache_o(axi_out_req_o.aw.cache),
          .aw_prot_o(axi_out_req_o.aw.prot),
          .aw_region_o(axi_out_req_o.aw.region),
          .aw_user_o(axi_out_req_o.aw.user),
          .aw_qos_o(axi_out_req_o.aw.qos),
          .aw_valid_o(axi_out_req_o.aw_valid),
          .aw_ready_i(axi_out_rsp_i.aw_ready),
          //.aw_ready_i('1),
          //.aw_size,

          .w_data_o (axi_out_req_o.w.data),
          .w_strb_o (axi_out_req_o.w.strb),
          .w_last_o (axi_out_req_o.w.last),
          .w_user_o (axi_out_req_o.w.user),
          .w_valid_o(axi_out_req_o.w_valid),
          .w_ready_i(axi_out_rsp_i.w_ready),
          //.w_ready_i('1),
          //.w_size,

          .b_id_i(axi_out_rsp_i.b.id),
          .b_resp_i(axi_out_rsp_i.b.resp),
          .b_valid_i(axi_out_rsp_i.b_valid),
          .b_user_i(axi_out_rsp_i.b.user),
          .b_ready_o(axi_out_req_o.b_ready),
          //.b_size,

          .ar_id_o(axi_out_req_o.ar.id),
          .ar_addr_o(axi_out_req_o.ar.addr),
          .ar_len_o(axi_out_req_o.ar.len),
          .ar_size_o(axi_out_req_o.ar.size),
          .ar_burst_o(axi_out_req_o.ar.burst),
          .ar_lock_o(axi_out_req_o.ar.lock),
          .ar_cache_o(axi_out_req_o.ar.cache),
          .ar_prot_o(axi_out_req_o.ar.prot),
          .ar_region_o(axi_out_req_o.ar.region),
          .ar_user_o(axi_out_req_o.ar.user),
          .ar_qos_o(axi_out_req_o.ar.qos),
          .ar_valid_o(axi_out_req_o.ar_valid),
          .ar_ready_i(axi_out_rsp_i.ar_ready),
          //.ar_size,

          .r_id_i(axi_out_rsp_i.r.id),
          .r_data_i(axi_out_rsp_i.r.data),
          .r_resp_i(axi_out_rsp_i.r.resp),
          .r_last_i(axi_out_rsp_i.r.last),
          .r_user_i(axi_out_rsp_i.r.user),  //.r_user_i('0),
          .r_valid_i(axi_out_rsp_i.r_valid),
          .r_ready_o(axi_out_req_o.r_ready)
          //.r_size
      );



axi_to_mem #(
  /// AXI4+ATOP request type. See `include/axi/typedef.svh`.
  axi_req_t(),
  /// AXI4+ATOP response type. See `include/axi/typedef.svh`.
  axi_resp_t(),
  /// Address width, has to be less or equal than the width off the AXI address field.
  /// Determines the width of `mem_addr_o`. Has to be wide enough to emit the memory region
  /// which should be accessible.
  .AddrWidth(32),
  /// AXI4+ATOP data width.
  .DataWidth,
  /// AXI4+ATOP ID width.
  parameter int unsigned IdWidth    = 0,
  /// Number of banks at output, must evenly divide `DataWidth`.
  parameter int unsigned NumBanks   = 0,
  /// Depth of memory response buffer. This should be equal to the memory response latency.
  parameter int unsigned BufDepth   = 1,
  /// Hide write requests if the strb == '0
  parameter bit          HideStrb   = 1'b0,
  /// Depth of output fifo/fall_through_register. Increase for asymmetric backpressure (contention) on banks.
  parameter int unsigned OutFifoDepth = 1,
  /// Dependent parameter, do not override. Memory address type.
  localparam type addr_t     = logic [AddrWidth-1:0],
  /// Dependent parameter, do not override. Memory data type.
  localparam type mem_data_t = logic [DataWidth/NumBanks-1:0],
  /// Dependent parameter, do not override. Memory write strobe type.
  localparam type mem_strb_t = logic [DataWidth/NumBanks/8-1:0]
) axi_to_mem_i(
  /// Clock input.
  .clk_i,
  /// Asynchronous reset, active low.
  .rst_ni,
  /// The unit is busy handling an AXI4+ATOP request.
  .busy_o(),
  /// AXI4+ATOP slave port, request input.
  .axi_req_i,
  /// AXI4+ATOP slave port, response output.
  .axi_resp_o,
  /// Memory stream master, request is valid for this bank.
  output logic           [NumBanks-1:0]  mem_req_o,
  /// Memory stream master, request can be granted by this bank.
  input  logic           [NumBanks-1:0]  mem_gnt_i,
  /// Memory stream master, byte address of the request.
  output addr_t          [NumBanks-1:0]  mem_addr_o,
  /// Memory stream master, write data for this bank. Valid when `mem_req_o`.
  output mem_data_t      [NumBanks-1:0]  mem_wdata_o,
  /// Memory stream master, byte-wise strobe (byte enable).
  output mem_strb_t      [NumBanks-1:0]  mem_strb_o,
  /// Memory stream master, `axi_pkg::atop_t` signal associated with this request.
  output axi_pkg::atop_t [NumBanks-1:0]  mem_atop_o,
  /// Memory stream master, write enable. Then asserted store of `mem_w_data` is requested.
  output logic           [NumBanks-1:0]  mem_we_o,
  /// Memory stream master, response is valid. This module expects always a response valid for a
  /// request regardless if the request was a write or a read.
  input  logic           [NumBanks-1:0]  mem_rvalid_i,
  /// Memory stream master, read response data.
  input  mem_data_t      [NumBanks-1:0]  mem_rdata_i
);
      



      // SERIAL LINK
      serial_link_occamy_wrapper #(
          .axi_req_t(core_v_mini_mcu_pkg::axi_req_t),
          .axi_rsp_t(core_v_mini_mcu_pkg::axi_resp_t),

          .aw_chan_t(core_v_mini_mcu_pkg::axi_aw_t),
          .ar_chan_t(core_v_mini_mcu_pkg::axi_ar_t),
          .r_chan_t(core_v_mini_mcu_pkg::axi_r_t),
          .w_chan_t(core_v_mini_mcu_pkg::axi_w_t),
          .b_chan_t(core_v_mini_mcu_pkg::axi_b_t),
          .cfg_rsp_t(reg_rsp_t),
          .cfg_req_t(reg_req_t),
          .NumChannels(1),
          .NumLanes(4)
      ) serial_link_occamy_wrapper_i (
          .clk_i     (clk_i),
          .fast_clock,
          .rst_ni    (rst_ni),
          .clk_reg_i (clk_i),   //intended for clock gating purposes
          .rst_reg_ni(rst_ni),  //intended for SW reset purposes

          .testmode_i  ('0),
          .axi_in_req_i(axi_out_req_o),
          .axi_in_rsp_o(axi_out_rsp_i),


          .axi_out_req_o(axi_in_req_i),
          .axi_out_rsp_i(axi_in_rsp_o),

          .cfg_req_i(cfg_req_ext),  //register configuration
          .cfg_rsp_o(cfg_rsp_ext),


          .ddr_i(ddr_o_xheep),  //Double-Data-Rate (DDR) input data
          .ddr_rcv_clk_i(clk_sl_int2ext),
          .ddr_rcv_clk_o(clk_sl_ext2int),
          .ddr_o(ddr_i_xheep)  //Double-Data-Rate (DDR) output data
      );






      //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



      // External xbar slave example port
`ifndef VERILATOR
      // Flash used for booting (execute from flash or copy from flash)
      spiflash flash_boot_i (
          .csb(spi_flash_csb[0]),
          .clk(spi_flash_sck),
          .io0(spi_flash_sd_io[0]),  // MOSI
          .io1(spi_flash_sd_io[1]),  // MISO
          .io2(spi_flash_sd_io[2]),
          .io3(spi_flash_sd_io[3])
      );
`endif

`ifndef VERILATOR
      // Flash used as an example device with an SPI interface
      spiflash flash_device_i (
          .csb(spi_csb[0]),
          .clk(spi_sck),
          .io0(spi_sd_io[0]),  // MOSI
          .io1(spi_sd_io[1]),  // MISO
          .io2(spi_sd_io[2]),
          .io3(spi_sd_io[3])
      );
`endif

      if ((core_v_mini_mcu_pkg::CpuType == cv32e40x || core_v_mini_mcu_pkg::CpuType == cv32e40px) && X_EXT != 0) begin: gen_fpu_ss_wrapper
        fpu_ss_wrapper #(
            .PULP_ZFINX(ZFINX),
            .INPUT_BUFFER_DEPTH(1),
            .OUT_OF_ORDER(0),
            .FORWARDING(1),
            .FPU_FEATURES(fpu_ss_pkg::FPU_FEATURES),
            .FPU_IMPLEMENTATION(fpu_ss_pkg::FPU_IMPLEMENTATION)
        ) fpu_ss_wrapper_i (
            // Clock and reset
            .clk_i,
            .rst_ni,

            // eXtension Interface
            .xif_compressed_if(ext_if),
            .xif_issue_if(ext_if),
            .xif_commit_if(ext_if),
            .xif_mem_if(ext_if),
            .xif_mem_result_if(ext_if),
            .xif_result_if(ext_if)
        );
      end

    end else begin : gen_DONT_USE_EXTERNAL_DEVICE_EXAMPLE
      assign slow_ram_slave_resp.gnt = '0;
      assign slow_ram_slave_resp.rdata = '0;
      assign slow_ram_slave_resp.rvalid = '0;

      assign ext_periph_slv_req = '0;
      assign ext_periph_slv_rsp = '0;

      assign ext_master_req[testharness_pkg::EXT_MASTER0_IDX].req = '0;
      assign ext_master_req[testharness_pkg::EXT_MASTER0_IDX].we = '0;
      assign ext_master_req[testharness_pkg::EXT_MASTER0_IDX].be = '0;
      assign ext_master_req[testharness_pkg::EXT_MASTER0_IDX].addr = '0;
      assign ext_master_req[testharness_pkg::EXT_MASTER0_IDX].wdata = '0;

      assign memcopy_intr = '0;
      assign iffifo_int_o = '0;
      assign periph_slave_rsp = '0;

    end
  endgenerate

endmodule  // testharness

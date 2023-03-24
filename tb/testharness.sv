// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

`ifdef USE_UPF
import UPF::*;
`endif

module testharness #(
    parameter PULP_XPULP    = 0,
    parameter FPU           = 0,
    parameter PULP_ZFINX    = 0,
    parameter X_EXT         = 0,         // eXtension interface in cv32e40x
    parameter JTAG_DPI      = 0,
    parameter CLK_FREQUENCY = 'd100_000  //KHz
) (
    inout wire clk_i,
    inout wire rst_ni,

    inout wire boot_select_i,
    inout wire execute_from_flash_i,

    inout  wire         jtag_tck_i,
    inout  wire         jtag_tms_i,
    inout  wire         jtag_trst_ni,
    inout  wire         jtag_tdi_i,
    inout  wire         jtag_tdo_o,
    output logic [31:0] exit_value_o,
    inout  wire         exit_valid_o
);

  `include "tb_util.svh"

  import obi_pkg::*;
  import reg_pkg::*;
  import testharness_pkg::*;

  localparam SWITCH_ACK_LATENCY = 15;

  wire uart_rx;
  wire uart_tx;
  logic sim_jtag_enable = (JTAG_DPI == 1);
  wire sim_jtag_tck;
  wire sim_jtag_tms;
  wire sim_jtag_trst;
  wire sim_jtag_tdi;
  wire sim_jtag_tdo;
  wire sim_jtag_trstn;
  wire [31:0] gpio;

  wire [3:0] spi_flash_sd_io;
  wire [1:0] spi_flash_csb;
  wire spi_flash_sck;

  wire [3:0] spi_sd_io;
  wire [1:0] spi_csb;
  wire spi_sck;

  // External xbar master/slave and peripheral ports
  obi_req_t [testharness_pkg::EXT_XBAR_NMASTER-1:0] master_req;
  obi_resp_t [testharness_pkg::EXT_XBAR_NMASTER-1:0] master_resp;
  obi_req_t slave_req;
  obi_resp_t slave_resp;
  reg_req_t periph_slave_req;
  reg_rsp_t periph_slave_resp;

  // External peripheral example port
  reg_req_t memcopy_periph_req;
  reg_rsp_t memcopy_periph_rsp;

  // External xbar slave example port
  obi_req_t slow_ram_slave_req;
  obi_resp_t slow_ram_slave_resp;

  // External interrupts
  logic [core_v_mini_mcu_pkg::NEXT_INT-1:0] intr_vector_ext;
  logic memcopy_intr;

  // External subsystems
  logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_subsystem_powergate_switch;
  logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_subsystem_powergate_switch_ack;
  logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_subsystem_powergate_iso;
  logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_subsystem_rst_n;
  logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_ram_banks_set_retentive;

  always_comb begin
    // All interrupt lines set to zero by default
    for (int i = 0; i < core_v_mini_mcu_pkg::NEXT_INT; i++) begin
      intr_vector_ext[i] = 1'b0;
    end
    // Re-assign the interrupt lines used here
    intr_vector_ext[0] = memcopy_intr;
  end

`ifdef USE_UPF
  initial begin
    $display("%t: All Power Supply ON", $time);
    supply_on("VDD", 1.2);
    supply_on("VSS", 0);
  end
`endif

  // eXtension Interface
  if_xif #() ext_if ();

  x_heep_system #(
      .PULP_XPULP(PULP_XPULP),
      .FPU(FPU),
      .PULP_ZFINX(PULP_ZFINX),
      .EXT_XBAR_NMASTER(testharness_pkg::EXT_XBAR_NMASTER),
      .X_EXT(X_EXT)
  ) x_heep_system_i (
      .clk_i,
      .rst_ni,
      .jtag_tck_i(sim_jtag_tck),
      .jtag_tms_i(sim_jtag_tms),
      .jtag_trst_ni(sim_jtag_trstn),
      .jtag_tdi_i(sim_jtag_tdi),
      .jtag_tdo_o(sim_jtag_tdo),
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
      .gpio_17_io(gpio[17]),
      .gpio_18_io(gpio[18]),
      .gpio_19_io(gpio[19]),
      .gpio_20_io(gpio[20]),
      .gpio_21_io(gpio[21]),
      .gpio_22_io(gpio[22]),
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
      .ext_xbar_master_req_i(master_req),
      .ext_xbar_master_resp_o(master_resp),
      .ext_xbar_slave_req_o(slave_req),
      .ext_xbar_slave_resp_i(slave_resp),
      .ext_peripheral_slave_req_o(periph_slave_req),
      .ext_peripheral_slave_resp_i(periph_slave_resp),
      .external_subsystem_powergate_switch_o(external_subsystem_powergate_switch),
      .external_subsystem_powergate_switch_ack_i(external_subsystem_powergate_switch_ack),
      .external_subsystem_powergate_iso_o(external_subsystem_powergate_iso),
      .external_subsystem_rst_no(external_subsystem_rst_n),
      .external_ram_banks_set_retentive_o(external_ram_banks_set_retentive)
  );

  //pretending to be SWITCH CELLs that delay by SWITCH_ACK_LATENCY cycles the ACK signal
  logic
      tb_cpu_subsystem_powergate_switch_ack[SWITCH_ACK_LATENCY+1],
      tb_peripheral_subsystem_powergate_switch_ack[SWITCH_ACK_LATENCY+1];
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] tb_memory_subsystem_banks_powergate_switch_ack[SWITCH_ACK_LATENCY+1];
  logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] tb_external_subsystem_powergate_switch_ack[SWITCH_ACK_LATENCY+1];
  logic delayed_tb_cpu_subsystem_powergate_switch_ack;
  logic delayed_tb_peripheral_subsystem_powergate_switch_ack;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] delayed_tb_memory_subsystem_banks_powergate_switch_ack;
  logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] delayed_tb_external_subsystem_powergate_switch_ack;

  always_ff @(negedge clk_i) begin
    tb_cpu_subsystem_powergate_switch_ack[0] <= x_heep_system_i.cpu_subsystem_powergate_switch;
    tb_peripheral_subsystem_powergate_switch_ack[0] <= x_heep_system_i.peripheral_subsystem_powergate_switch;
    tb_memory_subsystem_banks_powergate_switch_ack[0] <= x_heep_system_i.memory_subsystem_banks_powergate_switch;
    tb_external_subsystem_powergate_switch_ack[0] <= external_subsystem_powergate_switch;
    for (int i = 0; i < SWITCH_ACK_LATENCY; i++) begin
      tb_memory_subsystem_banks_powergate_switch_ack[i+1] <= tb_memory_subsystem_banks_powergate_switch_ack[i];
      tb_cpu_subsystem_powergate_switch_ack[i+1] <= tb_cpu_subsystem_powergate_switch_ack[i];
      tb_peripheral_subsystem_powergate_switch_ack[i+1] <= tb_peripheral_subsystem_powergate_switch_ack[i];
      tb_external_subsystem_powergate_switch_ack[i+1] <= tb_external_subsystem_powergate_switch_ack[i];
    end
  end

  assign delayed_tb_cpu_subsystem_powergate_switch_ack = tb_cpu_subsystem_powergate_switch_ack[SWITCH_ACK_LATENCY];
  assign delayed_tb_peripheral_subsystem_powergate_switch_ack = tb_peripheral_subsystem_powergate_switch_ack[SWITCH_ACK_LATENCY];
  assign delayed_tb_memory_subsystem_banks_powergate_switch_ack = tb_memory_subsystem_banks_powergate_switch_ack[SWITCH_ACK_LATENCY];
  assign delayed_tb_external_subsystem_powergate_switch_ack = tb_external_subsystem_powergate_switch_ack[SWITCH_ACK_LATENCY];

  always_comb begin
`ifndef VERILATOR
    force x_heep_system_i.core_v_mini_mcu_i.cpu_subsystem_powergate_switch_ack_i = delayed_tb_cpu_subsystem_powergate_switch_ack;
    force x_heep_system_i.core_v_mini_mcu_i.peripheral_subsystem_powergate_switch_ack_i = delayed_tb_peripheral_subsystem_powergate_switch_ack;
    force x_heep_system_i.core_v_mini_mcu_i.memory_subsystem_banks_powergate_switch_ack_i = delayed_tb_memory_subsystem_banks_powergate_switch_ack;
    force external_subsystem_powergate_switch_ack = delayed_tb_external_subsystem_powergate_switch_ack;
`else
    x_heep_system_i.cpu_subsystem_powergate_switch_ack = delayed_tb_cpu_subsystem_powergate_switch_ack;
    x_heep_system_i.peripheral_subsystem_powergate_switch_ack = delayed_tb_peripheral_subsystem_powergate_switch_ack;
    x_heep_system_i.memory_subsystem_banks_powergate_switch_ack = delayed_tb_memory_subsystem_banks_powergate_switch_ack;
    external_subsystem_powergate_switch_ack = delayed_tb_external_subsystem_powergate_switch_ack;
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

  assign slow_ram_slave_req = slave_req;
  assign slave_resp = slow_ram_slave_resp;

  assign memcopy_periph_req = periph_slave_req;
  assign periph_slave_resp = memcopy_periph_rsp;

`ifdef USE_EXTERNAL_DEVICE_EXAMPLE
  // External xbar slave memory example
  slow_memory #(
      .NumWords (128),
      .DataWidth(32'd32)
  ) slow_ram_i (
      .clk_i(clk_i),
      .rst_ni(rst_ni),
      .req_i(slow_ram_slave_req.req),
      .we_i(slow_ram_slave_req.we),
      .addr_i(slow_ram_slave_req.addr[8:2]),
      .wdata_i(slow_ram_slave_req.wdata),
      .be_i(slow_ram_slave_req.be),
      // output ports
      .gnt_o(slow_ram_slave_resp.gnt),
      .rdata_o(slow_ram_slave_resp.rdata),
      .rvalid_o(slow_ram_slave_resp.rvalid)
  );

  // External peripheral example with master port to access memory
  memcopy_periph #(
      .reg_req_t (reg_pkg::reg_req_t),
      .reg_rsp_t (reg_pkg::reg_rsp_t),
      .obi_req_t (obi_pkg::obi_req_t),
      .obi_resp_t(obi_pkg::obi_resp_t)
  ) memcopy_periph_i (
      .clk_i,
      .rst_ni,
      .reg_req_i(memcopy_periph_req),
      .reg_rsp_o(memcopy_periph_rsp),
      .master_req_o(master_req[testharness_pkg::EXT_MASTER0_IDX]),
      .master_resp_i(master_resp[testharness_pkg::EXT_MASTER0_IDX]),
      .memcopy_intr_o(memcopy_intr)
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

`else
  assign slow_ram_slave_resp.gnt = '0;
  assign slow_ram_slave_resp.rdata = '0;
  assign slow_ram_slave_resp.rvalid = '0;

  assign memcopy_periph_rsp.error = '0;
  assign memcopy_periph_rsp.ready = '0;
  assign memcopy_periph_rsp.rdata = '0;

  assign master_req[testharness_pkg::EXT_MASTER0_IDX].req = '0;
  assign master_req[testharness_pkg::EXT_MASTER0_IDX].we = '0;
  assign master_req[testharness_pkg::EXT_MASTER0_IDX].be = '0;
  assign master_req[testharness_pkg::EXT_MASTER0_IDX].addr = '0;
  assign master_req[testharness_pkg::EXT_MASTER0_IDX].wdata = '0;

  assign memcopy_intr = '0;
`endif

endmodule  // testharness

// Copyright(// Copyright) 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module peripheral_subsystem
  import obi_pkg::*;
  import reg_pkg::*;
#(
    parameter NEXT_INT = 0
) (
    input logic clk_i,
    input logic rst_ni,

    input  obi_req_t  slave_req_i,
    output obi_resp_t slave_resp_o,

    //PLIC
    input  logic [NEXT_INT-1:0] intr_vector_ext_i,
    output logic                irq_plic_o,
    output logic                msip_o,

    //UART
    input  logic uart_rx_i,
    output logic uart_tx_o,
    output logic uart_tx_en_o,

    //GPIO
    input  logic [31:0] cio_gpio_i,
    output logic [31:0] cio_gpio_o,
    output logic [31:0] cio_gpio_en_o,
    output logic [ 7:0] cio_gpio_intr_o,

    // I2C Interface
    input  logic cio_scl_i,
    output logic cio_scl_o,
    output logic cio_scl_en_o,
    input  logic cio_sda_i,
    output logic cio_sda_o,
    output logic cio_sda_en_o,

    //RV TIMER
    output logic rv_timer_2_intr_o,
    output logic rv_timer_3_intr_o,

    //External peripheral(s)
    output reg_req_t ext_peripheral_slave_req_o,
    input  reg_rsp_t ext_peripheral_slave_resp_i
);

  import core_v_mini_mcu_pkg::*;
  import tlul_pkg::*;
  import rv_plic_reg_pkg::*;

  reg_pkg::reg_req_t peripheral_req;
  reg_pkg::reg_rsp_t peripheral_rsp;

  reg_pkg::reg_req_t [core_v_mini_mcu_pkg::PERIPHERALS-1:0] peripheral_slv_req;
  reg_pkg::reg_rsp_t [core_v_mini_mcu_pkg::PERIPHERALS-1:0] peripheral_slv_rsp;

  tlul_pkg::tl_h2d_t uart_tl_h2d;
  tlul_pkg::tl_d2h_t uart_tl_d2h;

  tlul_pkg::tl_h2d_t plic_tl_h2d;
  tlul_pkg::tl_d2h_t plic_tl_d2h;

  tlul_pkg::tl_h2d_t gpio_tl_h2d;
  tlul_pkg::tl_d2h_t gpio_tl_d2h;

  tlul_pkg::tl_h2d_t i2c_tl_h2d;
  tlul_pkg::tl_d2h_t i2c_tl_d2h;

  tlul_pkg::tl_h2d_t rv_timer_tl_h2d;
  tlul_pkg::tl_d2h_t rv_timer_tl_d2h;

  logic [rv_plic_reg_pkg::NumTarget-1:0] irq_plic;
  logic [rv_plic_reg_pkg::NumSrc-1:0] intr_vector;
  logic [$clog2(rv_plic_reg_pkg::NumSrc)-1:0] irq_id[rv_plic_reg_pkg::NumTarget];
  logic [$clog2(rv_plic_reg_pkg::NumSrc)-1:0] unused_irq_id[rv_plic_reg_pkg::NumTarget];

  logic uart_intr_tx_watermark;
  logic uart_intr_rx_watermark;
  logic uart_intr_tx_empty;
  logic uart_intr_rx_overflow;
  logic uart_intr_rx_frame_err;
  logic uart_intr_rx_break_err;
  logic uart_intr_rx_timeout;
  logic uart_intr_rx_parity_err;

  logic [31:0] gpio_intr;

  logic intr_fmt_watermark;
  logic intr_rx_watermark;
  logic intr_fmt_overflow;
  logic intr_rx_overflow;
  logic intr_nak;
  logic intr_scl_interference;
  logic intr_sda_interference;
  logic intr_stretch_timeout;
  logic intr_sda_unstable;
  logic intr_trans_complete;
  logic intr_tx_empty;
  logic intr_tx_nonempty;
  logic intr_tx_overflow;
  logic intr_acq_overflow;
  logic intr_ack_stop;
  logic intr_host_timeout;

  // this avoids lint errors
  assign unused_irq_id = irq_id;

  // Assign internal interrupts
  assign intr_vector[0] = 1'b0;  // ID [0] is a special case and must be tied to zero.
  assign intr_vector[1] = uart_intr_tx_watermark;
  assign intr_vector[2] = uart_intr_rx_watermark;
  assign intr_vector[3] = uart_intr_tx_empty;
  assign intr_vector[4] = uart_intr_rx_overflow;
  assign intr_vector[5] = uart_intr_rx_frame_err;
  assign intr_vector[6] = uart_intr_rx_break_err;
  assign intr_vector[7] = uart_intr_rx_timeout;
  assign intr_vector[8] = uart_intr_rx_parity_err;
  assign intr_vector[32:9] = gpio_intr[31:8];
  assign intr_vector[33] = intr_fmt_watermark;
  assign intr_vector[34] = intr_rx_watermark;
  assign intr_vector[35] = intr_fmt_overflow;
  assign intr_vector[36] = intr_rx_overflow;
  assign intr_vector[37] = intr_nak;
  assign intr_vector[38] = intr_scl_interference;
  assign intr_vector[39] = intr_sda_interference;
  assign intr_vector[40] = intr_stretch_timeout;
  assign intr_vector[41] = intr_sda_unstable;
  assign intr_vector[42] = intr_trans_complete;
  assign intr_vector[43] = intr_tx_empty;
  assign intr_vector[44] = intr_tx_nonempty;
  assign intr_vector[45] = intr_tx_overflow;
  assign intr_vector[46] = intr_acq_overflow;
  assign intr_vector[47] = intr_ack_stop;
  assign intr_vector[48] = intr_host_timeout;

  // External interrupts assignement
  for (genvar i = 0; i < NEXT_INT; i++) begin
    assign intr_vector[i+PLIC_USED_NINT] = intr_vector_ext_i[i];
  end

  //Address Decoder
  logic [PERIPHERALS_PORT_SEL_WIDTH-1:0] peripheral_select;

  assign ext_peripheral_slave_req_o = peripheral_slv_req[core_v_mini_mcu_pkg::EXT_PERIPH_IDX];
  assign peripheral_slv_rsp[core_v_mini_mcu_pkg::EXT_PERIPH_IDX] = ext_peripheral_slave_resp_i;

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
      .NoIndices(core_v_mini_mcu_pkg::PERIPHERALS),
      .NoRules(core_v_mini_mcu_pkg::PERIPHERALS),
      .addr_t(logic [31:0]),
      .rule_t(addr_map_rule_pkg::addr_map_rule_t)
  ) i_addr_decode_soc_regbus_periph_xbar (
      .addr_i(peripheral_req.addr),
      .addr_map_i(core_v_mini_mcu_pkg::PERIPHERALS_ADDR_RULES),
      .idx_o(peripheral_select),
      .dec_valid_o(),
      .dec_error_o(),
      .en_default_idx_i(1'b0),
      .default_idx_i('0)
  );

  reg_demux #(
      .NoPorts(core_v_mini_mcu_pkg::PERIPHERALS),
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

  reg_to_tlul reg_to_tlul_plic_i (
      .tl_o(plic_tl_h2d),
      .tl_i(plic_tl_d2h),
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::PLIC_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::PLIC_IDX])
  );

  rv_plic rv_plic_i (
      .clk_i,
      .rst_ni,
      .tl_i(plic_tl_h2d),
      .tl_o(plic_tl_d2h),
      .intr_src_i(intr_vector),
      .irq_o(irq_plic_o),
      .irq_id_o(irq_id),
      .msip_o(msip_o)
  );

  reg_to_tlul reg_to_tlul_uart_i (
      .tl_o(uart_tl_h2d),
      .tl_i(uart_tl_d2h),
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::UART_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::UART_IDX])
  );

  uart uart_i (
      .clk_i,
      .rst_ni,
      .tl_i(uart_tl_h2d),
      .tl_o(uart_tl_d2h),
      .cio_rx_i(uart_rx_i),
      .cio_tx_o(uart_tx_o),
      .cio_tx_en_o(uart_tx_en_o),
      .intr_tx_watermark_o(uart_intr_tx_watermark),
      .intr_rx_watermark_o(uart_intr_rx_watermark),
      .intr_tx_empty_o(uart_intr_tx_empty),
      .intr_rx_overflow_o(uart_intr_rx_overflow),
      .intr_rx_frame_err_o(uart_intr_rx_frame_err),
      .intr_rx_break_err_o(uart_intr_rx_break_err),
      .intr_rx_timeout_o(uart_intr_rx_timeout),
      .intr_rx_parity_err_o(uart_intr_rx_parity_err)
  );

  reg_to_tlul reg_to_tlul_gpio_i (
      .tl_o(gpio_tl_h2d),
      .tl_i(gpio_tl_d2h),
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::GPIO_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::GPIO_IDX])
  );

  gpio gpio_i (
      .clk_i,
      .rst_ni,
      .tl_i(gpio_tl_h2d),
      .tl_o(gpio_tl_d2h),
      .cio_gpio_i(cio_gpio_i),
      .cio_gpio_o(cio_gpio_o),
      .cio_gpio_en_o(cio_gpio_en_o),
      .intr_gpio_o(gpio_intr)
  );

  assign cio_gpio_intr_o = gpio_intr[7:0];

  reg_to_tlul reg_to_tlul_i2c_i (
      .tl_o(i2c_tl_h2d),
      .tl_i(i2c_tl_d2h),
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::I2C_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::I2C_IDX])
  );

  i2c i2c_i (
      .clk_i,
      .rst_ni,
      .tl_i(i2c_tl_h2d),
      .tl_o(i2c_tl_d2h),
      .cio_scl_i,
      .cio_scl_o,
      .cio_scl_en_o,
      .cio_sda_i,
      .cio_sda_o,
      .cio_sda_en_o,
      .intr_fmt_watermark_o(intr_fmt_watermark),
      .intr_rx_watermark_o(intr_rx_watermark),
      .intr_fmt_overflow_o(intr_fmt_overflow),
      .intr_rx_overflow_o(intr_rx_overflow),
      .intr_nak_o(intr_nak),
      .intr_scl_interference_o(intr_scl_interference),
      .intr_sda_interference_o(intr_sda_interference),
      .intr_stretch_timeout_o(intr_stretch_timeout),
      .intr_sda_unstable_o(intr_sda_unstable),
      .intr_trans_complete_o(intr_trans_complete),
      .intr_tx_empty_o(intr_tx_empty),
      .intr_tx_nonempty_o(intr_tx_nonempty),
      .intr_tx_overflow_o(intr_tx_overflow),
      .intr_acq_overflow_o(intr_acq_overflow),
      .intr_ack_stop_o(intr_ack_stop),
      .intr_host_timeout_o(intr_host_timeout)
  );

  reg_to_tlul rv_timer_reg_to_tlul_i (
      .tl_o(rv_timer_tl_h2d),
      .tl_i(rv_timer_tl_d2h),
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::RV_TIMER_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::RV_TIMER_IDX])
  );

  rv_timer rv_timer_2_3_i (
      .clk_i,
      .rst_ni,
      .tl_i(rv_timer_tl_h2d),
      .tl_o(rv_timer_tl_d2h),
      .intr_timer_expired_0_0_o(rv_timer_2_intr_o),
      .intr_timer_expired_1_0_o(rv_timer_3_intr_o)
  );

endmodule : peripheral_subsystem

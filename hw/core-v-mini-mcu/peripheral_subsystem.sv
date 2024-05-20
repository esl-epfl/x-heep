// Copyright(// Copyright) 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1



module peripheral_subsystem
  import obi_pkg::*;
  import reg_pkg::*;
#(
    //do not touch these parameters
    parameter NEXT_INT_RND = core_v_mini_mcu_pkg::NEXT_INT == 0 ? 1 : core_v_mini_mcu_pkg::NEXT_INT
) (
    input logic clk_i,
    input logic rst_ni,

    // Clock-gating signal
    input logic clk_gate_en_ni,

    input  obi_req_t  slave_req_i,
    output obi_resp_t slave_resp_o,


    if_bundle__core_v_mini_mcu__pd_peripheral.pd_peripheral bundle__core_v_mini_mcu__pd_peripheral__if,
    if_bundle__ao_periph__pd_peripheral.pd_peripheral bundle__ao_periph__pd_peripheral__if,
    if_bundle__pd_peripheral__root.pd_peripheral bundle__pd_peripheral__root__if,
    if_bundle__pad_ring__pd_peripheral.pd_peripheral bundle__pad_ring__pd_peripheral__if
);

  import core_v_mini_mcu_pkg::*;
  import tlul_pkg::*;
  import rv_plic_reg_pkg::*;

  reg_pkg::reg_req_t peripheral_req;
  reg_pkg::reg_rsp_t peripheral_rsp;

  reg_pkg::reg_req_t [core_v_mini_mcu_pkg::PERIPHERAL_COUNT-1:0] peripheral_slv_req;
  reg_pkg::reg_rsp_t [core_v_mini_mcu_pkg::PERIPHERAL_COUNT-1:0] peripheral_slv_rsp;


  tlul_pkg::tl_h2d_t rv_timer_2_3_tl_h2d;
  tlul_pkg::tl_d2h_t rv_timer_2_3_tl_d2h;
  tlul_pkg::tl_h2d_t i2c_0_tl_h2d;
  tlul_pkg::tl_d2h_t i2c_0_tl_d2h;
  tlul_pkg::tl_h2d_t rv_plic_0_tl_h2d;
  tlul_pkg::tl_d2h_t rv_plic_0_tl_d2h;
  tlul_pkg::tl_h2d_t uart_0_tl_h2d;
  tlul_pkg::tl_d2h_t uart_0_tl_d2h;

  logic spi_host_0_error_intr_o;
  logic spi_host_1_error_intr_o;
  logic spi_host_1_spi_event_intr_o;
  logic i2c_0_fmt_watermark_intr_o;
  logic i2c_0_rx_watermark_intr_o;
  logic i2c_0_fmt_overflow_intr_o;
  logic i2c_0_rx_overflow_intr_o;
  logic i2c_0_nak_intr_o;
  logic i2c_0_scl_interference_intr_o;
  logic i2c_0_sda_interference_intr_o;
  logic i2c_0_stretch_timeout_intr_o;
  logic i2c_0_sda_unstable_intr_o;
  logic i2c_0_trans_complete_intr_o;
  logic i2c_0_tx_empty_intr_o;
  logic i2c_0_tx_nonempty_intr_o;
  logic i2c_0_tx_overflow_intr_o;
  logic i2c_0_acq_overflow_intr_o;
  logic i2c_0_ack_stop_intr_o;
  logic i2c_0_host_timeout_intr_o;
  logic i2s_0_i2s_event_intr_o;
  logic gpio_8_intr_o;
  logic gpio_9_intr_o;
  logic gpio_10_intr_o;
  logic gpio_11_intr_o;
  logic gpio_12_intr_o;
  logic gpio_13_intr_o;
  logic gpio_14_intr_o;
  logic gpio_15_intr_o;
  logic gpio_16_intr_o;
  logic gpio_17_intr_o;
  logic gpio_18_intr_o;
  logic gpio_19_intr_o;
  logic gpio_20_intr_o;
  logic gpio_21_intr_o;
  logic gpio_22_intr_o;
  logic gpio_23_intr_o;
  logic gpio_24_intr_o;
  logic gpio_25_intr_o;
  logic gpio_26_intr_o;
  logic gpio_27_intr_o;
  logic gpio_28_intr_o;
  logic gpio_29_intr_o;
  logic gpio_30_intr_o;
  logic gpio_31_intr_o;
  logic uart_0_tx_watermark_intr_o;
  logic uart_0_rx_watermark_intr_o;
  logic uart_0_tx_empty_intr_o;
  logic uart_0_rx_overflow_intr_o;
  logic uart_0_rx_frame_err_intr_o;
  logic uart_0_rx_break_err_intr_o;
  logic uart_0_rx_timeout_intr_o;
  logic uart_0_rx_parity_err_intr_o;


  //Address Decoder
  logic [PERIPHERAL_PORT_SEL_WIDTH-1:0] peripheral_select;

  obi_pkg::obi_req_t slave_fifo_req_sel;
  obi_pkg::obi_resp_t slave_fifo_resp_sel;

  // Clock-gating
  logic clk_cg;
  tc_clk_gating clk_gating_cell (
      .clk_i,
      .en_i(clk_gate_en_ni),
      .test_en_i(1'b0),
      .clk_o(clk_cg)
  );


`ifdef REMOVE_OBI_FIFO

  assign slave_fifo_req_sel = slave_req_i;
  assign slave_resp_o       = slave_fifo_resp_sel;

`else

  obi_pkg::obi_req_t  slave_fifoin_req;
  obi_pkg::obi_resp_t slave_fifoin_resp;

  obi_pkg::obi_req_t  slave_fifoout_req;
  obi_pkg::obi_resp_t slave_fifoout_resp;

  obi_fifo obi_fifo_i (
      .clk_i(clk_cg),
      .rst_ni,
      .producer_req_i(slave_fifoin_req),
      .producer_resp_o(slave_fifoin_resp),
      .consumer_req_o(slave_fifoout_req),
      .consumer_resp_i(slave_fifoout_resp)
  );

  assign slave_fifo_req_sel = slave_fifoout_req;
  assign slave_fifoout_resp = slave_fifo_resp_sel;
  assign slave_fifoin_req   = slave_req_i;
  assign slave_resp_o       = slave_fifoin_resp;

`endif

  periph_to_reg #(
      .req_t(reg_pkg::reg_req_t),
      .rsp_t(reg_pkg::reg_rsp_t),
      .IW(1)
  ) periph_to_reg_i (
      .clk_i(clk_cg),
      .rst_ni,
      .req_i(slave_fifo_req_sel.req),
      .add_i(slave_fifo_req_sel.addr),
      .wen_i(~slave_fifo_req_sel.we),
      .wdata_i(slave_fifo_req_sel.wdata),
      .be_i(slave_fifo_req_sel.be),
      .id_i('0),
      .gnt_o(slave_fifo_resp_sel.gnt),
      .r_rdata_o(slave_fifo_resp_sel.rdata),
      .r_opc_o(),
      .r_id_o(),
      .r_valid_o(slave_fifo_resp_sel.rvalid),
      .reg_req_o(peripheral_req),
      .reg_rsp_i(peripheral_rsp)
  );

  addr_decode #(
      .NoIndices(core_v_mini_mcu_pkg::PERIPHERAL_COUNT),
      .NoRules(core_v_mini_mcu_pkg::PERIPHERAL_COUNT),
      .addr_t(logic [31:0]),
      .rule_t(addr_map_rule_pkg::addr_map_rule_t)
  ) i_addr_decode_soc_regbus_periph_xbar (
      .addr_i(peripheral_req.addr),
      .addr_map_i(core_v_mini_mcu_pkg::PERIPHERAL_ADDR_RULES),
      .idx_o(peripheral_select),
      .dec_valid_o(),
      .dec_error_o(),
      .en_default_idx_i(1'b0),
      .default_idx_i('0)
  );

  reg_demux #(
      .NoPorts(core_v_mini_mcu_pkg::PERIPHERAL_COUNT),
      .req_t  (reg_pkg::reg_req_t),
      .rsp_t  (reg_pkg::reg_rsp_t)
  ) reg_demux_i (
      .clk_i(clk_cg),
      .rst_ni,
      .in_select_i(peripheral_select),
      .in_req_i(peripheral_req),
      .in_rsp_o(peripheral_rsp),
      .out_req_o(peripheral_slv_req),
      .out_rsp_i(peripheral_slv_rsp)
  );


  reg_to_tlul #(
      .req_t(reg_pkg::reg_req_t),
      .rsp_t(reg_pkg::reg_rsp_t),
      .tl_h2d_t(tlul_pkg::tl_h2d_t),
      .tl_d2h_t(tlul_pkg::tl_d2h_t),
      .tl_a_user_t(tlul_pkg::tl_a_user_t),
      .tl_a_op_e(tlul_pkg::tl_a_op_e),
      .TL_A_USER_DEFAULT(tlul_pkg::TL_A_USER_DEFAULT),
      .PutFullData(tlul_pkg::PutFullData),
      .Get(tlul_pkg::Get)
  ) reg_to_tlul_rv_timer_2_3_i (
      .tl_o(rv_timer_2_3_tl_h2d),
      .tl_i(rv_timer_2_3_tl_d2h),
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::RV_TIMER_2_3_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::RV_TIMER_2_3_IDX])
  );

  rv_timer rv_timer_2_3_i (
      .clk_i(clk_cg),
      .rst_ni,
      .intr_timer_expired_0_0_o(bundle__core_v_mini_mcu__pd_peripheral__if.rv_timer_2_3_timer_expired_0_0_intr_o),
      .intr_timer_expired_1_0_o(bundle__core_v_mini_mcu__pd_peripheral__if.rv_timer_2_3_timer_expired_1_0_intr_o),
      .tl_i(rv_timer_2_3_tl_h2d),
      .tl_o(rv_timer_2_3_tl_d2h)
  );

  spi_host #(
      .reg_req_t(reg_pkg::reg_req_t),
      .reg_rsp_t(reg_pkg::reg_rsp_t)
  ) spi_host_0_i (
      .clk_i(clk_cg),
      .rst_ni,
      .intr_error_o(spi_host_0_error_intr_o),
      .intr_spi_event_o(bundle__core_v_mini_mcu__pd_peripheral__if.spi_host_0_spi_event_intr_o),
      .cio_sd_i({
        bundle__pad_ring__pd_peripheral__if.spi_host0_sd3_i,
        bundle__pad_ring__pd_peripheral__if.spi_host0_sd2_i,
        bundle__pad_ring__pd_peripheral__if.spi_host0_sd1_i,
        bundle__pad_ring__pd_peripheral__if.spi_host0_sd0_i
      }),
      .cio_sd_o({
        bundle__pad_ring__pd_peripheral__if.spi_host0_sd3_o,
        bundle__pad_ring__pd_peripheral__if.spi_host0_sd2_o,
        bundle__pad_ring__pd_peripheral__if.spi_host0_sd1_o,
        bundle__pad_ring__pd_peripheral__if.spi_host0_sd0_o
      }),
      .cio_sd_en_o({
        bundle__pad_ring__pd_peripheral__if.spi_host0_sd3_oe,
        bundle__pad_ring__pd_peripheral__if.spi_host0_sd2_oe,
        bundle__pad_ring__pd_peripheral__if.spi_host0_sd1_oe,
        bundle__pad_ring__pd_peripheral__if.spi_host0_sd0_oe
      }),
      .cio_sck_o(bundle__pad_ring__pd_peripheral__if.spi_host0_sck0_o),
      .cio_sck_en_o(bundle__pad_ring__pd_peripheral__if.spi_host0_sck0_oe),
      .cio_csb_o({
        bundle__pad_ring__pd_peripheral__if.spi_host0_csb1_o,
        bundle__pad_ring__pd_peripheral__if.spi_host0_csb0_o
      }),
      .cio_csb_en_o({
        bundle__pad_ring__pd_peripheral__if.spi_host0_csb1_oe,
        bundle__pad_ring__pd_peripheral__if.spi_host0_csb0_oe
      }),
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::SPI_HOST_0_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::SPI_HOST_0_IDX]),
      .alert_rx_i(),
      .alert_tx_o(),
      .passthrough_i(spi_device_pkg::PASSTHROUGH_REQ_DEFAULT),
      .passthrough_o(),
      .rx_valid_o(bundle__ao_periph__pd_peripheral__if.spi_host_0_rx_valid),
      .tx_ready_o(bundle__ao_periph__pd_peripheral__if.spi_host_0_tx_valid)
  );

  spi_host #(
      .reg_req_t(reg_pkg::reg_req_t),
      .reg_rsp_t(reg_pkg::reg_rsp_t)
  ) spi_host_1_i (
      .clk_i(clk_cg),
      .rst_ni,
      .intr_error_o(spi_host_1_error_intr_o),
      .intr_spi_event_o(spi_host_1_spi_event_intr_o),
      .cio_sd_i({
        bundle__pd_peripheral__root__if.spi_host1_sd3_i,
        bundle__pd_peripheral__root__if.spi_host1_sd2_i,
        bundle__pd_peripheral__root__if.spi_host1_sd1_i,
        bundle__pd_peripheral__root__if.spi_host1_sd0_i
      }),
      .cio_sd_o({
        bundle__pd_peripheral__root__if.spi_host1_sd3_o,
        bundle__pd_peripheral__root__if.spi_host1_sd2_o,
        bundle__pd_peripheral__root__if.spi_host1_sd1_o,
        bundle__pd_peripheral__root__if.spi_host1_sd0_o
      }),
      .cio_sd_en_o({
        bundle__pd_peripheral__root__if.spi_host1_sd3_oe,
        bundle__pd_peripheral__root__if.spi_host1_sd2_oe,
        bundle__pd_peripheral__root__if.spi_host1_sd1_oe,
        bundle__pd_peripheral__root__if.spi_host1_sd0_oe
      }),
      .cio_sck_o(bundle__pd_peripheral__root__if.spi_host1_sck0_o),
      .cio_sck_en_o(bundle__pd_peripheral__root__if.spi_host1_sck0_oe),
      .cio_csb_o({
        bundle__pd_peripheral__root__if.spi_host1_csb1_o,
        bundle__pd_peripheral__root__if.spi_host1_csb0_o
      }),
      .cio_csb_en_o({
        bundle__pd_peripheral__root__if.spi_host1_csb1_oe,
        bundle__pd_peripheral__root__if.spi_host1_csb0_oe
      }),
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::SPI_HOST_1_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::SPI_HOST_1_IDX]),
      .alert_rx_i(),
      .alert_tx_o(),
      .passthrough_i(spi_device_pkg::PASSTHROUGH_REQ_DEFAULT),
      .passthrough_o(),
      .rx_valid_o(),
      .tx_ready_o()
  );

  reg_to_tlul #(
      .req_t(reg_pkg::reg_req_t),
      .rsp_t(reg_pkg::reg_rsp_t),
      .tl_h2d_t(tlul_pkg::tl_h2d_t),
      .tl_d2h_t(tlul_pkg::tl_d2h_t),
      .tl_a_user_t(tlul_pkg::tl_a_user_t),
      .tl_a_op_e(tlul_pkg::tl_a_op_e),
      .TL_A_USER_DEFAULT(tlul_pkg::TL_A_USER_DEFAULT),
      .PutFullData(tlul_pkg::PutFullData),
      .Get(tlul_pkg::Get)
  ) reg_to_tlul_i2c_0_i (
      .tl_o(i2c_0_tl_h2d),
      .tl_i(i2c_0_tl_d2h),
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::I2C_0_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::I2C_0_IDX])
  );

  i2c i2c_0_i (
      .clk_i(clk_cg),
      .rst_ni,
      .intr_fmt_watermark_o(i2c_0_fmt_watermark_intr_o),
      .intr_rx_watermark_o(i2c_0_rx_watermark_intr_o),
      .intr_fmt_overflow_o(i2c_0_fmt_overflow_intr_o),
      .intr_rx_overflow_o(i2c_0_rx_overflow_intr_o),
      .intr_nak_o(i2c_0_nak_intr_o),
      .intr_scl_interference_o(i2c_0_scl_interference_intr_o),
      .intr_sda_interference_o(i2c_0_sda_interference_intr_o),
      .intr_stretch_timeout_o(i2c_0_stretch_timeout_intr_o),
      .intr_sda_unstable_o(i2c_0_sda_unstable_intr_o),
      .intr_trans_complete_o(i2c_0_trans_complete_intr_o),
      .intr_tx_empty_o(i2c_0_tx_empty_intr_o),
      .intr_tx_nonempty_o(i2c_0_tx_nonempty_intr_o),
      .intr_tx_overflow_o(i2c_0_tx_overflow_intr_o),
      .intr_acq_overflow_o(i2c_0_acq_overflow_intr_o),
      .intr_ack_stop_o(i2c_0_ack_stop_intr_o),
      .intr_host_timeout_o(i2c_0_host_timeout_intr_o),
      .cio_sda_i(bundle__pd_peripheral__root__if.i2c0_sda0_i),
      .cio_sda_o(bundle__pd_peripheral__root__if.i2c0_sda0_o),
      .cio_sda_en_o(bundle__pd_peripheral__root__if.i2c0_sda0_oe),
      .cio_scl_i(bundle__pd_peripheral__root__if.i2c0_scl0_i),
      .cio_scl_o(bundle__pd_peripheral__root__if.i2c0_scl0_o),
      .cio_scl_en_o(bundle__pd_peripheral__root__if.i2c0_scl0_oe),
      .tl_i(i2c_0_tl_h2d),
      .tl_o(i2c_0_tl_d2h)
  );

  i2s #(
      .reg_req_t(reg_pkg::reg_req_t),
      .reg_rsp_t(reg_pkg::reg_rsp_t)
  ) i2s_0_i (
      .clk_i(clk_cg),
      .rst_ni,
      .intr_i2s_event_o(i2s_0_i2s_event_intr_o),
      .i2s_sck_i(bundle__pd_peripheral__root__if.i2s0_sck0_i),
      .i2s_sck_o(bundle__pd_peripheral__root__if.i2s0_sck0_o),
      .i2s_sck_oe_o(bundle__pd_peripheral__root__if.i2s0_sck0_oe),
      .i2s_ws_i(bundle__pd_peripheral__root__if.i2s0_ws0_i),
      .i2s_ws_o(bundle__pd_peripheral__root__if.i2s0_ws0_o),
      .i2s_ws_oe_o(bundle__pd_peripheral__root__if.i2s0_ws0_oe),
      .i2s_sd_i(bundle__pd_peripheral__root__if.i2s0_sd0_i),
      .i2s_sd_o(bundle__pd_peripheral__root__if.i2s0_sd0_o),
      .i2s_sd_oe_o(bundle__pd_peripheral__root__if.i2s0_sd0_oe),
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::I2S_0_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::I2S_0_IDX]),
      .i2s_rx_valid_o(bundle__ao_periph__pd_peripheral__if.i2s_0_rx_valid)
  );

  logic [rv_plic_reg_pkg::NumTarget-1:0] rv_plic_0_irq_plic;
  logic [rv_plic_reg_pkg::NumSrc-1:0] rv_plic_0_intr_vector;

  assign rv_plic_0_intr_vector[0]  = 1'b0;  // ID [0] is a special case and must be tied to zero.
  assign rv_plic_0_intr_vector[1]  = spi_host_0_error_intr_o;
  assign rv_plic_0_intr_vector[2]  = spi_host_1_error_intr_o;
  assign rv_plic_0_intr_vector[3]  = spi_host_1_spi_event_intr_o;
  assign rv_plic_0_intr_vector[4]  = i2c_0_fmt_watermark_intr_o;
  assign rv_plic_0_intr_vector[5]  = i2c_0_rx_watermark_intr_o;
  assign rv_plic_0_intr_vector[6]  = i2c_0_fmt_overflow_intr_o;
  assign rv_plic_0_intr_vector[7]  = i2c_0_rx_overflow_intr_o;
  assign rv_plic_0_intr_vector[8]  = i2c_0_nak_intr_o;
  assign rv_plic_0_intr_vector[9]  = i2c_0_scl_interference_intr_o;
  assign rv_plic_0_intr_vector[10] = i2c_0_sda_interference_intr_o;
  assign rv_plic_0_intr_vector[11] = i2c_0_stretch_timeout_intr_o;
  assign rv_plic_0_intr_vector[12] = i2c_0_sda_unstable_intr_o;
  assign rv_plic_0_intr_vector[13] = i2c_0_trans_complete_intr_o;
  assign rv_plic_0_intr_vector[14] = i2c_0_tx_empty_intr_o;
  assign rv_plic_0_intr_vector[15] = i2c_0_tx_nonempty_intr_o;
  assign rv_plic_0_intr_vector[16] = i2c_0_tx_overflow_intr_o;
  assign rv_plic_0_intr_vector[17] = i2c_0_acq_overflow_intr_o;
  assign rv_plic_0_intr_vector[18] = i2c_0_ack_stop_intr_o;
  assign rv_plic_0_intr_vector[19] = i2c_0_host_timeout_intr_o;
  assign rv_plic_0_intr_vector[20] = i2s_0_i2s_event_intr_o;
  assign rv_plic_0_intr_vector[21] = gpio_8_intr_o;
  assign rv_plic_0_intr_vector[22] = gpio_9_intr_o;
  assign rv_plic_0_intr_vector[23] = gpio_10_intr_o;
  assign rv_plic_0_intr_vector[24] = gpio_11_intr_o;
  assign rv_plic_0_intr_vector[25] = gpio_12_intr_o;
  assign rv_plic_0_intr_vector[26] = gpio_13_intr_o;
  assign rv_plic_0_intr_vector[27] = gpio_14_intr_o;
  assign rv_plic_0_intr_vector[28] = gpio_15_intr_o;
  assign rv_plic_0_intr_vector[29] = gpio_16_intr_o;
  assign rv_plic_0_intr_vector[30] = gpio_17_intr_o;
  assign rv_plic_0_intr_vector[31] = gpio_18_intr_o;
  assign rv_plic_0_intr_vector[32] = gpio_19_intr_o;
  assign rv_plic_0_intr_vector[33] = gpio_20_intr_o;
  assign rv_plic_0_intr_vector[34] = gpio_21_intr_o;
  assign rv_plic_0_intr_vector[35] = gpio_22_intr_o;
  assign rv_plic_0_intr_vector[36] = gpio_23_intr_o;
  assign rv_plic_0_intr_vector[37] = gpio_24_intr_o;
  assign rv_plic_0_intr_vector[38] = gpio_25_intr_o;
  assign rv_plic_0_intr_vector[39] = gpio_26_intr_o;
  assign rv_plic_0_intr_vector[40] = gpio_27_intr_o;
  assign rv_plic_0_intr_vector[41] = gpio_28_intr_o;
  assign rv_plic_0_intr_vector[42] = gpio_29_intr_o;
  assign rv_plic_0_intr_vector[43] = gpio_30_intr_o;
  assign rv_plic_0_intr_vector[44] = gpio_31_intr_o;
  assign rv_plic_0_intr_vector[45] = uart_0_tx_watermark_intr_o;
  assign rv_plic_0_intr_vector[46] = uart_0_rx_watermark_intr_o;
  assign rv_plic_0_intr_vector[47] = uart_0_tx_empty_intr_o;
  assign rv_plic_0_intr_vector[48] = uart_0_rx_overflow_intr_o;
  assign rv_plic_0_intr_vector[49] = uart_0_rx_frame_err_intr_o;
  assign rv_plic_0_intr_vector[50] = uart_0_rx_break_err_intr_o;
  assign rv_plic_0_intr_vector[51] = uart_0_rx_timeout_intr_o;
  assign rv_plic_0_intr_vector[52] = uart_0_rx_parity_err_intr_o;
  assign rv_plic_0_intr_vector[53] = bundle__pd_peripheral__root__if.ext_intr_0;
  assign rv_plic_0_intr_vector[54] = bundle__pd_peripheral__root__if.ext_intr_1;
  assign rv_plic_0_intr_vector[55] = bundle__ao_periph__pd_peripheral__if.dma_window_intr;
  assign rv_plic_0_intr_vector[55] = 1'b0;
  assign rv_plic_0_intr_vector[56] = 1'b0;
  assign rv_plic_0_intr_vector[57] = 1'b0;
  assign rv_plic_0_intr_vector[58] = 1'b0;
  assign rv_plic_0_intr_vector[59] = 1'b0;
  assign rv_plic_0_intr_vector[60] = 1'b0;
  assign rv_plic_0_intr_vector[61] = 1'b0;
  assign rv_plic_0_intr_vector[62] = 1'b0;
  assign rv_plic_0_intr_vector[63] = 1'b0;

  reg_to_tlul #(
      .req_t(reg_pkg::reg_req_t),
      .rsp_t(reg_pkg::reg_rsp_t),
      .tl_h2d_t(tlul_pkg::tl_h2d_t),
      .tl_d2h_t(tlul_pkg::tl_d2h_t),
      .tl_a_user_t(tlul_pkg::tl_a_user_t),
      .tl_a_op_e(tlul_pkg::tl_a_op_e),
      .TL_A_USER_DEFAULT(tlul_pkg::TL_A_USER_DEFAULT),
      .PutFullData(tlul_pkg::PutFullData),
      .Get(tlul_pkg::Get)
  ) reg_to_tlul_rv_plic_0_i (
      .tl_o(rv_plic_0_tl_h2d),
      .tl_i(rv_plic_0_tl_d2h),
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::RV_PLIC_0_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::RV_PLIC_0_IDX])
  );

  rv_plic rv_plic_0_i (
      .clk_i(clk_cg),
      .rst_ni,
      .tl_i(rv_plic_0_tl_h2d),
      .tl_o(rv_plic_0_tl_d2h),
      .intr_src_i(rv_plic_0_intr_vector),
      .irq_id_o(),
      .irq_o(bundle__core_v_mini_mcu__pd_peripheral__if.rv_plic_0_irq),
      .msip_o(bundle__core_v_mini_mcu__pd_peripheral__if.rv_plic_0_msip)
  );

  pdm2pcm #(
      .reg_req_t(reg_pkg::reg_req_t),
      .reg_rsp_t(reg_pkg::reg_rsp_t)
  ) pdm2pcm_0_i (
      .clk_i(clk_cg),
      .rst_ni,
      .pdm_i(bundle__pd_peripheral__root__if.pdm2pcm0_pdm0_i),
      .pdm_clk_o(bundle__pd_peripheral__root__if.pdm2pcm0_pdm_clk0_o),
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::PDM2PCM_0_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::PDM2PCM_0_IDX])
  );

  logic [32-1:0] gpio_0_intr;
  logic [32-1:0] gpio_0_in;
  logic [32-1:0] gpio_0_out;
  logic [32-1:0] gpio_0_out_en;
  assign bundle__core_v_mini_mcu__pd_peripheral__if.gpio_0_intr_o = gpio_0_intr[0];
  assign gpio_0_in[0] = bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_0_i;
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_0_o = gpio_0_out[0];
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_0_oe = gpio_0_out_en[0];

  assign bundle__core_v_mini_mcu__pd_peripheral__if.gpio_1_intr_o = gpio_0_intr[1];
  assign gpio_0_in[1] = bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_1_i;
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_1_o = gpio_0_out[1];
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_1_oe = gpio_0_out_en[1];

  assign bundle__core_v_mini_mcu__pd_peripheral__if.gpio_2_intr_o = gpio_0_intr[2];
  assign gpio_0_in[2] = bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_2_i;
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_2_o = gpio_0_out[2];
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_2_oe = gpio_0_out_en[2];

  assign bundle__core_v_mini_mcu__pd_peripheral__if.gpio_3_intr_o = gpio_0_intr[3];
  assign gpio_0_in[3] = bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_3_i;
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_3_o = gpio_0_out[3];
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_3_oe = gpio_0_out_en[3];

  assign bundle__core_v_mini_mcu__pd_peripheral__if.gpio_4_intr_o = gpio_0_intr[4];
  assign gpio_0_in[4] = bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_4_i;
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_4_o = gpio_0_out[4];
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_4_oe = gpio_0_out_en[4];

  assign bundle__core_v_mini_mcu__pd_peripheral__if.gpio_5_intr_o = gpio_0_intr[5];
  assign gpio_0_in[5] = bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_5_i;
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_5_o = gpio_0_out[5];
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_5_oe = gpio_0_out_en[5];

  assign bundle__core_v_mini_mcu__pd_peripheral__if.gpio_6_intr_o = gpio_0_intr[6];
  assign gpio_0_in[6] = bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_6_i;
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_6_o = gpio_0_out[6];
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_6_oe = gpio_0_out_en[6];

  assign bundle__core_v_mini_mcu__pd_peripheral__if.gpio_7_intr_o = gpio_0_intr[7];
  assign gpio_0_in[7] = bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_7_i;
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_7_o = gpio_0_out[7];
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_7_oe = gpio_0_out_en[7];

  assign gpio_0_in[8] = 1'b0;

  assign gpio_0_in[9] = 1'b0;

  assign gpio_0_in[10] = 1'b0;

  assign gpio_0_in[11] = 1'b0;

  assign gpio_0_in[12] = 1'b0;

  assign gpio_0_in[13] = 1'b0;

  assign gpio_0_in[14] = 1'b0;

  assign gpio_0_in[15] = 1'b0;

  assign gpio_0_in[16] = 1'b0;

  assign gpio_0_in[17] = 1'b0;

  assign gpio_0_in[18] = 1'b0;

  assign gpio_0_in[19] = 1'b0;

  assign gpio_0_in[20] = 1'b0;

  assign gpio_0_in[21] = 1'b0;

  assign gpio_0_in[22] = 1'b0;

  assign gpio_0_in[23] = 1'b0;

  assign gpio_0_in[24] = 1'b0;

  assign gpio_0_in[25] = 1'b0;

  assign gpio_0_in[26] = 1'b0;

  assign gpio_0_in[27] = 1'b0;

  assign gpio_0_in[28] = 1'b0;

  assign gpio_0_in[29] = 1'b0;

  assign gpio_0_in[30] = 1'b0;

  assign gpio_0_in[31] = 1'b0;

  gpio #(
      .reg_req_t(reg_pkg::reg_req_t),
      .reg_rsp_t(reg_pkg::reg_rsp_t)
  ) gpio_0_i (
      .clk_i(clk_cg),
      .rst_ni,
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::GPIO_0_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::GPIO_0_IDX]),
      .gpio_in(gpio_0_in),
      .gpio_out(gpio_0_out),
      .gpio_tx_en_o(gpio_0_out_en),
      .gpio_in_sync_o(),
      .pin_level_interrupts_o(gpio_0_intr),
      .global_interrupt_o()
  );

  logic [32-1:0] gpio_1_intr;
  logic [32-1:0] gpio_1_in;
  logic [32-1:0] gpio_1_out;
  logic [32-1:0] gpio_1_out_en;
  assign gpio_1_in[0] = 1'b0;

  assign gpio_1_in[1] = 1'b0;

  assign gpio_1_in[2] = 1'b0;

  assign gpio_1_in[3] = 1'b0;

  assign gpio_1_in[4] = 1'b0;

  assign gpio_1_in[5] = 1'b0;

  assign gpio_1_in[6] = 1'b0;

  assign gpio_1_in[7] = 1'b0;

  assign gpio_8_intr_o = gpio_1_intr[8];
  assign gpio_1_in[8] = bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_8_i;
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_8_o = gpio_1_out[8];
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_8_oe = gpio_1_out_en[8];

  assign gpio_9_intr_o = gpio_1_intr[9];
  assign gpio_1_in[9] = bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_9_i;
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_9_o = gpio_1_out[9];
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_9_oe = gpio_1_out_en[9];

  assign gpio_10_intr_o = gpio_1_intr[10];
  assign gpio_1_in[10] = bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_10_i;
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_10_o = gpio_1_out[10];
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_10_oe = gpio_1_out_en[10];

  assign gpio_11_intr_o = gpio_1_intr[11];
  assign gpio_1_in[11] = bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_11_i;
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_11_o = gpio_1_out[11];
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_11_oe = gpio_1_out_en[11];

  assign gpio_12_intr_o = gpio_1_intr[12];
  assign gpio_1_in[12] = bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_12_i;
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_12_o = gpio_1_out[12];
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_12_oe = gpio_1_out_en[12];

  assign gpio_13_intr_o = gpio_1_intr[13];
  assign gpio_1_in[13] = bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_13_i;
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_13_o = gpio_1_out[13];
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_13_oe = gpio_1_out_en[13];

  assign gpio_14_intr_o = gpio_1_intr[14];
  assign gpio_1_in[14] = bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_14_i;
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_14_o = gpio_1_out[14];
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_14_oe = gpio_1_out_en[14];

  assign gpio_15_intr_o = gpio_1_intr[15];
  assign gpio_1_in[15] = bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_15_i;
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_15_o = gpio_1_out[15];
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_15_oe = gpio_1_out_en[15];

  assign gpio_16_intr_o = gpio_1_intr[16];
  assign gpio_1_in[16] = bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_16_i;
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_16_o = gpio_1_out[16];
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_16_oe = gpio_1_out_en[16];

  assign gpio_17_intr_o = gpio_1_intr[17];
  assign gpio_1_in[17] = bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_17_i;
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_17_o = gpio_1_out[17];
  assign bundle__pad_ring__pd_peripheral__if.io_pad_target_gpio_17_oe = gpio_1_out_en[17];

  assign gpio_18_intr_o = gpio_1_intr[18];
  assign gpio_1_in[18] = bundle__pd_peripheral__root__if.io_pad_target_gpio_18_i;
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_18_o = gpio_1_out[18];
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_18_oe = gpio_1_out_en[18];

  assign gpio_19_intr_o = gpio_1_intr[19];
  assign gpio_1_in[19] = bundle__pd_peripheral__root__if.io_pad_target_gpio_19_i;
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_19_o = gpio_1_out[19];
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_19_oe = gpio_1_out_en[19];

  assign gpio_20_intr_o = gpio_1_intr[20];
  assign gpio_1_in[20] = bundle__pd_peripheral__root__if.io_pad_target_gpio_20_i;
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_20_o = gpio_1_out[20];
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_20_oe = gpio_1_out_en[20];

  assign gpio_21_intr_o = gpio_1_intr[21];
  assign gpio_1_in[21] = bundle__pd_peripheral__root__if.io_pad_target_gpio_21_i;
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_21_o = gpio_1_out[21];
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_21_oe = gpio_1_out_en[21];

  assign gpio_22_intr_o = gpio_1_intr[22];
  assign gpio_1_in[22] = bundle__pd_peripheral__root__if.io_pad_target_gpio_22_i;
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_22_o = gpio_1_out[22];
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_22_oe = gpio_1_out_en[22];

  assign gpio_23_intr_o = gpio_1_intr[23];
  assign gpio_1_in[23] = bundle__pd_peripheral__root__if.io_pad_target_gpio_23_i;
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_23_o = gpio_1_out[23];
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_23_oe = gpio_1_out_en[23];

  assign gpio_24_intr_o = gpio_1_intr[24];
  assign gpio_1_in[24] = bundle__pd_peripheral__root__if.io_pad_target_gpio_24_i;
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_24_o = gpio_1_out[24];
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_24_oe = gpio_1_out_en[24];

  assign gpio_25_intr_o = gpio_1_intr[25];
  assign gpio_1_in[25] = bundle__pd_peripheral__root__if.io_pad_target_gpio_25_i;
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_25_o = gpio_1_out[25];
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_25_oe = gpio_1_out_en[25];

  assign gpio_26_intr_o = gpio_1_intr[26];
  assign gpio_1_in[26] = bundle__pd_peripheral__root__if.io_pad_target_gpio_26_i;
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_26_o = gpio_1_out[26];
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_26_oe = gpio_1_out_en[26];

  assign gpio_27_intr_o = gpio_1_intr[27];
  assign gpio_1_in[27] = bundle__pd_peripheral__root__if.io_pad_target_gpio_27_i;
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_27_o = gpio_1_out[27];
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_27_oe = gpio_1_out_en[27];

  assign gpio_28_intr_o = gpio_1_intr[28];
  assign gpio_1_in[28] = bundle__pd_peripheral__root__if.io_pad_target_gpio_28_i;
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_28_o = gpio_1_out[28];
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_28_oe = gpio_1_out_en[28];

  assign gpio_29_intr_o = gpio_1_intr[29];
  assign gpio_1_in[29] = bundle__pd_peripheral__root__if.io_pad_target_gpio_29_i;
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_29_o = gpio_1_out[29];
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_29_oe = gpio_1_out_en[29];

  assign gpio_30_intr_o = gpio_1_intr[30];
  assign gpio_1_in[30] = bundle__pd_peripheral__root__if.io_pad_target_gpio_30_i;
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_30_o = gpio_1_out[30];
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_30_oe = gpio_1_out_en[30];

  assign gpio_31_intr_o = gpio_1_intr[31];
  assign gpio_1_in[31] = bundle__pd_peripheral__root__if.io_pad_target_gpio_31_i;
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_31_o = gpio_1_out[31];
  assign bundle__pd_peripheral__root__if.io_pad_target_gpio_31_oe = gpio_1_out_en[31];

  gpio #(
      .reg_req_t(reg_pkg::reg_req_t),
      .reg_rsp_t(reg_pkg::reg_rsp_t)
  ) gpio_1_i (
      .clk_i(clk_cg),
      .rst_ni,
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::GPIO_1_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::GPIO_1_IDX]),
      .gpio_in(gpio_1_in),
      .gpio_out(gpio_1_out),
      .gpio_tx_en_o(gpio_1_out_en),
      .gpio_in_sync_o(),
      .pin_level_interrupts_o(gpio_1_intr),
      .global_interrupt_o()
  );

  reg_to_tlul #(
      .req_t(reg_pkg::reg_req_t),
      .rsp_t(reg_pkg::reg_rsp_t),
      .tl_h2d_t(tlul_pkg::tl_h2d_t),
      .tl_d2h_t(tlul_pkg::tl_d2h_t),
      .tl_a_user_t(tlul_pkg::tl_a_user_t),
      .tl_a_op_e(tlul_pkg::tl_a_op_e),
      .TL_A_USER_DEFAULT(tlul_pkg::TL_A_USER_DEFAULT),
      .PutFullData(tlul_pkg::PutFullData),
      .Get(tlul_pkg::Get)
  ) reg_to_tlul_uart_0_i (
      .tl_o(uart_0_tl_h2d),
      .tl_i(uart_0_tl_d2h),
      .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::UART_0_IDX]),
      .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::UART_0_IDX])
  );

  uart uart_0_i (
      .clk_i(clk_cg),
      .rst_ni,
      .intr_tx_watermark_o(uart_0_tx_watermark_intr_o),
      .intr_rx_watermark_o(uart_0_rx_watermark_intr_o),
      .intr_tx_empty_o(uart_0_tx_empty_intr_o),
      .intr_rx_overflow_o(uart_0_rx_overflow_intr_o),
      .intr_rx_frame_err_o(uart_0_rx_frame_err_intr_o),
      .intr_rx_break_err_o(uart_0_rx_break_err_intr_o),
      .intr_rx_timeout_o(uart_0_rx_timeout_intr_o),
      .intr_rx_parity_err_o(uart_0_rx_parity_err_intr_o),
      .cio_rx_i(bundle__pad_ring__pd_peripheral__if.uart0_rx0_i),
      .cio_tx_o(bundle__pad_ring__pd_peripheral__if.uart0_tx0_o),
      .cio_tx_en_o(bundle__pad_ring__pd_peripheral__if.uart0_tx0_oe),
      .tl_i(uart_0_tl_h2d),
      .tl_o(uart_0_tl_d2h)
  );


endmodule : peripheral_subsystem

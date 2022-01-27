module peripheral_subsystem import obi_pkg::*;
(
    input logic         clk_i,
    input logic         rst_ni,

    input  obi_req_t    slave_req_i,
    output obi_resp_t   slave_resp_o,

    //SOC CTRL
    output logic         tests_passed_o,
    output logic         tests_failed_o,
    output logic         exit_valid_o,
    output logic [31:0]  exit_value_o,

    //UART
    input               uart_rx_i,
    output logic        uart_tx_o,
    output logic        uart_tx_en_o,
    output logic        uart_intr_tx_watermark_o ,
    output logic        uart_intr_rx_watermark_o ,
    output logic        uart_intr_tx_empty_o  ,
    output logic        uart_intr_rx_overflow_o  ,
    output logic        uart_intr_rx_frame_err_o ,
    output logic        uart_intr_rx_break_err_o ,
    output logic        uart_intr_rx_timeout_o   ,
    output logic        uart_intr_rx_parity_err_o

);

    import core_v_mini_mcu_pkg::*;
    import reg_pkg::*;

    reg_pkg::reg_req_t peripheral_req;
    reg_pkg::reg_rsp_t peripheral_rsp;

    reg_pkg::reg_req_t [core_v_mini_mcu_pkg::SYSTEM_NPERIPHERALS-1:0] peripheral_slv_req;
    reg_pkg::reg_rsp_t [core_v_mini_mcu_pkg::SYSTEM_NPERIPHERALS-1:0] peripheral_slv_rsp;

    //Address Decoder
    logic [core_v_mini_mcu_pkg::PERIPHERALS_PORT_SEL_WIDTH-1:0] peripheral_select;

    periph_to_reg #(
      .req_t(reg_pkg::reg_req_t),
      .rsp_t(reg_pkg::reg_rsp_t)
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
        .NoIndices(core_v_mini_mcu_pkg::SYSTEM_NPERIPHERALS),
        .NoRules(core_v_mini_mcu_pkg::SYSTEM_NPERIPHERALS),
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
      .NoPorts(core_v_mini_mcu_pkg::SYSTEM_NPERIPHERALS),
      .req_t(reg_pkg::reg_req_t),
      .rsp_t(reg_pkg::reg_rsp_t)
    ) reg_demux_i (
        .clk_i,
        .rst_ni,
        .in_select_i(peripheral_select),
        .in_req_i(peripheral_req),
        .in_rsp_o(peripheral_rsp),
        .out_req_o(peripheral_slv_req),
        .out_rsp_i(peripheral_slv_rsp)
    );

    uart #(
      .reg_req_t(reg_pkg::reg_req_t),
      .reg_rsp_t(reg_pkg::reg_rsp_t)
    ) uart_i (
        .clk_i,
        .rst_ni,
        .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::UART_IDX]),
        .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::UART_IDX]),
        .cio_rx_i(uart_rx_i),
        .cio_tx_o(uart_tx_o),
        .cio_tx_en_o(uart_tx_en_o),
        .intr_tx_watermark_o(uart_intr_tx_watermark_o),
        .intr_rx_watermark_o(uart_intr_rx_watermark_o),
        .intr_tx_empty_o(uart_intr_tx_empty_o),
        .intr_rx_overflow_o(uart_intr_rx_overflow_o),
        .intr_rx_frame_err_o(uart_intr_rx_frame_err_o),
        .intr_rx_break_err_o(uart_intr_rx_break_err_o),
        .intr_rx_timeout_o(uart_intr_rx_timeout_o),
        .intr_rx_parity_err_o(uart_intr_rx_parity_err_o)
    );

    soc_ctrl #(
      .reg_req_t(reg_pkg::reg_req_t),
      .reg_rsp_t(reg_pkg::reg_rsp_t)
    ) soc_ctrl_i (
       .clk_i,
       .rst_ni,
       .reg_req_i(peripheral_slv_req[core_v_mini_mcu_pkg::SOC_CTRL_IDX]),
       .reg_rsp_o(peripheral_slv_rsp[core_v_mini_mcu_pkg::SOC_CTRL_IDX]),
       .tests_passed_o,
       .tests_failed_o,
       .exit_valid_o,
       .exit_value_o
    );

endmodule : peripheral_subsystem
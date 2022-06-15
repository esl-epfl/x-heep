// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module system_xbar
  import obi_pkg::*;
  import addr_map_rule_pkg::*;
  import core_v_mini_mcu_pkg::*;
#(
    parameter core_v_mini_mcu_pkg::bus_type_e BUS_TYPE = core_v_mini_mcu_pkg::BusType,
    parameter XBAR_NMASTER = 3,
    parameter XBAR_NSLAVE = 6
) (
    input logic clk_i,
    input logic rst_ni,

    input  obi_req_t  [XBAR_NMASTER-1:0] master_req_i,
    output obi_resp_t [XBAR_NMASTER-1:0] master_resp_o,

    output obi_req_t  [XBAR_NSLAVE-1:0] slave_req_o,
    input  obi_resp_t [XBAR_NSLAVE-1:0] slave_resp_i

);

  localparam int unsigned LOG_XBAR_NMASTER = XBAR_NMASTER > 1 ? $clog2(XBAR_NMASTER) : 32'd1;
  localparam int unsigned LOG_XBAR_NSLAVE = XBAR_NSLAVE > 1 ? $clog2(XBAR_NSLAVE) : 32'd1;

  //Aggregated Request Data (from Master -> slaves)
  //WE + BE + ADDR + WDATA
  localparam int unsigned REQ_AGG_DATA_WIDTH = 1 + 4 + 32 + 32;
  localparam int unsigned RESP_AGG_DATA_WIDTH = 32;

  //Address Decoder
  logic [XBAR_NMASTER-1:0][LOG_XBAR_NSLAVE-1:0] port_sel;

  logic [0:0][LOG_XBAR_NSLAVE-1:0] port_sel_onetom;
  logic [0:0] neck_req_req;
  logic [0:0] neck_resp_gnt;
  logic [0:0] neck_resp_rvalid;
  logic [0:0][31:0] neck_resp_rdata;
  obi_req_t neck_req;
  logic [0:0][REQ_AGG_DATA_WIDTH-1:0] neck_req_out_data;


  logic [XBAR_NMASTER-1:0] master_req_req;
  logic [XBAR_NMASTER-1:0] master_resp_gnt;
  logic [XBAR_NMASTER-1:0] master_resp_rvalid;
  logic [XBAR_NMASTER-1:0][31:0] master_resp_rdata;

  logic [XBAR_NSLAVE-1:0] slave_req_req;
  logic [XBAR_NSLAVE-1:0] slave_resp_gnt;
  logic [XBAR_NSLAVE-1:0] slave_resp_rvalid;
  logic [XBAR_NSLAVE-1:0][31:0] slave_resp_rdata;


  logic [XBAR_NMASTER-1:0][REQ_AGG_DATA_WIDTH-1:0] master_req_out_data;
  logic [XBAR_NSLAVE-1:0][REQ_AGG_DATA_WIDTH-1:0] slave_req_out_data;

  if (BUS_TYPE == NtoM) begin : gen_addr_decoders_NtoM
    for (genvar i = 0; i < XBAR_NMASTER; i++) begin : gen_addr_decoders
      addr_decode #(
          /// Highest index which can happen in a rule.
          .NoIndices(XBAR_NSLAVE),
          .NoRules(XBAR_NSLAVE),
          .addr_t(logic [31:0]),
          .rule_t(addr_map_rule_pkg::addr_map_rule_t)
      ) addr_decode_i (
          .addr_i(master_req_i[i].addr),
          .addr_map_i(core_v_mini_mcu_pkg::XBAR_ADDR_RULES),
          .idx_o(port_sel[i]),
          .dec_valid_o(),
          .dec_error_o(),
          .en_default_idx_i(1'b1),
          .default_idx_i(core_v_mini_mcu_pkg::ERROR_IDX[LOG_XBAR_NSLAVE-1:0])
      );
    end
  end

  //unroll obi struct
  for (genvar i = 0; i < XBAR_NMASTER; i++) begin : gen_unroll_master
    assign master_req_req[i] = master_req_i[i].req;
    assign master_req_out_data[i] = {
      master_req_i[i].we, master_req_i[i].be, master_req_i[i].addr, master_req_i[i].wdata
    };
    assign master_resp_o[i].gnt = master_resp_gnt[i];
    assign master_resp_o[i].rdata = master_resp_rdata[i];
    assign master_resp_o[i].rvalid = master_resp_rvalid[i];
  end
  for (genvar i = 0; i < XBAR_NSLAVE; i++) begin : gen_unroll_slave
    assign slave_req_o[i].req = slave_req_req[i];
    assign {slave_req_o[i].we, slave_req_o[i].be, slave_req_o[i].addr, slave_req_o[i].wdata} = slave_req_out_data[i];
    assign slave_resp_rdata[i] = slave_resp_i[i].rdata;
    assign slave_resp_gnt[i] = slave_resp_i[i].gnt;
    assign slave_resp_rvalid[i] = slave_resp_i[i].rvalid;
  end


  if (BUS_TYPE == NtoM) begin : gen_xbar_NtoM

    //Crossbar instantiation
    xbar_varlat #(
        .AggregateGnt(1),
        .NumIn(XBAR_NMASTER),
        .NumOut(XBAR_NSLAVE),
        .ReqDataWidth(REQ_AGG_DATA_WIDTH),
        .RespDataWidth(RESP_AGG_DATA_WIDTH)
    ) i_xbar (
        .clk_i,
        .rst_ni,
        .req_i  (master_req_req),
        .add_i  (port_sel),
        .wdata_i(master_req_out_data),
        .gnt_o  (master_resp_gnt),
        .rdata_o(master_resp_rdata),
        .rr_i   ('0),
        .vld_o  (master_resp_rvalid),
        .gnt_i  (slave_resp_gnt),
        .req_o  (slave_req_req),
        .vld_i  (slave_resp_rvalid),
        .wdata_o(slave_req_out_data),
        .rdata_i(slave_resp_rdata)
    );

  end else begin : gen_xbar_1toM

    // Nto1 Crossbar instantiation
    xbar_varlat #(
        .NumIn(XBAR_NMASTER),
        .NumOut(1),
        .ReqDataWidth(REQ_AGG_DATA_WIDTH),
        .RespDataWidth(RESP_AGG_DATA_WIDTH)
    ) i_xbar_master (
        .clk_i,
        .rst_ni,
        .req_i  (master_req_req),
        .add_i  ('0),
        .wdata_i(master_req_out_data),
        .gnt_o  (master_resp_gnt),
        .rdata_o(master_resp_rdata),
        .rr_i   ('0),
        .vld_o  (master_resp_rvalid),
        .gnt_i  (neck_resp_gnt),
        .req_o  (neck_req_req),
        .vld_i  (neck_resp_rvalid),
        .wdata_o(neck_req_out_data),
        .rdata_i(neck_resp_rdata)
    );

    assign {neck_req.we, neck_req.be, neck_req.addr, neck_req.wdata} = neck_req_out_data[0];

    addr_decode #(
        /// Highest index which can happen in a rule.
        .NoIndices(XBAR_NSLAVE),
        .NoRules(XBAR_NSLAVE),
        .addr_t(logic [31:0]),
        .rule_t(addr_map_rule_pkg::addr_map_rule_t)
    ) addr_decode_i (
        .addr_i(neck_req.addr),
        .addr_map_i(core_v_mini_mcu_pkg::XBAR_ADDR_RULES),
        .idx_o(port_sel_onetom[0]),
        .dec_valid_o(),
        .dec_error_o(),
        .en_default_idx_i(1'b1),
        .default_idx_i(core_v_mini_mcu_pkg::ERROR_IDX[LOG_XBAR_NSLAVE-1:0])
    );

    // 1toM Crossbar instantiation
    xbar_varlat #(
        /*
          AggregateGnt should be 0 when a single master is actually aggregating multiple master requests.
          This is not needed when a real-single master is used or multiple masters are used as the
          rr_arb_tree dispatches the grant to each corresponding master.
          Whereas, when the xbar_varlat is used with a single master, which is shared among severals
          (as in this case as an output of another xbar_varlat), the rr_arb_tree gives all the grant to the
          shared single master, thus granting transactions that should not be granted
        */
        .AggregateGnt(0),
        .NumIn(1),
        .NumOut(XBAR_NSLAVE),
        .ReqDataWidth(REQ_AGG_DATA_WIDTH),
        .RespDataWidth(RESP_AGG_DATA_WIDTH)
    ) i_xbar_slave (
        .clk_i,
        .rst_ni,
        .req_i  (neck_req_req),
        .add_i  (port_sel_onetom[0]),
        .wdata_i(neck_req_out_data),
        .gnt_o  (neck_resp_gnt),
        .rdata_o(neck_resp_rdata),
        .rr_i   ('0),
        .vld_o  (neck_resp_rvalid),
        .gnt_i  (slave_resp_gnt),
        .req_o  (slave_req_req),
        .vld_i  (slave_resp_rvalid),
        .wdata_o(slave_req_out_data),
        .rdata_i(slave_resp_rdata)
    );
  end

endmodule : system_xbar

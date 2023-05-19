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
% if ram_numbanks_il == 0:
  logic [XBAR_NMASTER-1:0][LOG_XBAR_NSLAVE-1:0] port_sel;
% else:
  logic [XBAR_NMASTER-1:0][LOG_XBAR_NSLAVE-1:0] port_sel, pre_port_sel;
  logic [XBAR_NMASTER-1:0][31:0] post_master_req_addr;
% endif

  logic [0:0][LOG_XBAR_NSLAVE-1:0] port_sel_onetom;
  logic [0:0] neck_req_req, neck_req_oustanding_req;
  logic [0:0] neck_resp_gnt, neck_resp_outstanding_gnt;
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
% if ram_numbanks_il == 0:
          .idx_o(port_sel[i]),
% else:
          .idx_o(pre_port_sel[i]),
% endif          
          .dec_valid_o(),
          .dec_error_o(),
          .en_default_idx_i(1'b1),
          .default_idx_i(core_v_mini_mcu_pkg::ERROR_IDX[LOG_XBAR_NSLAVE-1:0])
      );
    end
% if ram_numbanks_il != 0:

    localparam ZERO = 32'h0;

    for (genvar j = 0; j < XBAR_NMASTER; j++) begin : gen_addr_napot
      always_comb begin
        port_sel[j] = 1;
        post_master_req_addr[j] = '0;
        if (pre_port_sel[j] == NUM_BANKS[LOG_XBAR_NSLAVE-1:0] - (NUM_BANKS_IL[LOG_XBAR_NSLAVE-1:0]-1)) begin
            port_sel[j] = NUM_BANKS[LOG_XBAR_NSLAVE-1:0] - (NUM_BANKS_IL[LOG_XBAR_NSLAVE-1:0]-1) + {ZERO[LOG_XBAR_NSLAVE-${1+log_ram_numbanks_il}:0],master_req_i[j].addr[${1+log_ram_numbanks_il}:2]};
          post_master_req_addr[j] = {master_req_i[j].addr[31:${2+log_ram_numbanks_il}], ${2+log_ram_numbanks_il}'h0};
        end else begin
          port_sel[j] = pre_port_sel[j];
          post_master_req_addr[j] = master_req_i[j].addr;
        end
      end
    end
% endif    
  end

  //unroll obi struct
  for (genvar i = 0; i < XBAR_NMASTER; i++) begin : gen_unroll_master
    assign master_req_req[i] = master_req_i[i].req;
    assign master_req_out_data[i] = {
% if ram_numbanks_il == 0:
      master_req_i[i].we, master_req_i[i].be, master_req_i[i].addr, master_req_i[i].wdata
% else:    
      master_req_i[i].we, master_req_i[i].be, post_master_req_addr[i], master_req_i[i].wdata
% endif      
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
        .gnt_i  (neck_resp_outstanding_gnt),
        .req_o  (neck_req_req),
        .vld_i  (neck_resp_rvalid),
        .wdata_o(neck_req_out_data),
        .rdata_i(neck_resp_rdata)
    );

    assign {neck_req.we, neck_req.be, neck_req.addr, neck_req.wdata} = neck_req_out_data[0];
    assign neck_req.req = neck_req_oustanding_req;

    //block outstanding transactions
    typedef enum logic {
      NECK_REQ,
      NECK_WAITFOR_VALID
    } neck_req_fsm_e;

    neck_req_fsm_e state_n, state_q;

    /* block outstanding transactions
      The Master M1 may get a GNT at cycle 1, then the Master M2
      gets a GNT at cycle 2 (from the same or different slaves)
      As outstanding transactions are not supported (neither in-order nor out-of-order)
      we need to block the transaction.

      The addr_dec_resp_mux_varlat gates the second request of a master until the valid of the first request returns
      However, when different masters collides into the 1toM bus, the second part of the xbar sees a single master (the neck) issueing multiple requests (although
      these 2 reqeusts can actually come from 2 different masters M1 and M2)
      The second neck.req is gated by the addr_dec_resp_mux_varlat, but not its grant, thus the following FSM gates the gnt as the original master (i.e. the one before the neck)
      The reason why we have to gate the grant is that the M2 issued a request, although the neck.req is gated by the valid of the M1 request, the grant is propagated
      to M2, which gets its request granted without begin correct.
    */
    always_comb begin
      state_n = state_q;
      neck_req_oustanding_req = neck_req_req;
      neck_resp_outstanding_gnt = neck_resp_gnt;

      case(state_q)

      NECK_REQ: begin
        if(neck_req_req && neck_resp_gnt) begin
          state_n = NECK_WAITFOR_VALID;
        end
      end

      NECK_WAITFOR_VALID: begin
        neck_req_oustanding_req = 1'b0;
        neck_resp_outstanding_gnt = 1'b0;
        if(neck_resp_rvalid) begin
          neck_req_oustanding_req = neck_req_req;
          neck_resp_outstanding_gnt = neck_resp_gnt;
          if(neck_req_req && neck_resp_gnt) begin
            state_n = NECK_WAITFOR_VALID;
          end else begin
            state_n = NECK_REQ;
          end
        end
      end

      endcase
    end

    always_ff @(posedge clk_i or negedge rst_ni) begin
      if (~rst_ni) begin
        state_q <= NECK_REQ;
      end else begin
        state_q <= state_n;
      end
    end

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
        .req_i  (neck_req_oustanding_req),
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

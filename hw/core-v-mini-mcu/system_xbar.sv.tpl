// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module system_xbar #(
    parameter core_v_mini_mcu_pkg::bus_type_e BUS_TYPE = core_v_mini_mcu_pkg::BusType,
    parameter int unsigned XBAR_NMASTER = 3,
    parameter int unsigned XBAR_NSLAVE = 6,
    // Dependent parameters: do not override!
    localparam int unsigned IdxWidth = cf_math_pkg::idx_width(XBAR_NSLAVE)
) (
    input logic clk_i,
    input logic rst_ni,

    // Address map
    input addr_map_rule_pkg::addr_map_rule_t [XBAR_NSLAVE-1:0] addr_map_i,

    // Default slave index
    input logic [IdxWidth-1:0] default_idx_i,

    // Master ports
    input  obi_pkg::obi_req_t  [XBAR_NMASTER-1:0] master_req_i,
    output obi_pkg::obi_resp_t [XBAR_NMASTER-1:0] master_resp_o,

    // Slave ports
    output obi_pkg::obi_req_t  [XBAR_NSLAVE-1:0] slave_req_o,
    input  obi_pkg::obi_resp_t [XBAR_NSLAVE-1:0] slave_resp_i

);
  import obi_pkg::*;
  import core_v_mini_mcu_pkg::*;

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

  // Neck crossbar
  obi_req_t neck_req;
  obi_resp_t neck_resp;

  logic [XBAR_NMASTER-1:0] master_req_req;
  logic [XBAR_NMASTER-1:0] master_resp_gnt;
  logic [XBAR_NMASTER-1:0] master_resp_rvalid;
  logic [XBAR_NMASTER-1:0][31:0] master_resp_rdata;

  logic [XBAR_NSLAVE-1:0] slave_req_req;
  logic [XBAR_NSLAVE-1:0] slave_resp_gnt;
  logic [XBAR_NSLAVE-1:0] slave_resp_rvalid;
  logic [XBAR_NSLAVE-1:0][31:0] slave_resp_rdata;


  logic [XBAR_NMASTER-1:0][REQ_AGG_DATA_WIDTH-1:0] master_req_data;
  logic [XBAR_NSLAVE-1:0][REQ_AGG_DATA_WIDTH-1:0] slave_req_out_data;
  obi_req_t [XBAR_NMASTER-1:0] master_req;

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
          .addr_map_i(addr_map_i),
% if ram_numbanks_il == 0:
          .idx_o(port_sel[i]),
% else:
          .idx_o(pre_port_sel[i]),
% endif          
          .dec_valid_o(),
          .dec_error_o(),
          .en_default_idx_i(1'b1),
          .default_idx_i(default_idx_i)
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

  // Propagate interleaved address
  generate
    for (genvar i = 0; i < XBAR_NMASTER; i++) begin : gen_unroll_master
      assign master_req[i] = '{
        req: master_req_i[i].req,
        we: master_req_i[i].we,
        be: master_req_i[i].be,
  % if ram_numbanks_il == 0:
        addr: master_req_i[i].addr,
  % else:
        addr: post_master_req_addr[i],
  % endif
        wdata: master_req_i[i].wdata
      };
    end
  endgenerate

  // Crossbar instantiation
  // ----------------------
  generate
    if (BUS_TYPE == NtoM) begin : gen_xbar_NtoM

      // Unroll OBI structs
      for (genvar i = 0; unsigned'(i) < XBAR_NMASTER; i++) begin: gen_unroll_master
        assign master_req_req[i] = master_req[i].req;
        assign master_req_data[i] = {
          master_req[i].we,
          master_req[i].be,
          master_req[i].addr,
          master_req[i].wdata
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

      // N-to-M crossbar
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
          .wdata_i(master_req_data),
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
      // N-to-1 crossbar
      xbar_varlat_n_to_one #(
        .XBAR_NMASTER (XBAR_NMASTER)
      ) i_xbar_master (
        .clk_i         (clk_i),
        .rst_ni        (rst_ni),
        .master_req_i  (master_req),
        .master_resp_o (master_resp_o),
        .slave_req_o   (neck_req),
        .slave_resp_i  (neck_resp)
      );

      // 1-to-N crossbar
      // NOTE: AGGREGATE_GNT should be 0 when a single master is actually 
      // aggregating multiple master requests. This is not needed when a 
      // real-single master is used or multiple masters are used as the 
      // rr_arb_tree dispatches the grant to each corresponding master.
      // Whereas, when the xbar_varlat is used with a single master, which is 
      // shared among severals (as in this case as an output of another 
      // xbar_varlat), the rr_arb_tree gives all the grant to the shared single 
      // master, thus granting transactions that should not be granted.
      xbar_varlat_one_to_n #(
        .XBAR_NSLAVE   (XBAR_NSLAVE),
        .AGGREGATE_GNT (32'd0) // the neck request is aggregating all the input masters
      ) u_xbar_varlat_one_to_n (
        .clk_i         (clk_i),
        .rst_ni        (rst_ni),
        .addr_map_i    (addr_map_i),
        .default_idx_i (default_idx_i),
        .master_req_i  (neck_req),
        .master_resp_o (neck_resp),
        .slave_req_o   (slave_req_o),
        .slave_resp_i  (slave_resp_i)
      );
    end
  endgenerate
endmodule : system_xbar

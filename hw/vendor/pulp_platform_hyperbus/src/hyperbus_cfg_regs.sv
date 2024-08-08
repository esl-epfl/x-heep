// Copyright 2023 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Paul Scheffler <paulsc@iis.ee.ethz.ch>

module hyperbus_cfg_regs #(
    parameter int unsigned  NumChips        = -1,
    parameter int unsigned  NumPhys         = -1,
    parameter int unsigned  RegAddrWidth    = -1,
    parameter int unsigned  RegDataWidth    = -1,
    parameter type          reg_req_t       = logic,
    parameter type          reg_rsp_t       = logic,
    parameter type          rule_t          = logic,
    parameter logic [RegDataWidth-1:0] RstChipBase  = -1,   // Base address for all chips
    parameter logic [RegDataWidth-1:0] RstChipSpace = -1,   // 64 KiB: Current maximum HyperBus device size
    parameter hyperbus_pkg::hyper_cfg_t RstCfg  = hyperbus_pkg::gen_RstCfg(NumPhys)
) (
    input logic     clk_i,
    input logic     rst_ni,

    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    output hyperbus_pkg::hyper_cfg_t    cfg_o,
    output rule_t [NumChips-1:0]        chip_rules_o,
    input                               trans_active_i
);
    `include "common_cells/registers.svh"

    // Internal Parameters
    localparam int unsigned NumBaseRegs     = 11;
    localparam int unsigned NumRegs         = 2*NumChips + NumBaseRegs;
    localparam int unsigned RegsBits        = cf_math_pkg::idx_width(NumRegs);
    localparam int unsigned RegStrbWidth    = RegDataWidth/8;                   // TODO ASSERT: Must be power of two >= 16!!

    // Data and index types
    typedef logic [RegsBits-1:0]        reg_idx_t;
    typedef logic [RegDataWidth-1:0]    reg_data_t;

    // Local signals
    hyperbus_pkg::hyper_cfg_t       cfg_d, cfg_q, cfg_rstval;
    reg_data_t [NumChips-1:0][1:0]  crange_d, crange_q, crange_rstval;
    reg_idx_t   sel_reg;
    logic       sel_reg_mapped;
    reg_data_t  wmask;

    assign sel_reg          = reg_req_i.addr[$clog2(RegStrbWidth) +: RegsBits];
    assign sel_reg_mapped   = (sel_reg < NumRegs);

    assign reg_rsp_o.ready  = ~trans_active_i;  // Config writeable unless currently in transfer
    assign reg_rsp_o.error  = ~sel_reg_mapped;

    // Read from register
    always_comb begin : proc_comb_read
        reg_data_t [NumRegs-1:0] rfield;
        reg_rsp_o.rdata = '0;
        if (sel_reg_mapped) begin
            rfield = {
                crange_q,
                reg_data_t'(cfg_q.t_csh_cycles),
                reg_data_t'(cfg_q.which_phy),
                reg_data_t'(cfg_q.phys_in_use),
                reg_data_t'(cfg_q.address_space),
                reg_data_t'(cfg_q.address_mask_msb),
                reg_data_t'(cfg_q.t_tx_clk_delay),
                reg_data_t'(cfg_q.t_rx_clk_delay),
                reg_data_t'(cfg_q.t_read_write_recovery),
                reg_data_t'(cfg_q.t_burst_max),
                reg_data_t'(cfg_q.en_latency_additional),
                reg_data_t'(cfg_q.t_latency_access)
            };
            reg_rsp_o.rdata = rfield[sel_reg];
        end
    end

    // Generate write mask
    for (genvar i = 0; unsigned'(i) < RegStrbWidth; ++i ) begin : gen_wmask
        assign wmask[8*i +: 8] = {8{reg_req_i.wstrb[i]}};
    end

    // Write to register
    always_comb begin : proc_comb_write
        logic  chip_reg;
        logic [$clog2(NumChips)-1:0] sel_chip;
        cfg_d     = cfg_q;
        crange_d  = crange_q;
        if (reg_req_i.valid & reg_req_i.write & sel_reg_mapped) begin
            case (sel_reg)
                'h0: cfg_d.t_latency_access         = (~wmask & cfg_q.t_latency_access        ) | (wmask & reg_req_i.wdata);
                'h1: cfg_d.en_latency_additional    = (~wmask & cfg_q.en_latency_additional   ) | (wmask & reg_req_i.wdata);
                'h2: cfg_d.t_burst_max              = (~wmask & cfg_q.t_burst_max             ) | (wmask & reg_req_i.wdata);
                'h3: cfg_d.t_read_write_recovery    = (~wmask & cfg_q.t_read_write_recovery   ) | (wmask & reg_req_i.wdata);
                'h4: cfg_d.t_rx_clk_delay           = (~wmask & cfg_q.t_rx_clk_delay          ) | (wmask & reg_req_i.wdata);
                'h5: cfg_d.t_tx_clk_delay           = (~wmask & cfg_q.t_tx_clk_delay          ) | (wmask & reg_req_i.wdata);
                'h6: cfg_d.address_mask_msb         = (~wmask & cfg_q.address_mask_msb        ) | (wmask & reg_req_i.wdata);
                'h7: cfg_d.address_space            = (~wmask & cfg_q.address_space           ) | (wmask & reg_req_i.wdata);
                'h8: cfg_d.phys_in_use              = (NumPhys==1) ? 0 : ( (~wmask & cfg_q.phys_in_use ) | (wmask & reg_req_i.wdata) );
                'h9: cfg_d.which_phy                = (NumPhys==1) ? 0 : ( (~wmask & cfg_q.which_phy   ) | (wmask & reg_req_i.wdata) );
                'ha: cfg_d.t_csh_cycles             = (~wmask & cfg_q.t_csh_cycles            ) | (wmask & reg_req_i.wdata);
                default: begin
                    {sel_chip, chip_reg} = sel_reg - NumBaseRegs;
                    crange_d[sel_chip][chip_reg] = (~wmask & crange_q[sel_chip][chip_reg]) |  (wmask & reg_req_i.wdata);
                end
            endcase // sel_reg
        end
    end

    for (genvar i = 0; unsigned'(i) < NumChips; i++) begin : gen_crange_rstval
            assign crange_rstval[i][0]  = RstChipBase + (RstChipSpace * i);
            assign crange_rstval[i][1]  = RstChipBase + (RstChipSpace * (i+1));     // Address decoder: end noninclusive
    end

    // Registers
    `FFARN(cfg_q, cfg_d, RstCfg, clk_i, rst_ni);
    `FFARN(crange_q, crange_d, crange_rstval, clk_i, rst_ni);

    // Outputs
    assign cfg_o  = cfg_q;
    for (genvar i = 0; unsigned'(i) < NumChips; ++i) begin : gen_crange_out
        assign chip_rules_o[i].idx         = unsigned'(i);   // No overlap: keep indices sequential
        assign chip_rules_o[i].start_addr  = crange_q[i][0];
        assign chip_rules_o[i].end_addr    = crange_q[i][1];
    end

    // pragma translate_off
    `ifndef VERILATOR
    initial assert (RegDataWidth >= 16 && $countones(RegDataWidth) == 1)
        else $error("RegDataWidth must be a power of two bigger than 16.");
    `endif
    // pragma translate_on

endmodule : hyperbus_cfg_regs

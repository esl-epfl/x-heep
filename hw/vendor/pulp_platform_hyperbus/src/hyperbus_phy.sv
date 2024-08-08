// Copyright 2023 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Armin Berger <bergerar@ethz.ch>
// Stephan Keck <kecks@ethz.ch>
// Thomas Benz <tbenz@iis.ee.ethz.ch>
// Paul Scheffler <paulsc@iis.ee.ethz.ch>

module hyperbus_phy import hyperbus_pkg::*; #(
    parameter int unsigned IsClockODelayed = -1,
    parameter int unsigned NumChips         = 2,
    parameter int unsigned NumPhys          = -1,
    parameter int unsigned TimerWidth       = 16,
    parameter int unsigned RxFifoLogDepth   = 3,
    parameter int unsigned SyncStages       = 2,
    parameter int unsigned StartupCycles    = 300 /*us*/ * 200 /*MHz*/ // Conservative maximum frequency estimate
)(
    input  logic                clk_i,
    input  logic                clk_i_90,
    input  logic                rst_ni,
    input  logic                test_mode_i,
    // Config registers
    input  hyper_cfg_t          cfg_i,
    // Transactions
    input  logic                trans_valid_i,
    output logic                trans_ready_o,
    input  hyper_tf_t           trans_i,            // TODO: increase burst width!
    input  logic [NumChips-1:0] trans_cs_i,
    // Transmitting channel
    input  logic                tx_valid_i,
    output logic                tx_ready_o,
    input  logic [15:0]         tx_data_i,
    input  logic [1:0]          tx_strb_i,
    input  logic                tx_last_i,
    // Receiving channel
    output logic                rx_valid_o,
    input  logic                rx_ready_i,
    output logic [15:0]         rx_data_o,
    output logic                rx_error_o,
    output logic                rx_last_o,
    // B response
    output logic                b_valid_o,
    input  logic                b_ready_i,
    output logic                b_error_o,
    // Physical interface
    output logic [NumChips-1:0] hyper_cs_no,
    output logic                hyper_ck_o,
    output logic                hyper_ck_no,
    output logic                hyper_rwds_o,
    input  logic                hyper_rwds_i,
    output logic                hyper_rwds_oe_o,
    input  logic [7:0]          hyper_dq_i,
    output logic [7:0]          hyper_dq_o,
    output logic                hyper_dq_oe_o,
    output logic                hyper_reset_no
);

    logic [1:0]                  phys_in_use;

    assign phys_in_use = (NumPhys==2) ? (cfg_i.phys_in_use + 1) : 1;

    // PHY state
    hyper_phy_state_t       state_d,    state_q;
    logic [TimerWidth-1:0]  timer_d,    timer_q;
    hyper_tf_t              tf_d,       tf_q;
    logic [NumChips-1:0]    cs_d,       cs_q;

    // Whether B response is pending
    logic b_pending_q;
    logic b_pending_set;
    logic b_pending_clear;

    // How many R response words are outstanding if any
    logic [RxFifoLogDepth:0]    r_outstand_q;
    logic                       r_outstand_inc;
    logic                       r_outstand_dec;

    // Auxiliar control signals
    logic ctl_write_zero_lat;
    logic ctl_add_latency;
    logic ctl_tf_burst_last;
    logic ctl_tf_burst_done;
    logic ctl_timer_two;
    logic ctl_timer_one;
    logic ctl_timer_zero;
    logic ctl_timer_rwr_done;
    logic ctl_rclk_ena;
    logic ctl_rcnt_ena;
    logic ctl_wclk_ena;

    // Command-address
    hyper_phy_ca_t  ca;

    // Transciever IO
    logic           trx_clk_ena;
    logic           trx_cs_ena;
    logic           trx_rwds_sample;
    logic           trx_rwds_sample_ena;
    logic [15:0]    trx_tx_data;
    logic           trx_tx_data_oe;
    logic [1:0]     trx_tx_rwds;
    logic           trx_tx_rwds_oe;
    logic           trx_rx_clk_set;
    logic           trx_rx_clk_reset;
    logic [15:0]    trx_rx_data;
    logic           trx_rx_valid;
    logic           trx_rx_ready;

    // =================
    //    Transciever
    // =================

    hyperbus_trx #(
        .IsClockODelayed( IsClockODelayed   ),
        .NumChips       ( NumChips          ),
        .RxFifoLogDepth ( RxFifoLogDepth    ),
        .SyncStages     ( SyncStages        )
    ) i_trx (
        .clk_i,
        .clk_i_90,
        .rst_ni,
        .test_mode_i,
        .cs_i               ( cs_q                  ),
        .cs_ena_i           ( trx_cs_ena            ),
        .rwds_sample_o      ( trx_rwds_sample       ),
        .rwds_sample_ena_i  ( trx_rwds_sample_ena   ),
        .tx_clk_delay_i     ( cfg_i.t_tx_clk_delay  ),
        .tx_clk_ena_i       ( trx_clk_ena           ),
        .tx_data_i          ( trx_tx_data           ),
        .tx_data_oe_i       ( trx_tx_data_oe        ),
        .tx_rwds_i          ( trx_tx_rwds           ),
        .tx_rwds_oe_i       ( trx_tx_rwds_oe        ),
        .rx_clk_delay_i     ( cfg_i.t_rx_clk_delay  ),
        .rx_clk_set_i       ( trx_rx_clk_set        ),
        .rx_clk_reset_i     ( trx_rx_clk_reset      ),
        .rx_data_o          ( trx_rx_data           ),
        .rx_valid_o         ( trx_rx_valid          ),
        .rx_ready_i         ( trx_rx_ready          ),
        .hyper_cs_no,
        .hyper_ck_o,
        .hyper_ck_no,
        .hyper_rwds_o,
        .hyper_rwds_i,
        .hyper_rwds_oe_o,
        .hyper_dq_i,
        .hyper_dq_o,
        .hyper_dq_oe_o,
        .hyper_reset_no
    );

    // ==============
    //    Dataflow
    // ==============

    // Command-address
    assign ca = hyper_phy_ca_t '{
        write:      ~tf_q.write,
        addr_space: tf_q.address_space,
        burst_type: tf_q.burst_type,
        addr_upper: tf_q.address[31:3],
        reserved:   '0,
        addr_lower: tf_q.address[2:0]
    };

    // Write dataflow
    always_comb begin : proc_comb_tx
        trx_tx_data     = '0;
        trx_tx_rwds     = '0;
        tx_ready_o      = 1'b0;
        ctl_wclk_ena    = 1'b0;
        if (state_q == SendCA) begin
            // In CA phase: use timer to select word
            trx_tx_data     = ca[(8'(timer_q) << 4) +: 16];
        end else if (state_q == Write) begin
            trx_tx_data     = tx_data_i;
            trx_tx_rwds     = ~tx_strb_i;
            tx_ready_o      = 1'b1;     // Memory always ready within HyperBus burst
            ctl_wclk_ena   = tx_valid_i;
        end
    end

    // Write response dataflow
    assign b_valid_o        = b_pending_q;
    assign b_error_o        = 1'b0;            // TODO
    assign b_pending_clear  = b_valid_o & b_ready_i;

    // FF indicating whether B response pending
    always_ff @(posedge clk_i or negedge rst_ni) begin : proc_ff_b_pending
        if      (~rst_ni)           b_pending_q <= 1'b0;
        else if (b_pending_set)     b_pending_q <= 1'b1;
        else if (b_pending_clear)   b_pending_q <= 1'b0;
    end

    // Read response dataflow
    assign rx_data_o = trx_rx_data;
    assign rx_error_o = 1'b0;
    assign rx_last_o = (state_q != Read) & ctl_tf_burst_done & (r_outstand_q == 1);

    assign trx_rx_ready     = rx_ready_i;
    assign rx_valid_o       = trx_rx_valid & (r_outstand_q != '0);
    // Suspend clock one cycle for every stall caused by upstream.
    // This ensures that a sufficiently large RX FIFO will not overflow.
    assign ctl_rclk_ena     = ~(rx_valid_o & ~rx_ready_i);
    // Disable incoming RWDS clock enable once all words received
    assign trx_rx_clk_reset = b_pending_clear;

    // Counter for outstanding R responses
    assign r_outstand_dec   = rx_valid_o & rx_ready_i;
    always_ff @(posedge clk_i or negedge rst_ni) begin : proc_ff_r_outstand
        if      (~rst_ni)                           r_outstand_q <= '0;
        else if (r_outstand_inc & ~r_outstand_dec)  r_outstand_q <= r_outstand_q + 1;
        else if (r_outstand_dec & ~r_outstand_inc)  r_outstand_q <= r_outstand_q - 1;
    end

    // =============
    //    Control
    // =============

    // Auxiliary control signals
    assign ctl_write_zero_lat   = tf_q.address_space & tf_q.write;
    // cfg_i.latency_addentional overwrites the trx_rwds_sample. Be careful.
    assign ctl_add_latency      = trx_rwds_sample | cfg_i.en_latency_additional;

    assign ctl_tf_burst_last    = (tf_q.burst == 1) || (tf_q.burst == phys_in_use);
    assign ctl_tf_burst_done    = (tf_q.burst == 0);

    assign ctl_timer_rwr_done   = (timer_q <= 3);
    assign ctl_timer_two        = (timer_q == 2);
    assign ctl_timer_one        = (timer_q == 1);
    assign ctl_timer_zero       = (timer_q == 0);

    // FSM logic
    always_comb begin : proc_comb_phy_fsm
        // Default outputs
        trans_ready_o       = 1'b0;
        r_outstand_inc      = 1'b0;
        b_pending_set       = 1'b0;
        trx_cs_ena          = 1'b1;
        trx_clk_ena         = 1'b0;
        trx_rx_clk_set      = 1'b0;
        trx_rwds_sample_ena = 1'b0;
        // Default next state
        state_d = state_q;
        timer_d = timer_q - 1;
        tf_d    = tf_q;
        cs_d    = cs_q;
        // Tri-state control of dq and rwds
        trx_tx_rwds_oe = 1'b0;
        trx_tx_data_oe = 1'b0;
        // State-dependent logic
        case (state_q)
            Startup: begin
                trx_cs_ena  = 1'b0;
                // Timer resets to parameterized startup delay
                if (ctl_timer_one) begin
                    state_d = Idle;
                end
            end
            Idle: begin
                trx_cs_ena  = 1'b0;
                timer_d     = timer_q;
                // Signal ready for, pop next transfer if Write response sent
                 trans_ready_o   = 1'b1;
                if (trans_valid_i & ~b_pending_q & r_outstand_q == '0) begin
                    tf_d    = trans_i;
                    cs_d    = trans_cs_i;
                    // Send 3 CA words (t_CSS respected through clock delay)
                    timer_d = 2;
                    state_d = SendCA;
                    // Enable output driver (needs to be enabled one cycle
                    // earlier since tri-state enables of IO pads are quite
                    // slow compared to the data pins)
                    trx_tx_data_oe = 1'b1;
                end
            end
            SendCA: begin
                // Dataflow handled outside FSM
                trx_clk_ena         = 1'b1;
                trx_tx_data_oe      = 1'b1;
                trx_rwds_sample_ena = ~ctl_write_zero_lat;
                if (ctl_timer_zero) begin
                    if (ctl_write_zero_lat) begin
                        timer_d = cfg_i.t_burst_max;
                        state_d = Write;
                    end else begin
                        timer_d = TimerWidth'(cfg_i.t_latency_access) << ctl_add_latency;
                        state_d = WaitLatAccess;
                    end
                end
            end
            WaitLatAccess: begin
                trx_clk_ena = 1'b1;
                trx_tx_data_oe = 1'b1;
                // Substract cycle for last CA and another for state delay
                if (ctl_timer_two) begin
                    timer_d = cfg_i.t_burst_max;
                    // Switch to write or read phase and already start
                    // turnaround of tri-state driver (depending on latency
                    // config and if read or write transaction).
                    if (tf_q.write) begin
                        state_d = Write;
                        trx_tx_data_oe = 1'b1;
                        // For zero latency writes, we must not drive the RWDS
                        // signal (see specs page 9). Depending on the latency
                        // mode we thus drive only the DQ signals or DQ + RWDS.
                        trx_tx_rwds_oe = ~ctl_write_zero_lat;
                    end else begin
                        state_d = Read;
                        trx_tx_data_oe = 1'b0;
                        trx_tx_rwds_oe = 1'b0;
                    end
                end
            end
            Read: begin
                // Dataflow handled outside FSM
                trx_rx_clk_set = 1'b1;
                if (ctl_rclk_ena) begin
                    trx_clk_ena     = 1'b1;
                    r_outstand_inc  = 1'b1;
                    tf_d.burst      = tf_q.burst - phys_in_use;
                    tf_d.address    = tf_q.address + 1;
                    if (ctl_tf_burst_last) begin
                        timer_d = cfg_i.t_csh_cycles;
                        state_d = WaitXfer;
                    end
                end
                // Force-terminate access on burst time limit
                if (ctl_timer_one) begin
                    timer_d = cfg_i.t_csh_cycles;
                    state_d = WaitXfer;
                end
            end
            Write: begin
                // Drive DQ lines in write mode
                trx_tx_data_oe = 1'b1;
                // For zero-latency writes we must not use RWDS as mask signal
                trx_tx_rwds_oe = ~ctl_write_zero_lat;
                // Dataflow handled outside FSM
                if (ctl_wclk_ena) begin
                    trx_clk_ena = 1'b1;
                    tf_d.burst  = tf_q.burst - phys_in_use;
                    tf_d.address    = tf_q.address + 1;
                    if (ctl_tf_burst_last) begin
                        b_pending_set   = 1'b1;
                        timer_d = cfg_i.t_csh_cycles;
                        state_d         = WaitXfer;
                    end
                end
                // Force-terminate access on burst time limit
                if (ctl_timer_one) begin
                    timer_d = cfg_i.t_csh_cycles;
                    state_d = WaitXfer;
                end
            end
            WaitXfer: begin
                // Wait for FFed Clock and output to stop
                // May have to be prolonged for potential future devices with t_CSH > 0
                if (ctl_timer_zero) begin
                    timer_d = cfg_i.t_read_write_recovery;
                    state_d = WaitRWR;
                end
            end
            WaitRWR: begin
                trx_cs_ena = 1'b0;
                if (ctl_timer_rwr_done) begin
                    if (ctl_tf_burst_done) begin
                        state_d = Idle;
                    end else begin
                        state_d = SendCA;
                        // Re-enable the io driver if we immediately start the
                        // next transaction.
                        trx_tx_data_oe = 1'b1;
                    end
                end
            end
        endcase
    end

    // PHY state registers, including timer and transfer
    always_ff @(posedge clk_i or negedge rst_ni) begin : proc_ff_phy
        if (~rst_ni) begin
            state_q <= Startup;
            timer_q <= StartupCycles;
            tf_q    <= hyper_tf_t'{burst_type: 1'b1, default:'0};
            cs_q    <= '0;
        end else begin
            state_q <= state_d;
            timer_q <= timer_d;
            tf_q    <= tf_d;
            cs_q    <= cs_d;
        end
    end

endmodule

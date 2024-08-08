// Copyright 2023 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Luca Valente <luca.valente@unibo.it>

module hyperbus_phy_if import hyperbus_pkg::*; #(
    parameter int unsigned IsClockODelayed = 1,
    parameter int unsigned NumChips = 2,
    parameter int unsigned NumPhys = 2,
    parameter int unsigned TimerWidth = 16,
    parameter int unsigned RxFifoLogDepth = 3,
    parameter int unsigned StartupCycles = 60000, /*MHz*/ // Conservative maximum frequency estimate
    parameter int unsigned  SyncStages  = 2,
    parameter type hyper_tx_t = logic,
    parameter type hyper_rx_t = logic
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
    input  hyper_tx_t           tx_i,
    // Receiving channel
    output logic                rx_valid_o,
    input  logic                rx_ready_i,
    output hyper_rx_t           rx_o,
    // B response
    output logic                b_valid_o,
    input  logic                b_ready_i,
    output logic                b_error_o,

    // Physical interace: facing HyperBus
    output logic [NumPhys-1:0][NumChips-1:0] hyper_cs_no,
    output logic [NumPhys-1:0]               hyper_ck_o,
    output logic [NumPhys-1:0]               hyper_ck_no,
    output logic [NumPhys-1:0]               hyper_rwds_o,
    input  logic [NumPhys-1:0]               hyper_rwds_i,
    output logic [NumPhys-1:0]               hyper_rwds_oe_o,
    input  logic [NumPhys-1:0][7:0]          hyper_dq_i,
    output logic [NumPhys-1:0][7:0]          hyper_dq_o,
    output logic [NumPhys-1:0]               hyper_dq_oe_o,
    output logic [NumPhys-1:0]               hyper_reset_no
);

      phy_rx_t [NumPhys-1:0]       phy_fifo_rx;
      phy_rx_t [NumPhys-1:0]       fifo_axi_rx;
      logic [NumPhys-1:0]          phy_fifo_valid;
      logic [NumPhys-1:0]          phy_fifo_ready;
      logic [NumPhys-1:0]          fifo_axi_valid;
      logic                        fifo_axi_ready;

      logic                        tx_both_ready, ts_both_ready;
      logic                        rx_both_valid, b_both_valid;

      logic [NumPhys-1:0]          phy_tx_ready;
      logic                        phy_tx_valid;

      logic [NumPhys-1:0]          phy_trans_ready;
      logic                        phy_trans_valid;

      logic [NumPhys-1:0]          phy_b_valid;
      logic [NumPhys-1:0]          phy_b_error;
      logic                        phy_b_ready;

      genvar                          i;
      generate

         if (NumPhys==2) begin : phy_wrap

            assign rx_both_valid = & fifo_axi_valid;
            assign rx_valid_o = rx_both_valid;
            assign fifo_axi_ready = rx_ready_i && rx_both_valid;

            assign rx_o.error = fifo_axi_rx[0].error || fifo_axi_rx[1].error;
            assign rx_o.last = fifo_axi_rx[0].last && fifo_axi_rx[1].last;
            assign tx_both_ready = & phy_tx_ready;
            assign tx_ready_o = tx_both_ready;
            assign phy_tx_valid = tx_both_ready && tx_valid_i;

            assign b_both_valid = & phy_b_valid;
            assign b_valid_o = b_both_valid;
            assign phy_b_ready = b_ready_i && b_both_valid;
            assign b_error_o = | phy_b_error;

            assign ts_both_ready = & phy_trans_ready;
            assign trans_ready_o = ts_both_ready;
            assign phy_trans_valid = ts_both_ready && trans_valid_i;

            for ( i=0; i<NumPhys;i++) begin : phy_unroll
               assign rx_o.data[i*16 +:16] = fifo_axi_rx[i].data;

               stream_fifo #(
                   .FALL_THROUGH ( 1'b0        ),
                   .DEPTH        ( 4           ),
                   .T            ( phy_rx_t    )
               ) rx_fifo (
                   .clk_i          ( clk_i             ),
                   .rst_ni         ( rst_ni            ),
                   .flush_i        ( 1'b0              ),
                   .testmode_i     ( 1'b0              ),
                   .usage_o        (                   ),
                   .data_i         ( phy_fifo_rx[i]    ),
                   .valid_i        ( phy_fifo_valid[i] ),
                   .ready_o        ( phy_fifo_ready[i] ),
                   .data_o         ( fifo_axi_rx[i]    ),
                   .valid_o        ( fifo_axi_valid[i] ),
                   .ready_i        ( fifo_axi_ready    )
               );


               hyperbus_phy #(
                   .IsClockODelayed( IsClockODelayed   ),
                   .NumChips       ( NumChips          ),
                   .StartupCycles  ( StartupCycles     ),
                   .NumPhys        ( NumPhys           ),
                   .SyncStages     ( SyncStages        )
               ) i_phy (
                   .clk_i          ( clk_i             ),
                   .clk_i_90       ( clk_i_90          ),
                   .rst_ni         ( rst_ni            ),
                   .test_mode_i    ( test_mode_i       ),

                   .cfg_i          ( cfg_i             ),

                   .rx_data_o      ( phy_fifo_rx[i].data  ),
                   .rx_last_o      ( phy_fifo_rx[i].last  ),
                   .rx_error_o     ( phy_fifo_rx[i].error ),
                   .rx_valid_o     ( phy_fifo_valid[i]    ),
                   .rx_ready_i     ( phy_fifo_ready[i]    ),

                   .tx_data_i      ( tx_i.data[16*i +:16] ),
                   .tx_strb_i      ( tx_i.strb[2*i   +:2] ),
                   .tx_last_i      ( tx_i.last            ),
                   .tx_valid_i     ( phy_tx_valid         ),
                   .tx_ready_o     ( phy_tx_ready[i]      ),

                   .b_error_o      ( phy_b_error[i]       ),
                   .b_valid_o      ( phy_b_valid[i]       ),
                   .b_ready_i      ( phy_b_ready          ),

                   .trans_i        ( trans_i              ),
                   .trans_cs_i     ( trans_cs_i           ),
                   .trans_valid_i  ( phy_trans_valid      ),
                   .trans_ready_o  ( phy_trans_ready[i]   ),

                   .hyper_cs_no    ( hyper_cs_no[i]       ),
                   .hyper_ck_o     ( hyper_ck_o[i]        ),
                   .hyper_ck_no    ( hyper_ck_no[i]       ),
                   .hyper_rwds_o   ( hyper_rwds_o[i]      ),
                   .hyper_rwds_i   ( hyper_rwds_i[i]      ),
                   .hyper_rwds_oe_o( hyper_rwds_oe_o[i]   ),
                   .hyper_dq_i     ( hyper_dq_i[i]        ),
                   .hyper_dq_o     ( hyper_dq_o[i]        ),
                   .hyper_dq_oe_o  ( hyper_dq_oe_o[i]     ),
                   .hyper_reset_no ( hyper_reset_no[i]    )
               );

            end // for ( i=0; i<NumPhys;i++)
         end else begin // if (NumPhys==2)

            hyperbus_phy #(
                 .IsClockODelayed( IsClockODelayed   ),
                 .NumChips       ( NumChips          ),
                 .StartupCycles  ( StartupCycles     ),
                 .NumPhys        ( NumPhys           ),
                 .SyncStages     ( SyncStages        )
             ) i_phy (
                 .clk_i          ( clk_i           ),
                 .clk_i_90       ( clk_i_90        ),
                 .rst_ni         ( rst_ni          ),
                 .test_mode_i    ( test_mode_i     ),

                 .cfg_i          ( cfg_i           ),

                 .rx_data_o      ( rx_o.data       ),
                 .rx_last_o      ( rx_o.last       ),
                 .rx_error_o     ( rx_o.error      ),
                 .rx_valid_o     ( rx_valid_o      ),
                 .rx_ready_i     ( rx_ready_i      ),

                 .tx_data_i      ( tx_i.data       ),
                 .tx_strb_i      ( tx_i.strb       ),
                 .tx_last_i      ( tx_i.last       ),
                 .tx_valid_i     ( tx_valid_i      ),
                 .tx_ready_o     ( tx_ready_o      ),

                 .b_error_o      ( b_error_o       ),
                 .b_valid_o      ( b_valid_o       ),
                 .b_ready_i      ( b_ready_i       ),

                 .trans_i        ( trans_i         ),
                 .trans_cs_i     ( trans_cs_i      ),
                 .trans_valid_i  ( trans_valid_i   ),
                 .trans_ready_o  ( trans_ready_o   ),

                 .hyper_cs_no    ( hyper_cs_no     ),
                 .hyper_ck_o     ( hyper_ck_o      ),
                 .hyper_ck_no    ( hyper_ck_no     ),
                 .hyper_rwds_o   ( hyper_rwds_o    ),
                 .hyper_rwds_i   ( hyper_rwds_i    ),
                 .hyper_rwds_oe_o( hyper_rwds_oe_o ),
                 .hyper_dq_i     ( hyper_dq_i      ),
                 .hyper_dq_o     ( hyper_dq_o      ),
                 .hyper_dq_oe_o  ( hyper_dq_oe_o   ),
                 .hyper_reset_no ( hyper_reset_no  )
            );
         end // else: !if(NumPhys==2)
     endgenerate


endmodule // hyperbus_wrap_0

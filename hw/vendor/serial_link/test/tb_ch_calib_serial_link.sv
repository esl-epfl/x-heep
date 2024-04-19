// Copyright 2022 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51

// Authors:
//  - Tim Fischer <fischeti@iis.ee.ethz.ch>

module tb_ch_calib_serial_link();

  `include "axi/assign.svh"
  `include "axi/typedef.svh"

  `include "register_interface/assign.svh"
  `include "register_interface/typedef.svh"

  import serial_link_pkg::*;
  import serial_link_reg_pkg::*;
  import serial_link_single_channel_reg_pkg::*;

  // ==============
  //    Config
  // ==============
  localparam int unsigned TestDuration    = 1000;
  localparam int unsigned FaultyChannels  = 3;
  localparam int unsigned NumLanes        = serial_link_pkg::NumLanes;
  localparam int unsigned NumChannels     = serial_link_pkg::NumChannels;
  localparam int unsigned MaxClkDiv       = serial_link_pkg::MaxClkDiv;

  localparam time         TckSys1         = 50ns;
  localparam time         TckSys2         = 54ns;
  localparam time         TckReg          = 200ns;
  localparam int unsigned RstClkCyclesSys = 1;

  localparam int unsigned AxiIdWidth      = 8;
  localparam int unsigned AxiAddrWidth    = 48;
  localparam int unsigned AxiDataWidth    = 512;
  localparam int unsigned AxiStrbWidth    = AxiDataWidth / 8;
  localparam int unsigned AxiUserWidth    = 1;

  localparam int unsigned RegAddrWidth    = 32;
  localparam int unsigned RegDataWidth    = 32;
  localparam int unsigned RegStrbWidth    = RegDataWidth / 8;

  localparam logic [NumLanes*2-1:0] CalibrationPattern = {{NumLanes/4}{4'b1010, 4'b0101}};

  // ==============
  //    DDR Link
  // ==============

  // AXI types for typedefs
  typedef logic [AxiIdWidth-1:0  ]  axi_id_t;
  typedef logic [AxiAddrWidth-1:0]  axi_addr_t;
  typedef logic [AxiDataWidth-1:0]  axi_data_t;
  typedef logic [AxiStrbWidth-1:0]  axi_strb_t;
  typedef logic [AxiUserWidth-1:0]  axi_user_t;

  `AXI_TYPEDEF_ALL(axi, axi_addr_t, axi_id_t, axi_data_t, axi_strb_t, axi_user_t)

  // RegBus types for typedefs
  typedef logic [RegAddrWidth-1:0]  cfg_addr_t;
  typedef logic [RegDataWidth-1:0]  cfg_data_t;
  typedef logic [RegStrbWidth-1:0]  cfg_strb_t;

  `REG_BUS_TYPEDEF_ALL(cfg, cfg_addr_t, cfg_data_t, cfg_strb_t)

  // Model signals
  axi_req_t   axi_out_req_1, axi_out_req_2;
  axi_resp_t  axi_out_rsp_1, axi_out_rsp_2;
  axi_req_t   axi_in_req_1,  axi_in_req_2;
  axi_resp_t  axi_in_rsp_1,  axi_in_rsp_2;
  cfg_req_t   cfg_req_1;
  cfg_rsp_t   cfg_rsp_1;
  cfg_req_t   cfg_req_2;
  cfg_rsp_t   cfg_rsp_2;

  // link
  logic [NumChannels-1:0][NumLanes-1:0] ddr_1_out, ddr_2_out;
  logic [NumChannels-1:0]               ddr_1_clk_out, ddr_2_clk_out;

  // Channel mask of working channels
  logic [NumChannels-1:0] channel_mask_1, channel_mask_2;

  // clock and reset
  logic clk_1, clk_2, clk_reg;
  logic rst_1_n, rst_2_n, rst_reg_n;

  // system clock and reset
  clk_rst_gen #(
    .ClkPeriod    ( TckReg          ),
    .RstClkCycles ( RstClkCyclesSys )
  ) i_clk_rst_gen_reg (
    .clk_o  ( clk_reg   ),
    .rst_no ( rst_reg_n )
  );

  clk_rst_gen #(
    .ClkPeriod    ( TckSys1         ),
    .RstClkCycles ( RstClkCyclesSys )
  ) i_clk_rst_gen_sys_1 (
    .clk_o  ( clk_1   ),
    .rst_no ( rst_1_n )
  );

  clk_rst_gen #(
    .ClkPeriod    ( TckSys2          ),
    .RstClkCycles ( RstClkCyclesSys  )
  ) i_clk_rst_gen_sys_2 (
    .clk_o  ( clk_2   ),
    .rst_no ( rst_2_n )
  );

  // first serial instance
  serial_link_occamy_wrapper #(
    .axi_req_t        ( axi_req_t       ),
    .axi_rsp_t        ( axi_resp_t      ),
    .aw_chan_t        ( axi_aw_chan_t   ),
    .w_chan_t         ( axi_w_chan_t    ),
    .b_chan_t         ( axi_b_chan_t    ),
    .ar_chan_t        ( axi_ar_chan_t   ),
    .r_chan_t         ( axi_r_chan_t    ),
    .cfg_req_t        ( cfg_req_t       ),
    .cfg_rsp_t        ( cfg_rsp_t       ),
    .NumChannels      ( NumChannels  ),
    .NumLanes         ( NumLanes     ),
    .MaxClkDiv        ( MaxClkDiv       )
  ) i_serial_link_1 (
      .clk_i          ( clk_1           ),
      .rst_ni         ( rst_1_n         ),
      .clk_reg_i      ( clk_reg         ),
      .rst_reg_ni     ( rst_reg_n       ),
      .testmode_i     ( 1'b0            ),
      .axi_in_req_i   ( axi_in_req_1    ),
      .axi_in_rsp_o   ( axi_in_rsp_1    ),
      .axi_out_req_o  ( axi_out_req_1   ),
      .axi_out_rsp_i  ( axi_out_rsp_1   ),
      .cfg_req_i      ( cfg_req_1       ),
      .cfg_rsp_o      ( cfg_rsp_1       ),
      .ddr_rcv_clk_i  ( ddr_2_clk_out   ),
      .ddr_rcv_clk_o  ( ddr_1_clk_out   ),
      .ddr_i          ( ddr_2_out       ),
      .ddr_o          ( ddr_1_out       )
  );


  // second serial instance
  serial_link_occamy_wrapper #(
    .axi_req_t        ( axi_req_t       ),
    .axi_rsp_t        ( axi_resp_t      ),
    .aw_chan_t        ( axi_aw_chan_t   ),
    .w_chan_t         ( axi_w_chan_t    ),
    .b_chan_t         ( axi_b_chan_t    ),
    .ar_chan_t        ( axi_ar_chan_t   ),
    .r_chan_t         ( axi_r_chan_t    ),
    .cfg_req_t        ( cfg_req_t       ),
    .cfg_rsp_t        ( cfg_rsp_t       ),
    .NumChannels      ( NumChannels  ),
    .NumLanes         ( NumLanes     ),
    .MaxClkDiv        ( MaxClkDiv       )
  ) i_serial_link_2 (
      .clk_i          ( clk_2           ),
      .rst_ni         ( rst_2_n         ),
      .clk_reg_i      ( clk_reg         ),
      .rst_reg_ni     ( rst_reg_n       ),
      .testmode_i     ( 1'b0            ),
      .axi_in_req_i   ( axi_in_req_2    ),
      .axi_in_rsp_o   ( axi_in_rsp_2    ),
      .axi_out_req_o  ( axi_out_req_2   ),
      .axi_out_rsp_i  ( axi_out_rsp_2   ),
      .cfg_req_i      ( cfg_req_2       ),
      .cfg_rsp_o      ( cfg_rsp_2       ),
      .ddr_rcv_clk_i  ( ddr_1_clk_out   ),
      .ddr_rcv_clk_o  ( ddr_2_clk_out   ),
      .ddr_i          ( ddr_1_out       ),
      .ddr_o          ( ddr_2_out       )
  );

  for (genvar i = 0; i < NumChannels; i++) begin : gen_invalidate_faulty_channels
    always_comb begin : invalidate_faulty_channels
      if (channel_mask_1[i] == 1'b0) begin
        force ddr_1_out[i] = 'X;
        force ddr_1_clk_out[i] = 'X;
      end else begin
        release ddr_1_out[i];
        release ddr_1_clk_out[i];
      end
      if (channel_mask_2[i] == 1'b0) begin
        force ddr_2_out[i] = 'X;
        force ddr_2_clk_out[i] = 'X;
      end else begin
        release ddr_2_out[i];
        release ddr_2_clk_out[i];
      end
    end
  end

  REG_BUS #(
    .ADDR_WIDTH (RegAddrWidth),
    .DATA_WIDTH (RegDataWidth)
  ) cfg_1(clk_reg), cfg_2(clk_reg);

  `REG_BUS_ASSIGN_TO_REQ(cfg_req_1, cfg_1)
  `REG_BUS_ASSIGN_FROM_RSP(cfg_1, cfg_rsp_1)

  `REG_BUS_ASSIGN_TO_REQ(cfg_req_2, cfg_2)
  `REG_BUS_ASSIGN_FROM_RSP(cfg_2, cfg_rsp_2)

  typedef reg_test::reg_driver #(
    .AW ( RegAddrWidth  ),
    .DW ( RegDataWidth  ),
    .TA ( 100ps         ),
    .TT ( 500ps         )
  ) reg_master_t;

  static reg_master_t reg_master_1 = new ( cfg_1 );
  static reg_master_t reg_master_2 = new ( cfg_2 );

  AXI_BUS_DV #(
    .AXI_ADDR_WIDTH ( AxiAddrWidth  ),
    .AXI_DATA_WIDTH ( AxiDataWidth  ),
    .AXI_ID_WIDTH   ( AxiIdWidth    ),
    .AXI_USER_WIDTH ( AxiUserWidth  )
  ) axi_in_1(clk_1), axi_out_2(clk_2);

  AXI_BUS_DV #(
    .AXI_ADDR_WIDTH ( AxiAddrWidth  ),
    .AXI_DATA_WIDTH ( AxiDataWidth  ),
    .AXI_ID_WIDTH   ( AxiIdWidth    ),
    .AXI_USER_WIDTH ( AxiUserWidth  )
  ) axi_in_2(clk_2), axi_out_1(clk_1);

  `AXI_ASSIGN_TO_REQ(axi_in_req_1, axi_in_1)
  `AXI_ASSIGN_FROM_RESP(axi_in_1, axi_in_rsp_1)

  `AXI_ASSIGN_TO_REQ(axi_in_req_2, axi_in_2)
  `AXI_ASSIGN_FROM_RESP(axi_in_2, axi_in_rsp_2)

  `AXI_ASSIGN_FROM_REQ(axi_out_1, axi_out_req_1)
  `AXI_ASSIGN_TO_RESP(axi_out_rsp_1, axi_out_1)

  `AXI_ASSIGN_FROM_REQ(axi_out_2, axi_out_req_2)
  `AXI_ASSIGN_TO_RESP(axi_out_rsp_2, axi_out_2)

  // master type
  typedef axi_test::axi_rand_master #(
    .AW                   ( AxiAddrWidth  ),
    .DW                   ( AxiDataWidth  ),
    .IW                   ( AxiIdWidth    ),
    .UW                   ( AxiUserWidth  ),
    .TA                   ( 100ps         ),
    .TT                   ( 500ps         ),
    .MAX_READ_TXNS        ( 2             ),
    .MAX_WRITE_TXNS       ( 2             ),
    .AX_MIN_WAIT_CYCLES   ( 0             ),
    .AX_MAX_WAIT_CYCLES   ( 100           ),
    .W_MIN_WAIT_CYCLES    ( 0             ),
    .W_MAX_WAIT_CYCLES    ( 100           ),
    .RESP_MIN_WAIT_CYCLES ( 0             ),
    .RESP_MAX_WAIT_CYCLES ( 100           ),
    .AXI_MAX_BURST_LEN    ( 0             ),
    .TRAFFIC_SHAPING      ( 0             ),
    .AXI_EXCLS            ( 1'b1          ),
    .AXI_ATOPS            ( 1'b0          ),
    .AXI_BURST_FIXED      ( 1'b1          ),
    .AXI_BURST_INCR       ( 1'b1          ),
    .AXI_BURST_WRAP       ( 1'b0          )
  ) axi_rand_master_t;

  // slave type
  typedef axi_test::axi_rand_slave #(
    .AW                   ( AxiAddrWidth  ),
    .DW                   ( AxiDataWidth  ),
    .IW                   ( AxiIdWidth    ),
    .UW                   ( AxiUserWidth  ),
    .TA                   ( 100ps         ),
    .TT                   ( 500ps         ),
    .RAND_RESP            ( 0             ),
    .AX_MIN_WAIT_CYCLES   ( 0             ),
    .AX_MAX_WAIT_CYCLES   ( 100           ),
    .R_MIN_WAIT_CYCLES    ( 0             ),
    .R_MAX_WAIT_CYCLES    ( 100           ),
    .RESP_MIN_WAIT_CYCLES ( 0             ),
    .RESP_MAX_WAIT_CYCLES ( 100           )
  ) axi_rand_slave_t;

  static axi_rand_master_t axi_rand_master_1 = new ( axi_in_1  );
  static axi_rand_master_t axi_rand_master_2 = new ( axi_in_2  );

  static axi_rand_slave_t axi_rand_slave_1   = new ( axi_out_1 );
  static axi_rand_slave_t axi_rand_slave_2   = new ( axi_out_2 );

  logic [1:0] mst_done;

  initial begin
    axi_rand_slave_1.reset();
    wait_for_reset_1();
    axi_rand_slave_1.run();
  end

  initial begin
    axi_rand_slave_2.reset();
    wait_for_reset_2();
    axi_rand_slave_2.run();
  end

  initial begin
    mst_done[0] = 0;
    axi_rand_master_1.reset();
    wait_for_reset_1();
    axi_rand_master_1.run(TestDuration, TestDuration);
    mst_done[0] = 1;
  end

  initial begin
    mst_done[1] = 0;
    axi_rand_master_2.reset();
    wait_for_reset_2();
    axi_rand_master_2.run(TestDuration, TestDuration);
    mst_done[1] = 1;
  end

  // Default number of faulty channels
  int NumChannelFaults1 = FaultyChannels;
  int NumChannelFaults2 = FaultyChannels;

  initial begin : calib_process
    if ($value$plusargs("NUM_FAULTS_1=%d", NumChannelFaults1)) begin
      $info("[DDR1] Number of faulty channels specified as %d", NumChannelFaults1);
    end
    if ($value$plusargs("NUM_FAULTS_2=%d", NumChannelFaults2)) begin
      $info("[DDR2] Number of faulty channels specified as %d", NumChannelFaults2);
    end
    reg_master_1.reset_master();
    reg_master_2.reset_master();
    fork
      wait_for_reset_1();
      wait_for_reset_2();
    join
    $info("[SYS] Reset complete");
    fork
      bringup_link(reg_master_1, 1);
      bringup_link(reg_master_2, 2);
    join
    // Initial fault injection and channel calibration
    random_channel_faults(channel_mask_1, channel_mask_2);
    fork
      calibrate_link(reg_master_1, 1);
      calibrate_link(reg_master_2, 2);
    join
    $info("[SYS] Links are ready and calibrated");
    while (mst_done != '1) begin
      @(posedge clk_1);
    end
    stop_sim();
  end

  // ==============
  //    Checks
  // ==============

  axi_channel_compare #(
    .aw_chan_t ( axi_aw_chan_t ),
    .w_chan_t  ( axi_w_chan_t  ),
    .b_chan_t  ( axi_b_chan_t  ),
    .ar_chan_t ( axi_ar_chan_t ),
    .r_chan_t  ( axi_r_chan_t  ),
    .req_t     ( axi_req_t     ),
    .resp_t    ( axi_resp_t    )
  ) i_axi_channel_compare_1_to_2 (
    .clk_a_i   ( clk_1          ),
    .clk_b_i   ( clk_2          ),
    .axi_a_req ( axi_in_req_1   ),
    .axi_a_res ( axi_in_rsp_1   ),
    .axi_b_req ( axi_out_req_2  ),
    .axi_b_res ( axi_out_rsp_2  )
  );

  axi_channel_compare #(
    .aw_chan_t ( axi_aw_chan_t ),
    .w_chan_t  ( axi_w_chan_t  ),
    .b_chan_t  ( axi_b_chan_t  ),
    .ar_chan_t ( axi_ar_chan_t ),
    .r_chan_t  ( axi_r_chan_t  ),
    .req_t     ( axi_req_t     ),
    .resp_t    ( axi_resp_t    )
  ) i_axi_channel_compare_2_to_1 (
    .clk_a_i   ( clk_2          ),
    .clk_b_i   ( clk_1          ),
    .axi_a_req ( axi_in_req_2   ),
    .axi_a_res ( axi_in_rsp_2   ),
    .axi_b_req ( axi_out_req_1  ),
    .axi_b_res ( axi_out_rsp_1  )
  );

  // ==============
  //    Tasks
  // ==============

  task automatic wait_for_reset_1();
    @(posedge rst_1_n);
  endtask

  task automatic wait_for_reset_2();
    @(posedge rst_2_n);
  endtask

  task automatic stop_sim();
    repeat(50) begin
      @(posedge clk_1);
    end
    $display("[SYS] Simulation Stopped (%d ns)", $time);
    $stop();
  endtask

  task automatic random_channel_faults(
    output logic [NumChannels-1:0] channel_mask_1,
    logic [NumChannels-1:0] channel_mask_2
  );
    automatic int rand_success;
    rand_success = std::randomize(channel_mask_1) with {
      $countones(~channel_mask_1) == NumChannelFaults1;
    };
    assert(rand_success) else $error("Randomization failed.");
    rand_success = std::randomize(channel_mask_2) with {
      $countones(~channel_mask_2) == NumChannelFaults2;
    };
    assert(rand_success) else $error("Randomization failed.");
    $info("Chip 1 to chip 2 %d faulty channels, enabled channels: %32b",
      NumChannelFaults1, channel_mask_1);
    $info("Chip 2 to chip 1 %d faulty channels, enabled channels: %32b",
      NumChannelFaults2, channel_mask_2);
  endtask

  task automatic cfg_write(reg_master_t drv, cfg_addr_t addr, cfg_data_t data, cfg_strb_t strb='1);
    automatic logic resp;
    drv.send_write(addr, data, strb, resp);
    assert (!resp) else $error("Not able to write cfg reg");
  endtask

  task automatic cfg_read(reg_master_t drv, cfg_addr_t addr, output cfg_data_t data);
    automatic logic resp;
    drv.send_read(addr, data, resp);
    assert (!resp) else $error("Not able to write cfg reg");
  endtask

  task automatic bringup_link(reg_master_t drv, int id);
    $info("[DDR%0d]: Enabling clock and deassert link reset.", id);
    // Reset and clock gate sequence, AXI isolation remains enabled
    // De-assert reset
    cfg_write(drv, SERIAL_LINK_CTRL_OFFSET, 32'h300);
    // Assert reset
    cfg_write(drv, SERIAL_LINK_CTRL_OFFSET, 32'h302);
    // Enable clock
    cfg_write(drv, SERIAL_LINK_CTRL_OFFSET, 32'h303);
  endtask

  task automatic calibrate_link(reg_master_t drv, int id);
    automatic phy_data_t pattern, pattern_q[$];
    automatic cfg_data_t data;
    automatic logic [NumChannels-1:0] working_tx_channels;
    automatic logic [NumChannels-1:0] working_rx_channels;
    automatic logic [2**$clog2(NumChannels)-1:0] raw_mode_data_in_valid;
    working_rx_channels = '1;
    working_tx_channels = '1;
    // Don't do calibration for single channels
    if (NumChannels == 1) begin
      $warning("[DDR%0d]: Single channel configurations are not calibrated", id);
      return;
    end
    $info("[DDR%0d]: Starting link calibration.", id);
    // Isolate AXI ports before calibrating. The slave in port needs to be
    // isolated before the master out port on the other side. Otherwise there might
    // transactions in flight that get lost
    $info("[DDR%0d]: Isolating AXI Slave In", id);
    cfg_write(drv, SERIAL_LINK_CTRL_OFFSET, 32'h103);
    do begin
      cfg_read(drv, SERIAL_LINK_ISOLATED_OFFSET, data);
    end while(data[0] != 1'b1); // Wait until isolation status bit is 1
    $info("[DDR%0d]: Isolated AXI Slave In", id);
    // Wait for a few clock cycles before isolating AXI Master out
    repeat(100) @(posedge clk_reg);
    $info("[DDR%0d]: Isolating AXI Master Out", id);
    cfg_write(drv, SERIAL_LINK_CTRL_OFFSET, 32'h303);
    do begin
      cfg_read(drv, SERIAL_LINK_ISOLATED_OFFSET, data);
    end while(data[1:0] != 2'b11); // Wait until both isolation status bit is are 1
    $info("[DDR%0d]: Isolated AXI Master Out", id);
    // Configure Raw Mode
    $info("[DDR%0d]: Preparing link for calibration...",id);
    // Prepare channel allocator for RAW mode
    // Enable bypass mode and auto flush feature but disable sync for RX side
    cfg_write(drv, SERIAL_LINK_CHANNEL_ALLOC_TX_CFG_OFFSET, 32'h3);
    cfg_write(drv, SERIAL_LINK_CHANNEL_ALLOC_RX_CFG_OFFSET, 32'h3);
    // Enable Raw Mode
    cfg_write(drv, SERIAL_LINK_RAW_MODE_EN_OFFSET, 1);
    // Set mask for sending out pattern
    cfg_write(drv, SERIAL_LINK_RAW_MODE_OUT_CH_MASK_0_OFFSET, '1);
    cfg_write(drv, SERIAL_LINK_RAW_MODE_OUT_CH_MASK_1_OFFSET, '1);
    // Enable same channels in TX side of channel allocator
    cfg_write(drv, SERIAL_LINK_CHANNEL_ALLOC_TX_CH_EN_0_OFFSET, '1);
    cfg_write(drv, SERIAL_LINK_CHANNEL_ALLOC_RX_CH_EN_0_OFFSET, '1);
    cfg_write(drv, SERIAL_LINK_CHANNEL_ALLOC_TX_CH_EN_1_OFFSET, '1);
    cfg_write(drv, SERIAL_LINK_CHANNEL_ALLOC_RX_CH_EN_1_OFFSET, '1);
    $info("[DDR%0d]: Sending calibration sequence", id);
    // Clear the TX Fifo
    cfg_write(drv, SERIAL_LINK_RAW_MODE_OUT_DATA_FIFO_CTRL_OFFSET, 32'h1);
    cfg_write(drv, SERIAL_LINK_RAW_MODE_OUT_DATA_FIFO_CTRL_OFFSET, 32'h0);
    // Send a pattern sequence to TX FIFO
    for (int i = 0; i < 8; i++) begin
      pattern = 16'haaaa << i;
      pattern_q.push_back(pattern);
      cfg_write(drv, SERIAL_LINK_RAW_MODE_OUT_DATA_FIFO_OFFSET, pattern);
    end
    // Send out pattern
    cfg_write(drv, SERIAL_LINK_RAW_MODE_OUT_EN_OFFSET, 1);
    // Wait until some channels have received data
    do begin
      cfg_read(drv, SERIAL_LINK_RAW_MODE_IN_DATA_VALID_0_OFFSET, raw_mode_data_in_valid[31:0]);
      cfg_read(drv, SERIAL_LINK_RAW_MODE_IN_DATA_VALID_1_OFFSET, raw_mode_data_in_valid[63:32]);
    end while(raw_mode_data_in_valid[NumChannels-1:0] == 0);
    // Iterate through every channel
    for (int c = 0; c < NumChannels; c++) begin
      // Select read channel
      cfg_write(drv, SERIAL_LINK_RAW_MODE_IN_CH_SEL_OFFSET, c);
      // Check read patterns
      foreach(pattern_q[i]) begin
        // Check first that there is valid data in the RX FIFO
      cfg_read(drv, SERIAL_LINK_RAW_MODE_IN_DATA_VALID_0_OFFSET, raw_mode_data_in_valid[31:0]);
      cfg_read(drv, SERIAL_LINK_RAW_MODE_IN_DATA_VALID_1_OFFSET, raw_mode_data_in_valid[63:32]);
      if (raw_mode_data_in_valid[c] == 1'b0) begin
          $info("[DDR%0d][CH%0d] No data in RX FIFO", id, c);
          working_rx_channels[c] = 1'b0;
          break;
        end
        // Read out first pattern
        cfg_read(drv, SERIAL_LINK_RAW_MODE_IN_DATA_OFFSET, data);
        if (pattern_q[i] != data) begin
          $error("[DDR%0d][CH%0d] Pattern missmatch actual %h data expected %h",
            id, c, data, pattern_q[i]);
          working_rx_channels[c] = 1'b0;
          break;
        end
      end
      if (working_rx_channels[c]) begin
        $info("[DDR%0d] Calibration successful for channel %0d.", id, c);
      end else begin
        $info("[DDR%0d] Calibration failed for channel %0d.", id, c);
      end
    end
    $info("[DDR%0d] RX channel mask %32b", id, working_rx_channels);
    // Check that there is no more valid data in the RX FIFOs
    // of all working channels
    cfg_read(drv, SERIAL_LINK_RAW_MODE_IN_DATA_VALID_0_OFFSET, raw_mode_data_in_valid[31:0]);
    cfg_read(drv, SERIAL_LINK_RAW_MODE_IN_DATA_VALID_1_OFFSET, raw_mode_data_in_valid[63:32]);
    assert ((raw_mode_data_in_valid[NumChannels-1:0] & working_rx_channels) == '0) else begin
      $error("[DDR%0d] Still data in RX FIFO %32b", id, data & working_rx_channels);
    end
    // Clear the TX Fifo
    cfg_write(drv, SERIAL_LINK_RAW_MODE_OUT_DATA_FIFO_CTRL_OFFSET, 32'h1);
    cfg_write(drv, SERIAL_LINK_RAW_MODE_OUT_DATA_FIFO_CTRL_OFFSET, 32'h0);
    // Load the channel mask of working channels into TX FIFO
    // They should be immediately sent as TX FIFO is still enabled
    $info("[DDR%0d] Sending out RX channel mask.", id);
    cfg_write(drv, SERIAL_LINK_RAW_MODE_OUT_DATA_FIFO_OFFSET, working_rx_channels[15:0]);
    cfg_write(drv, SERIAL_LINK_RAW_MODE_OUT_DATA_FIFO_OFFSET, working_rx_channels[31:16]);
    // Wait until the channel mask from the other side has arrived
    do begin
      cfg_read(drv, SERIAL_LINK_RAW_MODE_IN_DATA_VALID_0_OFFSET, raw_mode_data_in_valid[31:0]);
      cfg_read(drv, SERIAL_LINK_RAW_MODE_IN_DATA_VALID_1_OFFSET, raw_mode_data_in_valid[63:32]);
    end while(raw_mode_data_in_valid[NumChannels-1:0] == 0);
    // Only check RX channels that are working
    for (int c = 0; c < NumChannels; c++) begin
      if (working_rx_channels[c]) begin
        // Select channel to read from
        cfg_write(drv, SERIAL_LINK_RAW_MODE_IN_CH_SEL_OFFSET, c);
        // Read the mask
        cfg_read(drv, SERIAL_LINK_RAW_MODE_IN_DATA_OFFSET, working_tx_channels[15:0]);
        cfg_read(drv, SERIAL_LINK_RAW_MODE_IN_DATA_OFFSET, working_tx_channels[31:16]);
      end
    end
    $info("[DDR%0d] TX channel mask %32b", id, working_tx_channels);
    // Disable TX Fifo
    cfg_write(drv, SERIAL_LINK_RAW_MODE_OUT_EN_OFFSET, 0);
    // Enable RX/TX channels
    cfg_write(drv, SERIAL_LINK_CHANNEL_ALLOC_TX_CH_EN_0_OFFSET, working_tx_channels);
    cfg_write(drv, SERIAL_LINK_CHANNEL_ALLOC_RX_CH_EN_0_OFFSET, working_rx_channels);
    cfg_write(drv, SERIAL_LINK_CHANNEL_ALLOC_TX_CH_EN_1_OFFSET, working_tx_channels);
    cfg_write(drv, SERIAL_LINK_CHANNEL_ALLOC_RX_CH_EN_1_OFFSET, working_rx_channels);
    // Configure channel allocator
    // Set auto-flush count value == 2
    if ($countones(working_tx_channels) == NumChannels) begin
      // Enable bypass
      cfg_write(drv, SERIAL_LINK_CHANNEL_ALLOC_TX_CFG_OFFSET, 32'h203);
    end else begin
      // Disable bypass
      cfg_write(drv, SERIAL_LINK_CHANNEL_ALLOC_TX_CFG_OFFSET, 32'h202);
    end
    // Set auto-flush count value == 2 and re-enable RX synchronization
    if ($countones(working_rx_channels) == NumChannels) begin
      cfg_write(drv, SERIAL_LINK_CHANNEL_ALLOC_RX_CFG_OFFSET, 32'h10203);
    end else begin
      cfg_write(drv, SERIAL_LINK_CHANNEL_ALLOC_RX_CFG_OFFSET, 32'h10202);
    end
    // Configure normal operating mode
    cfg_write(drv, SERIAL_LINK_RAW_MODE_EN_OFFSET, 0);
    // Wait for the other Serial Link to be ready
    repeat(100) @(posedge clk_reg);
    $info("[DDR%0d] Enabling AXI ports...",id);
    cfg_write(drv, SERIAL_LINK_CTRL_OFFSET, 32'h03);
    do begin
      cfg_read(drv, SERIAL_LINK_ISOLATED_OFFSET, data);
    end while(data != 0); // Wait until both isolation status bits are 0 to
                          // indicate disabling of isolation
    $info("[DDR%0d] Link is ready", id);
  endtask;

endmodule : tb_ch_calib_serial_link

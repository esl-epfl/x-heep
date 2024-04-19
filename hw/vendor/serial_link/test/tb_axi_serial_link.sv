// Copyright 2022 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51

// Authors:
//  - Tim Fischer <fischeti@iis.ee.ethz.ch>

module tb_axi_serial_link();

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
  localparam int unsigned TestDuration    = 100;
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
  logic [NumChannels-1:0]  ddr_rcv_clk_1, ddr_rcv_clk_2;
  axi_req_t   axi_out_req_1, axi_out_req_2;
  axi_resp_t  axi_out_rsp_1, axi_out_rsp_2;
  axi_req_t   axi_in_req_1,  axi_in_req_2;
  axi_resp_t  axi_in_rsp_1,  axi_in_rsp_2;
  cfg_req_t   cfg_req_1;
  cfg_rsp_t   cfg_rsp_1;
  cfg_req_t   cfg_req_2;
  cfg_rsp_t   cfg_rsp_2;

  // link
  wire [NumChannels*NumLanes-1:0] ddr_o;
  wire [NumChannels*NumLanes-1:0] ddr_i;

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
    .NumChannels      ( NumChannels     ),
    .NumLanes         ( NumLanes        ),
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
      .ddr_rcv_clk_i  ( ddr_rcv_clk_2   ),
      .ddr_rcv_clk_o  ( ddr_rcv_clk_1   ),
      .ddr_i          ( ddr_i           ),
      .ddr_o          ( ddr_o           )
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
    .NumChannels      ( NumChannels     ),
    .NumLanes         ( NumLanes        ),
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
      .ddr_rcv_clk_i  ( ddr_rcv_clk_1   ),
      .ddr_rcv_clk_o  ( ddr_rcv_clk_2   ),
      .ddr_i          ( ddr_o           ),
      .ddr_o          ( ddr_i           )
  );

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

  // By default perform Testduration Reads & Writes
  int NumWrites_1 = TestDuration;
  int NumReads_1 = TestDuration;
  int NumWrites_2 = TestDuration;
  int NumReads_2 = TestDuration;

  initial begin
  end

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
    automatic time start_cycle, end_cycle;
    automatic int unsigned data_sent = 0;
    automatic int unsigned data_received = 0;
    if ($value$plusargs("NUM_WRITES_1=%d", NumWrites_1)) begin
      $info("[DDR1] Number of writes specified as %d", NumWrites_1);
    end
    if ($value$plusargs("NUM_READS_1=%d", NumReads_1)) begin
      $info("[DDR1] Number of reads specified as %d", NumReads_1);
    end
    mst_done[0] = 0;
    axi_rand_master_1.reset();
    wait_for_reset_1();
    start_cycle = $realtime;
    fork
      axi_rand_master_1.run(NumWrites_1, NumReads_1);
      forever begin
        @(posedge clk_1);
        if (axi_in_rsp_1.r_valid & axi_in_req_1.r_ready) data_received += $bits(axi_in_rsp_1.r);
        if (axi_in_rsp_1.b_valid & axi_in_req_1.b_ready) data_received += $bits(axi_in_rsp_1.b);
        if (axi_in_req_1.ar_valid & axi_in_rsp_1.ar_ready) data_sent += $bits(axi_in_req_1.ar);
        if (axi_in_req_1.aw_valid & axi_in_rsp_1.aw_ready) data_sent += $bits(axi_in_req_1.aw);
        if (axi_in_req_1.w_valid & axi_in_rsp_1.w_ready) data_sent += $bits(axi_in_req_1.w);
      end
    join_any
    end_cycle = $realtime;
    $info("BW %0d/%0d (sent/rcv) Mbit/s @ %0d/%0d MHz (SoC/PHY)",
      data_sent * 1000 / (end_cycle - start_cycle),
      data_received * 1000 / (end_cycle - start_cycle),
      1000 / TckSys1,
      1000 / TckSys1 / 8);
    mst_done[0] = 1;
  end

  initial begin
    if ($value$plusargs("NUM_WRITES_2=%d", NumWrites_2)) begin
      $info("[DDR2] Number of writes specified as %d", NumWrites_2);
    end
    if ($value$plusargs("NUM_READS_2=%d", NumReads_2)) begin
      $info("[DDR2] Number of reads specified as %d", NumReads_2);
    end
    mst_done[1] = 0;
    axi_rand_master_2.reset();
    wait_for_reset_2();
    axi_rand_master_2.run(NumWrites_2, NumReads_2);
    mst_done[1] = 1;
  end

  initial begin : stimuli_process
    reg_master_1.reset_master();
    reg_master_2.reset_master();
    fork
      wait_for_reset_1();
      wait_for_reset_2();
    join
    $info("[SYS] Reset complete");
    fork
      start_link(reg_master_1, 1);
      start_link(reg_master_2, 2);
    join
    $info("[SYS] Links are ready");
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

  task automatic start_link(reg_master_t drv, int id);
    automatic phy_data_t pattern, pattern_q[$];
    automatic cfg_data_t data;
    $info("[DDR%0d]: Enabling clock and deassert link reset.", id);
    // Reset and clock gate sequence, AXI isolation remains enabled
    // De-assert reset
    cfg_write(drv, SERIAL_LINK_CTRL_OFFSET, 32'h300);
    // Assert reset
    cfg_write(drv, SERIAL_LINK_CTRL_OFFSET, 32'h302);
    // Enable clock
    cfg_write(drv, SERIAL_LINK_CTRL_OFFSET, 32'h303);
    // Enable channel allocator bypass mode and
    // auto flush feature but disable sync for RX side
    cfg_write(drv, SERIAL_LINK_CHANNEL_ALLOC_TX_CFG_OFFSET, 32'h3);
    cfg_write(drv, SERIAL_LINK_CHANNEL_ALLOC_RX_CFG_OFFSET, 32'h3);
    // Wait for some clock cycles
    repeat(50) drv.cycle_end();
    // De-isolate AXI ports
    $info("[DDR%0d] Enabling AXI ports...",id);
    cfg_write(drv, SERIAL_LINK_CTRL_OFFSET, 32'h03);
    do begin
      cfg_read(drv, SERIAL_LINK_ISOLATED_OFFSET, data);
    end while(data != 0); // Wait until both isolation status bits are 0 to
                          // indicate disabling of isolation
    $info("[DDR%0d] Link is ready", id);
  endtask;

endmodule : tb_axi_serial_link

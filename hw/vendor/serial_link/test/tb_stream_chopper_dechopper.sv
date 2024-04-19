// Copyright 2022 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51

// Author: Manuel Eggimann <meggimann@iis.ee.ethz.ch>
module tb_stream_chopper_dechopper;

  localparam time         Tck = 200ns;
  localparam int unsigned ChannelCount = 32;
  localparam int unsigned Log2ChannelCount = $clog2(ChannelCount);
  localparam int unsigned RstClkCycles = 1;
  localparam int unsigned MaxWaitCycles = 0;
  localparam int unsigned TestLength = 100;
  localparam int unsigned NumWords = 1000;
  localparam int unsigned AutoFlushWaitCount = 16;

  typedef logic [15:0]  element_t;
  typedef element_t[ChannelCount-1:0]  payload_t;

  typedef stream_test::stream_driver #(
    .payload_t (payload_t),
    .TA (Tck*0.2),
    .TT (Tck*0.8)
  ) stream_driver_t;

  logic                   clk, rstn;
  logic                   sim_done = 0;

  logic [Log2ChannelCount-1:0] cfg_chopsize;
  logic                        cfg_auto_flush_en;
  logic [5:0]                  cfg_auto_flush_count;
  logic                        flush;
  logic                        bypass_en;
  logic [ChannelCount-1:0]     chopper2dechoper_valid;

  STREAM_DV #(.payload_t(payload_t)) source_bus(clk);
  STREAM_DV #(.payload_t(payload_t)) chopper2dechopper_bus(clk);
  STREAM_DV #(.payload_t(payload_t)) sink_bus(clk);

  stream_driver_t source_driver = new(source_bus);
  stream_driver_t sink_driver  = new(sink_bus);

  payload_t send_queue[$];

  int                          sent_count = 0;
  int                          rcv_count = 0;

  initial begin : proc_source
    automatic payload_t word;
    automatic int wait_cycl;
    @(posedge rstn);
    source_driver.reset_in();

    // Enable auto-flush feature with 16 wait cycles
    cfg_auto_flush_en    = 1'b1;
    cfg_auto_flush_count = 16;
    flush = 1'b0;

    // Test bypassing
    $info("Testing bypass mode...");
    bypass_en    = 1'b1;
    cfg_chopsize = ChannelCount;
    for (int i = 0; i < NumWords; i++) begin
      assert(std::randomize(word)) else
        $error("Randomization failed");
      // Push the word element by element into the queue
      send_queue.push_back(word);
      sent_count++;
      source_driver.send(word);
      // Randomly delay sending of next item
      wait_cycl = $urandom_range(0, MaxWaitCycles);
      repeat(wait_cycl) @(posedge clk);
    end
    // Wait a couple of cycles for the final elements to sink
    repeat(MaxWaitCycles+AutoFlushWaitCount+3) @(posedge clk);
    // Verify we received every elementp
    assert(rcv_count == sent_count) else
      $error("Did not receive the correct number of elements. Expected %0d but was %0d.",
        sent_count,rcv_count);
    rcv_count = 0;
    sent_count = 0;
    $info("Bypass test finished");

    // Test with different output chop sizes
    bypass_en = 1'b0;
    for (int j = 0; j < TestLength; j++) begin
      cfg_chopsize = $urandom_range(1, ChannelCount-1);
      $info("Testing with chopsize %0d...", cfg_chopsize);
      for (int i = 0; i < NumWords; i++) begin
        assert(std::randomize(word)) else
          $error("Randomization failed");
        // Push the word element by element into the queue
        send_queue.push_back(word);
        sent_count++;
        // Apply word to chopper
        source_driver.send(word);
        // Randomly delay sending of next item
        wait_cycl = $urandom_range(0, MaxWaitCycles);
        repeat(wait_cycl) @(posedge clk);
      end
      // Wait a couple of cycles for the final elements to sink
      repeat(MaxWaitCycles+AutoFlushWaitCount+4) @(posedge clk);
      // Verify we received every element
      assert(rcv_count == sent_count) else
        $error("Did not receive the correct number of elements. Expected %0d but was %0d.",
          sent_count,rcv_count);
      rcv_count  = 0;
      sent_count = 0;
      $info("Test done");
    end

    sim_done = 1'b1;
  end

  initial begin : proc_sink
    automatic int unsigned wait_cycl;
    automatic payload_t rcv_word;
    automatic payload_t expected_word;
    @(posedge rstn
);
    sink_driver.reset_out();

    forever begin
      wait_cycl = $urandom_range(0, MaxWaitCycles);
      repeat(wait_cycl) @(posedge clk);
      sink_driver.recv(rcv_word);
      expected_word = send_queue.pop_front();
      assert(expected_word == rcv_word) else
        $error("Received %0h but expected %0h.", rcv_word, expected_word);
      rcv_count++;
    end
  end


  initial begin : proc_stop_sim
    @(posedge rstn);
    wait (&sim_done);
    repeat (20) @(posedge clk);
    $display("Sim done.");
    $stop();
  end




  // system clock and reset
  clk_rst_gen #(
    .ClkPeriod    ( Tck          ),
    .RstClkCycles ( RstClkCycles )
  ) i_clk_rst_gen_reg (
    .clk_o  ( clk   ),
    .rst_no ( rstn )
  );

  stream_chopper #(
    .element_t ( element_t    ),
    .Width     ( ChannelCount )
  ) i_chopper (
    .clk_i                  ( clk                         ),
    .rst_ni                 ( rstn                        ),
    .bypass_en_i            ( bypass_en                   ),
    .flush_i                ( flush                       ),
    .cfg_auto_flush_en_i    ( cfg_auto_flush_en           ),
    .cfg_auto_flush_count_i ( cfg_auto_flush_count        ),
    .cfg_chopsize_i         ( cfg_chopsize                ),
    .data_i                 ( source_bus.data             ),
    .valid_i                ( source_bus.valid            ),
    .ready_o                ( source_bus.ready            ),
    .data_o                 ( chopper2dechopper_bus.data  ),
    .valid_o                ( chopper2dechoper_valid      ),
    .ready_i                ( chopper2dechopper_bus.ready )
  );
  assign chopper2dechopper_bus.valid = |chopper2dechoper_valid;

  stream_dechopper #(
    .element_t ( element_t    ),
    .Width     ( ChannelCount )
  ) i_dechopper(
    .clk_i       ( clk                         ),
    .rst_ni      ( rstn                        ),
    .bypass_en_i ( bypass_en                   ),
    .valid_i     ( chopper2dechoper_valid      ),
    .ready_o     ( chopper2dechopper_bus.ready ),
    .data_i      ( chopper2dechopper_bus.data  ),
    .valid_o     ( sink_bus.valid              ),
    .ready_i     ( sink_bus.ready              ),
    .data_o      ( sink_bus.data               )
  );


endmodule

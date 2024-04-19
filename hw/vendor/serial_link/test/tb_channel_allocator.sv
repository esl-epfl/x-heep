// Copyright 2022 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51

// Author: Manuel Eggimann <meggimann@iis.ee.ethz.ch>

/// Testbench to verify the channel allocator IP.
module tb_channel_allocator;
  localparam time         Tck = 200ns;
  localparam int unsigned ChannelCount = 32;
  localparam int unsigned Log2ChannelCount = $clog2(ChannelCount);
  localparam int unsigned RstClkCycles = 1;
  localparam bit          EnableRandomStalls = 0;
  localparam int unsigned MaxWaitCycles = 0;
  localparam int unsigned TestLength = 5;
  localparam int unsigned NumWords = 256; // Make sure the number of words is
                                // high enough to hide initial latency during
                                // bandwidth verification tests
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

  logic                   cfg_rx_clear;
  logic                   cfg_tx_clear;

  logic [ChannelCount-1:0] cfg_tx_channel_en;
  logic                    cfg_tx_bypass_en;
  logic                    cfg_tx_auto_flush_en;
  logic                    cfg_tx_flush_trigger;
  logic [7:0]              cfg_tx_auto_flush_count;
  logic [ChannelCount-1:0] cfg_rx_channel_en;
  logic                    cfg_rx_auto_flush_en;
  logic [7:0]              cfg_rx_auto_flush_count;
  logic                    cfg_rx_bypass_en;
  logic                    cfg_rx_sync_en;

  STREAM_DV #(.payload_t(payload_t)) source_bus(clk);
  STREAM_DV #(.payload_t(payload_t)) sink_bus(clk);

  stream_driver_t source_driver = new(source_bus);
  stream_driver_t sink_driver  = new(sink_bus);

  payload_t send_queue[$];

  int                          sent_count  = 0;
  int                          rcv_count   = 0;
  time                         start_time;
  time                         stop_time;
  real                         avg_cycles_per_word;
  real                         expected_cycl_per_word;
  int unsigned                 num_enabled_channels;

  assign num_enabled_channels = $countones(cfg_tx_channel_en);

  int                          error_count = 0;

  function automatic int unsigned get_despread_cycles(logic[ChannelCount-1:0] en_mask);
    int                        i = ChannelCount-1;
    int                        required_cycles = 1;
    while (en_mask[i] == 1'b0 && i >=0) begin
      i--;
    end
    while(i > 0) begin
      if (en_mask[i] == 1'b0) begin
        required_cycles++;
      end
      i--;
    end
    get_despread_cycles = required_cycles;
  endfunction

  initial begin : proc_source
    automatic payload_t word;
    automatic int wait_cycl;
    @(posedge rstn);
    source_driver.reset_in();

    cfg_tx_clear            = 1'b0;
    cfg_rx_clear            = 1'b0;
    // Enable auto-flush feature with 16 wait cycles
    cfg_tx_auto_flush_en    = 1'b1;
    cfg_tx_auto_flush_count = 16;
    cfg_tx_flush_trigger    = 1'b0;
    cfg_rx_auto_flush_en    = 1'b1;
    cfg_rx_sync_en          = 1'b1;
    cfg_rx_auto_flush_count = cfg_tx_auto_flush_count + 30;

    // Test bypassing
    $info("Testing bypass mode...");
    cfg_tx_bypass_en  = 1'b1;
    cfg_tx_channel_en = '1;
    cfg_rx_bypass_en  = 1'b1;
    cfg_rx_channel_en = '1;
    start_time = $realtime();
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
    repeat(MaxWaitCycles+AutoFlushWaitCount+200) @(posedge clk);
    // Verify we received every elementp
    assert(rcv_count == sent_count) else begin
      $error("Did not receive the correct number of elements. Expected %0d but was %0d.",
         sent_count,rcv_count);
      error_count++;
    end
    avg_cycles_per_word = (real'(stop_time-start_time))/Tck/NumWords;
    if (!EnableRandomStalls && MaxWaitCycles == 0) begin
      // If we did not simulate with starvation, backpressure and random delays,
      // verify the achieved throughput
        assert(avg_cycles_per_word < 1.2) else begin
          error_count++;
          $error("Achieved throughput %0f is to low.", avg_cycles_per_word,
                  "In bypass mode it should approach 1 cycl/word.");
        end
    end
    rcv_count = 0;
    sent_count = 0;
    $info("Bypass test finished. Achieved throughput: %0f", avg_cycles_per_word);

    // Test with randomly enabled/disabled channels
    cfg_tx_bypass_en = 1'b0;
    cfg_rx_bypass_en = 1'b0;
    for (int num_defects = 1; num_defects < ChannelCount; num_defects++) begin
      $info("Running tests with %0d defects...", num_defects);
      for (int j = 0; j < TestLength; j++) begin
        automatic int success = std::randomize(cfg_tx_channel_en) with {
          $countones(~cfg_tx_channel_en) == num_defects;
        };
        assert(success) else
          $error("Randomization failed.");
        cfg_rx_channel_en    = cfg_tx_channel_en;
        $info("Testing with channel mask %32b...", cfg_tx_channel_en);
        start_time = $realtime();
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
        repeat(MaxWaitCycles+AutoFlushWaitCount + 200) @(posedge clk);
        // Verify we received every element
        assert(rcv_count == sent_count) else begin
          $error("Did not receive the correct number of elements. Expected %0d but was %0d.",
            sent_count,rcv_count);
          error_count++;
        end
        avg_cycles_per_word = (real'(stop_time-start_time))/Tck/NumWords;
        if (!EnableRandomStalls && MaxWaitCycles == 0) begin
          // If we did not simulate with starvation, backpressure and random delays,
          // verify the achieved throughput
          expected_cycl_per_word = (1.0 + real'($countones(~cfg_tx_channel_en)))*
            real'(ChannelCount)/real'($countones(cfg_tx_channel_en));
          // Assert that #cycles per complete word is within +10% of expected
          // steady state value (the longer the tests, the closer the measured
          // value should approach it).
          assert(avg_cycles_per_word <= expected_cycl_per_word*1.1) else begin
            error_count++;
            $error("Achieved throughput %0f is to low.", avg_cycles_per_word,
                    "With %0d/%0d enabled channels we would expect %0f cycl/word.",
                    $countones(cfg_tx_channel_en), ChannelCount, expected_cycl_per_word);
          end
        end
        rcv_count  = 0;
        sent_count = 0;
        $info("Test done. Achieved throughput over %0d/%0d channels: %0f cycles per partial word",
          $countones(cfg_tx_channel_en), ChannelCount,
          avg_cycles_per_word*num_enabled_channels/ChannelCount);
      end
    end

    $info("Test synchronous clear.");
    $info("Disabling auto flush and pushing partial packet inside");
    for (int i = 0; i < TestLength; i++) begin
      do begin
        assert(std::randomize(cfg_tx_channel_en)) else
          $error("Randomization failed.");
        end while (cfg_tx_channel_en == '0 || cfg_tx_channel_en == '1 ||
                  (ChannelCount % $countones(cfg_tx_channel_en)) == 0);
      cfg_rx_channel_en              = cfg_tx_channel_en;
      cfg_tx_auto_flush_en           = 1'b0;
      assert(std::randomize(word)) else
        $error("Randomization failed");
      // Push the word into the allocator. Since we did not enable autoflush and
      // have at least one channel disabled, the word should never arrive at the
      // RX output.
      source_driver.send(word);
      // Wait a couple of cycles to check if RX really doesn't receive garbage
      // when it should be blocked
      repeat(MaxWaitCycles+AutoFlushWaitCount + 200) @(posedge clk);
      $info("Clearing channel allocator...");
      cfg_tx_clear = 1'b1;
      cfg_rx_clear = 1'b1;
      @(posedge clk);
      cfg_tx_clear = 1'b0;
      cfg_rx_clear = 1'b0;
      $info("Try sending some data...");
      cfg_tx_auto_flush_en = 1'b1;
      assert(std::randomize(word)) else
        $error("Randomization failed");
      send_queue.push_back(word);
      sent_count++;
      source_driver.send(word);
      // Make sure we receive on the other side since we now re-enabled the auto
      // flush feature
      repeat(MaxWaitCycles+AutoFlushWaitCount + 200) @(posedge clk);
      assert(rcv_count == sent_count) else begin
        $error("Did not receive the correct number of elements. Expected %0d but was %0d.",
          sent_count,rcv_count);
        error_count++;
      end
      rcv_count  = 0;
      sent_count = 0;
    end


    sim_done = 1'b1;
  end

  initial begin : proc_sink
    automatic int unsigned wait_cycl;
    automatic payload_t rcv_word;
    automatic payload_t expected_word;
    @(posedge rstn);
    sink_driver.reset_out();

    forever begin
      wait_cycl = $urandom_range(0, MaxWaitCycles);
      repeat(wait_cycl) @(posedge clk);
      sink_driver.recv(rcv_word);
      expected_word = send_queue.pop_front();
      assert(expected_word == rcv_word) else begin
        $error("Received %0h but expected %0h.", rcv_word, expected_word);
        error_count++;
      end
      rcv_count++;
      // If we received the expected number of items, store the stop timestamp
      // in the global variable such that the send souce task can compare the
      // throughput
      if (sent_count == rcv_count) begin
        stop_time = $realtime();
      end
    end
  end


  initial begin : proc_stop_sim
    @(posedge rstn);
    wait (&sim_done);
    repeat (20) @(posedge clk);
    $info("Sim done. Total error count: %0d", error_count);
    $stop();
  end


  // system clock and reset
  clk_rst_gen #(
    .ClkPeriod    ( Tck          ),
    .RstClkCycles ( RstClkCycles )
  ) i_clk_rst_gen_reg (
    .clk_o  ( clk  ),
    .rst_no ( rstn )
  );

  payload_t s_data_out_alloc2spill_data;
  logic [ChannelCount-1:0] s_data_out_alloc2spill_valid;
  logic [ChannelCount-1:0] s_data_out_spill2alloc_ready;

  payload_t s_data_out_spill2delay_data;
  logic [ChannelCount-1:0] s_data_out_spill2delay_valid;
  logic [ChannelCount-1:0] s_data_out_delay2spill_ready;


  payload_t s_data_in;
  logic [ChannelCount-1:0] s_data_in_valid;
  logic [ChannelCount-1:0] s_data_in_ready;

  logic [ChannelCount-1:0] source_bus_valid;

  logic [ChannelCount-1:0] sink_bus_valid;

  assign source_bus_valid = '{default: source_bus.valid};
  assign sink_bus.valid = |sink_bus_valid;

  serial_link_channel_allocator #(
    .NumChannels ( ChannelCount )
  ) i_channel_allocator (
    .clk_i                     ( clk                          ),
    .rst_ni                    ( rstn                         ),
    .cfg_tx_clear_i            ( cfg_tx_clear                 ),
    .cfg_rx_clear_i            ( cfg_rx_clear                 ),
    .cfg_tx_channel_en_i       ( cfg_tx_channel_en            ),
    .cfg_tx_bypass_en_i        ( cfg_tx_bypass_en             ),
    .cfg_tx_auto_flush_en_i    ( cfg_tx_auto_flush_en         ),
    .cfg_tx_flush_trigger_i    ( cfg_tx_flush_trigger         ),
    .cfg_tx_auto_flush_count_i ( cfg_tx_auto_flush_count      ),
    .cfg_rx_channel_en_i       ( cfg_rx_channel_en            ),
    .cfg_rx_auto_flush_en_i    ( cfg_rx_auto_flush_en         ),
    .cfg_rx_auto_flush_count_i ( cfg_rx_auto_flush_count      ),
    .cfg_rx_bypass_en_i        ( cfg_rx_bypass_en             ),
    .cfg_rx_sync_en_i          ( cfg_rx_sync_en               ),
    .data_out_i                ( source_bus.data              ),
    .data_out_valid_i          ( source_bus_valid             ),
    .data_out_ready_o          ( source_bus.ready             ),
    .data_out_o                ( s_data_out_alloc2spill_data  ),
    .data_out_valid_o          ( s_data_out_alloc2spill_valid ),
    .data_out_ready_i          ( s_data_out_spill2alloc_ready ),
    .data_in_o                 ( sink_bus.data                ),
    .data_in_valid_o           ( sink_bus_valid               ),
    .data_in_ready_i           ( '{default: sink_bus.ready}   ),
    .data_in_i                 ( s_data_in                    ),
    .data_in_valid_i           ( s_data_in_valid              ),
    .data_in_ready_o           ( s_data_in_ready              )
  );

  // Instantiate feedback through randomized delay modules
  for (genvar i = 0; i < ChannelCount; i++) begin : gen_feedback_delay_modules
    // Cut combinational paths between TX and RX

    spill_register #(
      .T(element_t),
      .Bypass(1'b0)
    ) i_feedback_decouple (
      .clk_i   ( clk                             ),
      .rst_ni  ( rstn                            ),
      .valid_i ( s_data_out_alloc2spill_valid[i] ),
      .ready_o ( s_data_out_spill2alloc_ready[i] ),
      .data_i  ( s_data_out_alloc2spill_data[i]  ),
      .valid_o ( s_data_out_spill2delay_valid[i] ),
      .ready_i ( s_data_out_delay2spill_ready[i] ),
      .data_o  ( s_data_out_spill2delay_data[i]  )
    );

    stream_delay #(
      .StallRandom ( EnableRandomStalls ),
      .payload_t   ( element_t          ),
      .FixedDelay  ( 0                  ),
      .Seed        ( i                  )
    ) i_feedback_delay(
      .clk_i     ( clk                             ),
      .rst_ni    ( rstn                            ),
      .payload_i ( s_data_out_spill2delay_data[i]  ),
      .ready_o   ( s_data_out_delay2spill_ready[i] ),
      .valid_i   ( s_data_out_spill2delay_valid[i] ),
      .payload_o ( s_data_in[i]                    ),
      .ready_i   ( s_data_in_ready[i]              ),
      .valid_o   ( s_data_in_valid[i]              )
    );
  end


  for (genvar i = 0; i < ChannelCount; i++) begin :gen_invalidate_disabled_channels
    always_comb begin : invalidate_disabled_channels
      if (cfg_tx_channel_en[i] == 1'b0) begin
        force s_data_in[i] = 'X;
      end else begin
        release s_data_in[i];
      end
      if (!s_data_in_valid[i]) begin
        force s_data_in[i] = 'X;
      end else begin
        release s_data_in[i];
      end
    end
  end

endmodule

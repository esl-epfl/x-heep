// Copyright 2022 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51

// Author: Manuel Eggimann <meggimann@iis.ee.ethz.ch>
module tb_stream_chopper;

  localparam time         Tck = 200ns;
  localparam int unsigned ChannelCount = 32;
  localparam int unsigned Log2ChannelCount = $clog2(ChannelCount);
  localparam int unsigned RstClkCycles = 1;
  localparam int unsigned MaxWaitCycles = 0;
  localparam int unsigned TestLength = 10;
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
  logic [ChannelCount-1:0]     valid_out;

  STREAM_DV #(.payload_t(payload_t)) master_bus(clk);
  STREAM_DV #(.payload_t(payload_t)) slave_bus(clk);

  stream_driver_t master_driver = new(master_bus);
  stream_driver_t slave_driver  = new(slave_bus);

  element_t send_queue[$];

  int                          sent_count = 0;
  int                          rcv_count = 0;

  initial begin : proc_source
    automatic payload_t word;
    automatic int wait_cycl;
    @(posedge rstn);
    master_driver.reset_in();

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
      master_driver.send(word);
      // Push the word element by element into the queue
      for (int i = 0; i < ChannelCount; i++) begin
        send_queue.push_back(word[i]);
        sent_count++;
      end
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
        for (int i = 0; i < ChannelCount; i++) begin
          send_queue.push_back(word[i]);
          sent_count++;
        end
        // Apply word to chopper
        master_driver.send(word);
        // Randomly delay sending of next item
        wait_cycl = $urandom_range(0, MaxWaitCycles);
        repeat(wait_cycl) @(posedge clk);
      end
      // Wait a couple of cycles for the final elements to sink
      repeat(MaxWaitCycles+AutoFlushWaitCount+3) @(posedge clk);
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
    automatic element_t expected_element;
    @(posedge rstn);
    slave_driver.reset_out();

    forever begin
      wait_cycl = $urandom_range(0, MaxWaitCycles);
      repeat(wait_cycl) @(posedge clk);
      slave_driver.recv(rcv_word);
      for (int i = 0; i < ChannelCount; i++) begin
        if (bypass_en | i < cfg_chopsize) begin
          if (send_queue.size() > 0) begin
            expected_element = send_queue.pop_front();
            assert(expected_element == rcv_word[i]) else
              $error("Received %0h but expected %0h.", rcv_word[i], expected_element);
            assert(valid_out[i] == 1'b1) else
              $error("The valid signal correpsonding to element idx %0d was not asserted.",i);
            rcv_count++;
          end else begin
            assert(valid_out[i] == 1'b0) else
              $error("Valid signal corresponding to invalid element idx %0d", i,
                "of flushed (partial) output word should not be assertd");
          end
        end else begin
          assert(valid_out[i] == 1'b0) else
            $error("The valid signal correpsonding to element idx %0d should not be asserted.",i);
        end

      end
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
  ) i_dut (
    .clk_i                  ( clk                  ),
    .rst_ni                 ( rstn                 ),
    .bypass_en_i            ( bypass_en            ),
    .flush_i                ( flush                ),
    .cfg_auto_flush_en_i    ( cfg_auto_flush_en    ),
    .cfg_auto_flush_count_i ( cfg_auto_flush_count ),
    .cfg_chopsize_i         ( cfg_chopsize         ),
    .data_i                 ( master_bus.data      ),
    .valid_i                ( master_bus.valid     ),
    .ready_o                ( master_bus.ready     ),
    .data_o                 ( slave_bus.data       ),
    .valid_o                ( valid_out            ),
    .ready_i                ( slave_bus.ready      )
  );

  assign slave_bus.valid = |valid_out;

endmodule

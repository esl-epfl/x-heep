module clock_multiplier (
    input  wire clk_in,  // Input clock signal
    input  wire reset,   // Reset signal
    output wire clk_out  // Output multiplied clock signal
);

  // Internal signals
  wire clk_fb;
  wire clk_fb_buf;
  wire locked;

  // Instantiate the MMCM primitive
  MMCME2_BASE #(
      .CLKIN1_PERIOD(10.0),  // Input clock period in ns (e.g., 100 MHz -> 10 ns)
      .CLKFBOUT_MULT_F(10.0),  // Multiply factor (e.g., 10.0 for 10x multiplication)
      .CLKOUT0_DIVIDE_F(1.0)  // Divide factor (e.g., 1.0 for no division)
  ) mmcm_inst (
      .CLKIN1(clk_in),
      .CLKFBIN(clk_fb_buf),
      .CLKFBOUT(clk_fb),
      .CLKOUT0(clk_out),
      .LOCKED(locked),
      .RST(reset),
      .PWRDWN(1'b0)
  );

  // Feedback buffer
  BUFG fb_buf (
      .O(clk_fb_buf),
      .I(clk_fb)
  );

endmodule

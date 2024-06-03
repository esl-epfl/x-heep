module clock_divider_simple (
    input  logic clk,
    input  logic rst_n,
    output logic clk_out
);

  // Internal signal to hold the divided clock state
  logic clk_div;

  // Always block to divide the clock
  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      clk_div <= 1'b0;  // Reset the divided clock
    end else begin
      clk_div <= ~clk_div;  // Toggle the divided clock
    end
  end

  // Assign the divided clock to the output
  assign clk_out = clk_div;

endmodule

module hw_w_fifo_ctrl (
    input logic hw_fifo_mode_i,
    output logic [31:0] data_o,
    input logic [31:0] data_i,
    input logic data_out_gnt_i,
    output logic pop_o
);

  always_comb begin
    pop_o = 1'b0;
    if (hw_fifo_mode_i == 1'b1 && data_out_gnt_i == 1'b1) begin
      pop_o = 1'b1;
    end
    data_o = data_i;
  end

endmodule

module hw_r_fifo_ctrl (
    input logic hw_fifo_mode_i,
    input logic [31:0] data_i,
    input logic data_valid_i,
    input logic hw_r_fifo_push_padding_i,
    output logic push_o,
    output logic [31:0] data_o
);

  always_comb begin
    push_o = 1'b0;
    data_o = '0;
    if (hw_fifo_mode_i == 1'b1 && data_valid_i == 1'b1 || hw_r_fifo_push_padding_i == 1'b1) begin
      push_o = 1'b1;
      data_o = data_i;
    end
  end

endmodule

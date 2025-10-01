module mult #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic
) (
    input logic clk_i,
    input logic rst_ni,

    // Register interface (hooked by *_reg_top.sv)
    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o
);
  import mult_reg_pkg::*;  // from autogen
  mult_reg2hw_t reg2hw;  // HW-readable mirror (A.q, B.q, ...)
  mult_hw2reg_t hw2reg;  // HW-writable (PRODUCT.d/de)

  // Auto-generated APB register block (name is fixed by reggen)
  mult_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) u_reg_top (
      .clk_i,
      .rst_ni,
      .reg_req_i,
      .reg_rsp_o,
      .reg2hw,
      .hw2reg,
      .devmode_i(1'b1)  // devmode: explicit error on unmapped access
  );

  // Core

  always_comb begin
    hw2reg = '0;  
    hw2reg.product.de = 1'b1;  // enable HW write
    hw2reg.product.d  = reg2hw.a.q * reg2hw.b.q;  // A*B (lo 32)
  end


endmodule

module my_ip #(
    parameter type reg_req_t = reg_pkg::reg_req_t,
    parameter type reg_rsp_t = reg_pkg::reg_rsp_t
) (
    input logic clk_i,
    input logic rst_ni,

    // Register interface
    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    // Done signal
    output logic my_ip_done_o,

    // Interrupt signal
    output logic my_ip_interrupt_o,

    // Master ports on the system bus
    output obi_pkg::obi_req_t  my_ip_master_bus_req_o,
    input  obi_pkg::obi_resp_t my_ip_master_bus_resp_i
);

  import my_ip_reg_pkg::*;

  /* Registers */
  my_ip_reg2hw_t reg2hw;
  my_ip_hw2reg_t hw2reg;
  
  assign my_ip_done_o = 1'b0;
  assign my_ip_interrupt_o = 1'b0;
  
  assign my_ip_master_bus_req_o.req = 1'b0; // No request
  assign my_ip_master_bus_req_o.we = 1'b0;
  assign my_ip_master_bus_req_o.be = 4'b1111;
  assign my_ip_master_bus_req_o.addr = 32'h0;
  assign my_ip_master_bus_req_o.wdata = 32'h0;
  
  /* Registers */
  my_ip_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) my_ip_reg_top_i (
      .clk_i(clk_i),
      .rst_ni,
      .reg_req_i,
      .reg_rsp_o,
      .reg2hw,
      .hw2reg,
      .devmode_i(1'b1)
  );


endmodule
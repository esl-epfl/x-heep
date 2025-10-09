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
  import core_v_mini_mcu_reg_pkg::*;
  import target_peripheral_reg_pkg::*;

  /* Registers */
  my_ip_reg2hw_t reg2hw;
  my_ip_hw2reg_t hw2reg;

  logic [31:0] my_register;
  
  assign my_ip_done_o = 1'b0;
  assign my_ip_interrupt_o = 1'b0;
  
  assign my_ip_master_bus_req_o.req = 1'b0; // No request
  assign my_ip_master_bus_req_o.we = 1'b0;
  assign my_ip_master_bus_req_o.be = 4'b1111;
  assign my_ip_master_bus_req_o.addr = 32'h0;
  assign my_ip_master_bus_req_o.wdata = 32'h0;

  enum logic {
    OBI_IDLE,
    OBI_ISSUE_REQ,
    OBI_WAIT_GNT,
    OBI_WAIT_RVALID
  }
      my_ip_state, my_ip_n_state;

  always_ff @( clk_i ) begin : fsm_one
    if ( !rst_ni ) begin
        my_ip_state <= OBI_IDLE;
      // reset
    end else begin
        my_ip_state <= my_ip_n_state;
    end
  end

  always_comb begin

    my_ip_n_state = my_ip_state;

    case ( my_ip_state )
      OBI_IDLE: begin
        if ( reg2hw.start == 1'b1 ) begin
            my_ip_n_state = OBI_ISSUE_REQ;
        end
      end
      OBI_ISSUE_REQ: begin
        my_ip_master_bus_req_o.req = 1'b1;
        my_ip_master_bus_req_o.addr = TARGET_PERIPHERAL_START_ADDRESS + {25'h0, TARGET_REG_1_TYPE_OFFSET};
        
        my_ip_n_state = OBI_WAIT_GNT;
      end
      OBI_WAIT_GNT: begin
        my_ip_master_bus_req_o.req = 1'b0;
        if ( my_ip_master_bus_resp_i.gnt == 1'b1 ) begin
            my_ip_n_state = OBI_WAIT_RVALID;
        end
      end
      OBI_WAIT_RVALID: begin
        if ( my_ip_master_bus_resp_i.rvalid ) begin
            // Do something with the read data
            my_ip_n_state = OBI_STORE_DATA;
        end
      end
      OBI_STORE_DATA: begin
        // Store the data into a register or process it
        my_ip_n_state = OBI_IDLE;
      end
      default: begin
        my_ip_n_state = OBI_IDLE;
      end
    endcase
    
  end

  always_ff @( posedge clk_i ) begin
    if ( !rst_ni ) begin
        my_register <= 32'h0;
    end else begin
        if ( my_ip_state == OBI_STORE_DATA ) begin
            my_register <= my_ip_master_bus_resp_i.rdata;
        end
    end
  end

  
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

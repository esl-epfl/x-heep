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
  import core_v_mini_mcu_pkg::*;
  import spi_host_reg_pkg::*;

  my_ip_reg2hw_t reg2hw;
  my_ip_hw2reg_t hw2reg;

  assign my_ip_interrupt_o = 1'b0;

  // OBI FSM
  enum logic [2:0] {
    OBI_IDLE,
    OBI_ISSUE_REQ,
    OBI_WAIT_GNT,
    OBI_WAIT_RVALID
  }
      my_ip_state, my_ip_n_state;

  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      my_ip_state  <= OBI_IDLE;
      read_value_q <= 32'h00000000;
    end else begin
      my_ip_state  <= my_ip_n_state;
      read_value_q <= read_value_d;
    end
  end

  // FIRST LET'S GET READ ONLY
  logic [31:0] read_value_d, read_value_q;

  always_comb begin
    my_ip_master_bus_req_o.req = 1'b0;
    my_ip_master_bus_req_o.we = 1'b0;
    my_ip_master_bus_req_o.be = 4'b1111;
    my_ip_master_bus_req_o.addr = 32'h00000000;
    my_ip_master_bus_req_o.wdata = 32'h00000000;

    my_ip_done_o = 1'b0;

    my_ip_n_state = my_ip_state;
    read_value_d = read_value_q;

    hw2reg.status.d = 1'b0;

    case (my_ip_state)
      OBI_IDLE: begin
        hw2reg.status.d = 1'b1;
        if (reg2hw.control.start) begin
          my_ip_n_state = OBI_ISSUE_REQ;
        end
      end

      OBI_ISSUE_REQ: begin
        my_ip_master_bus_req_o.req = 1'b1;
        my_ip_master_bus_req_o.we = reg2hw.control.write;
        my_ip_master_bus_req_o.addr = reg2hw.address;
        my_ip_master_bus_req_o.wdata = reg2hw.data;

        if (my_ip_master_bus_resp_i.gnt) begin  // In case gnt received same cycle as request
          my_ip_n_state = OBI_WAIT_RVALID;
        end else begin
          my_ip_n_state = OBI_WAIT_GNT;
        end
      end

      OBI_WAIT_GNT: begin
        my_ip_master_bus_req_o.req = 1'b1;  // Keep request as long as no grant has been received
        my_ip_master_bus_req_o.we = reg2hw.control.write;
        my_ip_master_bus_req_o.addr = reg2hw.address;
        my_ip_master_bus_req_o.wdata = reg2hw.data;

        if (my_ip_master_bus_resp_i.gnt) begin
          my_ip_n_state = OBI_WAIT_RVALID;
        end
      end

      OBI_WAIT_RVALID: begin
        if (my_ip_master_bus_resp_i.rvalid) begin
          read_value_d  = my_ip_master_bus_resp_i.rdata;
          my_ip_n_state = OBI_IDLE;
        end
      end

      default: begin
        my_ip_n_state = OBI_IDLE;
      end
    endcase
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

  // HW emulation of w25q128jw_read_standard
    // Preparation of command for TXDATA
  // CHECK FOR SIZE BEFORE
  function automatic [31:0] bitfield_byteswap32(input [31:0] data);
    bitfield_byteswap32 = {data[7:0], data[15:8], data[23:16], data[31:24]};
  endfunction

  logic [31:0] cmd_tx_data;
  assign cmd_tx_data = (bitfield_byteswap32(reg2hw.address) >> 8) | 32'h03;

  enum logic [1:0] {
    SPI_IDLE,
    SPI_WRITE_WORD,
    SPI_WAIT_FOR_READY,
    SPI_SET_CMD
    // REST FOR LATER
  } spi_state, spi_n_state;

  always_ff @( posedge clk_i or negedge rst_ni) begin : spi_fsm
    if ( !rst_ni ) begin
        spi_state <= SPI_IDLE;
    end else begin
        spi_state <= spi_n_state;
    end
  end

  always_comb begin

    spi_n_state = spi_state;

    case (spi_state)
      SPI_IDLE: begin
        if (reg2hw.control.start) begin
          spi_n_state = SPI_WRITE_WORD;
        end
      end

      SPI_WRITE_WORD: begin
        // SET OBI WRITE REQ with cmd_tx_data
        if (my_ip_master_bus_resp_i.rvalid) begin
          spi_n_state = SPI_WAIT_FOR_READY;
        end
      end

      
    endcase
  end


endmodule

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
    OBI_WRITE
  } my_ip_state, my_ip_n_state;

  always_ff @( posedge clk_i or negedge rst_ni ) begin
    if ( !rst_ni ) begin
        my_ip_state <= OBI_IDLE;
    end else begin
        my_ip_state <= my_ip_n_state;
    end
  end

  always_comb begin
    my_ip_master_bus_req_o.req = 1'b0;
    my_ip_master_bus_req_o.we = 1'b1;
    my_ip_master_bus_req_o.be = 4'b1111;
    my_ip_master_bus_req_o.addr = SPI_HOST_START_ADDRESS + {25'h0, SPI_HOST_ERROR_STATUS_OFFSET};;
    my_ip_master_bus_req_o.wdata = 32'hABCD1234;

    my_ip_done_o = 1'b0;
    
    my_ip_n_state = my_ip_state;

    case ( my_ip_state )
      OBI_IDLE: begin
        if ( reg2hw.test_reg_w == 32'b1 ) begin
            my_ip_n_state = OBI_ISSUE_REQ;
        end
      end

      OBI_ISSUE_REQ: begin
        my_ip_master_bus_req_o.req = 1'b1;     
        if (my_ip_master_bus_resp_i.gnt) begin
          my_ip_n_state = OBI_WRITE;
        end else begin
          my_ip_n_state = OBI_WAIT_GNT;
        end
      end
      
      OBI_WAIT_GNT: begin
        my_ip_master_bus_req_o.req = 1'b1; //VERIFY if this is needed
        if (my_ip_master_bus_resp_i.gnt) begin
            my_ip_n_state = OBI_WRITE;
        end
      end

      OBI_WRITE: begin
        my_ip_master_bus_req_o.addr = SPI_HOST_START_ADDRESS + {25'h0, SPI_HOST_ERROR_STATUS_OFFSET}; // Will be chosen in next FSM (Write to control register of SPI)
        my_ip_master_bus_req_o.wdata = 32'hABCD1234; // Will be chosen in next FSM (Set up SPIEN bit in control register of SPI)
        if ( my_ip_master_bus_resp_i.rvalid ) begin
            my_ip_n_state = OBI_IDLE;
            my_ip_done_o = 1'b1;
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

  // // FSM to set up SPI for a read
  // enum logic [1:0] {
  //   SPI_IDLE,
  //   SPI_SETUP,
  //   SPI_START,
  //   SPI_WAIT_DONE
  // } spi_state, spi_n_state;

  // always_ff @( posedge clk_i or negedge rst_ni) begin : spi_fsm
  //   if ( !rst_ni ) begin
  //       spi_state <= SPI_IDLE;
  //   end else begin
  //       spi_state <= spi_n_state;
  //       hw2reg.test_reg_w.d <= obi_start;
  //   end
  // end

  // logic [31:0] obi_start;

  // always_comb begin

  //   obi_start = 32'h0;

  //   my_ip_master_bus_req_o.addr = 32'h0;

  //   spi_n_state = spi_state;

  //   case ( spi_state )
  //     SPI_IDLE: begin
  //       if ( reg2hw.setup_spi == 32'b1 ) begin
  //           spi_n_state = SPI_SETUP;
  //       end
  //     end
  //     SPI_SETUP: begin
  //       // Configure SPI settings here
  //       // Configure control register
  //       obi_start = 32'h1; // See FSM above
  //       my_ip_master_bus_req_o.addr = SPI_HOST_START_ADDRESS + {25'h0, SPI_HOST_INTR_STATE_OFFSET};

  //       if(my_ip_done_o) begin
  //           obi_start = 32'h0; // Clear start signal
  //           spi_n_state = SPI_START; // Or next setup step
  //       end
  //     end
  //     SPI_START: begin
  //       // Start the SPI transaction
  //       spi_n_state = SPI_WAIT_DONE;
  //     end
  //     SPI_WAIT_DONE: begin
  //       // Wait for the SPI transaction to complete
  //       // For simulation, we can assume it's done immediately
  //       spi_n_state = SPI_IDLE;
  //     end
  //     default: begin
  //       spi_n_state = SPI_IDLE;
  //     end
  //   endcase
  // end

// Show GTKwave

// Understand how to set up SPI for a read
// spi_read (test_read)
// sw/device/bsp/w25q
// sw/device/lib/driver/spi_host.h
// TXDATA to SPI FIFO

// First fix signal in GTKwave (have wrong one (see actual outputs (terminal vs. GTKwave)))
// Will try TXDATA: failed hence write probably fails (still should not work for hro right?)
// Where to get txdata in gtkwave?

endmodule

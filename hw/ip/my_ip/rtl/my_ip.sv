module my_ip #(
    parameter type reg_req_t = reg_pkg::reg_req_t,
    parameter type reg_rsp_t = reg_pkg::reg_rsp_t,
    parameter logic [7:0] SPI_FLASH_TX_FIFO_DEPTH = 8'h48,
    parameter logic [7:0] SPI_FLASH_RX_FIFO_DEPTH = 8'h40
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
  assign my_ip_done_o = 1'b0;

  // OBI FSM
  enum logic [1:0] {
    OBI_IDLE,
    OBI_ISSUE_REQ,
    OBI_WAIT_GNT,
    OBI_WAIT_RVALID
  }
      obi_state_d, obi_state_q;

  logic [31:0] read_value_d, read_value_q;
  logic obi_finish;

  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      obi_state_q  <= OBI_IDLE;
      read_value_q <= 32'h00000000;
    end else begin
      obi_state_q  <= obi_state_d;
      read_value_q <= read_value_d;
    end
  end

  always_comb begin
    my_ip_master_bus_req_o.req = 1'b0;
    my_ip_master_bus_req_o.we = 1'b0;
    my_ip_master_bus_req_o.be = 4'b1111;
    my_ip_master_bus_req_o.addr = 32'h00000000;
    my_ip_master_bus_req_o.wdata = 32'h00000000;

    obi_finish = 1'b0;

    obi_state_d = obi_state_q;
    read_value_d = read_value_q;


    case (obi_state_q)
      OBI_IDLE: begin
        if (obi_start) begin
          obi_state_d = OBI_ISSUE_REQ;
        end
      end

      OBI_ISSUE_REQ: begin
        my_ip_master_bus_req_o.req = 1'b1;
        my_ip_master_bus_req_o.we = w_enable;
        my_ip_master_bus_req_o.addr = address;
        my_ip_master_bus_req_o.wdata = data;

        if (my_ip_master_bus_resp_i.gnt) begin  // In case gnt received same cycle as request
          obi_state_d = OBI_WAIT_RVALID;
        end else begin
          obi_state_d = OBI_WAIT_GNT;
        end
      end

      OBI_WAIT_GNT: begin
        my_ip_master_bus_req_o.req = 1'b1;  // Keep request as long as no grant has been received
        my_ip_master_bus_req_o.we = w_enable;
        my_ip_master_bus_req_o.addr = address;
        my_ip_master_bus_req_o.wdata = data;

        if (my_ip_master_bus_resp_i.gnt) begin
          obi_state_d = OBI_WAIT_RVALID;
        end
      end

      OBI_WAIT_RVALID: begin
        if (my_ip_master_bus_resp_i.rvalid) begin
          read_value_d = my_ip_master_bus_resp_i.rdata;
          obi_finish   = 1'b1;
          obi_state_d  = OBI_IDLE;
        end
      end

      default: begin
        obi_state_d = OBI_IDLE;
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
  function automatic [31:0] bitfield_byteswap32(input [31:0] data);
    bitfield_byteswap32 = {data[7:0], data[15:8], data[23:16], data[31:24]};
  endfunction

  enum logic [3:0] {
    SPI_IDLE,
    SPI_CHECK_TX_FIFO,
    SPI_FILL_TX_FIFO,
    SPI_WAIT_READY_1,
    SPI_SEND_CMD_1,
    SPI_WAIT_READY_2,
    SPI_SEND_CMD_2,
    SPI_SET_RX_WM,
    SPI_CHECK_RX_FIFO,
    SPI_READ_RX_FIFO,
    SPI_READ_RX_FIFO_PROCESSED,
    SPI_EXTRA_WORD
    // REST FOR LATER
  }
      spi_state_q, spi_state_d;

  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      spi_state_q <= SPI_IDLE;
      to_read_q <= 32'h00000000;
      flag_q <= 1'b0;
      loop_cnt_q <= 32'h00000000;
    end else begin
      spi_state_q <= spi_state_d;
      to_read_q <= to_read_d;
      flag_q <= flag_d;
      loop_cnt_q <= loop_cnt_d;
    end
  end

  logic [31:0] address, data, to_read_d, to_read_q, loop_cnt_d, loop_cnt_q;
  logic w_enable, obi_start, flag_d, flag_q;

  always_comb begin
    address = 32'h00000000;
    data = 32'h00000000;
    w_enable = 1'b0;
    obi_start = 1'b0;

    flag_d = flag_q;
    to_read_d = to_read_q;
    spi_state_d = spi_state_q;
    loop_cnt_d = loop_cnt_q;

    case (spi_state_q)
      SPI_IDLE: begin
        if (reg2hw.control.start) begin
          spi_state_d = SPI_CHECK_TX_FIFO;
        end
      end

      SPI_CHECK_TX_FIFO: begin
        address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value_d[7:0] < SPI_FLASH_TX_FIFO_DEPTH) begin
            spi_state_d = SPI_FILL_TX_FIFO;
          end
        end

      end

      SPI_FILL_TX_FIFO: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_TXDATA_OFFSET};
        data = (bitfield_byteswap32(reg2hw.address) >> 8) | 32'h03;
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          spi_state_d = SPI_WAIT_READY_1;
        end
      end

      SPI_WAIT_READY_1: begin
        address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value_d[31] == 1'b1) begin
            spi_state_d = SPI_SEND_CMD_1;
          end
        end
      end

      SPI_SEND_CMD_1: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_COMMAND_OFFSET};
        data = (32'h2 << 27) + (32'h0 << 25) + (32'h1 << 24) + 32'h3; // Direction + Speed + Csaat + Length
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          spi_state_d = SPI_WAIT_READY_2;
        end
      end

      SPI_WAIT_READY_2: begin
        address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value_d[31] == 1'b1) begin
            spi_state_d = SPI_SEND_CMD_1;
          end
        end
      end

      SPI_SEND_CMD_2: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_COMMAND_OFFSET};
        data = (32'h1 << 27) + (32'h0 << 25) + (32'h0 << 24) + (reg2hw.length-1); // Direction + Speed + Csaat + Length
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          spi_state_d = SPI_SET_RX_WM;
        end
      end

      SPI_SET_RX_WM: begin
        if ((reg2hw.length - loop_cnt_d * SPI_FLASH_RX_FIFO_DEPTH) >= SPI_FLASH_RX_FIFO_DEPTH) begin
          data = SPI_FLASH_RX_FIFO_DEPTH >> 2;
          to_read_d = SPI_FLASH_RX_FIFO_DEPTH >> 2;
          flag_d = 1'b0;
        end else begin
          flag_d = 1'b1;
          if ((reg2hw.length - loop_cnt_d * SPI_FLASH_RX_FIFO_DEPTH) % 4 == 0) begin
            data = (reg2hw.length - loop_cnt_d * SPI_FLASH_RX_FIFO_DEPTH) >> 2;
            to_read_d = (reg2hw.length - loop_cnt_d * SPI_FLASH_RX_FIFO_DEPTH) >> 2;
          end else begin
            data = ((reg2hw.length - loop_cnt_d * SPI_FLASH_RX_FIFO_DEPTH) >> 2) + 1;
            to_read_d = ((reg2hw.length - loop_cnt_d * SPI_FLASH_RX_FIFO_DEPTH) >> 2) + 1;
          end
        end
        address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_CONTROL_OFFSET};
        w_enable  = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          spi_state_d = SPI_CHECK_RX_FIFO;
        end
      end

      SPI_CHECK_RX_FIFO: begin
        address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value_d[20] == 1'b1) begin
            spi_state_d = SPI_READ_RX_FIFO;
          end
        end
      end

      SPI_READ_RX_FIFO: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0 , SPI_HOST_RXDATA_OFFSET}; // Receives data through bus here? If yes then save this data and send it back through OBI to necessary address
        obi_start = 1'b1;

        if (obi_finish) begin
          spi_state_d = SPI_READ_RX_FIFO_PROCESSED;
        end
      end

      SPI_READ_RX_FIFO_PROCESSED: begin
        to_read_d = to_read_q - 1;
        if (to_read_q == 1) begin
          if (flag_q) begin
            // Read everything
            spi_state_d = SPI_EXTRA_WORD;  // CHANGE LATER
            loop_cnt_d  = 32'h00000000;
          end else begin
            loop_cnt_d  = loop_cnt_q + 1;
            // Didn't read everything
            spi_state_d = SPI_SET_RX_WM;
          end
        end else begin
          spi_state_d = SPI_READ_RX_FIFO;
        end
      end

      SPI_EXTRA_WORD: begin
        if (reg2hw.length % 4 != 0) begin
          address = SPI_FLASH_START_ADDRESS + {25'b0 , SPI_HOST_RXDATA_OFFSET}; // Receives data through bus here? If yes then save this data and send it back through OBI to necessary address
          obi_start = 1'b1;

          if (obi_finish) begin
            spi_state_d = SPI_IDLE;
          end
        end else begin
          spi_state_d = SPI_IDLE;
        end
      end

      default: begin
        spi_state_d = SPI_IDLE;
      end
    endcase
  end

endmodule

// Instead of reading the RXDATA FIFO, set DMA to do that.
// Read documentation about dma
// Read Tommaso's message

module my_ip #(
    parameter type reg_req_t = reg_pkg::reg_req_t,
    parameter type reg_rsp_t = reg_pkg::reg_rsp_t,
    parameter logic [7:0] SPI_FLASH_TX_FIFO_DEPTH = 8'h48,
    parameter logic [31:0] SPI_FLASH_RX_FIFO_DEPTH = 32'h40
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
    output obi_pkg::obi_req_t my_ip_master_bus_req_o,
    input obi_pkg::obi_resp_t my_ip_master_bus_resp_i,
    input logic [core_v_mini_mcu_pkg::DMA_CH_NUM-1:0] dma_done
);

  import my_ip_reg_pkg::*;
  import core_v_mini_mcu_pkg::*;
  import spi_host_reg_pkg::*;
  import dma_reg_pkg::*;

  my_ip_reg2hw_t reg2hw;
  my_ip_hw2reg_t hw2reg;

  assign my_ip_interrupt_o = 1'b0;

  // OBI FSM
  enum logic [1:0] {
    OBI_IDLE,
    OBI_ISSUE_REQ,
    OBI_WAIT_GNT,
    OBI_WAIT_RVALID
  }
      obi_state_d, obi_state_q;

  function automatic [31:0] bitfield_byteswap32(input [31:0] data);
    bitfield_byteswap32 = {data[7:0], data[15:8], data[23:16], data[31:24]};
  endfunction

  logic [31:0] address, data, read_value_d, read_value_q;
  logic obi_start, obi_finish, w_enable;

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

  // // HW emulation of w25q128jw_read_standard

  // enum logic [3:0] {
  //   SPI_IDLE,
  //   SPI_CHECK_TX_FIFO,
  //   SPI_FILL_TX_FIFO,
  //   SPI_WAIT_READY_1,
  //   SPI_SEND_CMD_1,
  //   SPI_WAIT_READY_2,
  //   SPI_SEND_CMD_2,
  //   SPI_SET_RX_WM,
  //   SPI_CHECK_RX_FIFO,
  //   SPI_READ_RX_FIFO,
  //   SPI_READ_RX_FIFO_PROCESSED,
  //   SPI_EXTRA_WORD
  //   // REST FOR LATER
  // }
  //     spi_state_q, spi_state_d;

  // always_ff @(posedge clk_i or negedge rst_ni) begin
  //   if (!rst_ni) begin
  //     spi_state_q <= SPI_IDLE;
  //     to_read_q <= 32'h00000000;
  //     flag_q <= 1'b0;
  //     loop_cnt_q <= 32'h00000000;
  //   end else begin
  //     spi_state_q <= spi_state_d;
  //     to_read_q <= to_read_d;
  //     flag_q <= flag_d;
  //     loop_cnt_q <= loop_cnt_d;
  //   end
  // end

  // logic [31:0] to_read_d, to_read_q, loop_cnt_d, loop_cnt_q;
  // logic flag_d, flag_q;

  // always_comb begin
  //   address = 32'h00000000;
  //   data = 32'h00000000;
  //   w_enable = 1'b0;
  //   obi_start = 1'b0;

  //   flag_d = flag_q;
  //   to_read_d = to_read_q;
  //   spi_state_d = spi_state_q;
  //   loop_cnt_d = loop_cnt_q;

  //   case (spi_state_q)
  //     SPI_IDLE: begin
  //       if (reg2hw.control.start) begin
  //         spi_state_d = SPI_CHECK_TX_FIFO;
  //       end
  //     end

  //     SPI_CHECK_TX_FIFO: begin
  //       address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
  //       obi_start = 1'b1;

  //       if (obi_finish) begin
  //         if (read_value_d[7:0] < SPI_FLASH_TX_FIFO_DEPTH) begin
  //           spi_state_d = SPI_FILL_TX_FIFO;
  //         end
  //       end

  //     end

  //     SPI_FILL_TX_FIFO: begin
  //       address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_TXDATA_OFFSET};
  //       data = (bitfield_byteswap32(reg2hw.address) >> 8) | 32'h03;
  //       w_enable = 1'b1;
  //       obi_start = 1'b1;

  //       if (obi_finish) begin
  //         spi_state_d = SPI_WAIT_READY_1;
  //       end
  //     end

  //     SPI_WAIT_READY_1: begin
  //       address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
  //       obi_start = 1'b1;

  //       if (obi_finish) begin
  //         if (read_value_d[31] == 1'b1) begin
  //           spi_state_d = SPI_SEND_CMD_1;
  //         end
  //       end
  //     end

  //     SPI_SEND_CMD_1: begin
  //       address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_COMMAND_OFFSET};
  //       data = (32'h2 << 27) + (32'h0 << 25) + (32'h1 << 24) + 32'h3; // Direction + Speed + Csaat + Length
  //       w_enable = 1'b1;
  //       obi_start = 1'b1;

  //       if (obi_finish) begin
  //         spi_state_d = SPI_WAIT_READY_2;
  //       end
  //     end

  //     SPI_WAIT_READY_2: begin
  //       address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
  //       obi_start = 1'b1;

  //       if (obi_finish) begin
  //         if (read_value_d[31] == 1'b1) begin
  //           spi_state_d = SPI_SEND_CMD_1;
  //         end
  //       end
  //     end

  //     SPI_SEND_CMD_2: begin
  //       address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_COMMAND_OFFSET};
  //       data = (32'h1 << 27) + (32'h0 << 25) + (32'h0 << 24) + (reg2hw.length-1); // Direction + Speed + Csaat + Length
  //       w_enable = 1'b1;
  //       obi_start = 1'b1;

  //       if (obi_finish) begin
  //         spi_state_d = SPI_SET_RX_WM;
  //       end
  //     end

  //     SPI_SET_RX_WM: begin
  //       if ((reg2hw.length - loop_cnt_d * SPI_FLASH_RX_FIFO_DEPTH) >= SPI_FLASH_RX_FIFO_DEPTH) begin
  //         data = SPI_FLASH_RX_FIFO_DEPTH >> 2;
  //         to_read_d = SPI_FLASH_RX_FIFO_DEPTH >> 2;
  //         flag_d = 1'b0;
  //       end else begin
  //         flag_d = 1'b1;
  //         if ((reg2hw.length - loop_cnt_d * SPI_FLASH_RX_FIFO_DEPTH) % 4 == 0) begin
  //           data = (reg2hw.length - loop_cnt_d * SPI_FLASH_RX_FIFO_DEPTH) >> 2;
  //           to_read_d = (reg2hw.length - loop_cnt_d * SPI_FLASH_RX_FIFO_DEPTH) >> 2;
  //         end else begin
  //           data = ((reg2hw.length - loop_cnt_d * SPI_FLASH_RX_FIFO_DEPTH) >> 2) + 1;
  //           to_read_d = ((reg2hw.length - loop_cnt_d * SPI_FLASH_RX_FIFO_DEPTH) >> 2) + 1;
  //         end
  //       end
  //       address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_CONTROL_OFFSET};
  //       w_enable  = 1'b1;
  //       obi_start = 1'b1;

  //       if (obi_finish) begin
  //         spi_state_d = SPI_CHECK_RX_FIFO;
  //       end
  //     end

  //     SPI_CHECK_RX_FIFO: begin
  //       address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
  //       obi_start = 1'b1;

  //       if (obi_finish) begin
  //         if (read_value_d[20] == 1'b1) begin
  //           spi_state_d = SPI_READ_RX_FIFO;
  //         end
  //       end
  //     end

  //     SPI_READ_RX_FIFO: begin
  //       address = SPI_FLASH_START_ADDRESS + {25'b0 , SPI_HOST_RXDATA_OFFSET}; // Receives data through bus here? If yes then save this data and send it back through OBI to necessary address
  //       obi_start = 1'b1;

  //       if (obi_finish) begin
  //         spi_state_d = SPI_READ_RX_FIFO_PROCESSED;
  //       end
  //     end

  //     SPI_READ_RX_FIFO_PROCESSED: begin
  //       to_read_d = to_read_q - 1;
  //       if (to_read_q == 1) begin
  //         if (flag_q) begin
  //           // Read everything
  //           spi_state_d = SPI_EXTRA_WORD;  // CHANGE LATER
  //           loop_cnt_d  = 32'h00000000;
  //         end else begin
  //           loop_cnt_d  = loop_cnt_q + 1;
  //           // Didn't read everything
  //           spi_state_d = SPI_SET_RX_WM;
  //         end
  //       end else begin
  //         spi_state_d = SPI_READ_RX_FIFO;
  //       end
  //     end

  //     SPI_EXTRA_WORD: begin
  //       if (reg2hw.length % 4 != 0) begin
  //         address = SPI_FLASH_START_ADDRESS + {25'b0 , SPI_HOST_RXDATA_OFFSET}; // Receives data through bus here? If yes then save this data and send it back through OBI to necessary address
  //         obi_start = 1'b1;

  //         if (obi_finish) begin
  //           spi_state_d = SPI_IDLE;
  //         end
  //       end else begin
  //         spi_state_d = SPI_IDLE;
  //       end
  //     end

  //     default: begin
  //       spi_state_d = SPI_IDLE;
  //     end
  //   endcase
  // end

  // HW emulation of w25q128jw_read_standard_dma
  enum logic [4:0] {
    DMA_IDLE,
    DMA_POINTERS,
    DMA_SRC_PTR_,
    DMA_DST_PTR_,
    DMA_SRC_INC,
    DMA_DST_INC,
    DMA_SRC_TYPE,
    DMA_DST_TYPE,
    DMA_TRIG,
    DMA_SIZE_D1_,
    SPI_CHECK_TX_FIFO,
    SPI_FILL_TX_FIFO,
    SPI_WAIT_READY_1,
    SPI_SEND_CMD_1,
    SPI_WAIT_READY_2,
    SPI_SEND_CMD_2,
    WAIT_TRANS
  }
      dma_state_q, dma_state_d;

  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      dma_state_q <= DMA_IDLE;
    end else begin
      dma_state_q <= dma_state_d;
    end
  end

  always_comb begin
    address = 32'h00000000;
    data = 32'h00000000;
    w_enable = 1'b0;
    obi_start = 1'b0;
    my_ip_done_o = 1'b0;

    dma_state_d = dma_state_q;

    case (dma_state_q)
      DMA_IDLE: begin
        if (reg2hw.control) begin  // Necessary to avoid spaming the BUS
          address   = DMA_START_ADDRESS + {25'b0, DMA_STATUS_OFFSET};
          obi_start = 1'b1;

          if (obi_finish) begin
            if (read_value_d[0] == 1'b1) begin  // DMA ready
              dma_state_d = DMA_SRC_PTR_;
            end
          end
        end
      end

      DMA_SRC_PTR_: begin
        address = DMA_START_ADDRESS + {25'b0, DMA_SRC_PTR_OFFSET};
        data = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_RXDATA_OFFSET};
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          dma_state_d = DMA_DST_PTR_;
        end
      end

      DMA_DST_PTR_: begin
        address = DMA_START_ADDRESS + {25'b0, DMA_DST_PTR_OFFSET};
        data = reg2hw.s_address;
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          dma_state_d = DMA_SRC_INC;
        end
      end

      DMA_SRC_INC: begin
        address = DMA_START_ADDRESS + {25'b0, DMA_SRC_PTR_INC_D1_OFFSET};
        data = 32'h0;  // Remain at RX Data FIFO address
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          dma_state_d = DMA_DST_INC;
        end
      end

      DMA_DST_INC: begin
        address = DMA_START_ADDRESS + {25'b0, DMA_DST_PTR_INC_D1_OFFSET};
        data = 32'h1;  // Every address has a word or a byte?
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          dma_state_d = DMA_SRC_TYPE;
        end
      end

      DMA_SRC_TYPE: begin
        address = DMA_START_ADDRESS + {25'b0, DMA_SRC_DATA_TYPE_OFFSET};
        data = 32'h0;  // 32-bit word
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          dma_state_d = DMA_DST_TYPE;
        end
      end

      DMA_DST_TYPE: begin
        address = DMA_START_ADDRESS + {25'b0, DMA_DST_DATA_TYPE_OFFSET};
        data = 32'h0;  // 32-bit word
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          dma_state_d = DMA_TRIG;
        end
      end

      DMA_TRIG: begin
        address = DMA_START_ADDRESS + {25'b0, DMA_SLOT_OFFSET};
        data = {16'h0, 16'h4};  // TX_TRG: Memory write trigger + RX_TRG: SPI Host RX FIFO threshold
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          dma_state_d = DMA_SIZE_D1_;
        end
      end

      // Starts transaction 
      DMA_SIZE_D1_: begin
        address   = DMA_START_ADDRESS + {25'b0, DMA_SIZE_D1_OFFSET};
        w_enable  = 1'b1;
        obi_start = 1'b1;

        if (reg2hw.length % 4 == 0) begin
          data = reg2hw.length >> 2;  // Number of bytes to transfer
        end else begin
          data = (reg2hw.length >> 2) + 1;  // Number of bytes to transfer rounded to next word
        end

        if (obi_finish) begin
          dma_state_d = SPI_CHECK_TX_FIFO;
        end
      end

      SPI_CHECK_TX_FIFO: begin
        address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value_d[7:0] < SPI_FLASH_TX_FIFO_DEPTH) begin
            dma_state_d = SPI_FILL_TX_FIFO;
          end
        end
      end

      SPI_FILL_TX_FIFO: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_TXDATA_OFFSET};
        data = (bitfield_byteswap32(reg2hw.r_address) >> 8) | 32'h03;
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          dma_state_d = SPI_WAIT_READY_1;
        end
      end

      SPI_WAIT_READY_1: begin
        address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value_d[31] == 1'b1) begin
            dma_state_d = SPI_SEND_CMD_1;
          end
        end
      end

      SPI_SEND_CMD_1: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_COMMAND_OFFSET};
        data = {3'h0, 2'h2, 2'h0, 1'h1, 24'h3};  // Empty + Direction + Speed + Csaat + Length
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          dma_state_d = SPI_WAIT_READY_2;
        end
      end

      SPI_WAIT_READY_2: begin
        address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value_d[31] == 1'b1) begin
            dma_state_d = SPI_SEND_CMD_2;
          end
        end
      end

      SPI_SEND_CMD_2: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_COMMAND_OFFSET};
        data = {
          3'h0, 2'h1, 2'h0, 1'h0, reg2hw.length[23:0] - 1'h1
        };  // Empty + Direction + Speed + Csaat + Length
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          dma_state_d = WAIT_TRANS;
        end
      end

      // STAYS HERE
      // MOST LIKELY DUE TO THE FACT THAT THE ADDRESS GIVEN TO SPI IS NOT WITHIN FLASH HENCE DOESN'T READ HENCE DOESN'T SRC TRIGGER DMA HENCE DMA IS STILL WAITING
      // SHOULD SIZE_D1 OF DMA DECREASE AS WE GO? AS SHOULD THE DST PTR? 

      // erilator doesn't work for FLASH?
      // Go to server, change gitignore, pull here then launch waves?

      WAIT_TRANS: begin
        if (dma_done[0] == 1'b1) begin  // Transaction done
          dma_state_d  = DMA_IDLE;
          my_ip_done_o = 1'b1;
        end
      end

      default: begin
        dma_state_d = DMA_IDLE;
      end
    endcase
  end

  // ssh -X
  // make questasim-sim-opt
  // make run-gui
  // run -all
  // gtkwave waveform.vcd &
endmodule

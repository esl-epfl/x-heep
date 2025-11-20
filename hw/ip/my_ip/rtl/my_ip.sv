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

  // OBI FSM
  enum logic [1:0] {
    OBI_IDLE,
    OBI_ISSUE_REQ,
    OBI_WAIT_RVALID
  }
      obi_state_d, obi_state_q;

  function automatic [31:0] bitfield_byteswap32(input [31:0] adress_to_swap);
    bitfield_byteswap32 = {
      adress_to_swap[7:0], adress_to_swap[15:8], adress_to_swap[23:16], adress_to_swap[31:24]
    };
  endfunction

  logic [31:0] address, data, read_value;
  logic obi_start, obi_finish, w_enable;

  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      obi_state_q <= OBI_IDLE;
    end else begin
      obi_state_q <= obi_state_d;
    end
  end

  always_comb begin
    my_ip_master_bus_req_o.req = 1'b0;
    my_ip_master_bus_req_o.we = 1'b0;
    my_ip_master_bus_req_o.be = 4'b1111;
    my_ip_master_bus_req_o.addr = 32'h00000000;
    my_ip_master_bus_req_o.wdata = 32'h00000000;

    obi_finish = 1'b0;
    read_value = 32'h00000000;

    obi_state_d = obi_state_q;


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

        if (my_ip_master_bus_resp_i.gnt) begin
          obi_state_d = OBI_WAIT_RVALID;
        end
      end

      OBI_WAIT_RVALID: begin
        if (my_ip_master_bus_resp_i.rvalid) begin
          read_value  = my_ip_master_bus_resp_i.rdata;
          obi_finish  = 1'b1;
          obi_state_d = OBI_IDLE;
        end
      end

      default: begin
        obi_state_d = OBI_IDLE;
      end
    endcase
  end

  // HW emulation of w25q128jw_read_standard_dma
  enum logic [4:0] {
    DMA_IDLE,
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

    hw2reg.control.start.de = 1'b0;
    hw2reg.control.start.d = 1'b0;

    case (dma_state_q)
      DMA_IDLE: begin
        if (reg2hw.control.start) begin  // Necessary to avoid spaming the BUS
          address   = DMA_START_ADDRESS + {25'b0, DMA_STATUS_OFFSET};
          obi_start = 1'b1;

          if (obi_finish) begin
            if (read_value[0] == 1'b1) begin  // DMA ready
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
        data = 32'h4;  // Every address has a word or a byte?
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
          if (read_value[7:0] < SPI_FLASH_TX_FIFO_DEPTH) begin
            dma_state_d = SPI_FILL_TX_FIFO;
          end
        end
      end

      SPI_FILL_TX_FIFO: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_TXDATA_OFFSET};
        data = (((bitfield_byteswap32(reg2hw.r_address & 32'h00ffffff)) >> 8) << 8) | 32'h03;
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
          if (read_value[31] == 1'b1) begin
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
          if (read_value[31] == 1'b1) begin
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

      WAIT_TRANS: begin
        if (dma_done[0] == 1'b1) begin  // Transaction done
          dma_state_d = DMA_IDLE;
          my_ip_done_o = 1'b1;
          hw2reg.control.start.de = 1'b1;
          hw2reg.control.start.d = 1'b0;
        end
      end

      SET_RX_WM: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_CONTROL_COMMAND_OFFSET};
        data = 32'h00000001;
      end

      default: begin
        dma_state_d = DMA_IDLE;
      end
    endcase
  end

  // Assignments
  assign my_ip_interrupt_o = 1'b0;
  assign hw2reg.control.ready.d = (dma_state_q == DMA_IDLE);
  assign hw2reg.control.ready.de = 1'b1;

  // Verify we get correct data in memory through sw comparison
  // Also try the revert addresses to see I get correct from SPI SW vs GTKWAVE
  // If fails try the w25jw128 functions from spi_read_example
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


/*

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

  // OBI FSM
  enum logic [1:0] {
    OBI_IDLE,
    OBI_ISSUE_REQ,
    OBI_WAIT_RVALID
  }
      obi_state_d, obi_state_q;

  function automatic [31:0] bitfield_byteswap32(input [31:0] adress_to_swap);
    bitfield_byteswap32 = {
      adress_to_swap[7:0], adress_to_swap[15:8], adress_to_swap[23:16], adress_to_swap[31:24]
    };
  endfunction

  logic [31:0] address, data, read_value;
  logic obi_start, obi_finish, w_enable;

  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      obi_state_q <= OBI_IDLE;
    end else begin
      obi_state_q <= obi_state_d;
    end
  end

  always_comb begin
    my_ip_master_bus_req_o.req = 1'b0;
    my_ip_master_bus_req_o.we = 1'b0;
    my_ip_master_bus_req_o.be = 4'b1111;
    my_ip_master_bus_req_o.addr = 32'h00000000;
    my_ip_master_bus_req_o.wdata = 32'h00000000;

    obi_finish = 1'b0;
    read_value = 32'h00000000;

    obi_state_d = obi_state_q;


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

        if (my_ip_master_bus_resp_i.gnt) begin
          obi_state_d = OBI_WAIT_RVALID;
        end
      end

      OBI_WAIT_RVALID: begin
        if (my_ip_master_bus_resp_i.rvalid) begin
          read_value  = my_ip_master_bus_resp_i.rdata;
          obi_finish  = 1'b1;
          obi_state_d = OBI_IDLE;
        end
      end

      default: begin
        obi_state_d = OBI_IDLE;
      end
    endcase
  end

  // Top FSM
  typedef enum logic {
    TOP_IDLE,
    TOP_READ,
    TOP_ERASE
  } top_state_e;

  // READ FSM
  typedef enum logic {
    READ_IDLE,
    READ_DMA_SRC_PTR,
    READ_DMA_DST_PTR,
    READ_DMA_SRC_INC,
    READ_DMA_DST_INC,
    READ_DMA_SRC_TYPE,
    READ_DMA_DST_TYPE,
    READ_DMA_TRIG,
    READ_DMA_SIZE_D1,

    READ_SPI_CHECK_TX_FIFO,
    READ_SPI_FILL_TX_FIFO,
    READ_SPI_WAIT_READY_1,
    READ_SPI_SEND_CMD_1,
    READ_SPI_WAIT_READY_2,
    READ_SPI_SEND_CMD_2,
    READ_TRANS
  } read_state_e;

  // ERASE FSM
  typedef enum logic {
    ERASE_IDLE,
    ERASE_SET_RXWM_R,
    ERASE_SET_RXWM_W,
    ERASE_CHECK_TX,
    ERASE_FILL_TX,
    ERASE_READY_1,
    ERASE_SEND_CMD_1,
    ERASE_READY_2,
    ERASE_SEND_CMD_2,
    ERASE_WM,
    ERASE_READ_STATUS,

    ERASE_FLASH_WE,
    ERASE_TRANS
  } erase_state_e;

  top_state_e        top_state_q,        top_state_d;
  read_state_e       read_state_q,       read_state_d;
  erase_state_e      erase_state_q,      erase_state_d;

  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      top_state_q <= TOP_IDLE;
      read_state_q <= READ_IDLE;
      erase_state_q <= ERASE_IDLE;
    end else begin
      top_state_q <= top_state_d
      read_state_q <= read_state_d;
      erase_state_q <= erase_state_d;
    end
  end

  always_comb begin
    address = 32'h00000000;
    data = 32'h00000000;
    w_enable = 1'b0;
    obi_start = 1'b0;
    my_ip_done_o = 1'b0;

    top_state_d = top_state_q;
    read_state_d = read_state_q;
    erase_state_d = erase_state_q;

    hw2reg.control.start.de = 1'b0;
    hw2reg.control.start.d = 1'b0;

    // ========== TOP FSM ==================

    case (top_state_q)
      TOP_IDLE: begin
        if (reg2hw.control.start) begin
          address   = DMA_START_ADDRESS + {25'b0, DMA_STATUS_OFFSET};
          obi_start = 1'b1;

          if (obi_finish && read_value[0]) begin // DMA ready
            top_state_d = TOP_READ;
            read_state_d = READ_DMA_SRC_PTR;
          end
        end
      end

      // ========== READ FSM ==================

      TOP_READ: begin
        case (read_state_q)
          READ_IDLE: begin
            // Nothing to do here.
          end

          READ_DMA_SRC_PTR: begin
            address = DMA_START_ADDRESS + {25'b0, DMA_SRC_PTR_OFFSET};
            data = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_RXDATA_OFFSET};
            w_enable = 1'b1;
            obi_start = 1'b1;

            if (obi_finish) begin
              read_state_d = READ_DMA_DST_PTR;
            end
          end

          READ_DMA_DST_PTR: begin
            address = DMA_START_ADDRESS + {25'b0, DMA_DST_PTR_OFFSET};
            data = reg2hw.s_address;
            w_enable = 1'b1;
            obi_start = 1'b1;

            if (obi_finish) begin
              read_state_d = READ_DMA_SRC_INC;
            end
          end

          READ_DMA_SRC_INC: begin
            address = DMA_START_ADDRESS + {25'b0, DMA_SRC_PTR_INC_D1_OFFSET};
            data = 32'h0;  // Remain at RX Data FIFO address
            w_enable = 1'b1;
            obi_start = 1'b1;

            if (obi_finish) begin
              read_state_d = READ_DMA_DST_INC;
            end
          end

          READ_DMA_DST_INC: begin
            address = DMA_START_ADDRESS + {25'b0, DMA_DST_PTR_INC_D1_OFFSET};
            data = 32'h4;  // Every address has a word or a byte?
            w_enable = 1'b1;
            obi_start = 1'b1;

            if (obi_finish) begin
              read_state_d = READ_DMA_SRC_TYPE;
            end
          end

          READ_DMA_SRC_TYPE: begin
            address = DMA_START_ADDRESS + {25'b0, DMA_SRC_DATA_TYPE_OFFSET};
            data = 32'h0;  // 32-bit word
            w_enable = 1'b1;
            obi_start = 1'b1;

            if (obi_finish) begin
              read_state_d = READ_DMA_DST_TYPE;
            end
          end

          READ_DMA_DST_TYPE: begin
            address = DMA_START_ADDRESS + {25'b0, DMA_DST_DATA_TYPE_OFFSET};
            data = 32'h0;  // 32-bit word
            w_enable = 1'b1;
            obi_start = 1'b1;

            if (obi_finish) begin
              read_state_d = READ_DMA_TRIG;
            end
          end

          READ_DMA_TRIG: begin
            address = DMA_START_ADDRESS + {25'b0, DMA_SLOT_OFFSET};
            data = {16'h0, 16'h4};  // TX_TRG: Memory write trigger + RX_TRG: SPI Host RX FIFO threshold
            w_enable = 1'b1;
            obi_start = 1'b1;

            if (obi_finish) begin
              read_state_d = READ_DMA_SIZE_D1;
            end
          end

          // Starts transaction 
          READ_DMA_SIZE_D1: begin
            address   = DMA_START_ADDRESS + {25'b0, DMA_SIZE_D1_OFFSET};
            w_enable  = 1'b1;
            obi_start = 1'b1;

            if (reg2hw.length % 4 == 0) begin
              data = reg2hw.length >> 2;  // Number of bytes to transfer
            end else begin
              data = (reg2hw.length >> 2) + 1;  // Number of bytes to transfer rounded to next word
            end

            if (obi_finish) begin
              read_state_d = READ_SPI_CHECK_TX_FIFO;
            end
          end

          READ_SPI_CHECK_TX_FIFO: begin
            address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
            obi_start = 1'b1;

            if (obi_finish && read_value[7:0] < SPI_FLASH_TX_FIFO_DEPTH) begin
              read_state_d = READ_SPI_FILL_TX_FIFO;
            end
          end

          READ_SPI_FILL_TX_FIFO: begin
            address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_TXDATA_OFFSET};
            data = (((bitfield_byteswap32(reg2hw.r_address & 32'h00ffffff)) >> 8) << 8) | 32'h03;
            w_enable = 1'b1;
            obi_start = 1'b1;

            if (obi_finish) begin
              read_state_d = READ_SPI_WAIT_READY_1;
            end
          end

          READ_SPI_WAIT_READY_1: begin
            address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
            obi_start = 1'b1;

            if (obi_finish && read_value[31]) begin
              read_state_d = READ_SPI_SEND_CMD_1;
            end
          end

          READ_SPI_SEND_CMD_1: begin
            address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_COMMAND_OFFSET};
            data = {3'h0, 2'h2, 2'h0, 1'h1, 24'h3};  // Empty + Direction + Speed + Csaat + Length
            w_enable = 1'b1;
            obi_start = 1'b1;

            if (obi_finish) begin
              read_state_d = READ_SPI_WAIT_READY_2;
            end
          end

          READ_SPI_WAIT_READY_2: begin
            address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
            obi_start = 1'b1;

            if (obi_finish && read_value[31]) begin
              read_state_d = READ_SPI_SEND_CMD_2;
            end
          end

          READ_SPI_SEND_CMD_2: begin
            address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_COMMAND_OFFSET};
            data = {
              3'h0, 2'h1, 2'h0, 1'h0, reg2hw.length[23:0] - 1'h1
            };  // Empty + Direction + Speed + Csaat + Length
            w_enable = 1'b1;
            obi_start = 1'b1;

            if (obi_finish) begin
              read_state_d = READ_TRANS;
            end
          end

          READ_TRANS: begin
            if (dma_done[0]) begin  // Transaction done
              if (reg2hw.control.rnw) begin
                read_state_d              = READ_IDLE;
                top_state_d               = TOP_IDLE;
                my_ip_done_o              = 1'b1;
                hw2reg.control.start.de   = 1'b1;
                hw2reg.control.start.d    = 1'b0;
              end else begin
                read_state_d              = READ_IDLE;
                top_state_d               = TOP_ERASE;
                erase_state_d             = ERASE_SET_RXWM_R;; 
              end
            end
          end

          default: begin
            read_state_d = READ_IDLE;
          end
        endcase
      end

      // ========== ERASE FSM ==================

      TOP_ERASE: begin
        case (erase_state_q)
          ERASE_IDLE: begin
            // Nothing to do here.
          end



          // DO THE ERASE FSM

          // LAST STATE SIMPLY GOES TO ERASE_IDLE

          default: begin
            erase_state_d = ERASE_IDLE;
          end
        endcase
      end
    endcase


      

      // flash_wait()
      // ifdef flag for sim required
      ERASE_SET_RX_WM_R: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_CONTROL_COMMAND_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          read_state_d = ERASE_SET_RX_WM_W;
        end
      end

      ERASE_SET_RX_WM_W: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_CONTROL_COMMAND_OFFSET};
        data = read_value | 32'h00000001; 
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          read_state_d = ERASE_SPI_CHECK_TX_FIFO_2;
        end
      end

      ERASE_SPI_CHECK_TX_FIFO_2: begin
        address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value[7:0] < SPI_FLASH_TX_FIFO_DEPTH) begin
            read_state_d = ERASE_SPI_FILL_TX_FIFO_2;
          end
        end
      end

      ERASE_SPI_FILL_TX_FIFO_2: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_TXDATA_OFFSET};
        data = 32'h00000005;
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          read_state_d = ERASE_SPI_WAIT_READY_3;
        end
      end

      ERASE_SPI_WAIT_READY_3: begin
        address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value[31] == 1'b1) begin
            read_state_d = ERASE_SPI_SEND_CMD_3;
          end
        end
      end

      ERASE_SPI_SEND_CMD_3: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_COMMAND_OFFSET};
        data = {3'h0, 2'h2, 2'h0, 1'h1, 24'h0};  // Empty + Direction + Speed + Csaat + Length
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          read_state_d = ERASE_SPI_WAIT_READY_4;
        end
      end

      ERASE_SPI_WAIT_READY_4: begin
        address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value[31] == 1'b1) begin
            read_state_d = ERASE_SPI_SEND_CMD_4;
          end
        end
      end

      ERASE_SPI_SEND_CMD_4: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_COMMAND_OFFSET};
        data = {3'h0, 2'h1, 2'h0, 1'h0, 24'h0};  // Empty + Direction + Speed + Csaat + Length
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          read_state_d = ERASE_WAIT_RX_WM;
        end
      end

      ERASE_WAIT_RX_WM: begin
        address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value[20] == 1'b1) begin
            read_state_d = ERASE_READ_FLASH_STATUS;
          end
        end
      end

      ERASE_READ_FLASH_STATUS: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_RXDATA_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value[0] == 1'b0) begin
          read_state_d = SPI_CHECK_TX_FIFO_3;
          end else begin
            read_state_d = ERASE_SPI_CHECK_TX_FIFO_2;
          end
        end
      end

      // flash_write_enable()
      // Enable flash write in order to erase
      ERASE_SPI_CHECK_TX_FIFO_3: begin
        address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value[7:0] < SPI_FLASH_TX_FIFO_DEPTH) begin
            read_state_d = ERASE_SPI_FILL_TX_FIFO_3;
          end
        end
      end

      ERASE_SPI_FILL_TX_FIFO_3: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_TXDATA_OFFSET};
        data = 32'h00000006;
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          read_state_d = ERASE_SPI_WAIT_READY_5;
        end
      end

      ERASE_SPI_WAIT_READY_5: begin
        address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value[31] == 1'b1) begin
            read_state_d = ERASE_SPI_SEND_CMD_5;
          end
        end
      end

      ERASE_SPI_SEND_CMD_5: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_COMMAND_OFFSET};
        data = {3'h0, 2'h2, 2'h0, 1'h0, 24'h0};  // Empty + Direction + Speed + Csaat + Length
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          read_state_d = ERASE_SPI_CHECK_TX_FIFO_4;
        end
      end

      // Build and send erase command
      ERASE_SPI_CHECK_TX_FIFO_4: begin
        address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value[7:0] < SPI_FLASH_TX_FIFO_DEPTH) begin
            read_state_d = ERASE_SPI_FILL_TX_FIFO_4;
          end
        end
      end

      ERASE_SPI_FILL_TX_FIFO_4: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_TXDATA_OFFSET};
        data = (((bitfield_byteswap32(reg2hw.r_address & 32'h00ffffff)) >> 8) << 8) | 32'h20; // May need to change address!
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          read_state_d = ERASE_SPI_WAIT_READY_6;
        end
      end

      ERASE_SPI_WAIT_READY_6: begin
        address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value[31] == 1'b1) begin
            read_state_d = ERASE_SPI_SEND_CMD_6;
          end
        end
      end

      ERASE_SPI_SEND_CMD_6: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_COMMAND_OFFSET};
        data = {3'h0, 2'h2, 2'h0, 1'h0, 24'h3};  // Empty + Direction + Speed + Csaat + Length
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          read_state_d = TO CHANGE;
        end
      end

      // Second flash_wait() call
      // ifdef flag for sim required
      ERASE_SET_RX_WM_R: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_CONTROL_COMMAND_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          read_state_d = ERASE_SET_RX_WM_W;
        end
      end

      ERASE_SET_RX_WM_W: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_CONTROL_COMMAND_OFFSET};
        data = read_value | 32'h00000001; 
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          read_state_d = ERASE_SPI_CHECK_TX_FIFO_2;
        end
      end

      ERASE_SPI_CHECK_TX_FIFO_2: begin
        address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value[7:0] < SPI_FLASH_TX_FIFO_DEPTH) begin
            read_state_d = ERASE_SPI_FILL_TX_FIFO_2;
          end
        end
      end

      ERASE_SPI_FILL_TX_FIFO_2: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_TXDATA_OFFSET};
        data = 32'h00000005;
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          read_state_d = ERASE_SPI_WAIT_READY_3;
        end
      end

      ERASE_SPI_WAIT_READY_3: begin
        address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value[31] == 1'b1) begin
            read_state_d = ERASE_SPI_SEND_CMD_3;
          end
        end
      end

      ERASE_SPI_SEND_CMD_3: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_COMMAND_OFFSET};
        data = {3'h0, 2'h2, 2'h0, 1'h1, 24'h0};  // Empty + Direction + Speed + Csaat + Length
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          read_state_d = ERASE_SPI_WAIT_READY_4;
        end
      end

      ERASE_SPI_WAIT_READY_4: begin
        address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value[31] == 1'b1) begin
            read_state_d = ERASE_SPI_SEND_CMD_4;
          end
        end
      end

      ERASE_SPI_SEND_CMD_4: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_COMMAND_OFFSET};
        data = {3'h0, 2'h1, 2'h0, 1'h0, 24'h0};  // Empty + Direction + Speed + Csaat + Length
        w_enable = 1'b1;
        obi_start = 1'b1;

        if (obi_finish) begin
          read_state_d = ERASE_WAIT_RX_WM;
        end
      end

      ERASE_WAIT_RX_WM: begin
        address   = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_STATUS_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value[20] == 1'b1) begin
            read_state_d = ERASE_READ_FLASH_STATUS;
          end
        end
      end

      ERASE_READ_FLASH_STATUS: begin
        address = SPI_FLASH_START_ADDRESS + {25'b0, SPI_HOST_RXDATA_OFFSET};
        obi_start = 1'b1;

        if (obi_finish) begin
          if (read_value[0] == 1'b0) begin
          read_state_d = ERASE_SPI_CHECK_TX_FIFO_3;
          end else begin
            read_state_d = ERASE_SPI_CHECK_TX_FIFO_2;
          end
        end
      end

      default: begin
        read_state_d = ERASE_IDLE;
      end
    endcase
  end

  // Assignments
  assign my_ip_interrupt_o = 1'b0;
  assign hw2reg.status.d = (top_state_q == TOP_IDLE);
  assign hw2reg.status.de = 1'b1;

  // Registers 
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


*/
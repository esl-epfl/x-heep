module add_two_number #(
  parameter type reg_req_t = logic,
  parameter type reg_rsp_t = logic
) (
  input  logic clk_i,
  input  logic rst_ni,
  input  reg_req_t reg_req_i,
  output reg_rsp_t reg_rsp_o
);

  //-------------------------------------------------------------------------
  // Register Address Definitions (byte addresses)
  //-------------------------------------------------------------------------
  localparam int ADDR_OPERAND0 = 32'h00;
  localparam int ADDR_OPERAND1 = 32'h04;
  localparam int ADDR_RESULT   = 32'h08;
  localparam int ADDR_CTRL     = 32'h0C;

  //-------------------------------------------------------------------------
  // Internal Register File
  //-------------------------------------------------------------------------
  logic [31:0] operand0_reg;  // Holds first operand
  logic [31:0] operand1_reg;  // Holds second operand
  logic [31:0] result_reg;    // Holds addition result
  logic        start_reg;     // When '1', addition is to be performed

  //-------------------------------------------------------------------------
  // Register Write and Computation Process
  //-------------------------------------------------------------------------
  // We assume that the memory-mapped interface uses a simple protocol in which:
  //   - reg_req_i.valid indicates a valid request.
  //   - reg_req_i.write is high for a write, low for a read.
  //   - reg_req_i.addr is the target address.
  //   - reg_req_i.wdata is the write data.
  // Likewise, we assume that reg_rsp_o has at least an rdata field.
  //-------------------------------------------------------------------------
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      operand0_reg <= 32'd0;
      operand1_reg <= 32'd0;
      result_reg   <= 32'd0;
      start_reg    <= 1'b0;
    end else begin
      // Process register writes
      if (reg_req_i.valid && reg_req_i.write) begin
        case (reg_req_i.addr)
          ADDR_OPERAND0: operand0_reg <= reg_req_i.wdata;
          ADDR_OPERAND1: operand1_reg <= reg_req_i.wdata;
          // Writing to the control register: bit 0 is the START flag.
          ADDR_CTRL:     start_reg    <= reg_req_i.wdata[0];
          // (Any write to ADDR_RESULT or other addresses is ignored.)
          default: ;
        endcase
      end

      // If the start flag is set, perform the addition and clear the flag.
      if (start_reg) begin
        result_reg <= operand0_reg + operand1_reg;
        start_reg <= 1'b0;
      end
    end
  end

  //-------------------------------------------------------------------------
  // Register Read Response
  //-------------------------------------------------------------------------
  // On a read request, we return the content of the corresponding register.
  // For example, a read from address 0x08 returns the result.
  //-------------------------------------------------------------------------
  always_comb begin
    // Default response value
    reg_rsp_o.rdata = 32'd0;
    if (reg_req_i.valid && !reg_req_i.write) begin
      case (reg_req_i.addr)
        ADDR_OPERAND0: reg_rsp_o.rdata = operand0_reg;
        ADDR_OPERAND1: reg_rsp_o.rdata = operand1_reg;
        ADDR_RESULT:   reg_rsp_o.rdata = result_reg;
        // Return control register contents (only the start bit)
        ADDR_CTRL:     reg_rsp_o.rdata = {31'd0, start_reg};
        default:       reg_rsp_o.rdata = 32'd0;
      endcase
    end
  end

endmodule

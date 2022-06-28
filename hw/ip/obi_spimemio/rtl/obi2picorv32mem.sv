/*
*  Copyright 2022 EPFL Christoph Müller <christoph.mueller@epfl.ch>
*  Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
*  SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
*/

module obi2picorv32mem 
import obi_intf::*,
import Picorv32mem_intf::*;
(
    PICORV32_BUS.core P32,
    SIMPLE_OBI_BUS.slave OBI,
    input logic clk,
    input logic nrst
);
    enum logic [3:0] {
        READ_MEM = 4'b0000,
        WRITE_WORD = 4'b1111,
        WRITE_HALFWORD_1 = 4'b1100,
        WRITE_HALFWORD_0 = 4'b0011,
        WRITE_BYTE_3 = 4'b1000,
        WRITE_BYTE_2 = 4'b0100,
        WRITE_BYTE_1 = 4'b0010,
        WRITE_BYTE_0 = 4'b0001
    } picorv_request_t;

    enum logic {
    IDLE,
    READ,
    WRITE
    } state, state_next;


    logic [31:0] addr_buf, addr_buf_next;

always_ff @( clk, negedge nrst ) begin : state_transition
    if (~nrst) begin
        state <= IDLE;
        addr_buf <= '0;
    end else begin
        state <= state_next;
        addr_buf <= addr_buf_next;
    end
end

always_comb begin : fsm
    // default values
    addr_buf_next <= addr_buf;
    state_next <= state;
    P32.valid <= 1'b0;
    P32.addr <= '0;
    P32.wdata <= '0;
    P32.wstrb <= READ_MEM;
    OBI.gnt <= 1'b0;
    OBI.rvalid <= 1'b0;
    OBI.rdata <= '0;
    // fsm
    case(state)
        IDLE: begin
            if (OBI.req) begin
                if (OBI.we == 1'b1) begin
                    state_next <= WRITE;
                end else begin
                    state_next <= READ;
                    P32.addr <= OBI.addr; 
                    addr_buf_next <= OBI.addr; 
                    P32.valid <= 1'b1;
                    OBI.gnt <= 1'b1;
                end
            end
        end 
        READ: begin
            P32.addr <= addr_buf;
            if (P32.ready) begin
                state_next <= IDLE;
                OBI.rdata <= P32.rdata;
                OBI.rvalid <= 1'b1;
            end
            // Todo: add contineous read w/o idle
        end
        WRITE: begin
            state_next <= IDLE;
            OBI.gnt <= 1'b1;
            // Todo: add write.
        end
    endcase

end

endmodule
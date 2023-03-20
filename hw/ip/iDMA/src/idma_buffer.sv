// Copyright 2022 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Thomas Benz <tbenz@ethz.ch>

/// A byte-granular buffer holding data while it is copied.
module idma_buffer #(
    /// The depth of the buffer
    parameter int unsigned BufferDepth = 32'd3,
    /// The width of the buffer in bytes
    parameter int unsigned StrbWidth = 32'd1,
    /// Print the info of the FIFO configuration
    parameter bit PrintFifoInfo = 1'b0,
    /// The strobe type
    parameter type strb_t = logic,
    /// The byte type
    parameter type byte_t = logic [7:0]
)(
    input  logic clk_i,
    input  logic rst_ni,
    input  logic testmode_i,

    input  byte_t [StrbWidth-1:0] data_i,
    input  strb_t valid_i,
    output strb_t ready_o,

    output byte_t [StrbWidth-1:0] data_o,
    output strb_t valid_o,
    input  strb_t ready_i
);

    // buffer is implemented as an array of stream FIFOs
    for (genvar i = 0; i < StrbWidth; i++) begin : gen_fifo_buffer
        idma_stream_fifo #(
            .type_t       ( byte_t        ),
            .Depth        ( BufferDepth   ),
            .PrintInfo    ( PrintFifoInfo )
        ) i_byte_buffer (
            .clk_i,
            .rst_ni,
            .testmode_i,
            .flush_i      ( 1'b0                ),
            .usage_o      ( /* NOT CONNECTED */ ),
            .data_i       ( data_i  [i]         ),
            .valid_i      ( valid_i [i]         ),
            .ready_o      ( ready_o [i]         ),
            .data_o       ( data_o  [i]         ),
            .valid_o      ( valid_o [i]         ),
            .ready_i      ( ready_i [i]         )
        );
    end : gen_fifo_buffer

endmodule : idma_buffer

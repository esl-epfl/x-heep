// Copyright 2022 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Thomas Benz <tbenz@ethz.ch>

`include "common_cells/registers.svh"

/// Implementing the AXI4 transport layer in the iDMA backend.
module idma_axi_transport_layer #(
    /// Data width
    parameter int unsigned DataWidth = 32'd16,
    /// The depth of the internal reorder buffer:
    /// - '2': minimal possible configuration
    /// - '3': efficiently handle misaligned transfers (recommended)
    parameter int unsigned BufferDepth = 32'd3,
    /// Mask invalid data on the manager interface
    parameter bit MaskInvalidData = 1'b1,
    /// Print the info of the FIFO configuration
    parameter bit PrintFifoInfo = 1'b0,
    /// `r_dp_req_t` type:
    parameter type r_dp_req_t = logic,
    /// `r_dp_req_t` type:
    parameter type w_dp_req_t = logic,
    /// `r_dp_req_t` type:
    parameter type r_dp_rsp_t = logic,
    /// `r_dp_req_t` type:
    parameter type w_dp_rsp_t = logic,
    /// AXI 4 `AW` channel type
    parameter type axi_aw_chan_t = logic,
    /// AXI 4 `AR` channel type
    parameter type axi_ar_chan_t = logic,
    /// AXI 4 Request channel type
    parameter type axi_req_t = logic,
    /// AXI 4 Response channel type
    parameter type axi_rsp_t = logic
)(
    /// Clock
    input  logic clk_i,
    /// Asynchronous reset, active low
    input  logic rst_ni,
    /// Testmode in
    input  logic testmode_i,

    /// AXI4+ATOP manager port request
    output axi_req_t axi_req_o,
    /// AXI4+ATOP manager port response
    input  axi_rsp_t axi_rsp_i,

    /// Read datapath request
    input  r_dp_req_t r_dp_req_i,
    /// Read datapath request valid
    input  logic r_dp_valid_i,
    /// Read datapath request ready
    output logic r_dp_ready_o,

    /// Read datapath response
    output r_dp_rsp_t r_dp_rsp_o,
    /// Read datapath response valid
    output logic r_dp_valid_o,
    /// Read datapath response valid
    input  logic r_dp_ready_i,

    /// Write datapath request
    input  w_dp_req_t w_dp_req_i,
    /// Write datapath request valid
    input  logic w_dp_valid_i,
    /// Write datapath request ready
    output logic w_dp_ready_o,

    /// Write datapath response
    output w_dp_rsp_t w_dp_rsp_o,
    /// Write datapath response valid
    output logic w_dp_valid_o,
    /// Write datapath response valid
    input  logic w_dp_ready_i,

    /// Read meta request
    input  axi_ar_chan_t ar_req_i,
    /// Read meta request valid
    input  logic ar_valid_i,
    /// Read meta request ready
    output logic ar_ready_o,

    /// Write meta request
    input  axi_aw_chan_t aw_req_i,
    /// Write meta request valid
    input  logic aw_valid_i,
    /// Write meta request ready
    output logic aw_ready_o,

    /// Datapath poison signal
    input  logic dp_poison_i,

    /// Read part of the datapath is busy
    output logic r_dp_busy_o,
    /// Write part of the datapath is busy
    output logic w_dp_busy_o,
    /// Buffer is busy
    output logic buffer_busy_o
);

    /// Stobe width
    localparam int unsigned StrbWidth   = DataWidth / 8;

    /// Data type
    typedef logic [DataWidth-1:0] data_t;
    /// Offset type
    typedef logic [StrbWidth-1:0] strb_t;
    /// Byte type
    typedef logic [7:0] byte_t;

    // offsets needed for masks to fill/empty buffer
    strb_t r_first_mask;
    strb_t r_last_mask;
    strb_t w_first_mask;
    strb_t w_last_mask;

    // hold one bit state: it this the first read?
    logic first_r_d, first_r_q;

    // shifted data flowing into the buffer
    byte_t [StrbWidth-1:0] buffer_in;

    // read aligned in mask. needs to be shifted together with the data before
    // it can be used to mask valid data flowing into the buffer
    strb_t read_aligned_in_mask;

    // in mask is write aligned: it is the result of the read aligned in mask
    // that is shifted together with the data in the barrel shifter
    strb_t mask_in;

    // inbound control signals to the read buffer: controlled by the read process
    strb_t buffer_in_valid;
    strb_t buffer_in_ready;
    logic  in_valid;
    logic  in_ready;

    // corresponds to the strobe: the write aligned data that is currently valid in the buffer
    strb_t mask_out;

    // write signals: is this the first / last element in a burst?
    logic first_w;
    logic last_w;

    // aligned and coalesced data leaving the buffer
    byte_t [StrbWidth-1:0] buffer_out;

    // A temporary signal required to write the output of the buffer to before assigning it to
    // the AXI bus. This is required to be compatible with some of the Questasim Versions and some
    // of the parametrizations (e.g. DataWidth = 16)
    data_t buffer_data_masked;

    // outbound control signals of the buffer: controlled by the write process
    strb_t buffer_out_valid;
    strb_t buffer_out_ready;

    // write happens
    logic write_happening;
    // buffer is ready to write the requested data
    logic ready_to_write;
    // first transfer is possible - this signal is used to detect
    // the first write transfer in a burst
    logic first_possible;
    // buffer is completely empty
    logic buffer_clean;

    // we require a counter to hold the current beat in the burst
    logic [7:0] w_num_beats_d, w_num_beats_q;
    logic       w_cnt_valid_d, w_cnt_valid_q;


    //--------------------------------------
    // Mask pre-calculation
    //--------------------------------------
    // in contiguous transfers that are unaligned, there will be some
    // invalid bytes at the beginning and the end of the stream
    // example: 25B in 64 bit system
    //  iiiivvvv|vvvvvvvv|vvvvvvvv|vvvvviii
    // first msk|----full mask----|last msk

    // read align masks
    assign r_first_mask = '1 << r_dp_req_i.offset;
    assign r_last_mask  = '1 >> (StrbWidth - r_dp_req_i.tailer);

    // write align masks
    assign w_first_mask = '1 << w_dp_req_i.offset;
    assign w_last_mask  = '1 >> (StrbWidth - w_dp_req_i.tailer);


    //--------------------------------------
    // Read meta channel
    //--------------------------------------
    // connect the ar requests to the AXI bus
    assign axi_req_o.ar       = ar_req_i;
    assign axi_req_o.ar_valid = ar_valid_i;
    assign ar_ready_o         = axi_rsp_i.ar_ready;


    //--------------------------------------
    // In mask generation
    //--------------------------------------
    // in the case of unaligned reads -> not all data is valid
    always_comb begin : proc_in_mask_generator
        // default case: all ones
        read_aligned_in_mask = '1;
        // is first word: some bytes at the beginning may be invalid
        read_aligned_in_mask = first_r_q ?
            read_aligned_in_mask & r_first_mask : read_aligned_in_mask;
        // is last word in write burst: some bytes at the end may be invalid
        if (r_dp_req_i.tailer != '0) begin
            read_aligned_in_mask = axi_rsp_i.r.last ?
                read_aligned_in_mask & r_last_mask : read_aligned_in_mask;
        end
    end


    //--------------------------------------
    // Barrel shifter
    //--------------------------------------
    // data arrives in chunks of length DATA_WDITH, the buffer will be filled with
    // the realigned data. StrbWidth bytes will be inserted starting from the
    // provided address, overflows will naturally wrap

    // a barrel shifter is a concatenation of the same array with twice and a normal
    // shift. Optimized for Synopsys DesignWare.
    assign buffer_in = {axi_rsp_i.r.data, axi_rsp_i.r.data} >> (r_dp_req_i.shift * 8);
    assign mask_in   = {read_aligned_in_mask, read_aligned_in_mask}  >> r_dp_req_i.shift;


    //--------------------------------------
    // Read control
    //--------------------------------------
    // controls the next state of the read flag
    always_comb begin : proc_first_read
        // sticky is first bit for read
        if (!axi_rsp_i.r.last & axi_rsp_i.r_valid & axi_req_o.r_ready) begin
            // new transfer has started
            first_r_d = 1'b0;
        end else if (axi_rsp_i.r.last & axi_rsp_i.r_valid & axi_req_o.r_ready) begin
            // finish read burst
            first_r_d = 1'b1;
        end else begin
            // no change
            first_r_d = first_r_q;
        end
    end

    // the buffer can be pushed to if all the masked FIFO buffers (mask_in) are ready.
    assign in_ready = &(buffer_in_ready | ~mask_in);
    // the read can accept data if the buffer is ready and the response channel is ready
    assign axi_req_o.r_ready = in_ready & r_dp_ready_i;

    // once valid data is applied, it can be pushed in all the selected (mask_in) buffers
    // be sure the response channel is ready
    assign in_valid        = axi_rsp_i.r_valid & in_ready & r_dp_ready_i;
    assign buffer_in_valid = in_valid ? mask_in : '0;

    // r_dp_ready_o is triggered by the last element arriving from the read
    assign r_dp_ready_o = r_dp_valid_i & r_dp_ready_i &
                          axi_rsp_i.r.last & axi_rsp_i.r_valid & in_ready;

    // connect r_dp response payload
    assign r_dp_rsp_o.resp  = axi_rsp_i.r.resp;
    assign r_dp_rsp_o.last  = axi_rsp_i.r.last;
    assign r_dp_rsp_o.first = first_r_q;

    // r_dp_valid_o is triggered once the last element is here or an error occurs
    assign r_dp_valid_o = axi_rsp_i.r_valid & in_ready & (axi_rsp_i.r.last | (|axi_rsp_i.r.resp));


    //--------------------------------------
    // Write meta channel
    //--------------------------------------
    // connect the aw requests to the AXI bus
    assign axi_req_o.aw       = aw_req_i;
    assign axi_req_o.aw_valid = aw_valid_i;
    assign aw_ready_o         = axi_rsp_i.aw_ready;


    //--------------------------------------
    // Out mask generation -> (wstrb mask)
    //--------------------------------------
    // only pop the data actually needed for write from the buffer,
    // determine valid data to pop by calculation the wstrb
    always_comb begin : proc_out_mask_generator
        // default case: all ones
        mask_out = '1;
        // is first word: some bytes at the beginning may be invalid
        mask_out = first_w ? (mask_out & w_first_mask) : mask_out;
        // is last word in write burst: some bytes at the end may be invalid
        if (w_dp_req_i.tailer != '0 & last_w) begin
            mask_out = mask_out & w_last_mask;
        end
    end


    //--------------------------------------
    // Write control
    //--------------------------------------
    // write is decoupled from read, due to misalignment in the read/write
    // addresses, page crossing can be encountered at any time.
    // To handle this efficiently, a 2-to-1 or 1-to-2 mapping of r/w beats
    // is required. The write unit needs to keep track of progress through
    // a counter and cannot use `r last` for that.

    // Once buffer contains a full line -> all FIFOs are non-empty push it out.

    // all elements needed (defined by the mask) are in the buffer and the buffer is non-empty
    assign ready_to_write = ((buffer_out_valid & mask_out) == mask_out) & (buffer_out_valid != '0);

    // data needed by the first mask is available in the buffer -> r_first happened for sure
    // this signal can be high during a transfer as well, it needs to be masked
    assign first_possible = ((buffer_out_valid & w_first_mask) == w_first_mask) &
                            (buffer_out_valid != '0);

    // the buffer is completely empty and idle
    assign buffer_clean = &(~buffer_out_valid);

    // write happening: both the bus (w_ready) and the buffer (ready_to_write) is high
    assign write_happening = ready_to_write & axi_rsp_i.w_ready;

    // the main buffer is conditionally to the write mask popped
    assign buffer_out_ready = write_happening ? mask_out : '0;

    // signal the bus that we are ready
    assign axi_req_o.w_valid = ready_to_write;

    // connect data and strobe either directly or mask invalid data
    if (MaskInvalidData) begin : gen_mask_invalid_data

        // always_comb process implements masking of invalid data
        always_comb begin : proc_mask
            // defaults
            axi_req_o.w.data   = '0;
            axi_req_o.w.strb   = '0;
            buffer_data_masked = '0;
            // control the write to the bus apply data to the bus only if data should be written
            if (ready_to_write == 1'b1 & !dp_poison_i) begin
                // assign data from buffers, mask non valid entries
                for (int i = 0; i < StrbWidth; i++) begin
                    buffer_data_masked[i*8 +: 8] = mask_out[i] ? buffer_out[i] : 8'b0;
                end
                // assign the output
                axi_req_o.w.data = buffer_data_masked;
                // assign the out mask to the strobe
                axi_req_o.w.strb = mask_out;
            end
        end

    end else begin : gen_direct_connect
        // not used signal
        assign buffer_data_masked = '0;
        // simpler: direct connection
        assign axi_req_o.w.data   = buffer_out;
        assign axi_req_o.w.strb   = dp_poison_i ? '0 : mask_out;
    end

    // the w last signal should only be applied to the bus if an actual transfer happens
    assign axi_req_o.w.last = last_w & ready_to_write;

    // we are ready for the next transfer internally, once the w last signal is applied
    assign w_dp_ready_o = last_w & write_happening;

    // the write process: keeps track of remaining beats in burst
    always_comb begin : proc_write_control
        // defaults:
        // beat counter
        w_num_beats_d   = w_num_beats_q;
        w_cnt_valid_d   = w_cnt_valid_q;
        // mask control
        first_w      = 1'b0;
        last_w       = 1'b0;

        // differentiate between the burst and non-burst case. If a transfer
        // consists just of one beat the counters are disabled
        if (w_dp_req_i.is_single) begin
            // in the single case the transfer is both first and last.
            first_w = 1'b1;
            last_w  = 1'b1;

        // in the burst case the counters are needed to keep track of the progress of sending
        // beats. The w_last_o depends on the state of the counter
        end else begin
            // first transfer happens as soon as a) the buffer is ready for a first transfer and b)
            // the counter is currently invalid
            first_w = first_possible & ~w_cnt_valid_q;

            // last happens as soon as a) the counter is valid and b) the counter is now down to 1
            last_w  = w_cnt_valid_q & (w_num_beats_q == 8'h01);

            // load the counter with data in a first cycle, only modifying state if bus is ready
            if (first_w && write_happening) begin
                w_num_beats_d = w_dp_req_i.num_beats;
                w_cnt_valid_d = 1'b1;
            end

            // if we hit the last element, invalidate the counter, only modifying state
            // if bus is ready
            if (last_w && write_happening) begin
                w_cnt_valid_d = 1'b0;
            end

            // count down the beats if the counter is valid and valid data is written to the bus
            if (w_cnt_valid_q && write_happening) w_num_beats_d = w_num_beats_q - 8'h01;
        end
    end


    //--------------------------------------
    // Write response
    //--------------------------------------
    // connect w_dp response payload
    assign w_dp_rsp_o.resp = axi_rsp_i.b.resp;
    assign w_dp_rsp_o.user = axi_rsp_i.b.user;

    // w_dp_valid_o is triggered once the write answer is here
    assign w_dp_valid_o = axi_rsp_i.b_valid;

    // create back pressure on the b channel if the higher parts of the DMA cannot accept more
    // write responses
    assign axi_req_o.b_ready = w_dp_ready_i;


    //--------------------------------------
    // Write user signals
    //--------------------------------------
    // in the default implementation: no need for the write user signals
    assign axi_req_o.w.user = '0;


    //--------------------------------------
    // Buffer
    //--------------------------------------
    idma_buffer #(
        .BufferDepth   ( BufferDepth   ),
        .StrbWidth     ( StrbWidth     ),
        .PrintFifoInfo ( PrintFifoInfo ),
        .strb_t        ( strb_t        ),
        .byte_t        ( byte_t        )
    ) i_idma_buffer (
        .clk_i,
        .rst_ni,
        .testmode_i,
        .data_i      ( buffer_in        ),
        .valid_i     ( buffer_in_valid  ),
        .ready_o     ( buffer_in_ready  ),
        .data_o      ( buffer_out       ),
        .valid_o     ( buffer_out_valid ),
        .ready_i     ( buffer_out_ready )
    );


    //--------------------------------------
    // Module Control
    //--------------------------------------
    assign r_dp_busy_o   = r_dp_valid_i | r_dp_ready_o;
    assign w_dp_busy_o   = w_dp_valid_i | w_dp_ready_o;
    assign buffer_busy_o = !buffer_clean;


    //--------------------------------------
    // State
    //--------------------------------------
    `FF(first_r_q,     first_r_d,     '1, clk_i, rst_ni)
    `FF(w_cnt_valid_q, w_cnt_valid_d, '0, clk_i, rst_ni)
    `FF(w_num_beats_q, w_num_beats_d, '0, clk_i, rst_ni)

endmodule : idma_axi_transport_layer

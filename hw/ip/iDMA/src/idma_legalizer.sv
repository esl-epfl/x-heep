// Copyright 2022 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Thomas Benz <tbenz@ethz.ch>

`include "common_cells/registers.svh"
`include "common_cells/assertions.svh"

/// Legalizes a generic 1D transfer according to the rules given by the
/// AXI4 protocol. Bursts are cut at 4kiB boundaries and are a maximum of
/// 256 beats long.
module idma_legalizer #(
    /// Data width
    parameter int unsigned DataWidth = 32'd16,
    /// Address width
    parameter int unsigned AddrWidth = 32'd24,
    /// 1D iDMA request type:
    /// - `length`: the length of the transfer in bytes
    /// - `*_addr`: the source / target byte addresses of the transfer
    /// - `opt`: the options field
    parameter type idma_req_t = logic,
    /// Read request type
    parameter type idma_r_req_t = logic,
    /// Write request type
    parameter type idma_w_req_t = logic,
    /// Mutable transfer type
    parameter type idma_mut_tf_t = logic,
    /// Mutable options type
    parameter type idma_mut_tf_opt_t = logic
)(
    /// Clock
    input  logic clk_i,
    /// Asynchronous reset, active low
    input  logic rst_ni,

    /// 1D request
    input  idma_req_t req_i,
    /// 1D request valid
    input  logic valid_i,
    /// 1D request ready
    output logic ready_o,

    /// Read request; contains datapath and meta information
    output idma_r_req_t r_req_o,
    /// Read request valid
    output logic r_valid_o,
    /// Read request ready
    input  logic r_ready_i,

    /// Write request; contains datapath and meta information
    output idma_w_req_t w_req_o,
    /// Write request valid
    output logic w_valid_o,
    /// Write request ready
    input  logic w_ready_i,

    /// Invalidate the current burst transfer, stops emission of requests
    input  logic flush_i,
    /// Kill the active 1D transfer; reload a new transfer
    input  logic kill_i,

    /// Read machine of the legalizer is busy
    output logic r_busy_o,
    /// Write machine of the legalizer is busy
    output logic w_busy_o
);

    /// Stobe width
    localparam int unsigned StrbWidth     = DataWidth / 8;
    /// Offset width
    localparam int unsigned OffsetWidth   = $clog2(StrbWidth);
    /// The size of a page in byte
    localparam int unsigned PageSize      = (256 * StrbWidth > 4096) ? 4096 :  256 * StrbWidth;
    /// The width of page offset byte addresses
    localparam int unsigned PageAddrWidth = $clog2(PageSize);

    /// Offset type
    typedef logic [  OffsetWidth-1:0] offset_t;
    /// Address type
    typedef logic [    AddrWidth-1:0] addr_t;
    /// Page address type
    typedef logic [PageAddrWidth-1:0] page_addr_t;
    /// Page length type
    typedef logic [  PageAddrWidth:0] page_len_t;


    // state: internally hold one transfer, this is mutated
    idma_mut_tf_t     r_tf_d,   r_tf_q;
    idma_mut_tf_t     w_tf_d,   w_tf_q;
    idma_mut_tf_opt_t opt_tf_d, opt_tf_q;

    // enable signals for next mutable transfer storage
    logic r_tf_ena;
    logic w_tf_ena;

    // page boundaries
    page_addr_t r_page_offset;
    page_len_t  r_num_bytes_to_pb;
    page_addr_t w_page_offset;
    page_len_t  w_num_bytes_to_pb;
    page_len_t  c_num_bytes_to_pb;

    logic [3:0] r_page_addr_width;
    logic [3:0] w_page_addr_width;
    page_len_t  r_page_size;
    page_len_t  w_page_size;

    // read process
    page_len_t r_num_bytes_possible;
    page_len_t r_num_bytes;
    offset_t   r_addr_offset;
    logic      r_done;

    // write process
    page_len_t w_num_bytes_possible;
    page_len_t w_num_bytes;
    offset_t   w_addr_offset;
    logic      w_done;


    //--------------------------------------
    // read page boundary check
    //--------------------------------------
    // calculate the page with in bits
    always_comb begin : proc_read_addr_width
        // should the "virtual" page be reduced? e.g. the transfers split into
        // smaller chunks than the AXI page size?
        r_page_addr_width = OffsetWidth + (opt_tf_q.src_reduce_len ? opt_tf_q.src_max_llen : 'd8);
        // a page can be a maximum of 4kB (12 bit)
        r_page_addr_width = r_page_addr_width > 'd12 ? 'd12 : r_page_addr_width;
    end
    // calculate the page size in byte
    assign r_page_size = (1 << r_page_addr_width);

    // this is written very confusing due to system verilog not allowing variable
    // length ranges.
    // the goal is to get 'r_tf_q.addr[PageAddrWidth-1:0]' where PageAddrWidth is
    // r_page_addr_width and dynamically changing
    always_comb begin : proc_read_range_select
        r_page_offset = '0;
        for (int i = 0; i < PageAddrWidth; i++) begin
            r_page_offset[i] = r_page_addr_width > i ? r_tf_q.addr[i] : 1'b0;
        end
    end

    // calculate the number of bytes left in the page (number of bytes until
    // we reach the page boundary (bp)
    assign r_num_bytes_to_pb = r_page_size - r_page_offset;


    //--------------------------------------
    // write page boundary check
    //--------------------------------------
    // calculate the page with in bits
    always_comb begin : proc_write_addr_width
        // should the "virtual" page be reduced? e.g. the transfers split into
        // smaller chunks than the AXI page size?
        w_page_addr_width = OffsetWidth + (opt_tf_q.dst_reduce_len ? opt_tf_q.dst_max_llen : 'd8);
        // a page can be a maximum of 4kB (12 bit)
        w_page_addr_width = w_page_addr_width > 'd12 ? 'd12 : w_page_addr_width;
    end
    // calculate the page size in byte
    assign w_page_size = (1 << w_page_addr_width);

    // this is written very confusing due to system verilog not allowing variable
    // length ranges.
    // the goal is to get 'r_tf_q.addr[PageAddrWidth-1:0]' where PageAddrWidth is
    // r_page_addr_width and dynamically changing
    always_comb begin : proc_write_range_select
        w_page_offset = '0;
        for (int i = 0; i < PageAddrWidth; i++) begin
            w_page_offset[i] = w_page_addr_width > i ? w_tf_q.addr[i] : 1'b0;
        end
    end

    // calculate the number of bytes left in the page (number of bytes until
    // we reach the page boundary (bp)
    assign w_num_bytes_to_pb = w_page_size - w_page_offset;


    //--------------------------------------
    // page boundary check
    //--------------------------------------
    // how many transfers are remaining when concerning both r/w pages?
    // take the boundary that is closer
    assign c_num_bytes_to_pb = (r_num_bytes_to_pb > w_num_bytes_to_pb) ?
                                w_num_bytes_to_pb : r_num_bytes_to_pb;


    //--------------------------------------
    // Synchronized R/W process
    //--------------------------------------
    // max num bytes readable in page
    assign r_num_bytes_possible = opt_tf_q.decouple_rw ?
                                  r_num_bytes_to_pb : c_num_bytes_to_pb;

    // max num bytes writable in page
    assign w_num_bytes_possible = opt_tf_q.decouple_rw ?
                                  w_num_bytes_to_pb : c_num_bytes_to_pb;

    // calculate the address offsets aligned to transfer sizes.
    assign r_addr_offset = r_tf_q.addr[OffsetWidth-1:0];
    assign w_addr_offset = w_tf_q.addr[OffsetWidth-1:0];

    // legalization process -> read and write is coupled together
    always_comb begin : proc_read_write_transaction

        // default: keep state
        r_tf_d   = r_tf_q;
        w_tf_d   = w_tf_q;
        opt_tf_d = opt_tf_q;

        // default: not done
        r_done = 1'b0;
        w_done = 1'b0;

        //--------------------------------------
        // Legalize read transaction
        //--------------------------------------
        // more bytes remaining than we can read
        if (r_tf_q.length > r_num_bytes_possible) begin
            r_num_bytes = r_num_bytes_possible;
            // calculate remainder
            r_tf_d.length = r_tf_q.length - r_num_bytes_possible;
            // next address
            r_tf_d.addr = r_tf_q.addr + r_num_bytes;

        // remaining bytes fit in one burst
        end else begin
            r_num_bytes = r_tf_q.length[PageAddrWidth:0];
            // finished
            r_tf_d.valid = 1'b0;
            r_done = 1'b1;
        end

        //--------------------------------------
        // Legalize write transaction
        //--------------------------------------
        // more bytes remaining than we can write
        if (w_tf_q.length > w_num_bytes_possible) begin
            w_num_bytes = w_num_bytes_possible;
            // calculate remainder
            w_tf_d.length = w_tf_q.length - w_num_bytes_possible;
            // next address
            w_tf_d.addr = w_tf_q.addr + w_num_bytes;

        // remaining bytes fit in one burst
        end else begin
            w_num_bytes = w_tf_q.length[PageAddrWidth:0];
            // finished
            w_tf_d.valid = 1'b0;
            w_done = 1'b1;
        end

        //--------------------------------------
        // Kill
        //--------------------------------------
        if (kill_i) begin
            // kill the current state
            r_tf_d =  '0;
            r_done = 1'b1;
            w_tf_d =  '0;
            w_done = 1'b1;
        end

        //--------------------------------------
        // Refill
        //--------------------------------------
        // new request is taken in if both r and w machines are ready.
        if (ready_o & valid_i) begin

            // load all three mutable objects (source, destination, option)
            // source or read
            r_tf_d = '{
                length: req_i.length,
                addr:   req_i.src_addr,
                valid:   1'b1
            };
            // destination or write
            w_tf_d = '{
                length: req_i.length,
                addr:   req_i.dst_addr,
                valid:   1'b1
            };
            // options
            opt_tf_d = '{
                shift:          req_i.src_addr[OffsetWidth-1:0] - req_i.dst_addr[OffsetWidth-1:0],
                decouple_rw:    req_i.opt.beo.decouple_rw,
                decouple_aw:    req_i.opt.beo.decouple_aw,
                src_max_llen:   req_i.opt.beo.src_max_llen,
                dst_max_llen:   req_i.opt.beo.dst_max_llen,
                src_reduce_len: req_i.opt.beo.src_reduce_len,
                dst_reduce_len: req_i.opt.beo.dst_reduce_len,
                axi_id:         req_i.opt.axi_id,
                src_axi_opt:    req_i.opt.src,
                dst_axi_opt:    req_i.opt.dst,
                super_last:     req_i.opt.last
            };
        end
    end


    //--------------------------------------
    // Connect outputs
    //--------------------------------------
    // assign the signals for the read meta channel
    assign r_req_o.ar_req = '{
        id:     opt_tf_q.axi_id,
        addr:   { r_tf_q.addr[AddrWidth-1:OffsetWidth], {{OffsetWidth}{1'b0}} },
        len:    ((r_num_bytes + r_addr_offset - 'd1) >> OffsetWidth),
        size:   axi_pkg::size_t'(OffsetWidth),
        burst:  opt_tf_q.src_axi_opt.burst,
        lock:   opt_tf_q.src_axi_opt.lock,
        cache:  opt_tf_q.src_axi_opt.cache,
        prot:   opt_tf_q.src_axi_opt.prot,
        qos:    opt_tf_q.src_axi_opt.qos,
        region: opt_tf_q.src_axi_opt.region,
        user:   '0
    };

    // assign the signals for the write meta channel
    assign w_req_o.aw_req = '{
        id:     opt_tf_q.axi_id,
        addr:   { w_tf_q.addr[AddrWidth-1:OffsetWidth], {{OffsetWidth}{1'b0}} },
        len:    ((w_num_bytes + w_addr_offset - 'd1) >> OffsetWidth),
        size:   axi_pkg::size_t'(OffsetWidth),
        burst:  opt_tf_q.dst_axi_opt.burst,
        lock:   opt_tf_q.dst_axi_opt.lock,
        cache:  opt_tf_q.dst_axi_opt.cache,
        prot:   opt_tf_q.dst_axi_opt.prot,
        qos:    opt_tf_q.dst_axi_opt.qos,
        region: opt_tf_q.dst_axi_opt.region,
        user:   '0,
        atop:   '0
    };

    // assign the signals needed to set-up the read data path
    assign r_req_o.r_dp_req = '{
        offset:      r_addr_offset,
        tailer:      OffsetWidth'(r_num_bytes + r_addr_offset),
        shift:       opt_tf_q.shift,
        decouple_aw: opt_tf_q.decouple_aw
    };

    // assign the signals needed to set-up the write data path
    assign w_req_o.w_dp_req = '{
        offset:    w_addr_offset,
        tailer:    OffsetWidth'(w_num_bytes + w_addr_offset),
        num_beats: w_req_o.aw_req.len,
        is_single: w_req_o.aw_req.len == '0
    };

    // last burst in generic 1D transfer?
    assign w_req_o.last = w_done;

    // last burst indicated by midend
    assign w_req_o.super_last = opt_tf_q.super_last;

    // assign aw decouple flag
    assign w_req_o.decouple_aw = opt_tf_q.decouple_aw;

    // busy output
    assign r_busy_o = r_tf_q.valid;
    assign w_busy_o = w_tf_q.valid;


    //--------------------------------------
    // Flow Control
    //--------------------------------------
    // only advance to next state if:
    // * rw_coupled: both machines advance
    // * rw_decoupled: either machine advances
    always_comb begin : proc_legalizer_flow_control
        if (opt_tf_q.decouple_rw) begin
            r_tf_ena  = (r_ready_i & !flush_i) | kill_i;
            w_tf_ena  = (w_ready_i & !flush_i) | kill_i;

            r_valid_o = r_tf_q.valid & r_ready_i & !flush_i;
            w_valid_o = w_tf_q.valid & w_ready_i & !flush_i;
        end else begin
            r_tf_ena  = (r_ready_i & w_ready_i & !flush_i) | kill_i;
            w_tf_ena  = (r_ready_i & w_ready_i & !flush_i) | kill_i;

            r_valid_o = r_tf_q.valid & w_ready_i & r_ready_i & !flush_i;
            w_valid_o = w_tf_q.valid & r_ready_i & w_ready_i & !flush_i;
        end
    end

    // load next idma request: if both machines are done!
    assign ready_o = r_done & w_done & r_ready_i & w_ready_i & !flush_i;


    //--------------------------------------
    // State
    //--------------------------------------
    `FF(opt_tf_q, opt_tf_d, '0, clk_i, rst_ni)
    `FFL(r_tf_q, r_tf_d, r_tf_ena, '0, clk_i, rst_ni)
    `FFL(w_tf_q, w_tf_d, w_tf_ena, '0, clk_i, rst_ni)


    //--------------------------------------
    // Assertions
    //--------------------------------------
    // only support the decomposition of incremental bursts
    `ASSERT_NEVER(OnlyIncrementalBurstsSRC, (ready_o & valid_i &
                  req_i.opt.src.burst != axi_pkg::BURST_INCR), clk_i, !rst_ni)
    `ASSERT_NEVER(OnlyIncrementalBurstsDST, (ready_o & valid_i &
                  req_i.opt.dst.burst != axi_pkg::BURST_INCR), clk_i, !rst_ni)

endmodule : idma_legalizer

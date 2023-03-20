// Copyright 2022 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Thomas Benz <tbenz@ethz.ch>

// Macros to define iDMA structs

`ifndef IDMA_TYPEDEF_SVH_
`define IDMA_TYPEDEF_SVH_

////////////////////////////////////////////////////////////////////////////////////////////////////
// iDMA Request and Response Structs
//
// Usage Example:
// `IDMA_TYPEDEF_OPTIONS_T(options_t, axi_id_t)
// `IDMA_TYPEDEF_ERR_PAYLOAD_T(err_payload_t, axi_addr_t)
// `IDMA_TYPEDEF_REQ_T(idma_req_t, tf_len_t, axi_addr_t, options_t)
// `IDMA_TYPEDEF_RSP_T(idma_rsp_t, err_payload_t)
`define IDMA_TYPEDEF_OPTIONS_T(options_t, axi_id_t)                      \
    typedef struct packed {                                              \
        axi_id_t                    axi_id;                              \
        idma_pkg::axi_options_t     src;                                 \
        idma_pkg::axi_options_t     dst;                                 \
        idma_pkg::backend_options_t beo;                                 \
        logic                       last;                                \
    } options_t;
`define IDMA_TYPEDEF_ERR_PAYLOAD_T(err_payload_t, axi_addr_t)            \
    typedef struct packed {                                              \
        axi_pkg::resp_t      cause;                                      \
        idma_pkg::err_type_t err_type;                                   \
        axi_addr_t           burst_addr;                                 \
    } err_payload_t;
`define IDMA_TYPEDEF_REQ_T(idma_req_t, tf_len_t, axi_addr_t, options_t)  \
    typedef struct packed {                                              \
        tf_len_t   length;                                               \
        axi_addr_t src_addr;                                             \
        axi_addr_t dst_addr;                                             \
        options_t  opt;                                                  \
    } idma_req_t;
`define IDMA_TYPEDEF_RSP_T(idma_rsp_t, err_payload_t)                    \
    typedef struct packed {                                              \
        logic         last;                                              \
        logic         error;                                             \
        err_payload_t pld;                                               \
    } idma_rsp_t;
////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////
// iDMA Full Request and Response Structs
//
// Usage Example:
// `IDMA_TYPEDEF_FULL_REQ_T(idma_req_t, axi_id_t, axi_addr_t, tf_len_t)
// `IDMA_TYPEDEF_FULL_RSP_T(idma_rsp_t, axi_addr_t)
`define IDMA_TYPEDEF_FULL_REQ_T(idma_req_t, axi_id_t, axi_addr_t, tf_len_t) \
    `IDMA_TYPEDEF_OPTIONS_T(options_t, axi_id_t)                            \
    `IDMA_TYPEDEF_REQ_T(idma_req_t, tf_len_t, axi_addr_t, options_t)
`define IDMA_TYPEDEF_FULL_RSP_T(idma_rsp_t, axi_addr_t)                     \
    `IDMA_TYPEDEF_ERR_PAYLOAD_T(err_payload_t, axi_addr_t)                  \
    `IDMA_TYPEDEF_RSP_T(idma_rsp_t, err_payload_t)
////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////
// iDMA n-dimensional Request Struct
//
// Usage Example:
// `IDMA_TYPEDEF_D_REQ_T(idma_d_req_t, reps_t, strides_t)
// `IDMA_TYPEDEF_ND_REQ_T(idma_nd_req_t, idma_req_t, idma_d_req_t)
`define IDMA_TYPEDEF_D_REQ_T(idma_d_req_t, reps_t, strides_t)            \
    typedef struct packed {                                              \
        reps_t    reps;                                                  \
        strides_t src_strides;                                           \
        strides_t dst_strides;                                           \
    } idma_d_req_t;
`define IDMA_TYPEDEF_ND_REQ_T(idma_nd_req_t, idma_req_t, idma_d_req_t)   \
    typedef struct packed {                                              \
        idma_req_t                burst_req;                             \
        idma_d_req_t [NumDim-2:0] d_req;                                 \
    } idma_nd_req_t;
////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////
// iDMA Full n-dimensional Request Struct
//
// Usage Example:
// `IDMA_TYPEDEF_FULL_ND_REQ_T(idma_nd_req_t, idma_req_t, reps_t, strides_t)
`define IDMA_TYPEDEF_FULL_ND_REQ_T(idma_nd_req_t, idma_req_t, reps_t, strides_t) \
    `IDMA_TYPEDEF_D_REQ_T(idma_d_req_t, reps_t, strides_t)                       \
    `IDMA_TYPEDEF_ND_REQ_T(idma_nd_req_t, idma_req_t, idma_d_req_t)
////////////////////////////////////////////////////////////////////////////////////////////////////

`endif

// Copyright 2023 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51

package hyperbus_pkg;

    // Maximal burst size: 2^8 1024-bit words as 16-bit words (plus one as not decremented)
    localparam unsigned HyperBurstWidth = 8 + $clog2(1024/16) + 1;
    typedef logic [HyperBurstWidth-1:0] hyper_blen_t;

    // configuration type
    typedef struct packed {
        logic [3:0]     t_latency_access;
        logic           en_latency_additional;
        logic [15:0]    t_burst_max;
        logic [3:0]     t_read_write_recovery;
        logic [3:0]     t_rx_clk_delay;
        logic [3:0]     t_tx_clk_delay;
        logic [4:0]     address_mask_msb;
        logic           address_space;
        logic           phys_in_use;
        logic           which_phy;
        logic [3:0]     t_csh_cycles; //add an configurable Tcsh for high freq operation(200MHz Hyperram)
    } hyper_cfg_t;

    typedef struct packed {
        logic           write;     // transaction is a write
        hyper_blen_t    burst;
        logic           burst_type;
        logic           address_space;
        logic [31:0]    address;
    } hyper_tf_t;

    typedef struct packed {
           logic [15:0]    data;
           logic           last;
           logic           error;
    } phy_rx_t;

    typedef enum logic[3:0] {
        Startup,
        Idle,
        SendCA,
        WaitLatAccess,
        Read,
        Write,
        WaitXfer,
        WaitRWR
    } hyper_phy_state_t;

    typedef struct packed {
        logic           write;
        logic           addr_space;
        logic           burst_type;
        logic [28:0]    addr_upper;
        logic [12:0]    reserved;
        logic [2:0]     addr_lower;
    } hyper_phy_ca_t;


    // Register reset values
    function hyper_cfg_t gen_RstCfg(input int unsigned NumPhys, input int unsigned MinFreqMhz = 100);
        // MinFreqMHz = 100 is the spec conform version and should not be changed
        // It can be lowered if this frequency is not reachable in operation (may not with with certain HyperBus devices)
        // >200 is outside the spec and is unlikely to work with any HyperBus devices
        automatic hyper_cfg_t cfg = hyper_cfg_t'{
            t_latency_access:           'h6,
            en_latency_additional:      'b0,
            t_burst_max:                ((MinFreqMhz*35)/10), // t_{csm}: At lowest legal clock (100 MHz) 3.5us (0.5us safety margin)
            t_read_write_recovery:      'h6,
            t_rx_clk_delay:             'h8,
            t_tx_clk_delay:             'h8,
            address_mask_msb:           'd25,                // 26 bit addresses = 2^6*2^20B == 64 MB per chip (biggest availale as of now)
            address_space:              'b0,
            phys_in_use:                NumPhys-1,
            which_phy:                  NumPhys-1,
            t_csh_cycles:               'h1
        };

        return cfg;
    endfunction

endpackage

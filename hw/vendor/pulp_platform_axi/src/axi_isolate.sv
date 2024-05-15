// Copyright (c) 2019-2020 ETH Zurich, University of Bologna
//
// Copyright and related rights are licensed under the Solderpad Hardware
// License, Version 0.51 (the "License"); you may not use this file except in
// compliance with the License.  You may obtain a copy of the License at
// http://solderpad.org/licenses/SHL-0.51. Unless required by applicable law
// or agreed to in writing, software, hardware and materials distributed under
// this License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.
//
// Authors:
// - Wolfgang Roenninger <wroennin@iis.ee.ethz.ch>
// - Andreas Kurth <akurth@iis.ee.ethz.ch>

`include "axi/typedef.svh"
`include "common_cells/registers.svh"

/// This module can isolate the AXI4+ATOPs bus on the master port from the slave port.  When the
/// isolation is not active, the two ports are directly connected.
///
/// This module counts how many open transactions are currently in flight on the read and write
/// channels.  It is further capable of tracking the amount of open atomic transactions with read
/// responses.
///
/// The isolation interface has two signals: `isolate_i` and `isolated_o`.  When `isolate_i` is
/// asserted, all open transactions are gracefully terminated.  When no transactions are in flight
/// anymore, the `isolated_o` output is asserted.  As long as `isolated_o` is asserted, all output
/// signals in `mst_req_o` are silenced to `'0`.  When isolated, new transactions initiated on the
/// slave port are stalled until the isolation is terminated by deasserting `isolate_i`.
///
/// ## Response
///
/// If the `TerminateTransaction` parameter is set to `1'b1`, the module will return response errors
/// in case there is an incoming transaction while the module isolates.  The data returned on the
/// bus is `1501A7ED` (hexspeak for isolated).
///
/// If `TerminateTransaction` is set to `1'b0`, the transaction will block indefinitely until the
/// module is de-isolated again.
module axi_isolate #(
  /// Maximum number of pending requests per channel
  parameter int unsigned NumPending = 32'd16,
  /// Gracefully terminate all incoming transactions in case of isolation by returning proper error
  /// responses.
  parameter bit TerminateTransaction = 1'b0,
  /// Support atomic operations (ATOPs)
  parameter bit AtopSupport = 1'b1,
  /// Address width of all AXI4+ATOP ports
  parameter int unsigned AxiAddrWidth = 32'd0,
  /// Data width of all AXI4+ATOP ports
  parameter int unsigned AxiDataWidth = 32'd0,
  /// ID width of all AXI4+ATOP ports
  parameter int unsigned AxiIdWidth = 32'd0,
  /// User signal width of all AXI4+ATOP ports
  parameter int unsigned AxiUserWidth = 32'd0,
  /// Request struct type of all AXI4+ATOP ports
  parameter type         axi_req_t  = logic,
  /// Response struct type of all AXI4+ATOP ports
  parameter type         axi_resp_t = logic
) (
  /// Rising-edge clock of all ports
  input  logic      clk_i,
  /// Asynchronous reset, active low
  input  logic      rst_ni,
  /// Slave port request
  input  axi_req_t  slv_req_i,
  /// Slave port response
  output axi_resp_t slv_resp_o,
  /// Master port request
  output axi_req_t  mst_req_o,
  /// Master port response
  input  axi_resp_t mst_resp_i,
  /// Isolate master port from slave port
  input  logic      isolate_i,
  /// Master port is isolated from slave port
  output logic      isolated_o
);

  typedef logic [AxiIdWidth-1:0]     id_t;
  typedef logic [AxiAddrWidth-1:0]   addr_t;
  typedef logic [AxiDataWidth-1:0]   data_t;
  typedef logic [AxiDataWidth/8-1:0] strb_t;
  typedef logic [AxiUserWidth-1:0]   user_t;

  `AXI_TYPEDEF_AW_CHAN_T(aw_chan_t, addr_t, id_t, user_t)
  `AXI_TYPEDEF_W_CHAN_T(w_chan_t, data_t, strb_t, user_t)
  `AXI_TYPEDEF_B_CHAN_T(b_chan_t, id_t, user_t)
  `AXI_TYPEDEF_AR_CHAN_T(ar_chan_t, addr_t, id_t, user_t)
  `AXI_TYPEDEF_R_CHAN_T(r_chan_t, data_t, id_t, user_t)

  axi_req_t [1:0]   demux_req;
  axi_resp_t [1:0]  demux_rsp;

  if (TerminateTransaction) begin
    axi_demux #(
      .AxiIdWidth     ( AxiIdWidth  ),
      .AtopSupport    ( AtopSupport ),
      .aw_chan_t      ( aw_chan_t   ),
      .w_chan_t       ( w_chan_t    ),
      .b_chan_t       ( b_chan_t    ),
      .ar_chan_t      ( ar_chan_t   ),
      .r_chan_t       ( r_chan_t    ),
      .axi_req_t      ( axi_req_t   ),
      .axi_resp_t     ( axi_resp_t  ),
      .NoMstPorts     ( 2           ),
      .MaxTrans       ( NumPending  ),
      // We don't need many bits here as the common case will be to go for the pass-through.
      .AxiLookBits    ( 1           ),
      .UniqueIds      ( 1'b0        ),
      .FallThrough    ( 1'b1        ),
      .SpillAw        ( 1'b0        ),
      .SpillW         ( 1'b0        ),
      .SpillB         ( 1'b0        ),
      .SpillAr        ( 1'b0        ),
      .SpillR         ( 1'b0        )
    ) i_axi_demux (
      .clk_i,
      .rst_ni,
      .test_i          ( 1'b0       ),
      .slv_req_i,
      .slv_aw_select_i ( isolated_o ),
      .slv_ar_select_i ( isolated_o ),
      .slv_resp_o,
      .mst_reqs_o      ( demux_req ),
      .mst_resps_i     ( demux_rsp )
    );

    axi_err_slv #(
      .AxiIdWidth  ( AxiIdWidth           ),
      .axi_req_t   ( axi_req_t            ),
      .axi_resp_t  ( axi_resp_t           ),
      .Resp        ( axi_pkg::RESP_DECERR ),
      .RespData    ( 'h1501A7ED           ),
      .ATOPs       ( AtopSupport          ),
      .MaxTrans    ( 1                    )
    ) i_axi_err_slv (
      .clk_i,
      .rst_ni,
      .test_i     ( 1'b0         ),
      .slv_req_i  ( demux_req[1] ),
      .slv_resp_o ( demux_rsp[1] )
    );
  end else begin
    assign demux_req[0] = slv_req_i;
    assign slv_resp_o = demux_rsp[0];
  end

  axi_isolate_inner #(
    .NumPending ( NumPending  ),
    .axi_req_t  ( axi_req_t   ),
    .axi_resp_t ( axi_resp_t  )
  ) i_axi_isolate (
    .clk_i,
    .rst_ni,
    .slv_req_i  ( demux_req[0] ),
    .slv_resp_o ( demux_rsp[0] ),
    .mst_req_o,
    .mst_resp_i,
    .isolate_i,
    .isolated_o
  );
endmodule




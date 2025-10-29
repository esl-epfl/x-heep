// Copyright 2018 ETH Zurich and University of Bologna.
// Copyright and related rights are licensed under the Solderpad Hardware
// License, Version 0.51 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://solderpad.org/licenses/SHL-0.51. Unless required by applicable law
// or agreed to in writing, software, hardware and materials distributed under
// this License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

// Davide Rossi <davide.rossi@unibo.it>

module per2axi
#(
   parameter NB_CORES       = 4,
   parameter PER_ADDR_WIDTH = 32,
   parameter PER_ID_WIDTH   = 5,
   parameter AXI_ADDR_WIDTH = 32,
   parameter AXI_DATA_WIDTH = 64,
   parameter AXI_USER_WIDTH = 6,
   parameter AXI_ID_WIDTH   = 3,
   parameter AXI_STRB_WIDTH = AXI_DATA_WIDTH/8
)
(
   input  logic                      clk_i,
   input  logic                      rst_ni,
   input  logic                      test_en_i,
   
   // PERIPHERAL INTERCONNECT SLAVE
   //***************************************
   //REQUEST CHANNEL
   input  logic                      per_slave_req_i,
   input  logic [PER_ADDR_WIDTH-1:0] per_slave_add_i,
   input  logic                      per_slave_we_i,
   input  logic [31:0]               per_slave_wdata_i,
   input  logic [3:0]                per_slave_be_i,
   input  logic [PER_ID_WIDTH-1:0]   per_slave_id_i,
   output logic                      per_slave_gnt_o,
   
   //RESPONSE CHANNEL
   output logic                      per_slave_r_valid_o,
   output logic                      per_slave_r_opc_o,
   output logic [PER_ID_WIDTH-1:0]   per_slave_r_id_o,
   output logic [31:0]               per_slave_r_rdata_o,

   // AXI4 MASTER
   //***************************************
   // WRITE ADDRESS CHANNEL
   output logic                      axi_master_aw_valid_o,
   output logic [AXI_ADDR_WIDTH-1:0] axi_master_aw_addr_o,
   output logic [2:0]                axi_master_aw_prot_o,
   output logic [3:0]                axi_master_aw_region_o,
   output logic [7:0]                axi_master_aw_len_o,
   output logic [2:0]                axi_master_aw_size_o,
   output logic [1:0]                axi_master_aw_burst_o,
   output logic                      axi_master_aw_lock_o,
   output logic [3:0]                axi_master_aw_cache_o,
   output logic [3:0]                axi_master_aw_qos_o,
   output logic [AXI_ID_WIDTH-1:0]   axi_master_aw_id_o,
   output logic [AXI_USER_WIDTH-1:0] axi_master_aw_user_o,
   input  logic                      axi_master_aw_ready_i,

   // READ ADDRESS CHANNEL
   output logic                      axi_master_ar_valid_o,
   output logic [AXI_ADDR_WIDTH-1:0] axi_master_ar_addr_o,
   output logic [2:0]                axi_master_ar_prot_o,
   output logic [3:0]                axi_master_ar_region_o,
   output logic [7:0]                axi_master_ar_len_o,
   output logic [2:0]                axi_master_ar_size_o,
   output logic [1:0]                axi_master_ar_burst_o,
   output logic                      axi_master_ar_lock_o,
   output logic [3:0]                axi_master_ar_cache_o,
   output logic [3:0]                axi_master_ar_qos_o,
   output logic [AXI_ID_WIDTH-1:0]   axi_master_ar_id_o,
   output logic [AXI_USER_WIDTH-1:0] axi_master_ar_user_o,
   input  logic                      axi_master_ar_ready_i,

   // WRITE DATA CHANNEL
   output logic                      axi_master_w_valid_o,
   output logic [AXI_DATA_WIDTH-1:0] axi_master_w_data_o,
   output logic [AXI_STRB_WIDTH-1:0] axi_master_w_strb_o,
   output logic [AXI_USER_WIDTH-1:0] axi_master_w_user_o,
   output logic                      axi_master_w_last_o,
   input  logic                      axi_master_w_ready_i,

   // READ DATA CHANNEL
   input  logic                      axi_master_r_valid_i,
   input  logic [AXI_DATA_WIDTH-1:0] axi_master_r_data_i,
   input  logic [1:0]                axi_master_r_resp_i,
   input  logic                      axi_master_r_last_i,
   input  logic [AXI_ID_WIDTH-1:0]   axi_master_r_id_i,
   input  logic [AXI_USER_WIDTH-1:0] axi_master_r_user_i,
   output logic                      axi_master_r_ready_o,

   // WRITE RESPONSE CHANNEL
   input  logic                      axi_master_b_valid_i,
   input  logic [1:0]                axi_master_b_resp_i,
   input  logic [AXI_ID_WIDTH-1:0]   axi_master_b_id_i,
   input  logic [AXI_USER_WIDTH-1:0] axi_master_b_user_i,
   output logic                      axi_master_b_ready_o,

   // BUSY SIGNAL
   output logic                      busy_o
   );

   // SIGNAL DECLARATION
   logic                              s_aw_valid;
   logic [AXI_ADDR_WIDTH-1:0]         s_aw_addr;
   logic [2:0]                        s_aw_prot;
   logic [3:0]                        s_aw_region;
   logic [7:0]                        s_aw_len;
   logic [2:0]                        s_aw_size;
   logic [1:0]                        s_aw_burst;
   logic                              s_aw_lock;
   logic [3:0]                        s_aw_cache;
   logic [3:0]                        s_aw_qos;
   logic [AXI_ID_WIDTH-1:0]           s_aw_id;
   logic [AXI_USER_WIDTH-1:0]         s_aw_user;
   logic                              s_aw_ready;

   logic                              s_ar_valid;
   logic [AXI_ADDR_WIDTH-1:0]         s_ar_addr;
   logic [2:0]                        s_ar_prot;
   logic [3:0]                        s_ar_region;
   logic [7:0]                        s_ar_len;
   logic [2:0]                        s_ar_size;
   logic [1:0]                        s_ar_burst;
   logic                              s_ar_lock;
   logic [3:0]                        s_ar_cache;
   logic [3:0]                        s_ar_qos;
   logic [AXI_ID_WIDTH-1:0]           s_ar_id;
   logic [AXI_USER_WIDTH-1:0]         s_ar_user;
   logic                              s_ar_ready;

   logic                              s_w_valid;
   logic [AXI_DATA_WIDTH-1:0]         s_w_data;
   logic [AXI_STRB_WIDTH-1:0]         s_w_strb;
   logic [AXI_USER_WIDTH-1:0]         s_w_user;
   logic                              s_w_last;
   logic                              s_w_ready;

   logic                              s_r_valid;
   logic [AXI_DATA_WIDTH-1:0]         s_r_data;
   logic [1:0]                        s_r_resp;
   logic                              s_r_last;
   logic [AXI_ID_WIDTH-1:0]           s_r_id;
   logic [AXI_USER_WIDTH-1:0]         s_r_user;
   logic                              s_r_ready;

   logic                              s_b_valid;
   logic [1:0]                        s_b_resp;
   logic [AXI_ID_WIDTH-1:0]           s_b_id;
   logic [AXI_USER_WIDTH-1:0]         s_b_user;
   logic                              s_b_ready;

   logic                              s_trans_req;
   logic [AXI_ID_WIDTH-1:0]           s_trans_id;
   logic [AXI_ADDR_WIDTH-1:0]         s_trans_add;


   // PER2AXI REQUEST CHANNEL
   per2axi_req_channel
   #(
      .PER_ID_WIDTH     ( PER_ID_WIDTH    ),
      .PER_ADDR_WIDTH   ( PER_ADDR_WIDTH  ),
      .AXI_ADDR_WIDTH   ( AXI_ADDR_WIDTH  ),
      .AXI_DATA_WIDTH   ( AXI_DATA_WIDTH  ),
      .AXI_USER_WIDTH   ( AXI_USER_WIDTH  ),
      .AXI_ID_WIDTH     ( AXI_ID_WIDTH    )
   )
   req_channel_i
   (
      .per_slave_req_i         ( per_slave_req_i    ),
      .per_slave_add_i         ( per_slave_add_i    ),
      .per_slave_we_i          ( per_slave_we_i     ),
      .per_slave_wdata_i       ( per_slave_wdata_i  ),
      .per_slave_be_i          ( per_slave_be_i     ),
      .per_slave_id_i          ( per_slave_id_i     ),
      .per_slave_gnt_o         ( per_slave_gnt_o    ),

      .axi_master_aw_valid_o   ( s_aw_valid         ),
      .axi_master_aw_addr_o    ( s_aw_addr          ),
      .axi_master_aw_prot_o    ( s_aw_prot          ),
      .axi_master_aw_region_o  ( s_aw_region        ),
      .axi_master_aw_len_o     ( s_aw_len           ),
      .axi_master_aw_size_o    ( s_aw_size          ),
      .axi_master_aw_burst_o   ( s_aw_burst         ),
      .axi_master_aw_lock_o    ( s_aw_lock          ),
      .axi_master_aw_cache_o   ( s_aw_cache         ),
      .axi_master_aw_qos_o     ( s_aw_qos           ),
      .axi_master_aw_id_o      ( s_aw_id            ),
      .axi_master_aw_user_o    ( s_aw_user          ),
      .axi_master_aw_ready_i   ( s_aw_ready         ),

      .axi_master_ar_valid_o   ( s_ar_valid         ),
      .axi_master_ar_addr_o    ( s_ar_addr          ),
      .axi_master_ar_prot_o    ( s_ar_prot          ),
      .axi_master_ar_region_o  ( s_ar_region        ),
      .axi_master_ar_len_o     ( s_ar_len           ),
      .axi_master_ar_size_o    ( s_ar_size          ),
      .axi_master_ar_burst_o   ( s_ar_burst         ),
      .axi_master_ar_lock_o    ( s_ar_lock          ),
      .axi_master_ar_cache_o   ( s_ar_cache         ),
      .axi_master_ar_qos_o     ( s_ar_qos           ),
      .axi_master_ar_id_o      ( s_ar_id            ),
      .axi_master_ar_user_o    ( s_ar_user          ),
      .axi_master_ar_ready_i   ( s_ar_ready         ),

      .axi_master_w_valid_o    ( s_w_valid          ),
      .axi_master_w_data_o     ( s_w_data           ),
      .axi_master_w_strb_o     ( s_w_strb           ),
      .axi_master_w_user_o     ( s_w_user           ),
      .axi_master_w_last_o     ( s_w_last           ),
      .axi_master_w_ready_i    ( s_w_ready          ),

      .trans_req_o             ( s_trans_req        ),
      .trans_id_o              ( s_trans_id         ),
      .trans_add_o             ( s_trans_add        )
   );

   // PER2AXI RESPONSE CHANNEL
   per2axi_res_channel
   #(
      .PER_ID_WIDTH     ( PER_ID_WIDTH     ),
      .PER_ADDR_WIDTH   ( PER_ADDR_WIDTH   ),
      .AXI_ID_WIDTH     ( AXI_ID_WIDTH     ),
      .AXI_ADDR_WIDTH   ( AXI_ADDR_WIDTH   ),
      .AXI_DATA_WIDTH   ( AXI_DATA_WIDTH   ),
      .AXI_USER_WIDTH   ( AXI_USER_WIDTH   )
   )
   res_channel_i
   (
      .clk_i                 ( clk_i                ),
      .rst_ni                ( rst_ni               ),

      .per_slave_r_valid_o   ( per_slave_r_valid_o  ),
      .per_slave_r_opc_o     ( per_slave_r_opc_o    ),
      .per_slave_r_id_o      ( per_slave_r_id_o     ),
      .per_slave_r_rdata_o   ( per_slave_r_rdata_o  ),

      .axi_master_r_valid_i  ( s_r_valid            ),
      .axi_master_r_data_i   ( s_r_data             ),
      .axi_master_r_resp_i   ( s_r_resp             ),
      .axi_master_r_last_i   ( s_r_last             ),
      .axi_master_r_id_i     ( s_r_id               ),
      .axi_master_r_user_i   ( s_r_user             ),
      .axi_master_r_ready_o  ( s_r_ready            ),

      .axi_master_b_valid_i  ( s_b_valid            ),
      .axi_master_b_resp_i   ( s_b_resp             ),
      .axi_master_b_id_i     ( s_b_id               ),
      .axi_master_b_user_i   ( s_b_user             ),
      .axi_master_b_ready_o  ( s_b_ready            ),

      .trans_req_i           ( s_trans_req          ),
      .trans_id_i            ( s_trans_id           ),
      .trans_add_i           ( s_trans_add          )
   );

   // BUSY UNIT
   per2axi_busy_unit busy_unit_i
   (
      .clk_i      ( clk_i                             ),
      .rst_ni     ( rst_ni                            ),

      // WRITE INTERFACE
      .aw_sync_i  ( s_aw_valid & s_aw_ready           ),
      .b_sync_i   ( s_b_valid & s_b_ready             ),

      // READ INTERFACE
      .ar_sync_i  ( s_ar_valid & s_ar_ready           ),
      .r_sync_i   ( s_r_valid & s_r_ready & s_r_last  ),

      // BUSY SIGNAL
      .busy_o     ( busy_o                            )
   );

   // AXI WRITE ADDRESS CHANNEL BUFFER
   axi_aw_buffer
   #(
      .ID_WIDTH     ( AXI_ID_WIDTH    ),
      .ADDR_WIDTH   ( AXI_ADDR_WIDTH  ),
      .USER_WIDTH   ( AXI_USER_WIDTH  ),
      .BUFFER_DEPTH ( NB_CORES        )
   )
   aw_buffer_i
   (
      .clk_i           ( clk_i                  ),
      .rst_ni          ( rst_ni                 ),
      .test_en_i       ( test_en_i              ),

      .slave_valid_i   ( s_aw_valid             ),
      .slave_addr_i    ( s_aw_addr              ),
      .slave_prot_i    ( s_aw_prot              ),
      .slave_region_i  ( s_aw_region            ),
      .slave_len_i     ( s_aw_len               ),
      .slave_size_i    ( s_aw_size              ),
      .slave_burst_i   ( s_aw_burst             ),
      .slave_lock_i    ( s_aw_lock              ),
      .slave_cache_i   ( s_aw_cache             ),
      .slave_qos_i     ( s_aw_qos               ),
      .slave_id_i      ( s_aw_id                ),
      .slave_user_i    ( s_aw_user              ),
      .slave_ready_o   ( s_aw_ready             ),

      .master_valid_o  ( axi_master_aw_valid_o  ),
      .master_addr_o   ( axi_master_aw_addr_o   ),
      .master_prot_o   ( axi_master_aw_prot_o   ),
      .master_region_o ( axi_master_aw_region_o ),
      .master_len_o    ( axi_master_aw_len_o    ),
      .master_size_o   ( axi_master_aw_size_o   ),
      .master_burst_o  ( axi_master_aw_burst_o  ),
      .master_lock_o   ( axi_master_aw_lock_o   ),
      .master_cache_o  ( axi_master_aw_cache_o  ),
      .master_qos_o    ( axi_master_aw_qos_o    ),
      .master_id_o     ( axi_master_aw_id_o     ),
      .master_user_o   ( axi_master_aw_user_o   ),
      .master_ready_i  ( axi_master_aw_ready_i  )
   );

   // AXI READ ADDRESS CHANNEL BUFFER
   axi_ar_buffer
   #(
      .ID_WIDTH         ( AXI_ID_WIDTH            ),
      .ADDR_WIDTH       ( AXI_ADDR_WIDTH          ),
      .USER_WIDTH       ( AXI_USER_WIDTH          ),
      .BUFFER_DEPTH     ( NB_CORES                )
   )
   ar_buffer_i
   (
      .clk_i            ( clk_i                   ),
      .rst_ni           ( rst_ni                  ),
      .test_en_i        ( test_en_i               ),

      .slave_valid_i    ( s_ar_valid              ),
      .slave_addr_i     ( s_ar_addr               ),
      .slave_prot_i     ( s_ar_prot               ),
      .slave_region_i   ( s_ar_region             ),
      .slave_len_i      ( s_ar_len                ),
      .slave_size_i     ( s_ar_size               ),
      .slave_burst_i    ( s_ar_burst              ),
      .slave_lock_i     ( s_ar_lock               ),
      .slave_cache_i    ( s_ar_cache              ),
      .slave_qos_i      ( s_ar_qos                ),
      .slave_id_i       ( s_ar_id                 ),
      .slave_user_i     ( s_ar_user               ),
      .slave_ready_o    ( s_ar_ready              ),

      .master_valid_o   ( axi_master_ar_valid_o   ),
      .master_addr_o    ( axi_master_ar_addr_o    ),
      .master_prot_o    ( axi_master_ar_prot_o    ),
      .master_region_o  ( axi_master_ar_region_o  ),
      .master_len_o     ( axi_master_ar_len_o     ),
      .master_size_o    ( axi_master_ar_size_o    ),
      .master_burst_o   ( axi_master_ar_burst_o   ),
      .master_lock_o    ( axi_master_ar_lock_o    ),
      .master_cache_o   ( axi_master_ar_cache_o   ),
      .master_qos_o     ( axi_master_ar_qos_o     ),
      .master_id_o      ( axi_master_ar_id_o      ),
      .master_user_o    ( axi_master_ar_user_o    ),
      .master_ready_i   ( axi_master_ar_ready_i   )
   );

   // WRITE DATA CHANNEL BUFFER
   axi_w_buffer
   #(
      .DATA_WIDTH      ( AXI_DATA_WIDTH        ),
      .USER_WIDTH      ( AXI_USER_WIDTH        ),
      .BUFFER_DEPTH    ( NB_CORES              )
   )
   w_buffer_i
   (
      .clk_i           ( clk_i                 ),
      .rst_ni          ( rst_ni                ),
      .test_en_i       ( test_en_i             ),

      .slave_valid_i   ( s_w_valid             ),
      .slave_data_i    ( s_w_data              ),
      .slave_strb_i    ( s_w_strb              ),
      .slave_user_i    ( s_w_user              ),
      .slave_last_i    ( s_w_last              ),
      .slave_ready_o   ( s_w_ready             ),

      .master_valid_o  ( axi_master_w_valid_o  ),
      .master_data_o   ( axi_master_w_data_o   ),
      .master_strb_o   ( axi_master_w_strb_o   ),
      .master_user_o   ( axi_master_w_user_o   ),
      .master_last_o   ( axi_master_w_last_o   ),
      .master_ready_i  ( axi_master_w_ready_i  )
   );

   // READ DATA CHANNEL BUFFER
   axi_r_buffer
   #(
      .ID_WIDTH        ( AXI_ID_WIDTH          ),
      .DATA_WIDTH      ( AXI_DATA_WIDTH        ),
      .USER_WIDTH      ( AXI_USER_WIDTH        ),
      .BUFFER_DEPTH    ( 2                     )
   )
   r_buffer_i
   (
      .clk_i           ( clk_i                 ),
      .rst_ni          ( rst_ni                ),
      .test_en_i       ( test_en_i             ),

      .slave_valid_i   ( axi_master_r_valid_i  ),
      .slave_data_i    ( axi_master_r_data_i   ),
      .slave_resp_i    ( axi_master_r_resp_i   ),
      .slave_user_i    ( axi_master_r_user_i   ),
      .slave_id_i      ( axi_master_r_id_i     ),
      .slave_last_i    ( axi_master_r_last_i   ),
      .slave_ready_o   ( axi_master_r_ready_o  ),

      .master_valid_o  ( s_r_valid             ),
      .master_data_o   ( s_r_data              ),
      .master_resp_o   ( s_r_resp              ),
      .master_user_o   ( s_r_user              ),
      .master_id_o     ( s_r_id                ),
      .master_last_o   ( s_r_last              ),
      .master_ready_i  ( s_r_ready             )
   );

   // WRITE RESPONSE CHANNEL BUFFER
   axi_b_buffer
   #(
      .ID_WIDTH        ( AXI_ID_WIDTH          ),
      .USER_WIDTH      ( AXI_USER_WIDTH        ),
      .BUFFER_DEPTH    ( 2                     )
   )
   b_buffer_i
   (
      .clk_i           ( clk_i                 ),
      .rst_ni          ( rst_ni                ),
      .test_en_i       ( test_en_i             ),

      .slave_valid_i   ( axi_master_b_valid_i  ),
      .slave_resp_i    ( axi_master_b_resp_i   ),
      .slave_id_i      ( axi_master_b_id_i     ),
      .slave_user_i    ( axi_master_b_user_i   ),
      .slave_ready_o   ( axi_master_b_ready_o  ),

      .master_valid_o  ( s_b_valid             ),
      .master_resp_o   ( s_b_resp              ),
      .master_id_o     ( s_b_id                ),
      .master_user_o   ( s_b_user              ),
      .master_ready_i  ( s_b_ready             )
   );

endmodule

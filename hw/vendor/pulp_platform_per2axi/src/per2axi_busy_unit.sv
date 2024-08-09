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

module per2axi_busy_unit
(
   input  logic clk_i,
   input  logic rst_ni,

   // WRITE INTERFACE
   input  logic aw_sync_i,
   input  logic b_sync_i,

   // READ INTERFACE
   input  logic ar_sync_i,
   input  logic r_sync_i,

   // BUSY SIGNAL
   output logic busy_o
);

   logic [3:0]   s_aw_trans_count;
   logic [3:0]   s_ar_trans_count;
   
   //COUNTER FOR NUMBER OF AW TRANSACTIONS
   always_ff @ (posedge clk_i, negedge rst_ni)
     begin
        if(rst_ni == 1'b0)
          s_aw_trans_count <= '0;
        else
          if ( aw_sync_i == 1'b1 && b_sync_i == 1'b0 )
            s_aw_trans_count <= s_aw_trans_count+1;
          else
            if ( aw_sync_i == 1'b0 && b_sync_i == 1'b1 )
              s_aw_trans_count <= s_aw_trans_count-1;
            else
              s_aw_trans_count <= s_aw_trans_count;
     end
   
   //COUNTER FOR NUMBER OF AR TRANSACTIONS
   always_ff @ (posedge clk_i, negedge rst_ni)
     begin
        if(rst_ni == 1'b0)
          s_ar_trans_count <= '0;
        else
          if ( ar_sync_i == 1'b1 && r_sync_i == 1'b0 )
            s_ar_trans_count <= s_ar_trans_count+1;
          else
            if ( ar_sync_i == 1'b0 && r_sync_i == 1'b1 )
              s_ar_trans_count <= s_ar_trans_count-1;
            else
              s_ar_trans_count <= s_ar_trans_count;
     end
   
   // GENERATION OF BUSY SIGNAL
   always_comb
     begin
        if ( s_ar_trans_count == 0 && s_aw_trans_count == 0 )
          busy_o = 1'b0;
        else
          busy_o = 1'b1;
     end
   
endmodule

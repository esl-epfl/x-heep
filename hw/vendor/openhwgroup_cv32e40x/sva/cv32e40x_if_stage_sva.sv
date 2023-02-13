// Copyright 2018 ETH Zurich and University of Bologna.
// Copyright and related rights are licensed under the Solderpad Hardware
// License, Version 0.51 (the "License"); you may not use this file except in
// compliance with the License.  You may obtain a copy of the License at
// http://solderpad.org/licenses/SHL-0.51. Unless required by applicable law
// or agreed to in writing, software, hardware and materials distributed under
// this License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Authors:        Renzo Andri - andrire@student.ethz.ch                      //
//                 Igor Loi - igor.loi@unibo.it                               //
//                 Andreas Traber - atraber@student.ethz.ch                   //
//                 Sven Stucki - svstucki@student.ethz.ch                     //
//                 Halfdan Bechmann - halfdan.bechmann@silabs.com             //
//                                                                            //
// Description:    RTL assertions for the if_stage module                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

module cv32e40x_if_stage_sva
  import uvm_pkg::*;
  import cv32e40x_pkg::*;
(
  input  logic          clk,
  input  logic          rst_n,

  input logic           if_ready,
  input logic           if_valid_o,
  input  ctrl_fsm_t     ctrl_fsm_i,
  if_c_obi.monitor      m_c_obi_instr_if,
  input  logic          seq_valid,
  input  logic          seq_ready,
  input  logic          illegal_c_insn,
  input  logic          instr_compressed,
  input  logic          prefetch_is_tbljmp_ptr,
  input  logic          prefetch_is_clic_ptr,
  input  logic          prefetch_is_mret_ptr
);

  // Check that bus interface transactions are halfword aligned (will be forced word aligned at core boundary)
  property p_instr_addr_aligned;
    @(posedge clk) (1'b1) |-> (m_c_obi_instr_if.req_payload.addr[0] == 1'b0);
  endproperty

  a_instr_addr_aligned : assert property(p_instr_addr_aligned)
    else `uvm_error("if_stage", "Assertion a_instr_addr_aligned failed")

  // Halt implies not ready and not valid
  a_halt :
    assert property (@(posedge clk) disable iff (!rst_n)
                      (ctrl_fsm_i.halt_if && !ctrl_fsm_i.kill_if)
                      |-> (!if_ready && !if_valid_o))
      else `uvm_error("if_stage", "Halt should imply not ready and not valid")

  // Kill implies ready and not valid
  a_kill :
    assert property (@(posedge clk) disable iff (!rst_n)
                      (ctrl_fsm_i.kill_if)
                      |-> (if_ready && !if_valid_o))
      else `uvm_error("if_stage", "Kill should imply ready and not valid")


  // compressed_decoder and sequencer shall be mutually exclusive
  // Excluding table jumps pointers as these will set seq_valid=1 while the
  // compressed decoder ignore pointers (illegal_c_insn will be 0)
  a_compressed_seq_0:
  assert property (@(posedge clk) disable iff (!rst_n)
                    (seq_valid && !prefetch_is_tbljmp_ptr) |-> illegal_c_insn)
      else `uvm_error("if_stage", "Compressed decoder and sequencer not mutually exclusive.")

  a_compressed_seq_1:
  assert property (@(posedge clk) disable iff (!rst_n)
                    (instr_compressed && !illegal_c_insn && !prefetch_is_tbljmp_ptr)
                    |->
                    !seq_valid)
      else `uvm_error("if_stage", "Compressed decoder and sequencer not mutually exclusive.")

  // Kill implies ready and not valid
  a_seq_kill:
    assert property (@(posedge clk) disable iff (!rst_n)
                      ctrl_fsm_i.kill_if |-> (seq_ready && !seq_valid))
        else `uvm_error("if_stage", "Kill should imply ready and not valid.")

  // CLIC pointers and mret pointers can't both be set at the same time
  a_clic_mret_ptr_unique:
    assert property (@(posedge clk) disable iff (!rst_n)
                      (prefetch_is_mret_ptr || prefetch_is_clic_ptr)
                      |->
                      prefetch_is_mret_ptr != prefetch_is_clic_ptr)
        else `uvm_error("if_stage", "prefetch_is_mret_ptr high at the same time as prefetch_is_clic_ptr.")
endmodule // cv32e40x_if_stage


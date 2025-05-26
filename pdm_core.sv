// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Pierre Guillod <pierre.guillod@epfl.ch>, EPFL, STI-SEL
// Date: 14.12.2022
// Description: PDM to PCM converter core



module pdm_core #(
    // Number of stages of the CIC filter
    localparam STAGES_CIC = 4,
    // Width of the datapath
    localparam WIDTH = 18,
    // First decimator internal counter width
    localparam DECIM_COMBS_CNT_W = 4,
    // Width of the clock divider count
    localparam CLKDIVWIDTH = 16

) (
    // Clock input
    input logic clk_i,
    // Clock divider division index
    input logic [CLKDIVWIDTH-1:0] par_clkdiv_idx,
    // Reset input
    input logic rstn_i,
    // Enable input
    input logic en_i,
    // Clock output to the microphone
    output logic pdm_clk_o,

    // First decimator decimation index
    input logic [DECIM_COMBS_CNT_W-1:0] par_decim_idx_combs,

    // Input signal (PDM)
    input logic pdm_i,
    // Output signal (PCM)
    output logic [WIDTH-1:0] pcm_o,
    // Valid output data flag
    output logic pcm_data_valid_o
);

  logic             r_store;
  logic             r_send;
  logic             r_data;

  logic             div_clk;
  logic             div_clk_p;
  logic             div_clk_e;

  // Auxiliary signals to link the filter blocks
  logic [WIDTH-1:0] data;
  logic [WIDTH-1:0] integr_to_comb;
  logic [WIDTH-1:0] combs_to_hb1;

  logic             r_en;
  logic             s_clr;

  // Decimators output signals
  logic             combs_en;

  clk_int_div #(
      .DIV_VALUE_WIDTH(CLKDIVWIDTH)
  ) clk_int_div_inst (
      .clk_i(clk_i),
      .rst_ni(rstn_i),
      .en_i(en_i),
      .test_mode_en_i(1'b0),
      .clk_o(div_clk),
      .div_i(par_clkdiv_idx),
      .div_valid_i(1'b1),
      .div_ready_o(),
      .cycl_count_o()
  );

  always_ff @(posedge clk_i or negedge rstn_i) begin
    if (~rstn_i) begin
      div_clk_p <= 0;
    end else begin
      div_clk_p <= div_clk;
    end
  end

  assign div_clk_e = div_clk & ~div_clk_p;

  // Output synchronized with the last decimator
  assign pcm_data_valid_o = combs_en & div_clk & div_clk_e;

  ///////////////////////////////////////////////////////////////////////////
  //////////// PIECE OF CODE I NEED TO MAKE EASIER TO UNDERSTAND ////////////
  ///////////////////////////////////////////////////////////////////////////

  always_ff @(posedge div_clk or negedge rstn_i) begin : proc_r_store
    if (~rstn_i) begin
      r_store   <= 1;
      r_send    <= 0;
      r_data    <= 0;
      pdm_clk_o <= 0;
    end else begin
      if (en_i) begin
        r_store <= ~r_store;
        r_send  <= ~r_send;
        if (r_store) begin
          r_data    <= pdm_i;
          pdm_clk_o <= ~pdm_clk_o;
        end else begin
          r_store   <= 1'b1;
          r_send    <= 0;
          pdm_clk_o <= 0;
        end
      end
    end
  end

  assign s_clr = en_i & !r_en;

  always_ff @(posedge div_clk or negedge rstn_i) begin
    if (~rstn_i) r_en <= 'h0;
    else r_en <= en_i;
  end

  ///////////////////////////////////////////////////////////////////////////
  ////// END OF THE PIECE OF CODE I NEED TO MAKE EASIER TO UNDERSTAND ///////
  ///////////////////////////////////////////////////////////////////////////

  // Converts binary PDM {0,1} to bipolar PDM {-1,1}
  assign data = r_data ? 'h1 : {WIDTH{1'b1}};

  // Instantiation sequence
  //┌───────────────────────┐
  //│       CIC Filter      │
  //│┌─────┐ ┌─────┐ ┌─────┐│┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐
  //││Intgs├─►Decim├─►Combs├│►Hlfbd├─►Decim├─►Hlfbd├─►Decim├─► FIR │
  //│└─────┘ └─────┘ └─────┘│└─────┘ └─────┘ └─────┘ └─────┘ └─────┘
  //└───────────────────────┘
  // (made with asciiflow.com)

  cic_integrators #(STAGES_CIC, WIDTH) cic_integrators_inst (
      .clk_i (div_clk),
      .rstn_i(rstn_i),
      .clr_i (s_clr),
      .en_i  (r_send),
      .data_i(data),
      .data_o(integr_to_comb)
  );

  decimator #(DECIM_COMBS_CNT_W) decimator_before_hb1 (
      .clk_i(div_clk),
      .rst_i(rstn_i),
      .clr_i(s_clr),
      .par_decimation_index(par_decim_idx_combs),
      .en_i(r_send),
      .en_o(combs_en)
  );

  cic_combs #(STAGES_CIC, WIDTH) cic_combs_inst (
      .clk_i (div_clk),
      .rstn_i(rstn_i),
      .clr_i (s_clr),
      .en_i  (combs_en),
      .data_i(integr_to_comb),
      .data_o(combs_to_hb1)
  );

      assign pcm_o = combs_to_hb1;

  //
  // Some of the finest debugging goodness
  //
  //always_ff @(posedge clk_i)
  //begin
  //
  //  if (r_send == 1) begin
  //    //$display("integr_to_comb = ", integr_to_comb);
  //  end
  //
  //  if (combs_en == 1) begin
  //    //$display("combs_to_hb1 = ", combs_to_hb1);
  //  end
  //
  //  if (hb1_en == 1) begin
  //    //$display("hb1_to_hb2 = ", hb1_to_hb2);
  //  end
  //
  //  if (hb2_en == 1) begin
  //    //$display("hb2_to_fir = ", hb2_to_fir);
  //  end
  //
  //  if (hb2_en == 1) begin
  //    //$display("pcm_o = ", pcm_o);
  //  end
  //
  //end

endmodule

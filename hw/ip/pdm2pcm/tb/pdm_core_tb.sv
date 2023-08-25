// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Pierre Guillod <pierre.guillod@epfl.ch>, EPFL, STI-SEL
// Date: 14.12.2022
// Description: Testbench for `pdm_core`

module pdm_core_tb;

  logic        clk_i;
  logic        rstn_i;
  logic        pdm_clk_o;
  logic        pdm_clk_oh;
  logic [ 3:0] par_decim_idx_combs;
  logic [ 4:0] par_decim_idx_hfbd2;
  logic [ 5:0] par_decim_idx_fir;
  logic        en_i;
  logic        pdm_i;
  logic [19:0] pcm_o;
  logic        pcm_data_valid_o;
  logic [15:0] coeffs_hb1          [ 0:5];
  logic [15:0] coeffs_hb2          [0:11];
  logic [15:0] coeffs_fir          [0:13];
  logic [15:0] par_clkdiv_idx;

  initial begin

    int fpdm;
    int fpcm;
    int lineidx;

    string line;

    fpdm = $fopen("signals/pdm.txt", "r");
    if (fpdm) begin
      $display("PDM File opened successfully.");
    end else begin
      $display("Failed to open PDM file.");
    end

    fpcm = $fopen("signals/pcm.txt", "w");
    if (!fpcm) begin
      $display("Failed to open PCM file.");
      $stop();
    end

    rstn_i = 0;
    clk_i  = 0;

    $fgets(line, fpdm);
    line = line.substr(0, 0);
    if (line == "1") begin
      pdm_i = 1;
    end else begin
      pdm_i = 0;
    end

    en_i = 1'b0;

    #1 clk_i = 1;
    #1 clk_i = 0;

    rstn_i = 1;

    #1 clk_i = 1;

    en_i = 1'b1;

    pdm_clk_oh = 1'b0;

    forever begin

      #1 clk_i = 0;

      if (pdm_clk_o == 1 & pdm_clk_oh == 0) begin
        string line;
        $fgets(line, fpdm);
        line = line.substr(0, 0);
        if (line == "1") begin
          pdm_i = 1;
        end else begin
          pdm_i = 0;
        end
        lineidx = lineidx + 1;
      end

      pdm_clk_oh = pdm_clk_o;

      if (pcm_data_valid_o == 1) begin
        $fdisplay(fpcm, pcm_o);
        $display(pcm_o);
      end

      #1 clk_i = 1;

      if (lineidx >= 65536) begin
        $stop;
      end

    end

    $fclose(fpdm);
    $fclose(fpcm);

  end

  assign par_decim_idx_combs = 15;
  assign par_decim_idx_hfbd2 = 31;
  assign par_decim_idx_fir = 63;

  assign coeffs_hb1 = '{1, 0, 0, 0, 0, 0};
  assign coeffs_hb2 = '{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  assign coeffs_fir = '{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  assign par_clkdiv_idx = 0;

  pdm_core dut (

      .clk_i              (clk_i),
      .par_clkdiv_idx     (par_clkdiv_idx),
      .rstn_i             (rstn_i),
      .par_decim_idx_combs(par_decim_idx_combs),
      .par_decim_idx_hfbd2(par_decim_idx_hfbd2),
      .par_decim_idx_fir  (par_decim_idx_fir),
      .en_i               (en_i),
      .pdm_i              (pdm_i),
      .coeffs_hb1         (coeffs_hb1),
      .coeffs_hb2         (coeffs_hb2),
      .coeffs_fir         (coeffs_fir),
      .pdm_clk_o          (pdm_clk_o),
      .pcm_o              (pcm_o),
      .pcm_data_valid_o   (pcm_data_valid_o)
  );

endmodule : pdm_core_tb

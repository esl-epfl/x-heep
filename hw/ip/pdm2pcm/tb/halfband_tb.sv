module halfband_tb;

  logic clk_i;
  logic rstn_i;

  logic en_i;
  logic clr_i;

  logic [7:0] data_i;
  logic [7:0] data_o;

  logic [7:0] coeffs[0:1];

  initial begin

    int fin;
    int fout;
    int lineidx;

    string line;

    fin = $fopen("signals/sequence.txt", "r");
    if (fin) begin
      $display("Halfband input file opened successfully.");
    end else begin
      $display("Failed to input file.");
    end

    fout = $fopen("signals/halfbanded.txt", "w");
    if (fout) begin
      $display("Halfband output file opened successfully.");
    end else begin
      $display("Failed to open output file.");
    end

    rstn_i = 0;
    clk_i  = 0;

    data_i = 0;

    clr_i  = 0;
    en_i   = 1'b0;

    #1 clk_i = 1;
    #1 clk_i = 0;

    rstn_i = 1;

    #1 clk_i = 1;

    en_i = 1'b1;

    forever begin

      #1 clk_i = 0;

      $fgets(line, fin);
      data_i = line.atoreal();

      #1 clk_i = 1;

      $fdisplay(fout, data_o);
      $display(data_o);

      if (lineidx >= 10) begin
        $stop;
      end

    end

    $fclose(fin);
    $fclose(fout);

  end

  assign coeffs[0] = 10;
  assign coeffs[1] = 2;

  halfband #(8, 8, 1) dut (

      .clk_i (clk_i),
      .rstn_i(rstn_i),

      .en_i (en_i),
      .clr_i(clr_i),

      .data_i(data_i),
      .data_o(data_o),

      .coeffs(coeffs)

  );

endmodule : halfband_tb

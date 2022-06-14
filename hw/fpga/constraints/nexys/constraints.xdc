#Create constraint for the clock input of the nexys 4 board
create_clock -period 25.000 -name ref_clk [get_ports clk_i]

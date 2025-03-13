create_clock -add -name sys_clk_pin -period 10.00 -waveform {0 5} [get_ports {clk_i}];
create_clock -add -name jtag_clk_pin -period 100.00 -waveform {0 5} [get_ports {jtag_tck_i}];
create_clock -add -name spi_slave_clk_pin -period 20.00 -waveform {0 5} [get_ports {spi_slave_sck_io}];



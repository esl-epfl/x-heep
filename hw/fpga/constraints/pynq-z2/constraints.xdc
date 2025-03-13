create_clock -add -name sys_clk_pin -period 8.00 -waveform {0 5} [get_ports {clk_i}];
create_clock -add -name jtag_clk_pin -period 100.00 -waveform {0 5} [get_ports {jtag_tck_i}];
create_clock -add -name spi_slave_clk_pin -period 16.00 -waveform {0 5} [get_ports {spi_slave_sck_io}];

set_false_path -from x_heep_system_i/core_v_mini_mcu_i/ao_peripheral_subsystem_i/obi_spi_slave_i/u_slave_sm/u_spiregs/reg*_reg[*] -to x_heep_system_i/core_v_mini_mcu_i/ao_peripheral_subsystem_i/obi_spi_slave_i/u_obiplug/tx_counter_reg*
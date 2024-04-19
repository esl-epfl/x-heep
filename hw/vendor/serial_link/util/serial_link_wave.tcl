# Copyright 2023 ETH Zurich and University of Bologna.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

onerror {resume}
quietly WaveActivateNextPane {} 0

set tb_name tb_axi_serial_link

for {set i 1} {$i < 3} {incr i} {
    set group_name "DDR $i"
    set num_channels [expr {[llength [find instances -bydu serial_link_physical -recursive]] / 2}]
    if {$num_channels == 1} {
        set gen_block_name gen_single_channel_serial_link
    } else {
        set gen_block_name gen_multi_channel_serial_link
    }

    add wave -noupdate -expand -group $group_name -ports /$tb_name/i_serial_link_$i/$gen_block_name/i_serial_link/*

    add wave -noupdate -group $group_name -group {NETWORK} /$tb_name/i_serial_link_$i/$gen_block_name/i_serial_link/i_serial_link_network/*

    add wave -noupdate -group $group_name -group {LINK} /$tb_name/i_serial_link_$i/$gen_block_name/i_serial_link/i_serial_link_data_link/*

    if {$num_channels > 1} {
        add wave -noupdate -group $group_name -group {CHANNEL_ALLOCATOR} -ports /$tb_name/i_serial_link_$i/$gen_block_name/i_serial_link/gen_channel_alloc/i_channel_allocator/*
    }

    for {set c 0} {$c < $num_channels} {incr c} {
        add wave -noupdate -group $group_name -group PHY -group TX -group CH$c /$tb_name/i_serial_link_$i/$gen_block_name/i_serial_link/gen_phy_channels[$c]/i_serial_link_physical/i_serial_link_physical_tx/*
        add wave -noupdate -group $group_name -group PHY -group RX -group CH$c /$tb_name/i_serial_link_$i/$gen_block_name/i_serial_link/gen_phy_channels[0]/i_serial_link_physical/i_serial_link_physical_rx/*
    }

    add wave -noupdate -group $group_name -group {CONFIG} /$tb_name/i_serial_link_$i/$gen_block_name/i_serial_link/reg2hw
    add wave -noupdate -group $group_name -group {CONFIG} /$tb_name/i_serial_link_$i/$gen_block_name/i_serial_link/hw2reg
    add wave -noupdate -group $group_name -group {CONFIG} /$tb_name/i_serial_link_$i/$gen_block_name/i_serial_link/cfg_req
    add wave -noupdate -group $group_name -group {CONFIG} /$tb_name/i_serial_link_$i/$gen_block_name/i_serial_link/cfg_rsp
}

TreeUpdate [SetDefaultTree]
quietly wave cursor active 1
configure wave -namecolwidth 220
configure wave -valuecolwidth 110
configure wave -justifyvalue left
configure wave -signalnamewidth 1
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ns
update

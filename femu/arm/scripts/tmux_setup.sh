#!/bin/sh

tmux new-session \; \
	split-window -v \; \
	send-keys 'export HOME_PATH="/home/xilinx" ; sudo openocd -f $HOME_PATH/openocd_cfg/gpio_bitbang.cfg' C-m \; \
	split-window -h -p 75\; \
	send-keys 'sudo screen /dev/ttyPS1 115200 ' C-m \; \


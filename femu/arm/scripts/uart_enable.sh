#!/bin/bash

sudo su
mount -t configfs configfs /configfs
mkdir /configfs/device-tree/overlays/uart_test
cat ../uart_enable/uart_enable.dtbo >/configfs/device-tree/overlays/uart_enable/dtbo
exit


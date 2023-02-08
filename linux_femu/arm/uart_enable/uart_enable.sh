#!/bin/bash

mount -t configfs configfs /configfs
mkdir /configfs/device-tree/overlays/uart_enable
cat ./uart_enable.dtbo >/configfs/device-tree/overlays/uart_enable/dtbo

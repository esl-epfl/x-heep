/*
 * Copyright 2025 EPFL
 * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
 * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
 *
 * Author: Tommaso Terzano <tommaso.terzano@epfl.ch> 
 *                         <tommaso.terzano@gmail.com>
 *  
 * Info: Header file for the DMA subsystem.
 */

% if dma_addr_mode:
`define ADDR_MODE_EN
% endif
% if dma_zero_padding:
`define ZERO_PADDING_EN
% endif
% if dma_subaddr_mode:
`define SUBADDR_MODE_EN
% endif
% if dma_hw_fifo_mode:
`define HW_FIFO_MODE_EN
% endif

// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// Register Package auto-generated by `reggen` containing data structure

package serial_link_reg_pkg;

  // Param list
  parameter int NumChannels = 38;
  parameter int Log2NumChannels = 6;
  parameter int NumBits = 16;
  parameter int Log2MaxClkDiv = 10;
  parameter int FlushCounterWidth = 8;
  parameter int Log2RawModeTXFifoDepth = 3;

  // Address widths within the block
  parameter int BlockAw = 10;

  ////////////////////////////
  // Typedefs for registers //
  ////////////////////////////

  typedef struct packed {
    struct packed {
      logic        q;
    } clk_ena;
    struct packed {
      logic        q;
    } reset_n;
    struct packed {
      logic        q;
    } axi_in_isolate;
    struct packed {
      logic        q;
    } axi_out_isolate;
  } serial_link_reg2hw_ctrl_reg_t;

  typedef struct packed {
    logic [10:0] q;
  } serial_link_reg2hw_tx_phy_clk_div_mreg_t;

  typedef struct packed {
    logic [10:0] q;
  } serial_link_reg2hw_tx_phy_clk_start_mreg_t;

  typedef struct packed {
    logic [10:0] q;
  } serial_link_reg2hw_tx_phy_clk_end_mreg_t;

  typedef struct packed {
    logic        q;
  } serial_link_reg2hw_raw_mode_en_reg_t;

  typedef struct packed {
    logic [5:0]  q;
  } serial_link_reg2hw_raw_mode_in_ch_sel_reg_t;

  typedef struct packed {
    logic [15:0] q;
    logic        re;
  } serial_link_reg2hw_raw_mode_in_data_reg_t;

  typedef struct packed {
    logic        q;
  } serial_link_reg2hw_raw_mode_out_ch_mask_mreg_t;

  typedef struct packed {
    logic [15:0] q;
    logic        qe;
  } serial_link_reg2hw_raw_mode_out_data_fifo_reg_t;

  typedef struct packed {
    struct packed {
      logic        q;
      logic        qe;
    } clear;
  } serial_link_reg2hw_raw_mode_out_data_fifo_ctrl_reg_t;

  typedef struct packed {
    logic        q;
  } serial_link_reg2hw_raw_mode_out_en_reg_t;

  typedef struct packed {
    logic        q;
    logic        qe;
  } serial_link_reg2hw_flow_control_fifo_clear_reg_t;

  typedef struct packed {
    struct packed {
      logic        q;
    } bypass_en;
    struct packed {
      logic        q;
    } auto_flush_en;
    struct packed {
      logic [7:0]  q;
    } auto_flush_count;
  } serial_link_reg2hw_channel_alloc_tx_cfg_reg_t;

  typedef struct packed {
    logic        q;
  } serial_link_reg2hw_channel_alloc_tx_ch_en_mreg_t;

  typedef struct packed {
    struct packed {
      logic        q;
      logic        qe;
    } clear;
    struct packed {
      logic        q;
      logic        qe;
    } flush;
  } serial_link_reg2hw_channel_alloc_tx_ctrl_reg_t;

  typedef struct packed {
    struct packed {
      logic        q;
    } bypass_en;
    struct packed {
      logic        q;
    } auto_flush_en;
    struct packed {
      logic [7:0]  q;
    } auto_flush_count;
    struct packed {
      logic        q;
    } sync_en;
  } serial_link_reg2hw_channel_alloc_rx_cfg_reg_t;

  typedef struct packed {
    logic        q;
    logic        qe;
  } serial_link_reg2hw_channel_alloc_rx_ctrl_reg_t;

  typedef struct packed {
    logic        q;
  } serial_link_reg2hw_channel_alloc_rx_ch_en_mreg_t;

  typedef struct packed {
    struct packed {
      logic        d;
    } axi_in;
    struct packed {
      logic        d;
    } axi_out;
  } serial_link_hw2reg_isolated_reg_t;

  typedef struct packed {
    logic        d;
  } serial_link_hw2reg_raw_mode_in_data_valid_mreg_t;

  typedef struct packed {
    logic [15:0] d;
  } serial_link_hw2reg_raw_mode_in_data_reg_t;

  typedef struct packed {
    struct packed {
      logic [2:0]  d;
    } fill_state;
    struct packed {
      logic        d;
    } is_full;
  } serial_link_hw2reg_raw_mode_out_data_fifo_ctrl_reg_t;

  // Register -> HW type
  typedef struct packed {
    serial_link_reg2hw_ctrl_reg_t ctrl; // [1444:1441]
    serial_link_reg2hw_tx_phy_clk_div_mreg_t [37:0] tx_phy_clk_div; // [1440:1023]
    serial_link_reg2hw_tx_phy_clk_start_mreg_t [37:0] tx_phy_clk_start; // [1022:605]
    serial_link_reg2hw_tx_phy_clk_end_mreg_t [37:0] tx_phy_clk_end; // [604:187]
    serial_link_reg2hw_raw_mode_en_reg_t raw_mode_en; // [186:186]
    serial_link_reg2hw_raw_mode_in_ch_sel_reg_t raw_mode_in_ch_sel; // [185:180]
    serial_link_reg2hw_raw_mode_in_data_reg_t raw_mode_in_data; // [179:163]
    serial_link_reg2hw_raw_mode_out_ch_mask_mreg_t [37:0] raw_mode_out_ch_mask; // [162:125]
    serial_link_reg2hw_raw_mode_out_data_fifo_reg_t raw_mode_out_data_fifo; // [124:108]
    serial_link_reg2hw_raw_mode_out_data_fifo_ctrl_reg_t raw_mode_out_data_fifo_ctrl; // [107:106]
    serial_link_reg2hw_raw_mode_out_en_reg_t raw_mode_out_en; // [105:105]
    serial_link_reg2hw_flow_control_fifo_clear_reg_t flow_control_fifo_clear; // [104:103]
    serial_link_reg2hw_channel_alloc_tx_cfg_reg_t channel_alloc_tx_cfg; // [102:93]
    serial_link_reg2hw_channel_alloc_tx_ch_en_mreg_t [37:0] channel_alloc_tx_ch_en; // [92:55]
    serial_link_reg2hw_channel_alloc_tx_ctrl_reg_t channel_alloc_tx_ctrl; // [54:51]
    serial_link_reg2hw_channel_alloc_rx_cfg_reg_t channel_alloc_rx_cfg; // [50:40]
    serial_link_reg2hw_channel_alloc_rx_ctrl_reg_t channel_alloc_rx_ctrl; // [39:38]
    serial_link_reg2hw_channel_alloc_rx_ch_en_mreg_t [37:0] channel_alloc_rx_ch_en; // [37:0]
  } serial_link_reg2hw_t;

  // HW -> register type
  typedef struct packed {
    serial_link_hw2reg_isolated_reg_t isolated; // [59:58]
    serial_link_hw2reg_raw_mode_in_data_valid_mreg_t [37:0] raw_mode_in_data_valid; // [57:20]
    serial_link_hw2reg_raw_mode_in_data_reg_t raw_mode_in_data; // [19:4]
    serial_link_hw2reg_raw_mode_out_data_fifo_ctrl_reg_t raw_mode_out_data_fifo_ctrl; // [3:0]
  } serial_link_hw2reg_t;

  // Register offsets
  parameter logic [BlockAw-1:0] SERIAL_LINK_CTRL_OFFSET = 10'h 0;
  parameter logic [BlockAw-1:0] SERIAL_LINK_ISOLATED_OFFSET = 10'h 4;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_0_OFFSET = 10'h 8;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_1_OFFSET = 10'h c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_2_OFFSET = 10'h 10;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_3_OFFSET = 10'h 14;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_4_OFFSET = 10'h 18;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_5_OFFSET = 10'h 1c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_6_OFFSET = 10'h 20;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_7_OFFSET = 10'h 24;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_8_OFFSET = 10'h 28;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_9_OFFSET = 10'h 2c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_10_OFFSET = 10'h 30;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_11_OFFSET = 10'h 34;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_12_OFFSET = 10'h 38;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_13_OFFSET = 10'h 3c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_14_OFFSET = 10'h 40;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_15_OFFSET = 10'h 44;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_16_OFFSET = 10'h 48;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_17_OFFSET = 10'h 4c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_18_OFFSET = 10'h 50;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_19_OFFSET = 10'h 54;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_20_OFFSET = 10'h 58;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_21_OFFSET = 10'h 5c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_22_OFFSET = 10'h 60;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_23_OFFSET = 10'h 64;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_24_OFFSET = 10'h 68;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_25_OFFSET = 10'h 6c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_26_OFFSET = 10'h 70;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_27_OFFSET = 10'h 74;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_28_OFFSET = 10'h 78;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_29_OFFSET = 10'h 7c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_30_OFFSET = 10'h 80;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_31_OFFSET = 10'h 84;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_32_OFFSET = 10'h 88;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_33_OFFSET = 10'h 8c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_34_OFFSET = 10'h 90;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_35_OFFSET = 10'h 94;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_36_OFFSET = 10'h 98;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_DIV_37_OFFSET = 10'h 9c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_0_OFFSET = 10'h a0;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_1_OFFSET = 10'h a4;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_2_OFFSET = 10'h a8;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_3_OFFSET = 10'h ac;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_4_OFFSET = 10'h b0;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_5_OFFSET = 10'h b4;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_6_OFFSET = 10'h b8;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_7_OFFSET = 10'h bc;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_8_OFFSET = 10'h c0;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_9_OFFSET = 10'h c4;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_10_OFFSET = 10'h c8;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_11_OFFSET = 10'h cc;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_12_OFFSET = 10'h d0;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_13_OFFSET = 10'h d4;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_14_OFFSET = 10'h d8;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_15_OFFSET = 10'h dc;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_16_OFFSET = 10'h e0;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_17_OFFSET = 10'h e4;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_18_OFFSET = 10'h e8;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_19_OFFSET = 10'h ec;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_20_OFFSET = 10'h f0;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_21_OFFSET = 10'h f4;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_22_OFFSET = 10'h f8;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_23_OFFSET = 10'h fc;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_24_OFFSET = 10'h 100;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_25_OFFSET = 10'h 104;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_26_OFFSET = 10'h 108;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_27_OFFSET = 10'h 10c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_28_OFFSET = 10'h 110;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_29_OFFSET = 10'h 114;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_30_OFFSET = 10'h 118;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_31_OFFSET = 10'h 11c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_32_OFFSET = 10'h 120;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_33_OFFSET = 10'h 124;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_34_OFFSET = 10'h 128;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_35_OFFSET = 10'h 12c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_36_OFFSET = 10'h 130;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_START_37_OFFSET = 10'h 134;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_0_OFFSET = 10'h 138;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_1_OFFSET = 10'h 13c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_2_OFFSET = 10'h 140;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_3_OFFSET = 10'h 144;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_4_OFFSET = 10'h 148;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_5_OFFSET = 10'h 14c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_6_OFFSET = 10'h 150;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_7_OFFSET = 10'h 154;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_8_OFFSET = 10'h 158;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_9_OFFSET = 10'h 15c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_10_OFFSET = 10'h 160;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_11_OFFSET = 10'h 164;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_12_OFFSET = 10'h 168;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_13_OFFSET = 10'h 16c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_14_OFFSET = 10'h 170;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_15_OFFSET = 10'h 174;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_16_OFFSET = 10'h 178;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_17_OFFSET = 10'h 17c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_18_OFFSET = 10'h 180;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_19_OFFSET = 10'h 184;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_20_OFFSET = 10'h 188;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_21_OFFSET = 10'h 18c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_22_OFFSET = 10'h 190;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_23_OFFSET = 10'h 194;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_24_OFFSET = 10'h 198;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_25_OFFSET = 10'h 19c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_26_OFFSET = 10'h 1a0;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_27_OFFSET = 10'h 1a4;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_28_OFFSET = 10'h 1a8;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_29_OFFSET = 10'h 1ac;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_30_OFFSET = 10'h 1b0;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_31_OFFSET = 10'h 1b4;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_32_OFFSET = 10'h 1b8;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_33_OFFSET = 10'h 1bc;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_34_OFFSET = 10'h 1c0;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_35_OFFSET = 10'h 1c4;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_36_OFFSET = 10'h 1c8;
  parameter logic [BlockAw-1:0] SERIAL_LINK_TX_PHY_CLK_END_37_OFFSET = 10'h 1cc;
  parameter logic [BlockAw-1:0] SERIAL_LINK_RAW_MODE_EN_OFFSET = 10'h 1d0;
  parameter logic [BlockAw-1:0] SERIAL_LINK_RAW_MODE_IN_CH_SEL_OFFSET = 10'h 1d4;
  parameter logic [BlockAw-1:0] SERIAL_LINK_RAW_MODE_IN_DATA_VALID_0_OFFSET = 10'h 1d8;
  parameter logic [BlockAw-1:0] SERIAL_LINK_RAW_MODE_IN_DATA_VALID_1_OFFSET = 10'h 1dc;
  parameter logic [BlockAw-1:0] SERIAL_LINK_RAW_MODE_IN_DATA_OFFSET = 10'h 1e0;
  parameter logic [BlockAw-1:0] SERIAL_LINK_RAW_MODE_OUT_CH_MASK_0_OFFSET = 10'h 1e4;
  parameter logic [BlockAw-1:0] SERIAL_LINK_RAW_MODE_OUT_CH_MASK_1_OFFSET = 10'h 1e8;
  parameter logic [BlockAw-1:0] SERIAL_LINK_RAW_MODE_OUT_DATA_FIFO_OFFSET = 10'h 1ec;
  parameter logic [BlockAw-1:0] SERIAL_LINK_RAW_MODE_OUT_DATA_FIFO_CTRL_OFFSET = 10'h 1f0;
  parameter logic [BlockAw-1:0] SERIAL_LINK_RAW_MODE_OUT_EN_OFFSET = 10'h 1f4;
  parameter logic [BlockAw-1:0] SERIAL_LINK_FLOW_CONTROL_FIFO_CLEAR_OFFSET = 10'h 1f8;
  parameter logic [BlockAw-1:0] SERIAL_LINK_CHANNEL_ALLOC_TX_CFG_OFFSET = 10'h 1fc;
  parameter logic [BlockAw-1:0] SERIAL_LINK_CHANNEL_ALLOC_TX_CH_EN_0_OFFSET = 10'h 200;
  parameter logic [BlockAw-1:0] SERIAL_LINK_CHANNEL_ALLOC_TX_CH_EN_1_OFFSET = 10'h 204;
  parameter logic [BlockAw-1:0] SERIAL_LINK_CHANNEL_ALLOC_TX_CTRL_OFFSET = 10'h 208;
  parameter logic [BlockAw-1:0] SERIAL_LINK_CHANNEL_ALLOC_RX_CFG_OFFSET = 10'h 20c;
  parameter logic [BlockAw-1:0] SERIAL_LINK_CHANNEL_ALLOC_RX_CTRL_OFFSET = 10'h 210;
  parameter logic [BlockAw-1:0] SERIAL_LINK_CHANNEL_ALLOC_RX_CH_EN_0_OFFSET = 10'h 214;
  parameter logic [BlockAw-1:0] SERIAL_LINK_CHANNEL_ALLOC_RX_CH_EN_1_OFFSET = 10'h 218;

  // Reset values for hwext registers and their fields
  parameter logic [1:0] SERIAL_LINK_ISOLATED_RESVAL = 2'h 3;
  parameter logic [0:0] SERIAL_LINK_ISOLATED_AXI_IN_RESVAL = 1'h 1;
  parameter logic [0:0] SERIAL_LINK_ISOLATED_AXI_OUT_RESVAL = 1'h 1;
  parameter logic [31:0] SERIAL_LINK_RAW_MODE_IN_DATA_VALID_0_RESVAL = 32'h 0;
  parameter logic [5:0] SERIAL_LINK_RAW_MODE_IN_DATA_VALID_1_RESVAL = 6'h 0;
  parameter logic [15:0] SERIAL_LINK_RAW_MODE_IN_DATA_RESVAL = 16'h 0;
  parameter logic [31:0] SERIAL_LINK_RAW_MODE_OUT_DATA_FIFO_CTRL_RESVAL = 32'h 0;
  parameter logic [2:0] SERIAL_LINK_RAW_MODE_OUT_DATA_FIFO_CTRL_FILL_STATE_RESVAL = 3'h 0;
  parameter logic [0:0] SERIAL_LINK_RAW_MODE_OUT_DATA_FIFO_CTRL_IS_FULL_RESVAL = 1'h 0;
  parameter logic [0:0] SERIAL_LINK_FLOW_CONTROL_FIFO_CLEAR_RESVAL = 1'h 0;
  parameter logic [0:0] SERIAL_LINK_FLOW_CONTROL_FIFO_CLEAR_FLOW_CONTROL_FIFO_CLEAR_RESVAL = 1'h 0;
  parameter logic [1:0] SERIAL_LINK_CHANNEL_ALLOC_TX_CTRL_RESVAL = 2'h 0;
  parameter logic [0:0] SERIAL_LINK_CHANNEL_ALLOC_RX_CTRL_RESVAL = 1'h 0;

  // Register index
  typedef enum int {
    SERIAL_LINK_CTRL,
    SERIAL_LINK_ISOLATED,
    SERIAL_LINK_TX_PHY_CLK_DIV_0,
    SERIAL_LINK_TX_PHY_CLK_DIV_1,
    SERIAL_LINK_TX_PHY_CLK_DIV_2,
    SERIAL_LINK_TX_PHY_CLK_DIV_3,
    SERIAL_LINK_TX_PHY_CLK_DIV_4,
    SERIAL_LINK_TX_PHY_CLK_DIV_5,
    SERIAL_LINK_TX_PHY_CLK_DIV_6,
    SERIAL_LINK_TX_PHY_CLK_DIV_7,
    SERIAL_LINK_TX_PHY_CLK_DIV_8,
    SERIAL_LINK_TX_PHY_CLK_DIV_9,
    SERIAL_LINK_TX_PHY_CLK_DIV_10,
    SERIAL_LINK_TX_PHY_CLK_DIV_11,
    SERIAL_LINK_TX_PHY_CLK_DIV_12,
    SERIAL_LINK_TX_PHY_CLK_DIV_13,
    SERIAL_LINK_TX_PHY_CLK_DIV_14,
    SERIAL_LINK_TX_PHY_CLK_DIV_15,
    SERIAL_LINK_TX_PHY_CLK_DIV_16,
    SERIAL_LINK_TX_PHY_CLK_DIV_17,
    SERIAL_LINK_TX_PHY_CLK_DIV_18,
    SERIAL_LINK_TX_PHY_CLK_DIV_19,
    SERIAL_LINK_TX_PHY_CLK_DIV_20,
    SERIAL_LINK_TX_PHY_CLK_DIV_21,
    SERIAL_LINK_TX_PHY_CLK_DIV_22,
    SERIAL_LINK_TX_PHY_CLK_DIV_23,
    SERIAL_LINK_TX_PHY_CLK_DIV_24,
    SERIAL_LINK_TX_PHY_CLK_DIV_25,
    SERIAL_LINK_TX_PHY_CLK_DIV_26,
    SERIAL_LINK_TX_PHY_CLK_DIV_27,
    SERIAL_LINK_TX_PHY_CLK_DIV_28,
    SERIAL_LINK_TX_PHY_CLK_DIV_29,
    SERIAL_LINK_TX_PHY_CLK_DIV_30,
    SERIAL_LINK_TX_PHY_CLK_DIV_31,
    SERIAL_LINK_TX_PHY_CLK_DIV_32,
    SERIAL_LINK_TX_PHY_CLK_DIV_33,
    SERIAL_LINK_TX_PHY_CLK_DIV_34,
    SERIAL_LINK_TX_PHY_CLK_DIV_35,
    SERIAL_LINK_TX_PHY_CLK_DIV_36,
    SERIAL_LINK_TX_PHY_CLK_DIV_37,
    SERIAL_LINK_TX_PHY_CLK_START_0,
    SERIAL_LINK_TX_PHY_CLK_START_1,
    SERIAL_LINK_TX_PHY_CLK_START_2,
    SERIAL_LINK_TX_PHY_CLK_START_3,
    SERIAL_LINK_TX_PHY_CLK_START_4,
    SERIAL_LINK_TX_PHY_CLK_START_5,
    SERIAL_LINK_TX_PHY_CLK_START_6,
    SERIAL_LINK_TX_PHY_CLK_START_7,
    SERIAL_LINK_TX_PHY_CLK_START_8,
    SERIAL_LINK_TX_PHY_CLK_START_9,
    SERIAL_LINK_TX_PHY_CLK_START_10,
    SERIAL_LINK_TX_PHY_CLK_START_11,
    SERIAL_LINK_TX_PHY_CLK_START_12,
    SERIAL_LINK_TX_PHY_CLK_START_13,
    SERIAL_LINK_TX_PHY_CLK_START_14,
    SERIAL_LINK_TX_PHY_CLK_START_15,
    SERIAL_LINK_TX_PHY_CLK_START_16,
    SERIAL_LINK_TX_PHY_CLK_START_17,
    SERIAL_LINK_TX_PHY_CLK_START_18,
    SERIAL_LINK_TX_PHY_CLK_START_19,
    SERIAL_LINK_TX_PHY_CLK_START_20,
    SERIAL_LINK_TX_PHY_CLK_START_21,
    SERIAL_LINK_TX_PHY_CLK_START_22,
    SERIAL_LINK_TX_PHY_CLK_START_23,
    SERIAL_LINK_TX_PHY_CLK_START_24,
    SERIAL_LINK_TX_PHY_CLK_START_25,
    SERIAL_LINK_TX_PHY_CLK_START_26,
    SERIAL_LINK_TX_PHY_CLK_START_27,
    SERIAL_LINK_TX_PHY_CLK_START_28,
    SERIAL_LINK_TX_PHY_CLK_START_29,
    SERIAL_LINK_TX_PHY_CLK_START_30,
    SERIAL_LINK_TX_PHY_CLK_START_31,
    SERIAL_LINK_TX_PHY_CLK_START_32,
    SERIAL_LINK_TX_PHY_CLK_START_33,
    SERIAL_LINK_TX_PHY_CLK_START_34,
    SERIAL_LINK_TX_PHY_CLK_START_35,
    SERIAL_LINK_TX_PHY_CLK_START_36,
    SERIAL_LINK_TX_PHY_CLK_START_37,
    SERIAL_LINK_TX_PHY_CLK_END_0,
    SERIAL_LINK_TX_PHY_CLK_END_1,
    SERIAL_LINK_TX_PHY_CLK_END_2,
    SERIAL_LINK_TX_PHY_CLK_END_3,
    SERIAL_LINK_TX_PHY_CLK_END_4,
    SERIAL_LINK_TX_PHY_CLK_END_5,
    SERIAL_LINK_TX_PHY_CLK_END_6,
    SERIAL_LINK_TX_PHY_CLK_END_7,
    SERIAL_LINK_TX_PHY_CLK_END_8,
    SERIAL_LINK_TX_PHY_CLK_END_9,
    SERIAL_LINK_TX_PHY_CLK_END_10,
    SERIAL_LINK_TX_PHY_CLK_END_11,
    SERIAL_LINK_TX_PHY_CLK_END_12,
    SERIAL_LINK_TX_PHY_CLK_END_13,
    SERIAL_LINK_TX_PHY_CLK_END_14,
    SERIAL_LINK_TX_PHY_CLK_END_15,
    SERIAL_LINK_TX_PHY_CLK_END_16,
    SERIAL_LINK_TX_PHY_CLK_END_17,
    SERIAL_LINK_TX_PHY_CLK_END_18,
    SERIAL_LINK_TX_PHY_CLK_END_19,
    SERIAL_LINK_TX_PHY_CLK_END_20,
    SERIAL_LINK_TX_PHY_CLK_END_21,
    SERIAL_LINK_TX_PHY_CLK_END_22,
    SERIAL_LINK_TX_PHY_CLK_END_23,
    SERIAL_LINK_TX_PHY_CLK_END_24,
    SERIAL_LINK_TX_PHY_CLK_END_25,
    SERIAL_LINK_TX_PHY_CLK_END_26,
    SERIAL_LINK_TX_PHY_CLK_END_27,
    SERIAL_LINK_TX_PHY_CLK_END_28,
    SERIAL_LINK_TX_PHY_CLK_END_29,
    SERIAL_LINK_TX_PHY_CLK_END_30,
    SERIAL_LINK_TX_PHY_CLK_END_31,
    SERIAL_LINK_TX_PHY_CLK_END_32,
    SERIAL_LINK_TX_PHY_CLK_END_33,
    SERIAL_LINK_TX_PHY_CLK_END_34,
    SERIAL_LINK_TX_PHY_CLK_END_35,
    SERIAL_LINK_TX_PHY_CLK_END_36,
    SERIAL_LINK_TX_PHY_CLK_END_37,
    SERIAL_LINK_RAW_MODE_EN,
    SERIAL_LINK_RAW_MODE_IN_CH_SEL,
    SERIAL_LINK_RAW_MODE_IN_DATA_VALID_0,
    SERIAL_LINK_RAW_MODE_IN_DATA_VALID_1,
    SERIAL_LINK_RAW_MODE_IN_DATA,
    SERIAL_LINK_RAW_MODE_OUT_CH_MASK_0,
    SERIAL_LINK_RAW_MODE_OUT_CH_MASK_1,
    SERIAL_LINK_RAW_MODE_OUT_DATA_FIFO,
    SERIAL_LINK_RAW_MODE_OUT_DATA_FIFO_CTRL,
    SERIAL_LINK_RAW_MODE_OUT_EN,
    SERIAL_LINK_FLOW_CONTROL_FIFO_CLEAR,
    SERIAL_LINK_CHANNEL_ALLOC_TX_CFG,
    SERIAL_LINK_CHANNEL_ALLOC_TX_CH_EN_0,
    SERIAL_LINK_CHANNEL_ALLOC_TX_CH_EN_1,
    SERIAL_LINK_CHANNEL_ALLOC_TX_CTRL,
    SERIAL_LINK_CHANNEL_ALLOC_RX_CFG,
    SERIAL_LINK_CHANNEL_ALLOC_RX_CTRL,
    SERIAL_LINK_CHANNEL_ALLOC_RX_CH_EN_0,
    SERIAL_LINK_CHANNEL_ALLOC_RX_CH_EN_1
  } serial_link_id_e;

  // Register width information to check illegal writes
  parameter logic [3:0] SERIAL_LINK_PERMIT [135] = '{
    4'b 0011, // index[  0] SERIAL_LINK_CTRL
    4'b 0001, // index[  1] SERIAL_LINK_ISOLATED
    4'b 0011, // index[  2] SERIAL_LINK_TX_PHY_CLK_DIV_0
    4'b 0011, // index[  3] SERIAL_LINK_TX_PHY_CLK_DIV_1
    4'b 0011, // index[  4] SERIAL_LINK_TX_PHY_CLK_DIV_2
    4'b 0011, // index[  5] SERIAL_LINK_TX_PHY_CLK_DIV_3
    4'b 0011, // index[  6] SERIAL_LINK_TX_PHY_CLK_DIV_4
    4'b 0011, // index[  7] SERIAL_LINK_TX_PHY_CLK_DIV_5
    4'b 0011, // index[  8] SERIAL_LINK_TX_PHY_CLK_DIV_6
    4'b 0011, // index[  9] SERIAL_LINK_TX_PHY_CLK_DIV_7
    4'b 0011, // index[ 10] SERIAL_LINK_TX_PHY_CLK_DIV_8
    4'b 0011, // index[ 11] SERIAL_LINK_TX_PHY_CLK_DIV_9
    4'b 0011, // index[ 12] SERIAL_LINK_TX_PHY_CLK_DIV_10
    4'b 0011, // index[ 13] SERIAL_LINK_TX_PHY_CLK_DIV_11
    4'b 0011, // index[ 14] SERIAL_LINK_TX_PHY_CLK_DIV_12
    4'b 0011, // index[ 15] SERIAL_LINK_TX_PHY_CLK_DIV_13
    4'b 0011, // index[ 16] SERIAL_LINK_TX_PHY_CLK_DIV_14
    4'b 0011, // index[ 17] SERIAL_LINK_TX_PHY_CLK_DIV_15
    4'b 0011, // index[ 18] SERIAL_LINK_TX_PHY_CLK_DIV_16
    4'b 0011, // index[ 19] SERIAL_LINK_TX_PHY_CLK_DIV_17
    4'b 0011, // index[ 20] SERIAL_LINK_TX_PHY_CLK_DIV_18
    4'b 0011, // index[ 21] SERIAL_LINK_TX_PHY_CLK_DIV_19
    4'b 0011, // index[ 22] SERIAL_LINK_TX_PHY_CLK_DIV_20
    4'b 0011, // index[ 23] SERIAL_LINK_TX_PHY_CLK_DIV_21
    4'b 0011, // index[ 24] SERIAL_LINK_TX_PHY_CLK_DIV_22
    4'b 0011, // index[ 25] SERIAL_LINK_TX_PHY_CLK_DIV_23
    4'b 0011, // index[ 26] SERIAL_LINK_TX_PHY_CLK_DIV_24
    4'b 0011, // index[ 27] SERIAL_LINK_TX_PHY_CLK_DIV_25
    4'b 0011, // index[ 28] SERIAL_LINK_TX_PHY_CLK_DIV_26
    4'b 0011, // index[ 29] SERIAL_LINK_TX_PHY_CLK_DIV_27
    4'b 0011, // index[ 30] SERIAL_LINK_TX_PHY_CLK_DIV_28
    4'b 0011, // index[ 31] SERIAL_LINK_TX_PHY_CLK_DIV_29
    4'b 0011, // index[ 32] SERIAL_LINK_TX_PHY_CLK_DIV_30
    4'b 0011, // index[ 33] SERIAL_LINK_TX_PHY_CLK_DIV_31
    4'b 0011, // index[ 34] SERIAL_LINK_TX_PHY_CLK_DIV_32
    4'b 0011, // index[ 35] SERIAL_LINK_TX_PHY_CLK_DIV_33
    4'b 0011, // index[ 36] SERIAL_LINK_TX_PHY_CLK_DIV_34
    4'b 0011, // index[ 37] SERIAL_LINK_TX_PHY_CLK_DIV_35
    4'b 0011, // index[ 38] SERIAL_LINK_TX_PHY_CLK_DIV_36
    4'b 0011, // index[ 39] SERIAL_LINK_TX_PHY_CLK_DIV_37
    4'b 0011, // index[ 40] SERIAL_LINK_TX_PHY_CLK_START_0
    4'b 0011, // index[ 41] SERIAL_LINK_TX_PHY_CLK_START_1
    4'b 0011, // index[ 42] SERIAL_LINK_TX_PHY_CLK_START_2
    4'b 0011, // index[ 43] SERIAL_LINK_TX_PHY_CLK_START_3
    4'b 0011, // index[ 44] SERIAL_LINK_TX_PHY_CLK_START_4
    4'b 0011, // index[ 45] SERIAL_LINK_TX_PHY_CLK_START_5
    4'b 0011, // index[ 46] SERIAL_LINK_TX_PHY_CLK_START_6
    4'b 0011, // index[ 47] SERIAL_LINK_TX_PHY_CLK_START_7
    4'b 0011, // index[ 48] SERIAL_LINK_TX_PHY_CLK_START_8
    4'b 0011, // index[ 49] SERIAL_LINK_TX_PHY_CLK_START_9
    4'b 0011, // index[ 50] SERIAL_LINK_TX_PHY_CLK_START_10
    4'b 0011, // index[ 51] SERIAL_LINK_TX_PHY_CLK_START_11
    4'b 0011, // index[ 52] SERIAL_LINK_TX_PHY_CLK_START_12
    4'b 0011, // index[ 53] SERIAL_LINK_TX_PHY_CLK_START_13
    4'b 0011, // index[ 54] SERIAL_LINK_TX_PHY_CLK_START_14
    4'b 0011, // index[ 55] SERIAL_LINK_TX_PHY_CLK_START_15
    4'b 0011, // index[ 56] SERIAL_LINK_TX_PHY_CLK_START_16
    4'b 0011, // index[ 57] SERIAL_LINK_TX_PHY_CLK_START_17
    4'b 0011, // index[ 58] SERIAL_LINK_TX_PHY_CLK_START_18
    4'b 0011, // index[ 59] SERIAL_LINK_TX_PHY_CLK_START_19
    4'b 0011, // index[ 60] SERIAL_LINK_TX_PHY_CLK_START_20
    4'b 0011, // index[ 61] SERIAL_LINK_TX_PHY_CLK_START_21
    4'b 0011, // index[ 62] SERIAL_LINK_TX_PHY_CLK_START_22
    4'b 0011, // index[ 63] SERIAL_LINK_TX_PHY_CLK_START_23
    4'b 0011, // index[ 64] SERIAL_LINK_TX_PHY_CLK_START_24
    4'b 0011, // index[ 65] SERIAL_LINK_TX_PHY_CLK_START_25
    4'b 0011, // index[ 66] SERIAL_LINK_TX_PHY_CLK_START_26
    4'b 0011, // index[ 67] SERIAL_LINK_TX_PHY_CLK_START_27
    4'b 0011, // index[ 68] SERIAL_LINK_TX_PHY_CLK_START_28
    4'b 0011, // index[ 69] SERIAL_LINK_TX_PHY_CLK_START_29
    4'b 0011, // index[ 70] SERIAL_LINK_TX_PHY_CLK_START_30
    4'b 0011, // index[ 71] SERIAL_LINK_TX_PHY_CLK_START_31
    4'b 0011, // index[ 72] SERIAL_LINK_TX_PHY_CLK_START_32
    4'b 0011, // index[ 73] SERIAL_LINK_TX_PHY_CLK_START_33
    4'b 0011, // index[ 74] SERIAL_LINK_TX_PHY_CLK_START_34
    4'b 0011, // index[ 75] SERIAL_LINK_TX_PHY_CLK_START_35
    4'b 0011, // index[ 76] SERIAL_LINK_TX_PHY_CLK_START_36
    4'b 0011, // index[ 77] SERIAL_LINK_TX_PHY_CLK_START_37
    4'b 0011, // index[ 78] SERIAL_LINK_TX_PHY_CLK_END_0
    4'b 0011, // index[ 79] SERIAL_LINK_TX_PHY_CLK_END_1
    4'b 0011, // index[ 80] SERIAL_LINK_TX_PHY_CLK_END_2
    4'b 0011, // index[ 81] SERIAL_LINK_TX_PHY_CLK_END_3
    4'b 0011, // index[ 82] SERIAL_LINK_TX_PHY_CLK_END_4
    4'b 0011, // index[ 83] SERIAL_LINK_TX_PHY_CLK_END_5
    4'b 0011, // index[ 84] SERIAL_LINK_TX_PHY_CLK_END_6
    4'b 0011, // index[ 85] SERIAL_LINK_TX_PHY_CLK_END_7
    4'b 0011, // index[ 86] SERIAL_LINK_TX_PHY_CLK_END_8
    4'b 0011, // index[ 87] SERIAL_LINK_TX_PHY_CLK_END_9
    4'b 0011, // index[ 88] SERIAL_LINK_TX_PHY_CLK_END_10
    4'b 0011, // index[ 89] SERIAL_LINK_TX_PHY_CLK_END_11
    4'b 0011, // index[ 90] SERIAL_LINK_TX_PHY_CLK_END_12
    4'b 0011, // index[ 91] SERIAL_LINK_TX_PHY_CLK_END_13
    4'b 0011, // index[ 92] SERIAL_LINK_TX_PHY_CLK_END_14
    4'b 0011, // index[ 93] SERIAL_LINK_TX_PHY_CLK_END_15
    4'b 0011, // index[ 94] SERIAL_LINK_TX_PHY_CLK_END_16
    4'b 0011, // index[ 95] SERIAL_LINK_TX_PHY_CLK_END_17
    4'b 0011, // index[ 96] SERIAL_LINK_TX_PHY_CLK_END_18
    4'b 0011, // index[ 97] SERIAL_LINK_TX_PHY_CLK_END_19
    4'b 0011, // index[ 98] SERIAL_LINK_TX_PHY_CLK_END_20
    4'b 0011, // index[ 99] SERIAL_LINK_TX_PHY_CLK_END_21
    4'b 0011, // index[100] SERIAL_LINK_TX_PHY_CLK_END_22
    4'b 0011, // index[101] SERIAL_LINK_TX_PHY_CLK_END_23
    4'b 0011, // index[102] SERIAL_LINK_TX_PHY_CLK_END_24
    4'b 0011, // index[103] SERIAL_LINK_TX_PHY_CLK_END_25
    4'b 0011, // index[104] SERIAL_LINK_TX_PHY_CLK_END_26
    4'b 0011, // index[105] SERIAL_LINK_TX_PHY_CLK_END_27
    4'b 0011, // index[106] SERIAL_LINK_TX_PHY_CLK_END_28
    4'b 0011, // index[107] SERIAL_LINK_TX_PHY_CLK_END_29
    4'b 0011, // index[108] SERIAL_LINK_TX_PHY_CLK_END_30
    4'b 0011, // index[109] SERIAL_LINK_TX_PHY_CLK_END_31
    4'b 0011, // index[110] SERIAL_LINK_TX_PHY_CLK_END_32
    4'b 0011, // index[111] SERIAL_LINK_TX_PHY_CLK_END_33
    4'b 0011, // index[112] SERIAL_LINK_TX_PHY_CLK_END_34
    4'b 0011, // index[113] SERIAL_LINK_TX_PHY_CLK_END_35
    4'b 0011, // index[114] SERIAL_LINK_TX_PHY_CLK_END_36
    4'b 0011, // index[115] SERIAL_LINK_TX_PHY_CLK_END_37
    4'b 0001, // index[116] SERIAL_LINK_RAW_MODE_EN
    4'b 0001, // index[117] SERIAL_LINK_RAW_MODE_IN_CH_SEL
    4'b 1111, // index[118] SERIAL_LINK_RAW_MODE_IN_DATA_VALID_0
    4'b 0001, // index[119] SERIAL_LINK_RAW_MODE_IN_DATA_VALID_1
    4'b 0011, // index[120] SERIAL_LINK_RAW_MODE_IN_DATA
    4'b 1111, // index[121] SERIAL_LINK_RAW_MODE_OUT_CH_MASK_0
    4'b 0001, // index[122] SERIAL_LINK_RAW_MODE_OUT_CH_MASK_1
    4'b 0011, // index[123] SERIAL_LINK_RAW_MODE_OUT_DATA_FIFO
    4'b 1111, // index[124] SERIAL_LINK_RAW_MODE_OUT_DATA_FIFO_CTRL
    4'b 0001, // index[125] SERIAL_LINK_RAW_MODE_OUT_EN
    4'b 0001, // index[126] SERIAL_LINK_FLOW_CONTROL_FIFO_CLEAR
    4'b 0011, // index[127] SERIAL_LINK_CHANNEL_ALLOC_TX_CFG
    4'b 1111, // index[128] SERIAL_LINK_CHANNEL_ALLOC_TX_CH_EN_0
    4'b 0001, // index[129] SERIAL_LINK_CHANNEL_ALLOC_TX_CH_EN_1
    4'b 0001, // index[130] SERIAL_LINK_CHANNEL_ALLOC_TX_CTRL
    4'b 0111, // index[131] SERIAL_LINK_CHANNEL_ALLOC_RX_CFG
    4'b 0001, // index[132] SERIAL_LINK_CHANNEL_ALLOC_RX_CTRL
    4'b 1111, // index[133] SERIAL_LINK_CHANNEL_ALLOC_RX_CH_EN_0
    4'b 0001  // index[134] SERIAL_LINK_CHANNEL_ALLOC_RX_CH_EN_1
  };

endpackage

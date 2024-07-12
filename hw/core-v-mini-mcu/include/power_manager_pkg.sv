// Copyright 2024 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

package power_manager_pkg;

  typedef struct packed {logic pwrgate_ack_n;} power_manager_in_t;

  typedef struct packed {
    logic clkgate_en_n;
    logic pwrgate_en_n;
    logic isogate_en_n;
    logic retentive_en_n;
    logic rst_n;
  } power_manager_out_t;

endpackage

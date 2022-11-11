// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
{ name: "power_manager",
  clock_primary: "clk_i",
  bus_interfaces: [
    { protocol: "reg_iface", direction: "device" }
  ],
  regwidth: "32",
  registers: [
    { name:     "WAKEUP_STATE",
      desc:     "Wake-up state of the system",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "WAKEUP_STATE", desc: "Wake-up state Reg" }
      ]
    }

    { name:     "RESTORE_ADDRESS",
      desc:     "Restore xddress value",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "31:0", name: "RESTORE_XDDRESS", desc: "Restore xddress Reg" }
      ]
    }

    { name:     "POWER_GATE_CORE",
      desc:     "Used to power gate core",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "POWER_GATE_CORE", desc: "Power Gate Core Reg" }
      ]
    }

    { name:     "POWER_GATE_CORE_ACK",
      desc:     "Used by the core switch to ack the power manager",
      resval:   "0x00000000"
      swaccess: "ro",
      hwaccess: "hrw",
      fields: [
        { bits: "0", name: "POWER_GATE_CORE_ACK", desc: "Power Gate Core Ack Reg" }
      ]
    }

    { name:     "POWER_GATE_PERIPH",
      desc:     "Used to power gate peripheral_subsystem",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "POWER_GATE_PERIPH", desc: "Power Gate Periph Reg" }
      ]
    }

    { name:     "POWER_GATE_PERIPH_ACK",
      desc:     "Used by the periph switch to ack the power manager",
      resval:   "0x00000000"
      swaccess: "ro",
      hwaccess: "hrw",
      fields: [
        { bits: "0", name: "POWER_GATE_PERIPH_ACK", desc: "Power Gate Periph Ack Reg" }
      ]
    }

% for bank in range(ram_numbanks):
    { name:     "POWER_GATE_RAM_BLOCK_${bank}",
      desc:     "Used to power gate ram block ${bank}",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "POWER_GATE_RAM_BLOCK_${bank}", desc: "Power Gate Ram Block ${bank} Reg" }
      ]
    }

    { name:     "POWER_GATE_RAM_BLOCK_${bank}_ACK",
      desc:     "Used by the ram ${bank} switch to ack the power manager",
      resval:   "0x00000000"
      swaccess: "ro",
      hwaccess: "hrw",
      fields: [
        { bits: "0", name: "POWER_GATE_RAM_BLOCK_${bank}_ACK", desc: "Power Gate Ram Block ${bank} Ack Reg" }
      ]
    }

% endfor
% for bank in range(ram_numbanks):
    { name:     "SET_RETENTIVE_RAM_BLOCK_${bank}",
      desc:     "Used to set in retentive mode ram block ${bank}",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "SET_RETENTIVE_RAM_BLOCK_${bank}", desc: "Set Retentive Ram Block ${bank} Reg" }
      ]
    }

% endfor
% for ext in range(external_domains):
    { name:     "POWER_GATE_EXTERNAL_${ext}",
      desc:     "Used to power gate external ${ext} domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "POWER_GATE_EXTERNAL_${ext}", desc: "Power Gate External ${ext} Reg" }
      ]
    }

    { name:     "POWER_GATE_EXTERNAL_${ext}_ACK",
      desc:     "Used by the external ${ext} switch to ack the power manager",
      resval:   "0x00000000"
      swaccess: "ro",
      hwaccess: "hrw",
      fields: [
        { bits: "0", name: "POWER_GATE_EXTERNAL_${ext}_ACK", desc: "Power Gate External ${ext} Ack Reg" }
      ]
    }

% endfor
% for ext_bank in range(external_domains):
    { name:     "SET_RETENTIVE_EXTERNAL_RAM_BLOCK_${ext_bank}",
      desc:     "Used to set in retentive mode external ram block ${ext_bank}",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "SET_RETENTIVE_EXTERNAL_RAM_BLOCK_${ext_bank}", desc: "Set Retentive External Ram Block ${bank} Reg" }
      ]
    }

% endfor
    { name:     "EN_WAIT_FOR_INTR",
      desc:     "Enable wait for interrupt",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "31:0", name: "EN_WAIT_FOR_INTR", desc: "Enable wait for interrupt" }
      ]
    }

    { name:     "INTR_STATE",
      desc:     "Interrupt state",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "INTR_STATE", desc: "Interrupt state Reg" }
      ]
    }

    { name:     "CPU_RESET_ASSERT_COUNTER",
      desc:     "Counter before resetting the CPU domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "CPU_RESET_ASSERT_COUNTER", desc: "CPU counter before resetting" }
      ]
    }

    { name:     "CPU_RESET_DEASSERT_COUNTER",
      desc:     "Counter before unreset the CPU domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "CPU_RESET_DEASSERT_COUNTER", desc: "CPU counter before unresetting" }
      ]
    }

    { name:     "CPU_SWITCH_OFF_COUNTER",
      desc:     "Counter before switching off the CPU domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "CPU_SWITCH_OFF_COUNTER", desc: "CPU counter before switching off" }
      ]
    }

    { name:     "CPU_SWITCH_ON_COUNTER",
      desc:     "Counter before switching on the CPU domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "CPU_SWITCH_ON_COUNTER", desc: "CPU counter before switching on" }
      ]
    }

    { name:     "CPU_WAIT_ACK_SWITCH_ON_COUNTER",
      desc:     "Bit to set whether to further wait for the ACK from the core switch after the counter expired",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "CPU_WAIT_ACK_SWITCH_ON_COUNTER", desc: "wait for CPU switch ack before switching on" }
      ]
    }

    { name:     "CPU_ISO_OFF_COUNTER",
      desc:     "Counter before setting off the isolation of the CPU domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "CPU_ISO_OFF_COUNTER", desc: "CPU counter before setting off isolation" }
      ]
    }

    { name:     "CPU_ISO_ON_COUNTER",
      desc:     "Counter before setting on the isolation of the CPU domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "CPU_ISO_ON_COUNTER", desc: "CPU counter before setting on isolation" }
      ]
    }

    { name:     "CPU_COUNTERS_STOP",
      desc:     "Bits to stop the counters keeping the done_o signal high",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "CPU_RESET_ASSERT_STOP_BIT_COUNTER", desc: "Stop the CPU_RESET_ASSERT_COUNTER counter" }
        { bits: "1", name: "CPU_RESET_DEASSERT_STOP_BIT_COUNTER", desc: "Stop the CPU_RESET_DEASSERT_COUNTER counter" }
        { bits: "2", name: "CPU_SWITCH_OFF_STOP_BIT_COUNTER", desc: "Stop the CPU_SWITCH_OFF_COUNTER counter" }
        { bits: "3", name: "CPU_SWITCH_ON_STOP_BIT_COUNTER", desc: "Stop the CPU_SWITCH_ON_COUNTER counter" }
        { bits: "4", name: "CPU_ISO_OFF_STOP_BIT_COUNTER", desc: "Stop the CPU_ISO_OFF_COUNTER counter" }
        { bits: "5", name: "CPU_ISO_ON_STOP_BIT_COUNTER", desc: "Stop the CPU_ISO_ON_COUNTER counter" }
      ]
    }

    { name:     "PERIPH_RESET_ASSERT_COUNTER",
      desc:     "Counter before resetting the PERIPH domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "PERIPH_RESET_ASSERT_COUNTER", desc: "PERIPH counter before resetting" }
      ]
    }

    { name:     "PERIPH_RESET_DEASSERT_COUNTER",
      desc:     "Counter before unreset the PERIPH domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "PERIPH_RESET_DEASSERT_COUNTER", desc: "PERIPH counter before unresetting" }
      ]
    }

    { name:     "PERIPH_SWITCH_OFF_COUNTER",
      desc:     "Counter before switching off the PERIPH domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "PERIPH_SWITCH_OFF_COUNTER", desc: "PERIPH counter before switching off" }
      ]
    }

    { name:     "PERIPH_SWITCH_ON_COUNTER",
      desc:     "Counter before switching on the PERIPH domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "PERIPH_SWITCH_ON_COUNTER", desc: "PERIPH counter before switching on" }
      ]
    }

    { name:     "PERIPH_WAIT_ACK_SWITCH_ON_COUNTER",
      desc:     "Bit to set whether to further wait for the ACK from the periph switch after the counter expired",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "PERIPH_WAIT_ACK_SWITCH_ON_COUNTER", desc: "wait for PERIPH switch ack before switching on" }
      ]
    }

    { name:     "PERIPH_ISO_OFF_COUNTER",
      desc:     "Counter before setting off the isolation of the PERIPH domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "PERIPH_ISO_OFF_COUNTER", desc: "PERIPH counter before setting off isolation" }
      ]
    }

    { name:     "PERIPH_ISO_ON_COUNTER",
      desc:     "Counter before setting on the isolation of the PERIPH domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "PERIPH_ISO_ON_COUNTER", desc: "PERIPH counter before setting on isolation" }
      ]
    }

    { name:     "PERIPH_COUNTERS_STOP",
      desc:     "Bits to stop the counters keeping the done_o signal high",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "PERIPH_RESET_ASSERT_STOP_BIT_COUNTER", desc: "Stop the PERIPH_RESET_ASSERT_COUNTER counter" }
        { bits: "1", name: "PERIPH_RESET_DEASSERT_STOP_BIT_COUNTER", desc: "Stop the PERIPH_RESET_DEASSERT_COUNTER counter" }
        { bits: "2", name: "PERIPH_SWITCH_OFF_STOP_BIT_COUNTER", desc: "Stop the PERIPH_SWITCH_OFF_COUNTER counter" }
        { bits: "3", name: "PERIPH_SWITCH_ON_STOP_BIT_COUNTER", desc: "Stop the PERIPH_SWITCH_ON_COUNTER counter" }
        { bits: "4", name: "PERIPH_ISO_OFF_STOP_BIT_COUNTER", desc: "Stop the PERIPH_ISO_OFF_COUNTER counter" }
        { bits: "5", name: "PERIPH_ISO_ON_STOP_BIT_COUNTER", desc: "Stop the PERIPH_ISO_ON_COUNTER counter" }
      ]
    }

% for bank in range(ram_numbanks):
    { name:     "RAM_${bank}_SWITCH_OFF_COUNTER",
      desc:     "Counter before switching off the RAM_${bank} domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "RAM_${bank}_SWITCH_OFF_COUNTER", desc: "RAM_${bank} counter before switching off" }
      ]
    }

    { name:     "RAM_${bank}_SWITCH_ON_COUNTER",
      desc:     "Counter before switching on the RAM_${bank} domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "RAM_${bank}_SWITCH_ON_COUNTER", desc: "RAM_${bank} counter before switching on" }
      ]
    }

    { name:     "RAM_${bank}_WAIT_ACK_SWITCH_ON_COUNTER",
      desc:     "Bit to set whether to further wait for the ACK from the memory ${bank} switch after the counter expired",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "RAM_${bank}_WAIT_ACK_SWITCH_ON_COUNTER", desc: "wait for RAM_${bank} switch ack before switching on" }
      ]
    }

    { name:     "RAM_${bank}_ISO_OFF_COUNTER",
      desc:     "Counter before setting off the isolation of the RAM_${bank} domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "RAM_${bank}_ISO_OFF_COUNTER", desc: "RAM_${bank} counter before setting off isolation" }
      ]
    }

    { name:     "RAM_${bank}_ISO_ON_COUNTER",
      desc:     "Counter before setting on the isolation of the RAM_${bank} domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "RAM_${bank}_ISO_ON_COUNTER", desc: "RAM_${bank} counter before setting on isolation" }
      ]
    }

    { name:     "RAM_${bank}_RETENTIVE_OFF_COUNTER",
      desc:     "Counter before setting off retentive mode for the RAM_${bank} domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "RAM_${bank}_RETENTIVE_OFF_COUNTER", desc: "RAM_${bank} counter before setting off retentive" }
      ]
    }

    { name:     "RAM_${bank}_RETENTIVE_ON_COUNTER",
      desc:     "Counter before setting on retentive mode for the RAM_${bank} domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "RAM_${bank}_RETENTIVE_ON_COUNTER", desc: "RAM_${bank} counter before setting on retentive" }
      ]
    }

    { name:     "RAM_${bank}_COUNTERS_STOP",
      desc:     "Bits to stop the counters keeping the done_o signal high",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "RAM_${bank}_SWITCH_OFF_STOP_BIT_COUNTER", desc: "Stop the RAM_${bank}_SWITCH_OFF_COUNTER counter" }
        { bits: "1", name: "RAM_${bank}_SWITCH_ON_STOP_BIT_COUNTER", desc: "Stop the RAM_${bank}_SWITCH_ON_COUNTER counter" }
        { bits: "2", name: "RAM_${bank}_ISO_OFF_STOP_BIT_COUNTER", desc: "Stop the RAM_${bank}_ISO_OFF_COUNTER counter" }
        { bits: "3", name: "RAM_${bank}_ISO_ON_STOP_BIT_COUNTER", desc: "Stop the RAM_${bank}_ISO_ON_COUNTER counter" }
        { bits: "4", name: "RAM_${bank}_RETENTIVE_OFF_STOP_BIT_COUNTER", desc: "Stop the RAM_${bank}_RETENTIVE_OFF_COUNTER counter" }
        { bits: "5", name: "RAM_${bank}_RETENTIVE_ON_STOP_BIT_COUNTER", desc: "Stop the RAM_${bank}_RETENTIVE_ON_COUNTER counter" }
      ]
    }

% endfor
% for ext in range(external_domains):
    { name:     "EXTERNAL_${ext}_RESET_ASSERT_COUNTER",
      desc:     "Counter before resetting the EXTERNAL ${ext} domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "EXTERNAL_${ext}_RESET_ASSERT_COUNTER", desc: "EXTERNAL ${ext} counter before resetting" }
      ]
    }

    { name:     "EXTERNAL_${ext}_RESET_DEASSERT_COUNTER",
      desc:     "Counter before unreset the EXTERNAL ${ext} domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "EXTERNAL_${ext}_RESET_DEASSERT_COUNTER", desc: "EXTERNAL ${ext} counter before unresetting" }
      ]
    }

    { name:     "EXTERNAL_${ext}_SWITCH_OFF_COUNTER",
      desc:     "Counter before switching off the EXTERNAL ${ext} domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "EXTERNAL_${ext}_SWITCH_OFF_COUNTER", desc: "EXTERNAL ${ext} counter before switching off" }
      ]
    }

    { name:     "EXTERNAL_${ext}_SWITCH_ON_COUNTER",
      desc:     "Counter before switching on the EXTERNAL ${ext} domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "EXTERNAL_${ext}_SWITCH_ON_COUNTER", desc: "EXTERNAL ${ext} counter before switching on" }
      ]
    }

    { name:     "EXTERNAL_${ext}_WAIT_ACK_SWITCH_ON_COUNTER",
      desc:     "Bit to set whether to further wait for the ACK from the external ${ext} switch after the counter expired",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "EXTERNAL_${ext}_WAIT_ACK_SWITCH_ON_COUNTER", desc: "wait for external ${ext} switch ack before switching on" }
      ]
    }

    { name:     "EXTERNAL_${ext}_ISO_OFF_COUNTER",
      desc:     "Counter before setting off the isolation of the EXTERNAL ${ext} domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "EXTERNAL_${ext}_ISO_OFF_COUNTER", desc: "EXTERNAL ${ext} counter before setting off isolation" }
      ]
    }

    { name:     "EXTERNAL_${ext}_ISO_ON_COUNTER",
      desc:     "Counter before setting on the isolation of the EXTERNAL ${ext} domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "EXTERNAL_${ext}_ISO_ON_COUNTER", desc: "EXTERNAL ${ext} counter before setting on isolation" }
      ]
    }

    { name:     "EXTERNAL_${ext}_COUNTERS_STOP",
      desc:     "Bits to stop the counters keeping the done_o signal high",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "EXTERNAL_${ext}_RESET_ASSERT_STOP_BIT_COUNTER", desc: "Stop the EXTERNAL_${ext}_RESET_ASSERT_COUNTER counter" }
        { bits: "1", name: "EXTERNAL_${ext}_RESET_DEASSERT_STOP_BIT_COUNTER", desc: "Stop the EXTERNAL_${ext}_RESET_DEASSERT_COUNTER counter" }
        { bits: "2", name: "EXTERNAL_${ext}_SWITCH_OFF_STOP_BIT_COUNTER", desc: "Stop the EXTERNAL_${ext}_SWITCH_OFF_COUNTER counter" }
        { bits: "3", name: "EXTERNAL_${ext}_SWITCH_ON_STOP_BIT_COUNTER", desc: "Stop the EXTERNAL_${ext}_SWITCH_ON_COUNTER counter" }
        { bits: "4", name: "EXTERNAL_${ext}_ISO_OFF_STOP_BIT_COUNTER", desc: "Stop the EXTERNAL_${ext}_ISO_OFF_COUNTER counter" }
        { bits: "5", name: "EXTERNAL_${ext}_ISO_ON_STOP_BIT_COUNTER", desc: "Stop the EXTERNAL_${ext}_ISO_ON_COUNTER counter" }
      ]
    }

% endfor
% for ext_bank in range(external_domains):
    { name:     "EXTERNAL_RAM_${ext_bank}_RETENTIVE_OFF_COUNTER",
      desc:     "Counter before setting off retentive mode for external RAM_${ext_bank}",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "EXTERNAL_RAM_${ext_bank}_RETENTIVE_OFF_COUNTER", desc: "External RAM_${ext_bank} counter before setting off retentive" }
      ]
    }

    { name:     "EXTERNAL_RAM_${ext_bank}_RETENTIVE_ON_COUNTER",
      desc:     "Counter before setting on retentive mode for external RAM_${ext_bank}",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "EXTERNAL_RAM_${ext_bank}_RETENTIVE_ON_COUNTER", desc: "External RAM_${ext_bank} counter before setting on retentive" }
      ]
    }

    { name:     "EXTERNAL_RAM_${ext_bank}_COUNTERS_STOP",
      desc:     "Bits to stop the counters keeping the done_o signal high",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "EXTERNAL_RAM_${ext_bank}_RETENTIVE_OFF_STOP_BIT_COUNTER", desc: "Stop the EXTERNAL_RAM_${ext_bank}_RETENTIVE_OFF_COUNTER counter" }
        { bits: "1", name: "EXTERNAL_RAM_${ext_bank}_RETENTIVE_ON_STOP_BIT_COUNTER", desc: "Stop the EXTERNAL_RAM_${ext_bank}_RETENTIVE_ON_COUNTER counter" }
      ]
    }

% endfor
    { name:     "MONITOR_POWER_GATE_CORE",
      desc:     "Used to monitor the signals to power gate core",
      resval:   "0x00000000"
      swaccess: "ro",
      hwaccess: "hwo",
      fields: [
        { bits: "2:0", name: "MONITOR_POWER_GATE_CORE", desc: "Monitor Signals Power Gate Core Reg" }
      ]
    }

    { name:     "MONITOR_POWER_GATE_PERIPH",
      desc:     "Used to monitor the signals to power gate periph",
      resval:   "0x00000000"
      swaccess: "ro",
      hwaccess: "hwo",
      fields: [
        { bits: "2:0", name: "MONITOR_POWER_GATE_PERIPH", desc: "Monitor Signals Power Gate Periph Reg" }
      ]
    }

% for bank in range(ram_numbanks):
    { name:     "MONITOR_POWER_GATE_RAM_BLOCK_${bank}",
      desc:     "Used to monitor the signals to power gate ram block ${bank}",
      resval:   "0x00000000"
      swaccess: "ro",
      hwaccess: "hwo",
      fields: [
        { bits: "1:0", name: "MONITOR_POWER_GATE_RAM_BLOCK_${bank}", desc: "Monitor Signals Power Gate Ram Block ${bank} Reg" }
      ]
    }

% endfor
% for ext in range(external_domains):
    { name:     "MONITOR_POWER_GATE_EXTERNAL_${ext}",
      desc:     "Used to monitor the signals to power gate external ${ext}",
      resval:   "0x00000000"
      swaccess: "ro",
      hwaccess: "hwo",
      fields: [
        { bits: "2:0", name: "MONITOR_POWER_GATE_EXTERNAL_${ext}", desc: "Monitor Signals Power Gate External ${ext} Reg" }
      ]
    }

% endfor
    { name:     "CPU_FORCE_SLEEP",
      desc:     "Used to force the core to sleep",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "CPU_FORCE_SLEEP", desc: "Force Core Sleep Reg" }
      ]
    }

    { name:     "CPU_FORCE_WAKEUP",
      desc:     "Used to force the core to wakeup",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "CPU_FORCE_WAKEUP", desc: "Force Core Wakeup Reg" }
      ]
    }

    { name:     "CPU_FORCE_RESET",
      desc:     "Used to force the core reset",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "CPU_FORCE_RESET", desc: "Force Core Reset Reg" }
      ]
    }

    { name:     "CPU_FORCE_ISO",
      desc:     "Used to force the core iso",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "CPU_FORCE_ISO", desc: "Force Core Iso Reg" }
      ]
    }

   ]
}

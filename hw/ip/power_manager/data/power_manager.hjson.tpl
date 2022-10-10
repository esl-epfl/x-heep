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
    { name:     "POWER_GATE_CORE",
      desc:     "Used to power gate core",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "POWER_GATE_CORE", desc: "Power Gate Core Reg" }
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

    { name:     "POWER_GATE_RAM_BLOCK_0",
      desc:     "Used to power gate ram block 0",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "POWER_GATE_RAM_BLOCK_0", desc: "Power Gate Ram Block 0 Reg" }
      ]
    }

    { name:     "POWER_GATE_RAM_BLOCK_1",
      desc:     "Used to power gate ram block 1",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "POWER_GATE_RAM_BLOCK_1", desc: "Power Gate Ram Block 1 Reg" }
      ]
    }

    { name:     "POWER_GATE_RAM_BLOCK_2",
      desc:     "Used to power gate ram block 2",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "POWER_GATE_RAM_BLOCK_2", desc: "Power Gate Ram Block 2 Reg" }
      ]
    }

    { name:     "POWER_GATE_RAM_BLOCK_3",
      desc:     "Used to power gate ram block 3",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "POWER_GATE_RAM_BLOCK_3", desc: "Power Gate Ram Block 3 Reg" }
      ]
    }

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

% for i in range(31):
    { name:     "CORE_REG_X${i+1}",
      desc:     "Core reg x${i+1} value",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "31:0", name: "CORE_REG_X${i+1}", desc: "Core reg x${i+1} Reg" }
      ]
    }

% endfor
    { name:     "CORE_CSR_MSTATUS",
      desc:     "Core csr mstatus value",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "31:0", name: "CORE_CSR_MSTATUS", desc: "Core reg mstatus Reg" }
      ]
    }

    { name:     "CORE_CSR_MIE",
      desc:     "Core csr mie value",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "31:0", name: "CORE_CSR_MIE", desc: "Core reg mie Reg" }
      ]
    }

    { name:     "CORE_CSR_MSCRATCH",
      desc:     "Core csr mscratch value",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "31:0", name: "CORE_CSR_MSCRATCH", desc: "Core reg mscratch Reg" }
      ]
    }

    { name:     "CORE_CSR_MEPC",
      desc:     "Core csr mepc value",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "31:0", name: "CORE_CSR_MEPC", desc: "Core reg mepc Reg" }
      ]
    }

    { name:     "CORE_CSR_MCAUSE",
      desc:     "Core csr mcause value",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "31:0", name: "CORE_CSR_MCAUSE", desc: "Core reg mcause Reg" }
      ]
    }

    { name:     "CORE_CSR_MTVAL",
      desc:     "Core csr mtval value",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "31:0", name: "CORE_CSR_MTVAL", desc: "Core reg mtval Reg" }
      ]
    }

    { name:     "CORE_CSR_MTVEC",
      desc:     "Core csr mtvec value",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "31:0", name: "CORE_CSR_MTVEC", desc: "Core reg mtvec Reg" }
      ]
    }

    { name:     "CORE_CSR_DCSR",
      desc:     "Core csr dcsr value",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "31:0", name: "CORE_CSR_DCSR", desc: "Core reg dcsr Reg" }
      ]
    }

    { name:     "CORE_CSR_MCYCLE",
      desc:     "Core csr mcycle value",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "31:0", name: "CORE_CSR_MCYCLE", desc: "Core reg mcycle Reg" }
      ]
    }

    { name:     "CORE_CSR_MINSTRET",
      desc:     "Core csr minstret value",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "31:0", name: "CORE_CSR_MINSTRET", desc: "Core reg minstret Reg" }
      ]
    }

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
      ]
    }

% for bank in range(ram_numbanks):
    { name:     "RAM_${bank}_RESET_ASSERT_COUNTER",
      desc:     "Counter before resetting the RAM_${bank} domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "RAM_${bank}_RESET_ASSERT_COUNTER", desc: "RAM_${bank} counter before resetting" }
      ]
    }

    { name:     "RAM_${bank}_RESET_DEASSERT_COUNTER",
      desc:     "Counter before unreset the RAM_${bank} domain",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hrw",
      fields: [
        { bits: "31:0", name: "RAM_${bank}_RESET_DEASSERT_COUNTER", desc: "RAM_${bank} counter before unresetting" }
      ]
    }

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

    { name:     "RAM_${bank}_COUNTERS_STOP",
      desc:     "Bits to stop the counters keeping the done_o signal high",
      resval:   "0x00000000"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "0", name: "RAM_${bank}_RESET_ASSERT_STOP_BIT_COUNTER", desc: "Stop the RAM_${bank}_RESET_ASSERT_COUNTER counter" }
        { bits: "1", name: "RAM_${bank}_RESET_DEASSERT_STOP_BIT_COUNTER", desc: "Stop the RAM_${bank}_RESET_DEASSERT_COUNTER counter" }
        { bits: "2", name: "RAM_${bank}_SWITCH_OFF_STOP_BIT_COUNTER", desc: "Stop the RAM_${bank}_SWITCH_OFF_COUNTER counter" }
        { bits: "3", name: "RAM_${bank}_SWITCH_ON_STOP_BIT_COUNTER", desc: "Stop the RAM_${bank}_SWITCH_ON_COUNTER counter" }
      ]
    }

% endfor
   ]
}

// See LICENSE.SiFive for license details.

#include "core_v_mini_mcu.h"
#include "soc_ctrl_regs.h"

#define SOC_CTRL_START_ADDRESS_20bit (SOC_CTRL_START_ADDRESS >> 12)
#define SPI_FLASH_START_ADDRESS_20bit (SPI_FLASH_START_ADDRESS >> 12)

#define SEXT_IMM(x) ((x) | (-(((x) >> 11) & 1) << 11))

        .option norvc
        .global entry

entry:
       // read boot sel register
       lui a1, SOC_CTRL_START_ADDRESS_20bit
       lbu a0, SOC_CTRL_BOOT_SELECT_REG_OFFSET(a1)
       bnez a0, _jump_to_flash


_jump_to_debug_rom:
       lbu a0, SOC_CTRL_BOOT_EXIT_LOOP_REG_OFFSET(a1)
       //check whether exit_loop flag is 1, otherwise jump back to entry
       beqz a0, entry
       lw a1, SOC_CTRL_BOOT_ADDRESS_REG_OFFSET(a1)
       jalr a1

_jump_to_flash:
       lui a1, SPI_FLASH_START_ADDRESS_20bit
       jalr a1

// See LICENSE.SiFive for license details.

#include "core_v_mini_mcu.h"
#include "soc_ctrl_regs.h"
#include "spimem_regs.h"
#include "power_manager_regs.h"
#include "spi_host_regs.h"

#define SOC_CTRL_START_ADDRESS_20bit (SOC_CTRL_START_ADDRESS >> 12)
#define SPI_FLASH_START_ADDRESS_20bit (SPI_FLASH_START_ADDRESS >> 12)
#define SPI_MEMIO_START_ADDRESS_20bit (SPI_MEMIO_START_ADDRESS >> 12)
#define POWER_MANAGER_START_ADDRESS_20bit (POWER_MANAGER_START_ADDRESS >> 12)
#define SPI_HOST_START_ADDRESS_20bit (SPI_HOST_START_ADDRESS >> 12)

#define SEXT_IMM(x) ((x) | (-(((x) >> 11) & 1) << 11))

       .option norvc
       .global entry

entry:
       // Check if recovering from sleep state
       lui     a1, POWER_MANAGER_START_ADDRESS_20bit
       lbu     a0, POWER_MANAGER_WAKEUP_STATE_REG_OFFSET(a1)
       beqz    a0, boot
       // Restoring
       lw      a0, POWER_MANAGER_RESTORE_ADDRESS_REG_OFFSET(a1)
       jalr    a0
boot:
       // Read boot sel register
       lui     a1, SOC_CTRL_START_ADDRESS_20bit
       lbu     a0, SOC_CTRL_BOOT_SELECT_REG_OFFSET(a1)
       bnez    a0, _jump_to_flash

_jump_to_debug_rom:
       lbu    a0, SOC_CTRL_BOOT_EXIT_LOOP_REG_OFFSET(a1)
       // Check whether exit_loop flag is 1, otherwise jump back to entry
       beqz   a0, entry
       lw     a1, SOC_CTRL_BOOT_ADDRESS_REG_OFFSET(a1)
       jalr   a1

_jump_to_flash:
       lbu    a0, SOC_CTRL_USE_SPIMEMIO_REG_OFFSET(a1)
       beqz   a0, _copy_from_flash

_execute_from_flash:
       lui    a1, SPI_MEMIO_START_ADDRESS_20bit
       addi   a0, zero, 1
       sw     a0, OBI_SPIMEMIO_START_SPIMEM_REG_OFFSET(a1)
       lui    a1, SPI_FLASH_START_ADDRESS_20bit
       addi   a1, a1, SEXT_IMM(0x180+0x10)
       jalr   a1

_copy_from_flash:
       lui    a1, SPI_HOST_START_ADDRESS_20bit
       // Enable spi
       lui    a0, 0x80000
       lw     a4, SPI_HOST_CONTROL_REG_OFFSET(a1)
       or     a4, a4, a0
       sw     a4, SPI_HOST_CONTROL_REG_OFFSET(a1)
       // Create config_opt: 0x0FFF0001
       lui    a4, 0xfff0
       addi   a4, a4, 1
       sw     a4, SPI_HOST_CONFIGOPTS_0_REG_OFFSET(a1)
       // Set spi csid
       li     a0, 0
       sw     a0, SPI_HOST_CSID_REG_OFFSET(a1)
       // Set spi receiver watermark to 3
       lw     a4, SPI_HOST_CONTROL_REG_OFFSET(a1)
       andi   a4, a4, -256 # ~SPI_HOST_CONTROL_RX_WATERMARK_MASK
       ori    a4, a4, 3
       sw     a4, SPI_HOST_CONTROL_REG_OFFSET(a1)

       // Power up flash (0xab flash command)
       li     a4, 0xab
       sw     a4, SPI_HOST_DATA_REG_OFFSET(a1)
       // Power up command: 0x00002003 (transmit 4 words)
       lui    a4, 0x2
       addi   a4, a4, 3
       sw     a4, SPI_HOST_COMMAND_REG_OFFSET(a1)

_wait_spi_ready_cmd_pwr:
       // Wait spi is ready for next command
       lw     a4, SPI_HOST_STATUS_REG_OFFSET(a1)
       bgez   a4, _wait_spi_ready_cmd_pwr

       // Fill TX FIFO with TX data (read command 0x03 + 3B address 0x000)
       li     a4, 0x03
       sw     a4, SPI_HOST_DATA_REG_OFFSET(a1)

_wait_spi_ready_tx_init:
       nop    # otherwise ready bit check is too fast
       lw     a4, SPI_HOST_STATUS_REG_OFFSET(a1)
       bgez   a4, _wait_spi_ready_tx_init
       // Read command: 0x00002203
       lui    a4, 0x2
       addi   a4, a4, 515 # spi cmd: txonly + stdspeed + csaat + 4B
       sw     a4, SPI_HOST_COMMAND_REG_OFFSET(a1)

_wait_spi_ready_read_init_cmd:
       nop    # otherwise ready bit check is too fast
       lw     a4, SPI_HOST_STATUS_REG_OFFSET(a1)
       bgez   a4, _wait_spi_ready_read_init_cmd
       // Read 3 words @0x0000: source address, copy size, and ram destination address
       lui    a4, 0x1
       addi   a4, a4, 11 # spi cmd: rxonly + stdspeed + 3 words (12B)
       sw     a4, SPI_HOST_COMMAND_REG_OFFSET(a1)

_wait_spi_ready_rx_cmd:
       nop    # otherwise ready bit check is too fast
       lw     a4, SPI_HOST_STATUS_REG_OFFSET(a1)
       bgez   a4, _wait_spi_ready_rx_cmd

_wait_spi_rxwm_init:
       // Poll spi status register and check rxwm bit
       lw     a4, SPI_HOST_STATUS_REG_OFFSET(a1)
       srli   a4, a4, SPI_HOST_STATUS_RXWM_BIT
       andi   a4, a4, 1
       beqz   a4, _wait_spi_rxwm_init
       // Load the three words
       lw     a2, SPI_HOST_DATA_REG_OFFSET(a1) # src ptr (flash)
       lw     a3, SPI_HOST_DATA_REG_OFFSET(a1) # copy size
       lw     s1, SPI_HOST_DATA_REG_OFFSET(a1) # dst ptr (ram)

       // Set spi receiver watermark to 8
       lw     a5, SPI_HOST_CONTROL_REG_OFFSET(a1)
       andi   a5, a5, -256 # ~SPI_HOST_CONTROL_RX_WATERMARK_MASK
       ori    a5, a5, 8
       sw     a5, SPI_HOST_CONTROL_REG_OFFSET(a1)

       // ((x << 24) | (((x>>16)<<24)>>16) |  (((x<<16)>>24)<<16))
       slli   a4, a2, 24
       srli   s0, a2, 16
       slli   s0, s0, 24
       srli   s0, s0, 16
       slli   a5, a2, 16
       srli   a5, a5, 24
       slli   a5, a5, 16
       or     a4, a4, s0
       or     a2, a4, a5
       or     a2, a2, 0x03
       sw     a2, SPI_HOST_DATA_REG_OFFSET(a1)

_wait_spi_ready_read_prog:
       nop    # otherwise ready bit check is too fast
       lw     a5, SPI_HOST_STATUS_REG_OFFSET(a1)
       bgez   a5, _wait_spi_ready_read_prog
       // Read command: 0x00002203
       lui    a4, 0x2
       addi   a4, a4, 515 # spi cmd: txonly + stdspeed + csaat + 4B
       sw     a4, SPI_HOST_COMMAND_REG_OFFSET(a1)

_wait_spi_ready_copy_cmd:
       nop    # otherwise ready bit check is too fast
       lw     a4, SPI_HOST_STATUS_REG_OFFSET(a1)
       bgez   a4, _wait_spi_ready_copy_cmd

       // For loop until the copy from flash to ram is done
       // Try first to do as much as 256-bytes copies as possible
       mv     s0, a3
       li     s6, 256
       lui    s5, 0x1
       addi   s5, s5, 767 # spi cmd: rxonly + stdspeed + csaat + 255 bytes

_32B_chunk_loop:
       blt    s6, s0, _read_32B_chunk
       // Read remaining bytes
       lui    a5, 0x1
       addi   a4, s0, -1
       or     a5, a5, a4 # spi cmd: rxonly + stdspeed + remaining bytes (s0-1)
       sw     a5, SPI_HOST_COMMAND_REG_OFFSET(a1)

_wait_spi_ready_remaining_bytes:
       nop    # otherwise ready bit check is too fast
       lw     a5, SPI_HOST_STATUS_REG_OFFSET(a1)
       bgez   a5, _wait_spi_ready_remaining_bytes
       li     s4, 32
       j      _last_bytes_read_start

_read_32B_chunk:
       sw     s5, SPI_HOST_COMMAND_REG_OFFSET(a1)

_wait_spi_ready_read_32B_chunk:
       nop    # otherwise ready bit check is too fast
       lw     a5, SPI_HOST_STATUS_REG_OFFSET(a1)
       bgez   a5, _wait_spi_ready_read_32B_chunk
       addi   s7, s1, 256 # add 32*8 (256 bytes) to dst ptr

_wait_spi_rxwm_8_words:
       lw     a5, SPI_HOST_STATUS_REG_OFFSET(a1)
       srli   a5, a5, 0x14
       andi   a5, a5, 1 # SPI_HOST_STATUS_RXWM_BIT
       beqz   a5, _wait_spi_rxwm_8_words
       // Read 32B (8 words) from RX FIFO
       lw     a7, SPI_HOST_DATA_REG_OFFSET(a1)
       lw     t0, SPI_HOST_DATA_REG_OFFSET(a1)
       lw     t1, SPI_HOST_DATA_REG_OFFSET(a1)
       lw     t2, SPI_HOST_DATA_REG_OFFSET(a1)
       lw     t3, SPI_HOST_DATA_REG_OFFSET(a1)
       lw     t4, SPI_HOST_DATA_REG_OFFSET(a1)
       lw     t5, SPI_HOST_DATA_REG_OFFSET(a1)
       lw     t6, SPI_HOST_DATA_REG_OFFSET(a1)
       sw     a7, 0(s1)
       sw     t0, 4(s1)
       sw     t1, 8(s1)
       sw     t2, 12(s1)
       sw     t3, 16(s1)
       sw     t4, 20(s1)
       sw     t5, 24(s1)
       sw     t6, 28(s1)
       addi   s1, s1, 32
       bne    s1, s7, _wait_spi_rxwm_8_words
       addi   s0, s0, -256
       j      _32B_chunk_loop

_wait_spi_rxwm_8_words_1:
       lw     a5, SPI_HOST_STATUS_REG_OFFSET(a1)
       srli   a5, a5, 0x14
       andi   a5, a5, 1 # SPI_HOST_STATUS_RXWM_BIT
       beqz   a5, _wait_spi_rxwm_8_words_1
       // Read 32B (8 words) from RX FIFO
       lw     a7, SPI_HOST_DATA_REG_OFFSET(a1)
       lw     t0, SPI_HOST_DATA_REG_OFFSET(a1)
       lw     t1, SPI_HOST_DATA_REG_OFFSET(a1)
       lw     t2, SPI_HOST_DATA_REG_OFFSET(a1)
       lw     t3, SPI_HOST_DATA_REG_OFFSET(a1)
       lw     t4, SPI_HOST_DATA_REG_OFFSET(a1)
       lw     t5, SPI_HOST_DATA_REG_OFFSET(a1)
       lw     t6, SPI_HOST_DATA_REG_OFFSET(a1)
       sw     a7, 0(s1)
       sw     t0, 4(s1)
       sw     t1, 8(s1)
       sw     t2, 12(s1)
       sw     t3, 16(s1)
       sw     t4, 20(s1)
       sw     t5, 24(s1)
       sw     t6, 28(s1)
       addi   s0, s0, -32
       addi   s1, s1, 32 # dst ptr move 32B further

_last_bytes_read_start:
       bltu   s4, s0, _wait_spi_rxwm_8_words_1
       // Update RX watermark with remaining words (<=8)
       lw     a4, SPI_HOST_CONTROL_REG_OFFSET(a1)
       andi   a4, a4, -256 # ~SPI_HOST_CONTROL_RX_WATERMARK_MASK
       srli   a5, s0, 0x2 # 4 bytes = 1 word
       or     a5, a5, a4
       sw     a5, SPI_HOST_CONTROL_REG_OFFSET(a1)

_wait_spi_rxwm_n_words:
       lw     a5, SPI_HOST_STATUS_REG_OFFSET(a1)
       srli   a5, a5, 0x14
       andi   a5, a5, 1 # SPI_HOST_STATUS_RXWM_BIT
       beqz   a5, _wait_spi_rxwm_n_words

_remaining_bytes_loop:
       bnez   s0,_spi_read_word
       // Copy from flash to ram finished, jump to ram boot address
       lui    a1, SOC_CTRL_START_ADDRESS_20bit
       // Enable SPI selection
       li     a0, 1
       sw     a0, SOC_CTRL_ENABLE_SPI_SEL_REG_OFFSET(a1)
       // Select SPIMEMIO
       sw     a0, SOC_CTRL_USE_SPIMEMIO_REG_OFFSET(a1)
       // Load boram boot address
       lw     a2, SOC_CTRL_BOOT_ADDRESS_REG_OFFSET(a1)
       // Enable SPIMEMIO for data section copy in crt0
       lui    a1, SPI_MEMIO_START_ADDRESS_20bit
       sw     a0, OBI_SPIMEMIO_START_SPIMEM_REG_OFFSET(a1)
       jalr   a2

_spi_read_word:
       lw     a7, SPI_HOST_DATA_REG_OFFSET(a1)
       sw     a7, 0(s1)
       addi   s0, s0, -4
       addi   s1, s1, 4
       j      _remaining_bytes_loop
       // Leave some nops otherwise prefetcher out of bound
       nop
       nop

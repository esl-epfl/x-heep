/* Copyright (c) 2017  SiFive Inc. All rights reserved.
 * Copyright (c) 2019  ETH Zürich and University of Bologna
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the FreeBSD License.   This program is distributed in the hope that
 * it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
 * including the implied warranties of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  A copy of this license is available at
 * http://www.opensource.org/licenses.
 */

#include "x-heep.h"
#include "core_v_mini_mcu.h"
#include "soc_ctrl_regs.h"
#include "spi_host_regs.h"

/* Entry point for bare metal programs */
.section .text.start
.global _start
.type _start, @function

_start:
/* initialize global pointer */
.option push
.option norelax
1:  auipc  gp, %pcrel_hi(__global_pointer$)
    addi   gp, gp, %pcrel_lo(1b)
.option pop

/* initialize stack pointer */
    la     sp, _sp

/* set the frequency */
    li    a0, SOC_CTRL_START_ADDRESS
    li    a2, REFERENCE_CLOCK_Hz
    sw    a2, SOC_CTRL_SYSTEM_FREQUENCY_HZ_REG_OFFSET(a0)

#ifdef EXTERNAL_CRTO
   #include "external_crt0.S"
#endif

/* copy the remaining (if any) text and data sections */
    // Setup the in/out pointers and copy size knowing 1KiB as already been copied
    li     a2, FLASH_MEM_START_ADDRESS # src ptr (flash)
    addi   a2, a2, 1024
    // This assumes ram base address is 0x00000000
    li     s1, 1024 # dst ptr (ram)
    la     a0, _edata
    addi   a0, a0, -4 # _edata point to next address, so decrease it by one word
    // Skip if everything has already been copied
    blt    a0, s1, _init_bss
    addi   a3, a0, -1024 # copy size in bytes (_edata is word aligned, so -1020 to make sure last word is copied)

    li     a1, SPI_FLASH_START_ADDRESS
    // Spi should already be enabled, powered-up and TXWM to 8
    // Address in byte reversed order (might be useless for the 1KiB address value)
    // Highest byte is discarded(address on 3 bytes)
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
    sw     a2, SPI_HOST_TXDATA_REG_OFFSET(a1)
    nop    # otherwise ready bit check is too fast

_wait_spi_ready_read_prog:
    lw     a5, SPI_HOST_STATUS_REG_OFFSET(a1)
    bgez   a5, _wait_spi_ready_read_prog
    // Read command: 0x11000003
    lui    a4, 0x11000
    addi   a4, a4, 3 # spi cmd: txonly + stdspeed + csaat + 4B
    sw     a4, SPI_HOST_COMMAND_REG_OFFSET(a1)
    nop    # otherwise ready bit check is too fast

_wait_spi_ready_copy_cmd:
    lw     a4, SPI_HOST_STATUS_REG_OFFSET(a1)
    bgez   a4, _wait_spi_ready_copy_cmd

    // For loop until the copy from flash to ram is done
    // Try first to do as much as 256-bytes copies as possible
    // mv     s0, a3
    li     s6, 256
    // Read command: 0x90000FF
   lui    s5, 0x9000
    addi   s5, s5, 255 # spi cmd: rxonly + stdspeed + csaat + 255 bytes

_32B_chunk_loop:
    blt    s6, a3, _read_32B_chunk
    // Read remaining bytes
    lui    a5, 0x8000
    addi   a4, a3, -1
    or     a5, a5, a4 # spi cmd: rxonly + stdspeed + remaining bytes (a3-1)
    sw     a5, SPI_HOST_COMMAND_REG_OFFSET(a1)
    nop    # otherwise ready bit check is too fast

_wait_spi_ready_remaining_bytes:
    lw     a5, SPI_HOST_STATUS_REG_OFFSET(a1)
    bgez   a5, _wait_spi_ready_remaining_bytes
    li     s4, 32
    j      _last_bytes_read_start

_read_32B_chunk:
    sw     s5, SPI_HOST_COMMAND_REG_OFFSET(a1)
    nop    # otherwise ready bit check is too fast

_wait_spi_ready_read_32B_chunk:
    lw     a5, SPI_HOST_STATUS_REG_OFFSET(a1)
    bgez   a5, _wait_spi_ready_read_32B_chunk
    addi   s7, s1, 256 # add 32*8 (256 bytes) to dst ptr

_wait_spi_rxwm_8_words:
    lw     a5, SPI_HOST_STATUS_REG_OFFSET(a1)
    srli   a5, a5, 0x14
    andi   a5, a5, 1 # SPI_HOST_STATUS_RXWM_BIT
    beqz   a5, _wait_spi_rxwm_8_words
    addi   a2, s1, 32 # add 32 bytes (4 words) to dst ptr

_spi_fifo_read_8_words:
    // Read 32B (8 words) from RX FIFO
    lw     a7, SPI_HOST_RXDATA_REG_OFFSET(a1)
    sw     a7, 0(s1)
    addi   s1, s1, 4
    bne    s1, a2, _spi_fifo_read_8_words
    bne    s1, s7, _wait_spi_rxwm_8_words
    addi   a3, a3, -256
    j      _32B_chunk_loop

_wait_spi_rxwm_8_words_1:
    lw     a5, SPI_HOST_STATUS_REG_OFFSET(a1)
    srli   a5, a5, 0x14
    andi   a5, a5, 1 # SPI_HOST_STATUS_RXWM_BIT
    beqz   a5, _wait_spi_rxwm_8_words_1
    addi   a2, s1, 32 # add 32 bytes (4 words) to dst ptr

_spi_fifo_read_8_words_1:
    // Read 32B (8 words) from RX FIFO
    lw     a7, SPI_HOST_RXDATA_REG_OFFSET(a1)
    sw     a7, 0(s1)
    addi   s1, s1, 4
    bne    s1, a2, _spi_fifo_read_8_words_1
    addi   a3, a3, -32

_last_bytes_read_start:
    bltu   s4, a3, _wait_spi_rxwm_8_words_1
    // Update RX watermark with remaining words (<=8)
    lw     a4, SPI_HOST_CONTROL_REG_OFFSET(a1)
    andi   a4, a4, -256 # ~SPI_HOST_CONTROL_RX_WATERMARK_MASK
    srli   a5, a3, 0x2 # 4 bytes = 1 word
    or     a5, a5, a4
    sw     a5, SPI_HOST_CONTROL_REG_OFFSET(a1)

_wait_spi_rxwm_n_words:
    lw     a5, SPI_HOST_STATUS_REG_OFFSET(a1)
    srli   a5, a5, 0x14
    andi   a5, a5, 1 # SPI_HOST_STATUS_RXWM_BIT
    beqz   a5, _wait_spi_rxwm_n_words

_remaining_bytes_loop:
    bnez   a3, _spi_read_word
    j      _init_bss

_spi_read_word:
    lw     a7, SPI_HOST_RXDATA_REG_OFFSET(a1)
    sw     a7, 0(s1)
    addi   a3, a3, -4
    addi   s1, s1, 4
    j      _remaining_bytes_loop

/* clear the bss segment */
_init_bss:
    la     a0, _sbss
    la     a2, _ebss
    sub    a2, a2, a0
    li     a1, 0
    call   memset

/* set vector table address and vectored mode */
    la     a0, _vector_start
    ori    a0, a0, 0x1
    csrw   mtvec, a0

/* new-style constructors and destructors */
    la     a0, __libc_fini_array
    call   atexit
    call   __libc_init_array

/* call main */
    lw    a0, 0(sp)                  /* a0 = argc */
    addi  a1, sp, __SIZEOF_POINTER__ /* a1 = argv */
    li    a2, 0                      /* a2 = envp = NULL */
    call  main
    tail  exit

.size _start, .-_start

.global _init
.type   _init, @function
.global _fini
.type   _fini, @function
_init:
    call init
_fini:
 /* These don't have to do anything since we use init_array/fini_array. Prevent
    missing symbol error */
    ret
.size  _init, .-_init
.size _fini, .-_fini

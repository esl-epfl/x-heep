// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#ifndef COREV_INTR_H_
#define COREV_INTR_H_

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef enum{
    NULL_INTR  = 0,
    UART_INTR_TX_WATERMARK  = 1,
    UART_INTR_RX_WATERMARK  = 2,
    UART_INTR_TX_EMPTY  = 3,
    UART_INTR_RX_OVERFLOW  = 4,
    UART_INTR_RX_FRAME_ERR  = 5,
    UART_INTR_RX_BREAK_ERR  = 6,
    UART_INTR_RX_TIMEOUT  = 7,
    UART_INTR_RX_PARITY_ERR  = 8,
    GPIO_INTR_8  = 9,
    GPIO_INTR_9  = 10,
    GPIO_INTR_10  = 11,
    GPIO_INTR_11  = 12,
    GPIO_INTR_12  = 13,
    GPIO_INTR_13  = 14,
    GPIO_INTR_14  = 15,
    GPIO_INTR_15  = 16,
    GPIO_INTR_16  = 17,
    GPIO_INTR_17  = 18,
    GPIO_INTR_18  = 19,
    GPIO_INTR_19  = 20,
    GPIO_INTR_20  = 21,
    GPIO_INTR_21  = 22,
    GPIO_INTR_22  = 23,
    GPIO_INTR_23  = 24,
    GPIO_INTR_24  = 25,
    GPIO_INTR_25  = 26,
    GPIO_INTR_26  = 27,
    GPIO_INTR_27  = 28,
    GPIO_INTR_28  = 29,
    GPIO_INTR_29  = 30,
    GPIO_INTR_30  = 31,
    GPIO_INTR_31  = 32,
    INTR_FMT_WATERMARK  = 33,
    INTR_RX_WATERMARK  = 34,
    INTR_FMT_OVERFLOW  = 35,
    INTR_RX_OVERFLOW  = 36,
    INTR_NAK  = 37,
    INTR_SCL_INTERFERENCE  = 38,
    INTR_SDA_INTERFERENCE  = 39,
    INTR_STRETCH_TIMEOUT  = 40,
    INTR_SDA_UNSTABLE  = 41,
    INTR_TRANS_COMPLETE  = 42,
    INTR_TX_EMPTY  = 43,
    INTR_TX_NONEMPTY  = 44,
    INTR_TX_OVERFLOW  = 45,
    INTR_ACQ_OVERFLOW  = 46,
    INTR_ACK_STOP  = 47,
    INTR_HOST_TIMEOUT  = 48,
    SPI2_INTR_EVENT  = 49,
    I2S_INTR_EVENT  = 50,
    DMA_WINDOW_INTR  = 51,
    EXT_INTR_0  = 52,
    EXT_INTR_1  = 53,
    EXT_INTR_2  = 54,
    EXT_INTR_3  = 55,
    EXT_INTR_4  = 56,
    EXT_INTR_5  = 57,
    EXT_INTR_6  = 58,
    EXT_INTR_7  = 59,
    EXT_INTR_8  = 60,
    EXT_INTR_9  = 61,
    EXT_INTR_10  = 62,
    EXT_INTR_11  = 63,
    INTR__size
} core_intr_id_t;

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // COREV_INTR_H_

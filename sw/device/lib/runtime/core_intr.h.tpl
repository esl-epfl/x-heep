// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#ifndef COREV_INTR_H_
#define COREV_INTR_H_

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef enum{
% for key, value in interrupts.items():
    ${key.upper()}  = ${value},
% endfor
    INTR__size
} core_intr_id_t;

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // COREV_INTR_H_

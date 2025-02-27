// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include "handler.h"
#include "csr.h"
#include "stdasm.h"
#include "syscalls.h"

/**
 * Default Error Handling
 * @param msg error message supplied by caller
 * TODO - this will be soon by a real print formatting
 */
static void print_exc_msg(const char *msg) {
  _writestr(msg );
  uint32_t mtval; 
  CSR_READ(CSR_REG_MTVAL, &mtval);
  _writestr("MTVAL value is ");
  const char p[6] = {(mtval >> 24) & 0xFF, (mtval >> 16) & 0xFF, (mtval >> 8) & 0xFF, mtval & 0xFF, '\n', 0};
  _writestr(p);
  while (1) {
  };
}

// Below functions are default weak exception handlers meant to be overriden
__attribute__((weak)) void handler_exception(void) {
  uint32_t mcause;
  exc_id_t exc_cause;

  CSR_READ(CSR_REG_MCAUSE, &mcause);
  exc_cause = (exc_id_t)(mcause & kIdMax);

  switch (exc_cause) {
    case kInstMisa:
      handler_instr_acc_fault();
      break;
    case kInstAccFault:
      handler_instr_acc_fault();
      break;
    case kInstIllegalFault:
      handler_instr_ill_fault();
      break;
    case kBkpt:
      handler_bkpt();
      break;
    case kLoadAccFault:
      handler_lsu_fault();
      break;
    case kStrAccFault:
      handler_lsu_fault();
      break;
    case kECall:
      handler_ecall();
      break;
    default:
      while (1) {
      };
  }
}

__attribute__((weak)) void handler_irq_software(void) {
  _writestr("Software IRQ triggered!\n"); 
  while (1) {
  }
}

__attribute__((weak)) void handler_irq_timer(void) {
  _writestr("Timer IRQ triggered!\n"); 
  while (1) {
  }
}

__attribute__((weak)) void handler_irq_external(void) {
  _writestr("External IRQ triggered!\n");
  while (1) {
  }
}

__attribute__((weak)) void handler_instr_acc_fault(void) {
  print_exc_msg("Instruction access fault, mtval shows fault address\n");
}

__attribute__((weak)) void handler_instr_ill_fault(void) {
  print_exc_msg( "Illegal Instruction fault, mtval shows instruction content\n");
}

__attribute__((weak)) void handler_bkpt(void) {
  print_exc_msg("Breakpoint triggerd, mtval shows the breakpoint address\n");
}

__attribute__((weak)) void handler_lsu_fault(void) {
  print_exc_msg("Load/Store fault, mtval shows the fault address\n");
}

__attribute__((weak)) void handler_ecall(void) {
  _writestr("Environment call encountered\n");
  while (1) {
  }
}
#ifdef __cplusplus
}
#endif  // __cplusplus


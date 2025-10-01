/*
                              *******************
******************************* C SOURCE FILE *******************************
**                            *******************                          **
**                                                                         **
** project  : x-heep                                                       **
** filename : main.c                                                       **
** date     : 23/08/2025                                                   **
**                                                                         **
*****************************************************************************
**                                                                         **
** MULT peripheral verification app                                        **
** - writes operands A/B                                                   **
** - reads PRODUCT                                                         **
** - directed (smoke) + random tests                                       **
**                                                                         **
*****************************************************************************
*/

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include "mmio.h"
#include "mult.h"
#include "mult_regs.h"


#define MULT_BASE_ADDR 0x30080000u


/* -------------------------------------------------------------------------- */
/*                                UTILITIES                                    */
/* -------------------------------------------------------------------------- */

static inline uint32_t mul_lo32(uint32_t a, uint32_t b) {
  return (uint32_t)((uint64_t)a * (uint64_t)b);
}

/* Deterministic tiny PRNG (LCG) for reproducible tests) */
static uint32_t rnd_state = 0xC001D00Du;
static uint32_t rnd(void) {
  rnd_state = 1103515245u * rnd_state + 12345u;
  return rnd_state;
}

/* -------------------------------------------------------------------------- */
/*                                 TESTS                                       */
/* -------------------------------------------------------------------------- */

static int test_one(const mult_t *m, uint32_t a, uint32_t b, const char *tag) {
  printf("[MULT] %s test A=0x%08x B=0x%08x\n", tag, a, b);
  uint32_t got = mult_mul_blocking(m, a, b);
  printf("[MULT] %s test done\n", tag);
  uint32_t exp = mul_lo32(a, b);
  if (got != exp) {
    printf("[FAIL] %s A=0x%08x B=0x%08x -> got=0x%08x exp=0x%08x\n",tag, a, b, got, exp);
    return 1;
  }
  return 0;
}

static int smoke(const mult_t *m) {
  int fails = 0;
  fails += test_one(m, 0u,          0u,          "smoke");
  printf("[MULT] smoke test 1 done\n");
  fails += test_one(m, 1u,          0u,          "smoke");
  fails += test_one(m, 0u,          1u,          "smoke");
  fails += test_one(m, 1u,          1u,          "smoke");
  fails += test_one(m, 7u,          9u,          "smoke");
  fails += test_one(m, 0xFFFFu,     2u,          "smoke");
  fails += test_one(m, 0xFFFFFFFFu, 0x00000000u, "smoke");
  fails += test_one(m, 0xFFFFFFFFu, 0xFFFFFFFFu, "smoke"); /* wrap -> 1 */
  /* Commutativity spot check */
  fails += test_one(m, 0x12345678u, 0x00009ABCu, "smoke");
  fails += test_one(m, 0x00009ABCu, 0x12345678u, "smoke");
  return fails;
}

static int random_stress(const mult_t *m, int n) {
  int fails = 0;
  for (int i = 0; i < n; ++i) {
    uint32_t a = rnd();
    uint32_t b = rnd();
    fails += test_one(m, a, b, "rand");
    /* Also exercise write order B then A via raw MMIO */
    mmio_region_t base = m->base_addr;
    mmio_region_write32(base, MULT_B_REG_OFFSET, b);
    mmio_region_write32(base, MULT_A_REG_OFFSET, a);
    uint32_t got = mmio_region_read32(base, MULT_PRODUCT_REG_OFFSET);
    uint32_t exp = mul_lo32(a, b);
    if (got != exp) {
      printf("[FAIL] raw A=0x%08x B=0x%08x -> got=0x%08x exp=0x%08x\n",
             a, b, got, exp);
      ++fails;
    }
  }
  return fails;
}

/* -------------------------------------------------------------------------- */
/*                                   MAIN                                      */
/* -------------------------------------------------------------------------- */

int main(void) {
  printf("[MULT] test start\n");

  mult_t m;
  printf("[MULT] test init\n");
  mult_init(&m, MULT_BASE_ADDR);
  printf("[MULT] inited\n");

  int fails = 0;
  fails += smoke(&m);
  printf("[MULT] smoke done\n");
  fails += random_stress(&m, 200); /* increase for more coverage */

  if (fails == 0) {
    printf("[MULT] ALL TESTS PASSED\n");
    return 0;
  } else {
    printf("[MULT] TESTS FAILED: %d\n", fails);
    return 1;
  }
}

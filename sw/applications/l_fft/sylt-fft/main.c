// BENCHMARKING FOR FRDM-K20D50M
// D. TAYLOR 2014

// * Do not build with operating system
// * Uses FTM0 as core clock cycle counter
// * Requires C99 standard

#define BENCH_RUNS 100 // Not really necessary without OS

#include <MK20D5.h>

#include <stdint.h>
#include <stdbool.h>

#include "config.h"
#include "intrinsics.h"
#include "fpmath.h"
#include "fft.h"

static volatile unsigned count_hi; // FTM0 high counter
static volatile unsigned count;    // Performance counter

// FFT data structure
fft_complex_t complex[256];

// This function contains code to benchmark
static void benchmark(void) {
  fft_inverse(complex, 8);
}

// Initialize FTM0
static void bench_init(void) {
  SIM->SCGC6 |= SIM_SCGC6_FTM0_MASK;
  FTM0->MOD = 0xFFFF;
  NVIC_EnableIRQ(FTM0_IRQn);
}

// Resets cycle counts, begins benchmarking
__INLINE
static void bench_begin(void) {
  FTM0->SC = 0;
  count_hi = 0;
  FTM0->CNT = 0;
  FTM0->SC = FTM_SC_CLKS(1) | FTM_SC_TOIE_MASK;
}

// Ends benchmarking, returns cycle count
__INLINE
static unsigned bench_end(void) {
  FTM0->SC = 0;
  return (FTM0->CNT | (count_hi << 16)) - 2;
}

// FTM0 overflow counter
void FTM0_IRQHandler(void) {
  FTM0->SC &= ~FTM_SC_TOF_MASK;
  count_hi++;
}

int main() {
  bench_init();
  while(1) {
    // Perform benchmark
    unsigned ack = 0;
    for(unsigned n = 0; n < BENCH_RUNS; n++) {
      bench_begin();
      benchmark();
      ack += bench_end();
    }
    count = ack / BENCH_RUNS;
    // Count is reported here - use a breakpoint or add communication code
    count = count;
  }
}

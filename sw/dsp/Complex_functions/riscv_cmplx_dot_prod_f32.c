#include "riscv_math.h"

void riscv_cmplx_dot_prod_f32(
  float32_t * pSrcA,
  float32_t * pSrcB,
  uint32_t numSamples,
  float32_t * realResult,
  float32_t * imagResult)
{
  float32_t real_sum = 0.0f, imag_sum = 0.0f;    /* Temporary result storage */
  float32_t a0,b0,c0,d0;
  while(numSamples > 0u)
  {
      a0 = *pSrcA++;
      b0 = *pSrcA++;
      c0 = *pSrcB++;
      d0 = *pSrcB++;  
  
      real_sum += a0 * c0;
      imag_sum += a0 * d0;
      real_sum -= b0 * d0;
      imag_sum += b0 * c0;

      /* Decrement the loop counter */
      numSamples--;
  }

  /* Store the real and imaginary results in the destination buffers */
  *realResult = real_sum;
  *imagResult = imag_sum;
}
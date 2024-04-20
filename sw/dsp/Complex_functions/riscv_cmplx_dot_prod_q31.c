#include "riscv_math.h"

void riscv_cmplx_dot_prod_q31(
  q31_t * pSrcA,
  q31_t * pSrcB,
  uint32_t numSamples,
  q63_t * realResult,
  q63_t * imagResult)
{
  q63_t real_sum = 0, imag_sum = 0;              /* Temporary result storage */
  q31_t a0,b0,c0,d0;
  /* Run the below code for Cortex-M0 */

  while(numSamples > 0u)
  {
      a0 = *pSrcA++;
      b0 = *pSrcA++;
      c0 = *pSrcB++;
      d0 = *pSrcB++;  
  
      real_sum += ((q63_t)a0 * c0) >> 14;
      imag_sum += ((q63_t)a0 * d0) >> 14;
      real_sum -= ((q63_t)b0 * d0) >> 14;
      imag_sum += ((q63_t)b0 * c0) >> 14;

      /* Decrement the loop counter */
      numSamples--;
  }

  /* Store the real and imaginary results in 16.48 format  */
  *realResult = real_sum;
  *imagResult = imag_sum;
}
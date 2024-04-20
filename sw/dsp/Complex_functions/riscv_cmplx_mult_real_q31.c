#include "riscv_math.h"

void riscv_cmplx_mult_real_q31(
  q31_t * pSrcCmplx,
  q31_t * pSrcReal,
  q31_t * pCmplxDst,
  uint32_t numSamples)
{
  q31_t inA1;                                    /* Temporary variable to store input value */
  while(numSamples > 0u)
  {
    /* realOut = realA * realB.            */
    /* imagReal = imagA * realB.               */
    inA1 = *pSrcReal++;
    /* store the result in the destination buffer. */
    *pCmplxDst++ =
      (q31_t) clip_q63_to_q31(((q63_t) * pSrcCmplx++ * inA1) >> 31);
    *pCmplxDst++ =
      (q31_t) clip_q63_to_q31(((q63_t) * pSrcCmplx++ * inA1) >> 31);

    /* Decrement the numSamples loop counter */
    numSamples--;
  }
}
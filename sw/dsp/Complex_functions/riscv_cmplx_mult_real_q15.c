#include "riscv_math.h"
#include "x_heep_emul.h"

void riscv_cmplx_mult_real_q15(
  q15_t * pSrcCmplx,
  q15_t * pSrcReal,
  q15_t * pCmplxDst,
  uint32_t numSamples)
{
  q15_t in;                                      /* Temporary variable to store input value */

#if defined (USE_DSP_RISCV)

  q31_t mul1;                  /* temporary variables */

  while (numSamples > 0u)
  {
    mul1 = x_heep_mulsN(*pSrcReal, *pSrcCmplx++,15);
    *pCmplxDst++ =  (q15_t)x_heep_clip(mul1,15);
    mul1 = x_heep_mulsN(*pSrcReal++, *pSrcCmplx++,15);
    *pCmplxDst++ =  (q15_t)x_heep_clip(mul1,15);
    numSamples--;
  }

#else
  while(numSamples > 0u)
  {
    /* realOut = realA * realB.            */
    /* imagOut = imagA * realB.                */
    in = *pSrcReal++;
    /* store the result in the destination buffer. */
    *pCmplxDst++ =
      (q15_t) __SSAT((((q31_t) (*pSrcCmplx++) * (in)) >> 15), 16);
    *pCmplxDst++ =
      (q15_t) __SSAT((((q31_t) (*pSrcCmplx++) * (in)) >> 15), 16);

    /* Decrement the numSamples loop counter */
    numSamples--;
  }
#endif
}
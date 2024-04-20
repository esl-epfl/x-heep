#include "riscv_math.h"
#include "x_heep_emul.h"

void riscv_cmplx_mult_cmplx_q15(
  q15_t * pSrcA,
  q15_t * pSrcB,
  q15_t * pDst,
  uint32_t numSamples)
{
  q15_t a, b, c, d;                              /* Temporary variables to store real and imaginary values */
#if defined (USE_DSP_RISCV)

  q31_t mul1;                  /* temporary variables */
  while (numSamples > 0u)
  {
     /* C[2 * i] = A[2 * i] * B[2 * i] - A[2 * i + 1] * B[2 * i + 1].  */
    /* C[2 * i + 1] = A[2 * i] * B[2 * i + 1] + A[2 * i + 1] * B[2 * i].  */
    a = *pSrcA++;
    b = *pSrcA++;
    c = *pSrcB++;
    d = *pSrcB++;

    /* store the result in 3.13 format in the destination buffer. */
    *pDst++ = (q15_t)( (q31_t)x_heep_mulsN(a, c,17) - (q31_t)x_heep_mulsN(b, d,17) );
    /* store the result in 3.13 format in the destination buffer. */
    *pDst++ = (q15_t) ( (q31_t)x_heep_mulsN(a, d,17) + (q31_t)x_heep_mulsN(b, c,17) );

    /* Decrement the blockSize loop counter */
    numSamples--;
  }

#else
  while(numSamples > 0u)
  {
    /* C[2 * i] = A[2 * i] * B[2 * i] - A[2 * i + 1] * B[2 * i + 1].  */
    /* C[2 * i + 1] = A[2 * i] * B[2 * i + 1] + A[2 * i + 1] * B[2 * i].  */
    a = *pSrcA++;
    b = *pSrcA++;
    c = *pSrcB++;
    d = *pSrcB++;

    /* store the result in 3.13 format in the destination buffer. */
    *pDst++ =
      (q15_t) (q31_t) (((q31_t) a * c) >> 17) - (((q31_t) b * d) >> 17);
    /* store the result in 3.13 format in the destination buffer. */
    *pDst++ =
      (q15_t) (q31_t) (((q31_t) a * d) >> 17) + (((q31_t) b * c) >> 17);

    /* Decrement the blockSize loop counter */
    numSamples--;
  }
#endif
}

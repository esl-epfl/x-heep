#include "riscv_math.h"

void riscv_cmplx_mult_real_f32(
  float32_t * pSrcCmplx,
  float32_t * pSrcReal,
  float32_t * pCmplxDst,
  uint32_t numSamples)
{
  float32_t in;                                  /* Temporary variable to store input value */
  uint32_t blkCnt;                               /* loop counters */
  blkCnt = numSamples;

  while(blkCnt > 0u)
  {
    /* C[2 * i] = A[2 * i] * B[i].            */
    /* C[2 * i + 1] = A[2 * i + 1] * B[i].        */
    in = *pSrcReal++;
    /* store the result in the destination buffer. */
    *pCmplxDst++ = (*pSrcCmplx++) * (in);
    *pCmplxDst++ = (*pSrcCmplx++) * (in);

    /* Decrement the numSamples loop counter */
    blkCnt--;
  }
}
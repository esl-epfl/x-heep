#include "riscv_math.h" //AFTER MODIFICATION FOR SQRT FUNCTION

void riscv_cmplx_mag_q31(
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t numSamples)
{
  q31_t real, imag;                              /* Temporary variables to hold input values */
  q31_t acc0, acc1;                              /* Accumulators */
  uint32_t blkCnt;                               /* loop counter */
  blkCnt = numSamples;


  while(blkCnt > 0u)
  {
    /* C[0] = sqrt(A[0] * A[0] + A[1] * A[1]) */
    real = *pSrc++;
    imag = *pSrc++;
    acc0 = (q31_t) (((q63_t) real * real) >> 33);
    acc1 = (q31_t) (((q63_t) imag * imag) >> 33);
    /* store the result in 2.30 format in the destination buffer. */
    riscv_sqrt_q31(acc0 + acc1, pDst++);

    /* Decrement the loop counter */
    blkCnt--;
  }
}
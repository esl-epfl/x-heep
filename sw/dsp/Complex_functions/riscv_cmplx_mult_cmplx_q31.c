#include "riscv_math.h"

void riscv_cmplx_mult_cmplx_q31(
  q31_t * pSrcA,
  q31_t * pSrcB,
  q31_t * pDst,
  uint32_t numSamples)
{
  q31_t a, b, c, d;                              /* Temporary variables to store real and imaginary values */
  uint32_t blkCnt;                               /* loop counters */
  q31_t mul1, mul2, mul3, mul4;
  q31_t out1, out2;

  /* loop Unrolling */
  blkCnt = numSamples >> 1u;

  /* First part of the processing with loop unrolling.  Compute 2 outputs at a time.     
   ** a second loop below computes the remaining 1 sample. */
  while(blkCnt > 0u)
  {
    /* C[2 * i] = A[2 * i] * B[2 * i] - A[2 * i + 1] * B[2 * i + 1].  */
    /* C[2 * i + 1] = A[2 * i] * B[2 * i + 1] + A[2 * i + 1] * B[2 * i].  */
    a = *pSrcA++;
    b = *pSrcA++;
    c = *pSrcB++;
    d = *pSrcB++;

    mul1 = (q31_t) (((q63_t) a * c) >> 32);
    mul2 = (q31_t) (((q63_t) b * d) >> 32);
    mul3 = (q31_t) (((q63_t) a * d) >> 32);
    mul4 = (q31_t) (((q63_t) b * c) >> 32);

    mul1 = (mul1 >> 1);
    mul2 = (mul2 >> 1);
    mul3 = (mul3 >> 1);
    mul4 = (mul4 >> 1);

    out1 = mul1 - mul2;
    out2 = mul3 + mul4;

    /* store the real result in 3.29 format in the destination buffer. */
    *pDst++ = out1;
    /* store the imag result in 3.29 format in the destination buffer. */
    *pDst++ = out2;

    a = *pSrcA++;
    b = *pSrcA++;
    c = *pSrcB++;
    d = *pSrcB++;

    mul1 = (q31_t) (((q63_t) a * c) >> 32);
    mul2 = (q31_t) (((q63_t) b * d) >> 32);
    mul3 = (q31_t) (((q63_t) a * d) >> 32);
    mul4 = (q31_t) (((q63_t) b * c) >> 32);

    mul1 = (mul1 >> 1);
    mul2 = (mul2 >> 1);
    mul3 = (mul3 >> 1);
    mul4 = (mul4 >> 1);

    out1 = mul1 - mul2;
    out2 = mul3 + mul4;

    /* store the real result in 3.29 format in the destination buffer. */
    *pDst++ = out1;
    /* store the imag result in 3.29 format in the destination buffer. */
    *pDst++ = out2;

    /* Decrement the blockSize loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 2, compute any remaining output samples here.     
   ** No loop unrolling is used. */
  blkCnt = numSamples % 0x2u;

  while(blkCnt > 0u)
  {
    /* C[2 * i] = A[2 * i] * B[2 * i] - A[2 * i + 1] * B[2 * i + 1].  */
    /* C[2 * i + 1] = A[2 * i] * B[2 * i + 1] + A[2 * i + 1] * B[2 * i].  */
    a = *pSrcA++;
    b = *pSrcA++;
    c = *pSrcB++;
    d = *pSrcB++;

    mul1 = (q31_t) (((q63_t) a * c) >> 32);
    mul2 = (q31_t) (((q63_t) b * d) >> 32);
    mul3 = (q31_t) (((q63_t) a * d) >> 32);
    mul4 = (q31_t) (((q63_t) b * c) >> 32);

    mul1 = (mul1 >> 1);
    mul2 = (mul2 >> 1);
    mul3 = (mul3 >> 1);
    mul4 = (mul4 >> 1);

    out1 = mul1 - mul2;
    out2 = mul3 + mul4;

    /* store the real result in 3.29 format in the destination buffer. */
    *pDst++ = out1;
    /* store the imag result in 3.29 format in the destination buffer. */
    *pDst++ = out2;

    /* Decrement the blockSize loop counter */
    blkCnt--;
  }

}
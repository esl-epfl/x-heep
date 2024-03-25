#include "/home/antoine/FORKs/CMSIS-DSP-PULPino/inc/riscv_math.h"

  void riscv_negate_q31(
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{
  q31_t in;                                      /* Temporary variable */
  uint32_t blkCnt;                               /* loop counter */

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = -A */
    /* Negate and then store the result in the destination buffer. */
    in = *pSrc++;
    *pDst++ = (in == INT32_MIN) ? INT32_MAX : -in;

    /* Decrement the loop counter */
    blkCnt--;
  }
}

#include "riscv_math.h"

void riscv_dot_prod_q31(
  q31_t * pSrcA,
  q31_t * pSrcB,
  uint32_t blockSize,
  q63_t * result)
{
  q63_t sum = 0;                                 /* Temporary result storage */
  uint32_t blkCnt;                               /* loop counter */


  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = A[0]* B[0] + A[1]* B[1] + A[2]* B[2] + .....+ A[blockSize-1]* B[blockSize-1] */
    /* Calculate dot product and then store the result in a temporary buffer. */
    sum += ((q63_t) * pSrcA++ * *pSrcB++) >> 14u;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Store the result in the destination buffer in 16.48 format */
  *result = sum;
}

  
#include "riscv_math.h"
void riscv_add_f32(
  float32_t * pSrcA,
  float32_t * pSrcB,
  float32_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = A + B */
    /* Add and then store the results in the destination buffer. */
    *pDst++ = (*pSrcA++) + (*pSrcB++);

    /* Decrement the loop counter */
    blkCnt--;
  }
}

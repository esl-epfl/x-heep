#include "riscv_math.h"
#include "x_heep_emul.h"
// HAD RO MODIFY THE X_HEEP_EMUL FILE 

void riscv_offset_q15(
  q15_t * pSrc,
  q15_t offset,
  q15_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */

#if defined (USE_DSP_RISCV)
  q31_t inA1, inA2, inB1, inB2;

  /*loop Unrolling */
  blkCnt = blockSize >> 1u;
  while (blkCnt > 0u)
  {
    /* C = A + B */
    /*read 2 elements from source buffer*/
    inA1 = *pSrc++;
    inA2 = *pSrc++;
    /*add the 2 elements to the offset value then saturate*/
    *pDst++ =(q15_t)x_heep_clip((inA1 + offset),15);
    *pDst++ =(q15_t)x_heep_clip((inA2 + offset),15);
    /* Decrement the loop counter */
    blkCnt--;
  }

  blkCnt = blockSize % 0x2u;

  while (blkCnt > 0u)
  {
    /* C = A + B */
    /* Add and then store the results in the destination buffer. */
    *pDst++ = (q15_t)x_heep_clip((*pSrc++ + offset),15);
    /* Decrement the loop counter */
    blkCnt--;
  }

#else

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = A + offset */
    /* Add offset and then store the results in the destination buffer. */
    *pDst++ = (q15_t) __SSAT(((q31_t) * pSrc++ + offset), 16);
    /* Decrement the loop counter */
    blkCnt--;
  }
#endif

}

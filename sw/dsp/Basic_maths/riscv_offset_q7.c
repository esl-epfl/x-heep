#include "riscv_math.h"
#include "x_heep_emul.h"
// HAD RO MODIFY THE X_HEEP_EMUL FILE AND ADD ADD2v

void riscv_offset_q7(
  q7_t * pSrc,
  q7_t offset,
  q7_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */

#if defined (USE_DSP_RISCV)

  shortV VectInA;
  shortV VectInB;  
  shortV VectInC; 
  /*loop Unrolling */
  blkCnt = blockSize >> 1u;
  /*pack to copies from offest*/
  VectInB = x_heep_pack2(offset,offset);
  while (blkCnt > 0u)
  {
    /*read 2 elements from memory*/
    VectInA[0] = (short)(*pSrc++);
    VectInA[1] = (short)(*pSrc++);
    /*add the packed offset to the memory read*/
    VectInC = x_heep_add2(VectInA,VectInB); 
    /*check for saturation then save to destination buffer*/
    *pDst++ =(q7_t)x_heep_clip(VectInC[0],7);
    *pDst++ =(q7_t)x_heep_clip(VectInC[1],7);
    /* Decrement the loop counter */
    blkCnt--;
  }

  blkCnt = blockSize % 0x2u;

  while (blkCnt > 0u)
  {
    /* C = A + B */
    /* Add and then store the results in the destination buffer. */
    *pDst++ =(q7_t)x_heep_clip((*pSrc++ +offset ),7);

    /* Decrement the loop counter */
    blkCnt--;
  }

#else
  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = A + offset */
    /* Add offset and then store the result in the destination buffer. */
    *pDst++ = (q7_t) __SSAT((q15_t) * pSrc++ + offset, 8);

    /* Decrement the loop counter */
    blkCnt--;
  }
#endif
}

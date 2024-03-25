#include "/home/antoine/FORKs/CMSIS-DSP-PULPino/inc/riscv_math.h"
#include "/home/antoine/FORKs/CMSIS-DSP-PULPino/inc/x_heep_emul.h"

  void riscv_add_q7(
  q7_t * pSrcA,
  q7_t * pSrcB,
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

  while (blkCnt > 0u)
  {
   /*read 2 elements from each source*/
    VectInA[0] = (short)pSrcA[0];
    VectInA[1] = (short)pSrcA[1];
    VectInB[0] = (short)pSrcB[0];
    VectInB[1] = (short)pSrcB[1];
  /*add them*/
    VectInC = x_heep_add2(VectInA,VectInB); 
  /*clip to saturate the result*/
    *pDst++ =(q7_t)x_heep_clip(VectInC[0],7);
    *pDst++ =(q7_t)x_heep_clip(VectInC[1],7);
  /*increment source buffers*/
    pSrcA+=2;
    pSrcB+=2;
  /*decrement loop counter*/
    blkCnt--;
  }

  blkCnt = blockSize % 0x2u;
  while (blkCnt > 0u)
  {
    /* C = A + B */
    /* Add and then store the results in the destination buffer. */
    *pDst++ =(q7_t)x_heep_clip((*pSrcA++ + *pSrcB++),7);

    /* Decrement the loop counter */
    blkCnt--;
  }

#else
  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = A + B */
    /* Add and then store the results in the destination buffer. */
    *pDst++ = (q7_t) __SSAT((q15_t) * pSrcA++ + *pSrcB++, 8);
    /* Decrement the loop counter */
    blkCnt--;
  }
#endif


}




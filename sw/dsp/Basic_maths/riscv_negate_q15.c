#include "riscv_math.h"


void riscv_negate_q15(
  q15_t * pSrc,
  q15_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
  q15_t in;
  /* Initialize blkCnt with number of samples */
#if defined (USE_DSP_RISCV)

  shortV *VectInA;
  shortV VectInC; 
  /*loop Unrolling */
  blkCnt = blockSize >> 1u;
  while (blkCnt > 0u)
  {
    /* C = A + B */
    /* read 2 elements from source buffer */
    VectInA = (shortV*)pSrc;
    /*find the negative*/
    VectInC = x_heep_neg2(*VectInA);
    /*check for saturation*/ 
    *pDst++ = ( VectInC[0] == -32768)?0x7fff:VectInC[0];
    *pDst++ = ( VectInC[1] == -32768)?0x7fff:VectInC[1];
    /*increment source buffer*/
    pSrc+=2;
    /* Decrement the loop counter */
    blkCnt--;
  }

  blkCnt = blockSize % 0x2u;
#else
  blkCnt = blockSize;
#endif
  while(blkCnt > 0u)
  {
    /* C = -A */
    /* Negate and then store the result in the destination buffer. */
    in = *pSrc++;
    *pDst++ = (in == (q15_t) 0x8000) ? 0x7fff : -in;
    
    /* Decrement the loop counter */
    blkCnt--;
  }

}

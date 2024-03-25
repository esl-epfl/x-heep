#include "riscv_math.h"
#include "x_heep_emul.h"

void riscv_mult_q7(
  q7_t * pSrcA,
  q7_t * pSrcB,
  q7_t * pDst,
  uint32_t blockSize)
{
    uint32_t blkCnt;                               /* loop counters */

    #if defined (USE_DSP_RISCV)

    q15_t out1;                   /* Temporary variables to store the product */
    blkCnt = blockSize;
    while (blkCnt > 0u)
    {
        /* C = A * B */
        /*multiply and normailze then accumulate*/
    // out1 =  mulsN(*pSrcA++, *pSrcB++,7);// (((q15_t) (*pSrcA++) * (*pSrcB++)) >> 7)
        q15_t x=(q15_t)(*pSrcA++) * (*pSrcB++)>>7;
        int precision=8;
        *pDst++ =  (q7_t)x_heep_clip(x,precision);
        /* Decrement the blockSize loop counter */
        blkCnt--;
    }
    #else

    /* Initialize blkCnt with number of samples */
    blkCnt = blockSize;




    while (blkCnt > 0u)
    {
        /* C = A * B */
        /* Multiply the inputs and store the result in the destination buffer */
        *pDst++ = (q7_t) __SSAT((((q15_t) (*pSrcA++) * (*pSrcB++)) >> 7), 8);

        /* Decrement the blockSize loop counter */
        blkCnt--;
    }
    #endif
}

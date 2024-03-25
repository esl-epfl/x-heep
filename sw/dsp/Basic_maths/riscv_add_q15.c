#include "riscv_math.h"
#include "x_heep_emul.h"

void riscv_add_q15(
    q15_t * pSrcA,
    q15_t * pSrcB,
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
        /* Add and then store the results in the destination buffer. */
    /*read 2 elements from each buffer*/
        inA1 = *pSrcA++;
        inA2 = *pSrcA++;
        inB1 = *pSrcB++;
        inB2 = *pSrcB++;
    /*add and saturate them*/
        *pDst++ =(q15_t)x_heep_clip((inA1 + inB1),15);
        *pDst++ =(q15_t)x_heep_clip((inA2 + inB2),15);

        /* Decrement the loop counter */
        blkCnt--;
    }

    blkCnt = blockSize % 0x2u;

    while (blkCnt > 0u)
    {
        /* C = A + B */
        /* Add and then store the results in the destination buffer. */
        *pDst++ = (q15_t)x_heep_clip((*pSrcA++ + *pSrcB++),15);
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
        *pDst++ = (q15_t) __SSAT(((q31_t) * pSrcA++ + *pSrcB++), 16);

        /* Decrement the loop counter */
        blkCnt--;
    }
    #endif 
}

#include "riscv_math.h"
#include "x_heep_emul.h"

void riscv_mult_q15(
q15_t * pSrcA,
q15_t * pSrcB,
q15_t * pDst,
uint32_t blockSize)
{
    uint32_t blkCnt;                               /* loop counters */

    #if defined (USE_DSP_RISCV)

    q31_t mul1;                  /* temporary variables */

    blkCnt = blockSize;
    while (blkCnt > 0u)
    {
    /*mulsN perform multiplication the normalization*/
        mul1 = x_heep_mulsN(*pSrcA++, *pSrcB++,15);
    /*saturate teh result then save it to the destination buffer*/
        *pDst++ =  (q15_t)x_heep_clip(mul1,15);
    /* decrement loop counter*/
        blkCnt--;
    }



    #else
    /* Initialize blkCnt with number of samples */
    blkCnt = blockSize;

    while(blkCnt > 0u)
    {
        /* C = A * B */
        /* Multiply the inputs and store the result in the destination buffer */
        *pDst++ = (q15_t) __SSAT((((q31_t) (*pSrcA++) * (*pSrcB++)) >> 15), 16);

        /* Decrement the blockSize loop counter */
        blkCnt--;
    }
    #endif
}


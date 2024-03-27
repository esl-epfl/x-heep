#include "riscv_math.h"
// HAD RO MODIFY THE INLINE FUNCTION FOR  __attribute__((always_inline)) q31_t clip_q63_to_q31

void riscv_add_q31(
q31_t * pSrcA,
q31_t * pSrcB,
q31_t * pDst,
uint32_t blockSize)
{
    uint32_t blkCnt;                               /* loop counter */

    /* Initialize blkCnt with number of samples */
    blkCnt = blockSize;

    while(blkCnt > 0u)
    {
        /* C = A + B */
        /* Add and then store the results in the destination buffer. */
        *pDst++ = (q31_t) clip_q63_to_q31((q63_t) * pSrcA++ + *pSrcB++);
        blkCnt--;
    }
}
  


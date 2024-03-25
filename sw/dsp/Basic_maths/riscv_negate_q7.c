#include "riscv_math.h"

void riscv_negate_q7(
q7_t * pSrc,
q7_t * pDst,
uint32_t blockSize)
{
    uint32_t blkCnt;                               /* loop counter */
    q7_t in;

    /* Initialize blkCnt with number of samples */
    blkCnt = blockSize;
    while(blkCnt > 0u)
    {
        /* C = -A */
        /* Negate and then store the results in the destination buffer. */ 
        in = *pSrc++;
        *pDst++ = (in == (q7_t) 0x80) ? 0x7f : -in;

        /* Decrement the loop counter */
        blkCnt--;
    }   
}

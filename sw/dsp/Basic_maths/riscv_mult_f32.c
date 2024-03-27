#include "riscv_math.h"

    void riscv_mult_f32(
    float32_t * pSrcA,
    float32_t * pSrcB,
    float32_t * pDst,
    uint32_t blockSize)
    {
    uint32_t blkCnt;                               /* loop counters */

    blkCnt = blockSize;

    while(blkCnt > 0u)
    {
        /* C = A * B */
        /* Multiply the inputs and store the results in output buffer */
        *pDst++ = (*pSrcA++) * (*pSrcB++);
        /* Decrement the blockSize loop counter */
        blkCnt--;
    }
}


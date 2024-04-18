#include "riscv_math.h"
//CHANGE IN THE INLINE FUNCTION (ALWAYS INLINE)
void riscv_mult_q31(
  q31_t * pSrcA,
  q31_t * pSrcB,
  q31_t * pDst,
  uint32_t blockSize)
{
    uint32_t blkCnt;                               /* loop counters */
    blkCnt = blockSize;
    while (blkCnt > 0u)
    {
        /* C = A * B */
        /* Multiply the inputs and then store the results in the destination buffer. */
        *pDst++ = (q31_t) clip_q63_to_q31(((q63_t) (*pSrcA++) * (*pSrcB++)) >> 31);

        /* Decrement the blockSize loop counter */
        blkCnt--;
    }

}
 
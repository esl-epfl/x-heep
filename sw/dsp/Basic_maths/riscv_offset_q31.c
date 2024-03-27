
#include "riscv_math.h"

void riscv_offset_q31(
  q31_t * pSrc,
  q31_t offset,
  q31_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;
  int A[4]={1,2,3,4};
  int * ptr=A;
  int b;
  b= * ptr++ + * ptr++;
  printf("b : %d\n",b);
  printf("A[%d] : %d\n",ptr,*ptr);
  while(blkCnt > 0u)
  {
    /* C = A + offset */
    /* Add offset and then store the result in the destination buffer. */

    *pDst++ = (q31_t) clip_q63_to_q31((q63_t) * pSrc++ + offset);
    /* Decrement the loop counter */
    blkCnt--;
  }
}

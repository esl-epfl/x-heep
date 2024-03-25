#include "riscv_math.h"
#include "x_heep_emul.h"
// HAD RO MODIFY THE X_HEEP_EMUL FILE AND ADD ADD2v
void riscv_dot_prod_q7(
  q7_t * pSrcA,
  q7_t * pSrcB,
  uint32_t blockSize,
  q31_t * result)
{
    uint32_t blkCnt;                               /* loop counter */

    q31_t sum = 0;                                 /* Temporary variables to store output */

  #if defined (USE_DSP_RISCV)

    charV *VectInA;
    charV *VectInB;

    /*loop Unrolling */
    blkCnt = blockSize >> 2u;
    /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
    ** a second loop below computes the remaining 1 to 3 samples. */
    while (blkCnt > 0u)
    {
      /*read 4 elements from each buffer*/
      VectInA =  (charV*)pSrcA;
      VectInB =  (charV*)pSrcB;
      /*increment source buffers*/
      pSrcA+=4;
      pSrcB+=4;
      /* sumdotpv4 to perform dot product for the 4 pairs and accumulate the sum */
      sum = x_heep_sumdotp4(*VectInA, *VectInB, sum);
      /*decrement loop counter*/
      blkCnt--;
    }

    /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
    ** No loop unrolling is used. */
    blkCnt = blockSize % 0x4u;

    while (blkCnt > 0u)
    {
      /* C = A[0]* B[0] + A[1]* B[1] + A[2]* B[2] + .....+ A[blockSize-1]* B[blockSize-1] */
      /* multiply and accumulate to perform dot product for 1 pair*/
      sum =  x_heep_macs(sum,*pSrcA++,*pSrcB++);
      /* Decrement the loop counter */
      blkCnt--;
    }

  #else

    /* Initialize blkCnt with number of samples */
    blkCnt = blockSize;
    while(blkCnt > 0u)
    {
      /* C = A[0]* B[0] + A[1]* B[1] + A[2]* B[2] + .....+ A[blockSize-1]* B[blockSize-1] */
      /* Dot product and then store the results in a temporary buffer. */
      sum += (q31_t) ((q15_t) * pSrcA++ * *pSrcB++);
      /* Decrement the loop counter */
      blkCnt--;
    }
    /* Store the result in the destination buffer in 18.14 format */
  #endif
    *result = sum;

}
 
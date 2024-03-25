#include "riscv_math.h"
#include "x_heep_emul.h"
// HAD RO MODIFY THE X_HEEP_EMUL FILE AND ADD ADD2v

void riscv_dot_prod_q15(
    q15_t * pSrcA,
    q15_t * pSrcB,
    uint32_t blockSize,
    q63_t * result)
{
    q63_t sum = 0;                                 /* Temporary result storage */
    uint32_t blkCnt;                               /* loop counter */

    #if defined (USE_DSP_RISCV)

    shortV *VectInA;
    shortV *VectInB;

    /*loop Unrolling */
    blkCnt = blockSize >> 1u;

    while (blkCnt > 0u)
    {
        /* C = A[0]* B[0] + A[1]* B[1] + A[2]* B[2] + .....+ A[blockSize-1]* B[blockSize-1] */
        /*read 2 elements from each buffer */
        VectInA =  (shortV*)pSrcA;
        VectInB =  (shortV*)pSrcB;
        /*increment source buffers */
        pSrcA+=2;
        pSrcB+=2;
        /*dotpv2 to perform dot product, then accumulate the sum*/
        sum += x_heep_dotp2(*VectInA, *VectInB);

        /* Decrement the loop counter */
        blkCnt--;
    }

    blkCnt = blockSize % 0x2u;
    /*the remaning sample if vector size is odd*/
    while (blkCnt > 0u) 
    {
        sum +=  (q63_t) ((q31_t) * pSrcA++ * *pSrcB++);
        /* Decrement the loop counter */
        blkCnt--;
    }


    #else
    /* Initialize blkCnt with number of samples */
    blkCnt = blockSize;

    while(blkCnt > 0u)
    {
        /* C = A[0]* B[0] + A[1]* B[1] + A[2]* B[2] + .....+ A[blockSize-1]* B[blockSize-1] */
        /* Calculate dot product and then store the results in a temporary buffer. */
        sum += (q63_t) ((q31_t) * pSrcA++ * *pSrcB++);

        /* Decrement the loop counter */
        blkCnt--;
    }
    #endif

    /* Store the result in the destination buffer in 34.30 format */
    *result = sum;

}
 
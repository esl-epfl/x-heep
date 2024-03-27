#include "riscv_math.h"

    void riscv_abs_q7(
    q7_t * pSrc,
    q7_t * pDst,
    uint32_t blockSize)
    {
    uint32_t blkCnt;                               /* loop counter */
    q7_t in;                                       /* Input value1 */

    #if defined (USE_DSP_RISCV)


    q31_t in1, in2, in3, in4;                      /* temporary input variables */
    q31_t out1, out2, out3, out4;                  /* temporary output variables */
    charV *VectInA;
    charV VectInC; 
    /*loop Unrolling */
    blkCnt = blockSize >> 2u;

    while (blkCnt > 0u)
    {
        VectInA = (charV*)pSrc; 
        VectInC = x_heep_abs4(*VectInA); /*calculate the absolute of 4 q7 at the same time */
        /*check for each one to saturate it*/
        *pDst++ = ( VectInC[0] == -128)?0x7f:VectInC[0];
        *pDst++ = ( VectInC[1] == -128)?0x7f:VectInC[1];
        *pDst++ = ( VectInC[2] == -128)?0x7f:VectInC[2];
        *pDst++ = ( VectInC[3] == -128)?0x7f:VectInC[3];
        
        pSrc+=4; /*inc pointer for next loop*/
        blkCnt--; /*dec loop counter*/
        
        /*
        VectInA = (charV*)pSrc; 
        int absValues = x_heep_abs4(*VectInA); // Calculate absolute of 4 q7 values
        VectInC[0] = absValues & 0xFF; // Extract and assign the absolute values individually
        VectInC[1] = (absValues >> 8) & 0xFF;
        VectInC[2] = (absValues >> 16) & 0xFF;
        VectInC[3] = (absValues >> 24) & 0xFF; /*calculate the absolute of 4 q7 at the same time */
        
        /*check for each one to saturate it*/
        /*
        *pDst++ = ( VectInC[0] == -128)?0x7f:VectInC[0];
        *pDst++ = ( VectInC[1] == -128)?0x7f:VectInC[1];
        *pDst++ = ( VectInC[2] == -128)?0x7f:VectInC[2];
        *pDst++ = ( VectInC[3] == -128)?0x7f:VectInC[3];
        
        pSrc+=4; /*inc pointer for next loop*/
        // blkCnt--; /*dec loop counter*/
        
    }


    blkCnt = blockSize % 0x4u;
    #else
    blkCnt = blockSize;
    #endif
    while(blkCnt > 0u)
    {
        /* C = |A| */
        /* Read the input */
        in = *pSrc++;

        /* Store the Absolute result in the destination buffer */
        *pDst++ = (in > 0) ? in : ((in == (q7_t) 0x80) ? 0x7f : -in);

        /* Decrement the loop counter */
        blkCnt--;
    }
}

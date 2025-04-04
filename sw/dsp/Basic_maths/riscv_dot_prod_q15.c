/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_dot_prod_q15.c    
*    
* Description:	Q15 dot product.    
*    
* Target Processor: Cortex-M4/Cortex-M3/Cortex-M0
*  
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions
* are met:
*   - Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   - Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the 
*     distribution.
*   - Neither the name of ARM LIMITED nor the names of its contributors
*     may be used to endorse or promote products derived from this
*     software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.

 Modifications 2017  Mostafa Saleh       (Ported to RISC-V PULPino)
 Modifications 2024  ESL
* -------------------------------------------------------------------- */

#include "riscv_math.h"
#include "x_heep_emul.h"
/**    
 * @ingroup groupMath    
 */

/**    
 * @addtogroup dot_prod    
 * @{    
 */

/**    
 * @brief Dot product of Q15 vectors.    
 * @param[in]       *pSrcA points to the first input vector    
 * @param[in]       *pSrcB points to the second input vector    
 * @param[in]       blockSize number of samples in each vector    
 * @param[out]      *result output result returned here    
 * @return none.    
 *    
 * <b>Scaling and Overflow Behavior:</b>    
 * \par    
 * The intermediate multiplications are in 1.15 x 1.15 = 2.30 format and these    
 * results are added to a 64-bit accumulator in 34.30 format.    
 * Nonsaturating additions are used and given that there are 33 guard bits in the accumulator    
 * there is no risk of overflow.    
 * The return result is in 34.30 format.    
 */

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

/**    
 * @} end of dot_prod group    
 */


 